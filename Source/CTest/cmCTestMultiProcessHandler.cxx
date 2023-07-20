/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestMultiProcessHandler.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef> // IWYU pragma: keep
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>
#include <cm3p/uv.h>

#include "cmsys/FStream.hxx"
#include "cmsys/SystemInformation.hxx"

#include "cmAffinity.h"
#include "cmCTest.h"
#include "cmCTestBinPacker.h"
#include "cmCTestRunTest.h"
#include "cmCTestTestHandler.h"
#include "cmDuration.h"
#include "cmJSONState.h"
#include "cmListFileCache.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVSignalHackRAII.h" // IWYU pragma: keep
#include "cmWorkingDirectory.h"

namespace cmsys {
class RegularExpression;
}

class TestComparator
{
public:
  TestComparator(cmCTestMultiProcessHandler* handler)
    : Handler(handler)
  {
  }

  // Sorts tests in descending order of cost
  bool operator()(int index1, int index2) const
  {
    return this->Handler->Properties[index1]->Cost >
      this->Handler->Properties[index2]->Cost;
  }

private:
  cmCTestMultiProcessHandler* Handler;
};

cmCTestMultiProcessHandler::cmCTestMultiProcessHandler()
{
  this->ParallelLevel = 1;
  this->TestLoad = 0;
  this->FakeLoadForTesting = 0;
  this->Completed = 0;
  this->RunningCount = 0;
  this->ProcessorsAvailable = cmAffinity::GetProcessorsAvailable();
  this->HaveAffinity = this->ProcessorsAvailable.size();
  this->HasCycles = false;
  this->HasInvalidGeneratedResourceSpec = false;
  this->SerialTestRunning = false;
}

cmCTestMultiProcessHandler::~cmCTestMultiProcessHandler() = default;

// Set the tests
void cmCTestMultiProcessHandler::SetTests(TestMap& tests,
                                          PropertiesMap& properties)
{
  this->Tests = tests;
  this->Properties = properties;
  this->Total = this->Tests.size();
  // set test run map to false for all
  for (auto const& t : this->Tests) {
    this->TestRunningMap[t.first] = false;
    this->TestFinishMap[t.first] = false;
  }
  if (!this->CTest->GetShowOnly()) {
    this->ReadCostData();
    this->HasCycles = !this->CheckCycles();
    this->HasInvalidGeneratedResourceSpec =
      !this->CheckGeneratedResourceSpec();
    if (this->HasCycles || this->HasInvalidGeneratedResourceSpec) {
      return;
    }
    this->CreateTestCostList();
  }
}

// Set the max number of tests that can be run at the same time.
void cmCTestMultiProcessHandler::SetParallelLevel(size_t level)
{
  this->ParallelLevel = level < 1 ? 1 : level;
}

void cmCTestMultiProcessHandler::SetTestLoad(unsigned long load)
{
  this->TestLoad = load;

  std::string fake_load_value;
  if (cmSystemTools::GetEnv("__CTEST_FAKE_LOAD_AVERAGE_FOR_TESTING",
                            fake_load_value)) {
    if (!cmStrToULong(fake_load_value, &this->FakeLoadForTesting)) {
      cmSystemTools::Error("Failed to parse fake load value: " +
                           fake_load_value);
    }
  }
}

void cmCTestMultiProcessHandler::RunTests()
{
  this->CheckResume();
  if (this->HasCycles || this->HasInvalidGeneratedResourceSpec) {
    return;
  }
#ifdef CMAKE_UV_SIGNAL_HACK
  cmUVSignalHackRAII hackRAII;
#endif
  this->TestHandler->SetMaxIndex(this->FindMaxIndex());

  uv_loop_init(&this->Loop);
  this->StartNextTests();
  uv_run(&this->Loop, UV_RUN_DEFAULT);
  uv_loop_close(&this->Loop);

  if (!this->StopTimePassed && !this->CheckStopOnFailure()) {
    assert(this->Completed == this->Total);
    assert(this->Tests.empty());
  }
  assert(this->AllResourcesAvailable());

  this->MarkFinished();
  this->UpdateCostData();
}

bool cmCTestMultiProcessHandler::StartTestProcess(int test)
{
  if (this->HaveAffinity && this->Properties[test]->WantAffinity) {
    size_t needProcessors = this->GetProcessorsUsed(test);
    if (needProcessors > this->ProcessorsAvailable.size()) {
      return false;
    }
    std::vector<size_t> affinity;
    affinity.reserve(needProcessors);
    for (size_t i = 0; i < needProcessors; ++i) {
      auto p = this->ProcessorsAvailable.begin();
      affinity.push_back(*p);
      this->ProcessorsAvailable.erase(p);
    }
    this->Properties[test]->Affinity = std::move(affinity);
  }

  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "test " << test << "\n", this->Quiet);
  this->TestRunningMap[test] = true; // mark the test as running
  // now remove the test itself
  this->EraseTest(test);
  this->RunningCount += this->GetProcessorsUsed(test);

  auto testRun = cm::make_unique<cmCTestRunTest>(*this);

  if (this->RepeatMode != cmCTest::Repeat::Never) {
    testRun->SetRepeatMode(this->RepeatMode);
    testRun->SetNumberOfRuns(this->RepeatCount);
  }
  testRun->SetIndex(test);
  testRun->SetTestProperties(this->Properties[test]);
  if (this->UseResourceSpec) {
    testRun->SetUseAllocatedResources(true);
    testRun->SetAllocatedResources(this->AllocatedResources[test]);
  }

  // Find any failed dependencies for this test. We assume the more common
  // scenario has no failed tests, so make it the outer loop.
  for (std::string const& f : *this->Failed) {
    if (cm::contains(this->Properties[test]->RequireSuccessDepends, f)) {
      testRun->AddFailedDependency(f);
    }
  }

  // Always lock the resources we'll be using, even if we fail to set the
  // working directory because FinishTestProcess() will try to unlock them
  this->LockResources(test);

  if (!this->ResourceAllocationErrors[test].empty()) {
    std::ostringstream e;
    e << "Insufficient resources for test " << this->Properties[test]->Name
      << ":\n\n";
    for (auto const& it : this->ResourceAllocationErrors[test]) {
      switch (it.second) {
        case ResourceAllocationError::NoResourceType:
          e << "  Test requested resources of type '" << it.first
            << "' which does not exist\n";
          break;

        case ResourceAllocationError::InsufficientResources:
          e << "  Test requested resources of type '" << it.first
            << "' in the following amounts:\n";
          for (auto const& group : this->Properties[test]->ResourceGroups) {
            for (auto const& requirement : group) {
              if (requirement.ResourceType == it.first) {
                e << "    " << requirement.SlotsNeeded
                  << (requirement.SlotsNeeded == 1 ? " slot\n" : " slots\n");
              }
            }
          }
          e << "  but only the following units were available:\n";
          for (auto const& res :
               this->ResourceAllocator.GetResources().at(it.first)) {
            e << "    '" << res.first << "': " << res.second.Total
              << (res.second.Total == 1 ? " slot\n" : " slots\n");
          }
          break;
      }
      e << "\n";
    }
    e << "Resource spec file:\n\n  " << this->ResourceSpecFile;
    cmCTestRunTest::StartFailure(std::move(testRun), this->Total, e.str(),
                                 "Insufficient resources");
    return false;
  }

  cmWorkingDirectory workdir(this->Properties[test]->Directory);
  if (workdir.Failed()) {
    cmCTestRunTest::StartFailure(std::move(testRun), this->Total,
                                 "Failed to change working directory to " +
                                   this->Properties[test]->Directory + " : " +
                                   std::strerror(workdir.GetLastResult()),
                                 "Failed to change working directory");
    return false;
  }

  // Ownership of 'testRun' has moved to another structure.
  // When the test finishes, FinishTestProcess will be called.
  return cmCTestRunTest::StartTest(std::move(testRun), this->Completed,
                                   this->Total);
}

bool cmCTestMultiProcessHandler::AllocateResources(int index)
{
  if (!this->UseResourceSpec) {
    return true;
  }

  std::map<std::string, std::vector<cmCTestBinPackerAllocation>> allocations;
  if (!this->TryAllocateResources(index, allocations)) {
    return false;
  }

  auto& allocatedResources = this->AllocatedResources[index];
  allocatedResources.resize(this->Properties[index]->ResourceGroups.size());
  for (auto const& it : allocations) {
    for (auto const& alloc : it.second) {
      bool result = this->ResourceAllocator.AllocateResource(
        it.first, alloc.Id, alloc.SlotsNeeded);
      (void)result;
      assert(result);
      allocatedResources[alloc.ProcessIndex][it.first].push_back(
        { alloc.Id, static_cast<unsigned int>(alloc.SlotsNeeded) });
    }
  }

  return true;
}

bool cmCTestMultiProcessHandler::TryAllocateResources(
  int index,
  std::map<std::string, std::vector<cmCTestBinPackerAllocation>>& allocations,
  std::map<std::string, ResourceAllocationError>* errors)
{
  allocations.clear();

  std::size_t processIndex = 0;
  for (auto const& process : this->Properties[index]->ResourceGroups) {
    for (auto const& requirement : process) {
      for (int i = 0; i < requirement.UnitsNeeded; ++i) {
        allocations[requirement.ResourceType].push_back(
          { processIndex, requirement.SlotsNeeded, "" });
      }
    }
    ++processIndex;
  }

  bool result = true;
  auto const& availableResources = this->ResourceAllocator.GetResources();
  for (auto& it : allocations) {
    if (!availableResources.count(it.first)) {
      if (errors) {
        (*errors)[it.first] = ResourceAllocationError::NoResourceType;
        result = false;
      } else {
        return false;
      }
    } else if (!cmAllocateCTestResourcesRoundRobin(
                 availableResources.at(it.first), it.second)) {
      if (errors) {
        (*errors)[it.first] = ResourceAllocationError::InsufficientResources;
        result = false;
      } else {
        return false;
      }
    }
  }

  return result;
}

void cmCTestMultiProcessHandler::DeallocateResources(int index)
{
  if (!this->UseResourceSpec) {
    return;
  }

  {
    auto& allocatedResources = this->AllocatedResources[index];
    for (auto const& processAlloc : allocatedResources) {
      for (auto const& it : processAlloc) {
        auto resourceType = it.first;
        for (auto const& it2 : it.second) {
          bool success = this->ResourceAllocator.DeallocateResource(
            resourceType, it2.Id, it2.Slots);
          (void)success;
          assert(success);
        }
      }
    }
  }
  this->AllocatedResources.erase(index);
}

bool cmCTestMultiProcessHandler::AllResourcesAvailable()
{
  for (auto const& it : this->ResourceAllocator.GetResources()) {
    for (auto const& it2 : it.second) {
      if (it2.second.Locked != 0) {
        return false;
      }
    }
  }

  return true;
}

void cmCTestMultiProcessHandler::CheckResourcesAvailable()
{
  if (this->UseResourceSpec) {
    for (auto test : this->SortedTests) {
      std::map<std::string, std::vector<cmCTestBinPackerAllocation>>
        allocations;
      this->TryAllocateResources(test, allocations,
                                 &this->ResourceAllocationErrors[test]);
    }
  }
}

bool cmCTestMultiProcessHandler::CheckStopOnFailure()
{
  return this->CTest->GetStopOnFailure();
}

bool cmCTestMultiProcessHandler::CheckStopTimePassed()
{
  if (!this->StopTimePassed) {
    std::chrono::system_clock::time_point stop_time =
      this->CTest->GetStopTime();
    if (stop_time != std::chrono::system_clock::time_point() &&
        stop_time <= std::chrono::system_clock::now()) {
      this->SetStopTimePassed();
    }
  }
  return this->StopTimePassed;
}

void cmCTestMultiProcessHandler::SetStopTimePassed()
{
  if (!this->StopTimePassed) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "The stop time has been passed. "
               "Stopping all tests."
                 << std::endl);
    this->StopTimePassed = true;
  }
}

void cmCTestMultiProcessHandler::LockResources(int index)
{
  this->LockedResources.insert(
    this->Properties[index]->LockedResources.begin(),
    this->Properties[index]->LockedResources.end());

  if (this->Properties[index]->RunSerial) {
    this->SerialTestRunning = true;
  }
}

void cmCTestMultiProcessHandler::UnlockResources(int index)
{
  for (std::string const& i : this->Properties[index]->LockedResources) {
    this->LockedResources.erase(i);
  }
  if (this->Properties[index]->RunSerial) {
    this->SerialTestRunning = false;
  }
}

void cmCTestMultiProcessHandler::EraseTest(int test)
{
  this->Tests.erase(test);
  this->SortedTests.erase(
    std::find(this->SortedTests.begin(), this->SortedTests.end(), test));
}

inline size_t cmCTestMultiProcessHandler::GetProcessorsUsed(int test)
{
  size_t processors = static_cast<int>(this->Properties[test]->Processors);
  // If processors setting is set higher than the -j
  // setting, we default to using all of the process slots.
  if (processors > this->ParallelLevel) {
    processors = this->ParallelLevel;
  }
  // Cap tests that want affinity to the maximum affinity available.
  if (this->HaveAffinity && processors > this->HaveAffinity &&
      this->Properties[test]->WantAffinity) {
    processors = this->HaveAffinity;
  }
  return processors;
}

std::string cmCTestMultiProcessHandler::GetName(int test)
{
  return this->Properties[test]->Name;
}

bool cmCTestMultiProcessHandler::StartTest(int test)
{
  // Check for locked resources
  for (std::string const& i : this->Properties[test]->LockedResources) {
    if (cm::contains(this->LockedResources, i)) {
      return false;
    }
  }

  // Allocate resources
  if (this->ResourceAllocationErrors[test].empty() &&
      !this->AllocateResources(test)) {
    this->DeallocateResources(test);
    return false;
  }

  // if there are no depends left then run this test
  if (this->Tests[test].empty()) {
    return this->StartTestProcess(test);
  }
  // This test was not able to start because it is waiting
  // on depends to run
  this->DeallocateResources(test);
  return false;
}

void cmCTestMultiProcessHandler::StartNextTests()
{
  if (this->TestLoadRetryTimer.get() != nullptr) {
    // This timer may be waiting to call StartNextTests again.
    // Since we have been called it is no longer needed.
    uv_timer_stop(this->TestLoadRetryTimer);
  }

  if (this->Tests.empty()) {
    this->TestLoadRetryTimer.reset();
    return;
  }

  if (this->CheckStopTimePassed()) {
    return;
  }

  if (this->CheckStopOnFailure() && !this->Failed->empty()) {
    return;
  }

  size_t numToStart = 0;

  if (this->RunningCount < this->ParallelLevel) {
    numToStart = this->ParallelLevel - this->RunningCount;
  }

  if (numToStart == 0) {
    return;
  }

  // Don't start any new tests if one with the RUN_SERIAL property
  // is already running.
  if (this->SerialTestRunning) {
    return;
  }

  bool allTestsFailedTestLoadCheck = false;
  size_t minProcessorsRequired = this->ParallelLevel;
  std::string testWithMinProcessors;

  cmsys::SystemInformation info;

  unsigned long systemLoad = 0;
  size_t spareLoad = 0;
  if (this->TestLoad > 0) {
    // Activate possible wait.
    allTestsFailedTestLoadCheck = true;

    // Check for a fake load average value used in testing.
    if (this->FakeLoadForTesting > 0) {
      systemLoad = this->FakeLoadForTesting;
      // Drop the fake load for the next iteration to a value low enough
      // that the next iteration will start tests.
      this->FakeLoadForTesting = 1;
    }
    // If it's not set, look up the true load average.
    else {
      systemLoad = static_cast<unsigned long>(ceil(info.GetLoadAverage()));
    }
    spareLoad =
      (this->TestLoad > systemLoad ? this->TestLoad - systemLoad : 0);

    // Don't start more tests than the spare load can support.
    if (numToStart > spareLoad) {
      numToStart = spareLoad;
    }
  }

  TestList copy = this->SortedTests;
  for (auto const& test : copy) {
    // Take a nap if we're currently performing a RUN_SERIAL test.
    if (this->SerialTestRunning) {
      break;
    }
    // We can only start a RUN_SERIAL test if no other tests are also
    // running.
    if (this->Properties[test]->RunSerial && this->RunningCount > 0) {
      continue;
    }

    size_t processors = this->GetProcessorsUsed(test);
    bool testLoadOk = true;
    if (this->TestLoad > 0) {
      if (processors <= spareLoad) {
        cmCTestLog(this->CTest, DEBUG,
                   "OK to run " << this->GetName(test) << ", it requires "
                                << processors << " procs & system load is: "
                                << systemLoad << std::endl);
        allTestsFailedTestLoadCheck = false;
      } else {
        testLoadOk = false;
      }
    }

    if (processors <= minProcessorsRequired) {
      minProcessorsRequired = processors;
      testWithMinProcessors = this->GetName(test);
    }

    if (testLoadOk && processors <= numToStart && this->StartTest(test)) {
      numToStart -= processors;
    } else if (numToStart == 0) {
      break;
    }
  }

  if (allTestsFailedTestLoadCheck) {
    // Find out whether there are any non RUN_SERIAL tests left, so that the
    // correct warning may be displayed.
    bool onlyRunSerialTestsLeft = true;
    for (auto const& test : copy) {
      if (!this->Properties[test]->RunSerial) {
        onlyRunSerialTestsLeft = false;
      }
    }
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "***** WAITING, ");

    if (this->SerialTestRunning) {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "Waiting for RUN_SERIAL test to finish.");
    } else if (onlyRunSerialTestsLeft) {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "Only RUN_SERIAL tests remain, awaiting available slot.");
    } else {
      /* clang-format off */
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "System Load: " << systemLoad << ", "
                 "Max Allowed Load: " << this->TestLoad << ", "
                 "Smallest test " << testWithMinProcessors <<
                 " requires " << minProcessorsRequired);
      /* clang-format on */
    }
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "*****" << std::endl);

    // Wait between 1 and 5 seconds before trying again.
    unsigned int milliseconds = (cmSystemTools::RandomSeed() % 5 + 1) * 1000;
    if (this->FakeLoadForTesting) {
      milliseconds = 10;
    }
    if (this->TestLoadRetryTimer.get() == nullptr) {
      this->TestLoadRetryTimer.init(this->Loop, this);
    }
    this->TestLoadRetryTimer.start(
      &cmCTestMultiProcessHandler::OnTestLoadRetryCB, milliseconds, 0);
  }
}

void cmCTestMultiProcessHandler::OnTestLoadRetryCB(uv_timer_t* timer)
{
  auto* self = static_cast<cmCTestMultiProcessHandler*>(timer->data);
  self->StartNextTests();
}

void cmCTestMultiProcessHandler::FinishTestProcess(
  std::unique_ptr<cmCTestRunTest> runner, bool started)
{
  this->Completed++;

  int test = runner->GetIndex();
  auto* properties = runner->GetTestProperties();

  cmCTestRunTest::EndTestResult testResult =
    runner->EndTest(this->Completed, this->Total, started);
  if (testResult.StopTimePassed) {
    this->SetStopTimePassed();
  }
  if (started) {
    if (!this->StopTimePassed &&
        cmCTestRunTest::StartAgain(std::move(runner), this->Completed)) {
      this->Completed--; // remove the completed test because run again
      return;
    }
  }

  if (testResult.Passed) {
    this->Passed->push_back(properties->Name);
  } else if (!properties->Disabled) {
    this->Failed->push_back(properties->Name);
  }

  for (auto& t : this->Tests) {
    t.second.erase(test);
  }

  this->TestFinishMap[test] = true;
  this->TestRunningMap[test] = false;
  this->WriteCheckpoint(test);
  this->DeallocateResources(test);
  this->UnlockResources(test);
  this->RunningCount -= this->GetProcessorsUsed(test);

  for (auto p : properties->Affinity) {
    this->ProcessorsAvailable.insert(p);
  }
  properties->Affinity.clear();

  runner.reset();
  if (started) {
    this->StartNextTests();
  }
}

void cmCTestMultiProcessHandler::UpdateCostData()
{
  std::string fname = this->CTest->GetCostDataFile();
  std::string tmpout = fname + ".tmp";
  cmsys::ofstream fout;
  fout.open(tmpout.c_str());

  PropertiesMap temp = this->Properties;

  if (cmSystemTools::FileExists(fname)) {
    cmsys::ifstream fin;
    fin.open(fname.c_str());

    std::string line;
    while (std::getline(fin, line)) {
      if (line == "---") {
        break;
      }
      std::vector<std::string> parts = cmSystemTools::SplitString(line, ' ');
      // Format: <name> <previous_runs> <avg_cost>
      if (parts.size() < 3) {
        break;
      }

      std::string name = parts[0];
      int prev = atoi(parts[1].c_str());
      float cost = static_cast<float>(atof(parts[2].c_str()));

      int index = this->SearchByName(name);
      if (index == -1) {
        // This test is not in memory. We just rewrite the entry
        fout << name << " " << prev << " " << cost << "\n";
      } else {
        // Update with our new average cost
        fout << name << " " << this->Properties[index]->PreviousRuns << " "
             << this->Properties[index]->Cost << "\n";
        temp.erase(index);
      }
    }
    fin.close();
    cmSystemTools::RemoveFile(fname);
  }

  // Add all tests not previously listed in the file
  for (auto const& i : temp) {
    fout << i.second->Name << " " << i.second->PreviousRuns << " "
         << i.second->Cost << "\n";
  }

  // Write list of failed tests
  fout << "---\n";
  for (std::string const& f : *this->Failed) {
    fout << f << "\n";
  }
  fout.close();
  cmSystemTools::RenameFile(tmpout, fname);
}

void cmCTestMultiProcessHandler::ReadCostData()
{
  std::string fname = this->CTest->GetCostDataFile();

  if (cmSystemTools::FileExists(fname, true)) {
    cmsys::ifstream fin;
    fin.open(fname.c_str());
    std::string line;
    while (std::getline(fin, line)) {
      if (line == "---") {
        break;
      }

      std::vector<std::string> parts = cmSystemTools::SplitString(line, ' ');

      // Probably an older version of the file, will be fixed next run
      if (parts.size() < 3) {
        fin.close();
        return;
      }

      std::string name = parts[0];
      int prev = atoi(parts[1].c_str());
      float cost = static_cast<float>(atof(parts[2].c_str()));

      int index = this->SearchByName(name);
      if (index == -1) {
        continue;
      }

      this->Properties[index]->PreviousRuns = prev;
      // When not running in parallel mode, don't use cost data
      if (this->ParallelLevel > 1 && this->Properties[index] &&
          this->Properties[index]->Cost == 0) {
        this->Properties[index]->Cost = cost;
      }
    }
    // Next part of the file is the failed tests
    while (std::getline(fin, line)) {
      if (!line.empty()) {
        this->LastTestsFailed.push_back(line);
      }
    }
    fin.close();
  }
}

int cmCTestMultiProcessHandler::SearchByName(std::string const& name)
{
  int index = -1;

  for (auto const& p : this->Properties) {
    if (p.second->Name == name) {
      index = p.first;
    }
  }
  return index;
}

void cmCTestMultiProcessHandler::CreateTestCostList()
{
  if (this->ParallelLevel > 1) {
    this->CreateParallelTestCostList();
  } else {
    this->CreateSerialTestCostList();
  }
}

void cmCTestMultiProcessHandler::CreateParallelTestCostList()
{
  TestSet alreadySortedTests;

  std::list<TestSet> priorityStack;
  priorityStack.emplace_back();
  TestSet& topLevel = priorityStack.back();

  // In parallel test runs add previously failed tests to the front
  // of the cost list and queue other tests for further sorting
  for (auto const& t : this->Tests) {
    if (cm::contains(this->LastTestsFailed, this->Properties[t.first]->Name)) {
      // If the test failed last time, it should be run first.
      this->SortedTests.push_back(t.first);
      alreadySortedTests.insert(t.first);
    } else {
      topLevel.insert(t.first);
    }
  }

  // In parallel test runs repeatedly move dependencies of the tests on
  // the current dependency level to the next level until no
  // further dependencies exist.
  while (!priorityStack.back().empty()) {
    TestSet& previousSet = priorityStack.back();
    priorityStack.emplace_back();
    TestSet& currentSet = priorityStack.back();

    for (auto const& i : previousSet) {
      TestSet const& dependencies = this->Tests[i];
      currentSet.insert(dependencies.begin(), dependencies.end());
    }

    for (auto const& i : currentSet) {
      previousSet.erase(i);
    }
  }

  // Remove the empty dependency level
  priorityStack.pop_back();

  // Reverse iterate over the different dependency levels (deepest first).
  // Sort tests within each level by COST and append them to the cost list.
  for (TestSet const& currentSet : cmReverseRange(priorityStack)) {
    TestList sortedCopy;
    cm::append(sortedCopy, currentSet);
    std::stable_sort(sortedCopy.begin(), sortedCopy.end(),
                     TestComparator(this));

    for (auto const& j : sortedCopy) {
      if (!cm::contains(alreadySortedTests, j)) {
        this->SortedTests.push_back(j);
        alreadySortedTests.insert(j);
      }
    }
  }
}

void cmCTestMultiProcessHandler::GetAllTestDependencies(int test,
                                                        TestList& dependencies)
{
  TestSet const& dependencySet = this->Tests[test];
  for (int i : dependencySet) {
    this->GetAllTestDependencies(i, dependencies);
    dependencies.push_back(i);
  }
}

void cmCTestMultiProcessHandler::CreateSerialTestCostList()
{
  TestList presortedList;

  for (auto const& i : this->Tests) {
    presortedList.push_back(i.first);
  }

  std::stable_sort(presortedList.begin(), presortedList.end(),
                   TestComparator(this));

  TestSet alreadySortedTests;

  for (int test : presortedList) {
    if (cm::contains(alreadySortedTests, test)) {
      continue;
    }

    TestList dependencies;
    this->GetAllTestDependencies(test, dependencies);

    for (int testDependency : dependencies) {
      if (!cm::contains(alreadySortedTests, testDependency)) {
        alreadySortedTests.insert(testDependency);
        this->SortedTests.push_back(testDependency);
      }
    }

    alreadySortedTests.insert(test);
    this->SortedTests.push_back(test);
  }
}

void cmCTestMultiProcessHandler::WriteCheckpoint(int index)
{
  std::string fname =
    this->CTest->GetBinaryDir() + "/Testing/Temporary/CTestCheckpoint.txt";
  cmsys::ofstream fout;
  fout.open(fname.c_str(), std::ios::app);
  fout << index << "\n";
  fout.close();
}

void cmCTestMultiProcessHandler::MarkFinished()
{
  std::string fname =
    this->CTest->GetBinaryDir() + "/Testing/Temporary/CTestCheckpoint.txt";
  cmSystemTools::RemoveFile(fname);
}

static Json::Value DumpToJsonArray(const std::set<std::string>& values)
{
  Json::Value jsonArray = Json::arrayValue;
  for (const auto& it : values) {
    jsonArray.append(it);
  }
  return jsonArray;
}

static Json::Value DumpToJsonArray(const std::vector<std::string>& values)
{
  Json::Value jsonArray = Json::arrayValue;
  for (const auto& it : values) {
    jsonArray.append(it);
  }
  return jsonArray;
}

static Json::Value DumpRegExToJsonArray(
  const std::vector<std::pair<cmsys::RegularExpression, std::string>>& values)
{
  Json::Value jsonArray = Json::arrayValue;
  for (const auto& it : values) {
    jsonArray.append(it.second);
  }
  return jsonArray;
}

static Json::Value DumpMeasurementToJsonArray(
  const std::map<std::string, std::string>& values)
{
  Json::Value jsonArray = Json::arrayValue;
  for (const auto& it : values) {
    Json::Value measurement = Json::objectValue;
    measurement["measurement"] = it.first;
    measurement["value"] = it.second;
    jsonArray.append(measurement);
  }
  return jsonArray;
}

static Json::Value DumpTimeoutAfterMatch(
  cmCTestTestHandler::cmCTestTestProperties& testProperties)
{
  Json::Value timeoutAfterMatch = Json::objectValue;
  timeoutAfterMatch["timeout"] = testProperties.AlternateTimeout.count();
  timeoutAfterMatch["regex"] =
    DumpRegExToJsonArray(testProperties.TimeoutRegularExpressions);
  return timeoutAfterMatch;
}

static Json::Value DumpResourceGroupsToJsonArray(
  const std::vector<
    std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>&
    resourceGroups)
{
  Json::Value jsonResourceGroups = Json::arrayValue;
  for (auto const& it : resourceGroups) {
    Json::Value jsonResourceGroup = Json::objectValue;
    Json::Value requirements = Json::arrayValue;
    for (auto const& it2 : it) {
      Json::Value res = Json::objectValue;
      res[".type"] = it2.ResourceType;
      // res[".units"] = it2.UnitsNeeded; // Intentionally commented out
      res["slots"] = it2.SlotsNeeded;
      requirements.append(res);
    }
    jsonResourceGroup["requirements"] = requirements;
    jsonResourceGroups.append(jsonResourceGroup);
  }
  return jsonResourceGroups;
}

static Json::Value DumpCTestProperty(std::string const& name,
                                     Json::Value value)
{
  Json::Value property = Json::objectValue;
  property["name"] = name;
  property["value"] = std::move(value);
  return property;
}

static Json::Value DumpCTestProperties(
  cmCTestTestHandler::cmCTestTestProperties& testProperties)
{
  Json::Value properties = Json::arrayValue;
  if (!testProperties.AttachOnFail.empty()) {
    properties.append(DumpCTestProperty(
      "ATTACHED_FILES_ON_FAIL", DumpToJsonArray(testProperties.AttachOnFail)));
  }
  if (!testProperties.AttachedFiles.empty()) {
    properties.append(DumpCTestProperty(
      "ATTACHED_FILES", DumpToJsonArray(testProperties.AttachedFiles)));
  }
  if (testProperties.Cost != 0.0f) {
    properties.append(
      DumpCTestProperty("COST", static_cast<double>(testProperties.Cost)));
  }
  if (!testProperties.Depends.empty()) {
    properties.append(
      DumpCTestProperty("DEPENDS", DumpToJsonArray(testProperties.Depends)));
  }
  if (testProperties.Disabled) {
    properties.append(DumpCTestProperty("DISABLED", testProperties.Disabled));
  }
  if (!testProperties.Environment.empty()) {
    properties.append(DumpCTestProperty(
      "ENVIRONMENT", DumpToJsonArray(testProperties.Environment)));
  }
  if (!testProperties.EnvironmentModification.empty()) {
    properties.append(DumpCTestProperty(
      "ENVIRONMENT_MODIFICATION",
      DumpToJsonArray(testProperties.EnvironmentModification)));
  }
  if (!testProperties.ErrorRegularExpressions.empty()) {
    properties.append(DumpCTestProperty(
      "FAIL_REGULAR_EXPRESSION",
      DumpRegExToJsonArray(testProperties.ErrorRegularExpressions)));
  }
  if (!testProperties.SkipRegularExpressions.empty()) {
    properties.append(DumpCTestProperty(
      "SKIP_REGULAR_EXPRESSION",
      DumpRegExToJsonArray(testProperties.SkipRegularExpressions)));
  }
  if (!testProperties.FixturesCleanup.empty()) {
    properties.append(DumpCTestProperty(
      "FIXTURES_CLEANUP", DumpToJsonArray(testProperties.FixturesCleanup)));
  }
  if (!testProperties.FixturesRequired.empty()) {
    properties.append(DumpCTestProperty(
      "FIXTURES_REQUIRED", DumpToJsonArray(testProperties.FixturesRequired)));
  }
  if (!testProperties.FixturesSetup.empty()) {
    properties.append(DumpCTestProperty(
      "FIXTURES_SETUP", DumpToJsonArray(testProperties.FixturesSetup)));
  }
  if (!testProperties.Labels.empty()) {
    properties.append(
      DumpCTestProperty("LABELS", DumpToJsonArray(testProperties.Labels)));
  }
  if (!testProperties.Measurements.empty()) {
    properties.append(DumpCTestProperty(
      "MEASUREMENT", DumpMeasurementToJsonArray(testProperties.Measurements)));
  }
  if (!testProperties.RequiredRegularExpressions.empty()) {
    properties.append(DumpCTestProperty(
      "PASS_REGULAR_EXPRESSION",
      DumpRegExToJsonArray(testProperties.RequiredRegularExpressions)));
  }
  if (!testProperties.ResourceGroups.empty()) {
    properties.append(DumpCTestProperty(
      "RESOURCE_GROUPS",
      DumpResourceGroupsToJsonArray(testProperties.ResourceGroups)));
  }
  if (testProperties.WantAffinity) {
    properties.append(
      DumpCTestProperty("PROCESSOR_AFFINITY", testProperties.WantAffinity));
  }
  if (testProperties.Processors != 1) {
    properties.append(
      DumpCTestProperty("PROCESSORS", testProperties.Processors));
  }
  if (!testProperties.RequiredFiles.empty()) {
    properties.append(DumpCTestProperty(
      "REQUIRED_FILES", DumpToJsonArray(testProperties.RequiredFiles)));
  }
  if (!testProperties.LockedResources.empty()) {
    properties.append(DumpCTestProperty(
      "RESOURCE_LOCK", DumpToJsonArray(testProperties.LockedResources)));
  }
  if (testProperties.RunSerial) {
    properties.append(
      DumpCTestProperty("RUN_SERIAL", testProperties.RunSerial));
  }
  if (testProperties.SkipReturnCode != -1) {
    properties.append(
      DumpCTestProperty("SKIP_RETURN_CODE", testProperties.SkipReturnCode));
  }
  if (testProperties.Timeout) {
    properties.append(
      DumpCTestProperty("TIMEOUT", testProperties.Timeout->count()));
  }
  if (!testProperties.TimeoutRegularExpressions.empty()) {
    properties.append(DumpCTestProperty(
      "TIMEOUT_AFTER_MATCH", DumpTimeoutAfterMatch(testProperties)));
  }
  if (testProperties.WillFail) {
    properties.append(DumpCTestProperty("WILL_FAIL", testProperties.WillFail));
  }
  if (!testProperties.Directory.empty()) {
    properties.append(
      DumpCTestProperty("WORKING_DIRECTORY", testProperties.Directory));
  }
  return properties;
}

class BacktraceData
{
  std::unordered_map<std::string, Json::ArrayIndex> CommandMap;
  std::unordered_map<std::string, Json::ArrayIndex> FileMap;
  std::unordered_map<cmListFileContext const*, Json::ArrayIndex> NodeMap;
  Json::Value Commands = Json::arrayValue;
  Json::Value Files = Json::arrayValue;
  Json::Value Nodes = Json::arrayValue;

  Json::ArrayIndex AddCommand(std::string const& command)
  {
    auto i = this->CommandMap.find(command);
    if (i == this->CommandMap.end()) {
      i = this->CommandMap.emplace(command, this->Commands.size()).first;
      this->Commands.append(command);
    }
    return i->second;
  }

  Json::ArrayIndex AddFile(std::string const& file)
  {
    auto i = this->FileMap.find(file);
    if (i == this->FileMap.end()) {
      i = this->FileMap.emplace(file, this->Files.size()).first;
      this->Files.append(file);
    }
    return i->second;
  }

public:
  bool Add(cmListFileBacktrace const& bt, Json::ArrayIndex& index);
  Json::Value Dump();
};

bool BacktraceData::Add(cmListFileBacktrace const& bt, Json::ArrayIndex& index)
{
  if (bt.Empty()) {
    return false;
  }
  cmListFileContext const* top = &bt.Top();
  auto found = this->NodeMap.find(top);
  if (found != this->NodeMap.end()) {
    index = found->second;
    return true;
  }
  Json::Value entry = Json::objectValue;
  entry["file"] = this->AddFile(top->FilePath);
  if (top->Line) {
    entry["line"] = static_cast<int>(top->Line);
  }
  if (!top->Name.empty()) {
    entry["command"] = this->AddCommand(top->Name);
  }
  Json::ArrayIndex parent;
  if (this->Add(bt.Pop(), parent)) {
    entry["parent"] = parent;
  }
  index = this->NodeMap[top] = this->Nodes.size();
  this->Nodes.append(std::move(entry)); // NOLINT(*)
  return true;
}

Json::Value BacktraceData::Dump()
{
  Json::Value backtraceGraph;
  this->CommandMap.clear();
  this->FileMap.clear();
  this->NodeMap.clear();
  backtraceGraph["commands"] = std::move(this->Commands);
  backtraceGraph["files"] = std::move(this->Files);
  backtraceGraph["nodes"] = std::move(this->Nodes);
  return backtraceGraph;
}

static void AddBacktrace(BacktraceData& backtraceGraph, Json::Value& object,
                         cmListFileBacktrace const& bt)
{
  Json::ArrayIndex backtrace;
  if (backtraceGraph.Add(bt, backtrace)) {
    object["backtrace"] = backtrace;
  }
}

static Json::Value DumpCTestInfo(
  cmCTestRunTest& testRun,
  cmCTestTestHandler::cmCTestTestProperties& testProperties,
  BacktraceData& backtraceGraph)
{
  Json::Value testInfo = Json::objectValue;
  // test name should always be present
  testInfo["name"] = testProperties.Name;
  std::string const& config = testRun.GetCTest()->GetConfigType();
  if (!config.empty()) {
    testInfo["config"] = config;
  }
  std::string const& command = testRun.GetActualCommand();
  if (!command.empty()) {
    std::vector<std::string> commandAndArgs;
    commandAndArgs.push_back(command);
    const std::vector<std::string>& args = testRun.GetArguments();
    if (!args.empty()) {
      commandAndArgs.reserve(args.size() + 1);
      cm::append(commandAndArgs, args);
    }
    testInfo["command"] = DumpToJsonArray(commandAndArgs);
  }
  Json::Value properties = DumpCTestProperties(testProperties);
  if (!properties.empty()) {
    testInfo["properties"] = properties;
  }
  if (!testProperties.Backtrace.Empty()) {
    AddBacktrace(backtraceGraph, testInfo, testProperties.Backtrace);
  }
  return testInfo;
}

static Json::Value DumpVersion(int major, int minor)
{
  Json::Value version = Json::objectValue;
  version["major"] = major;
  version["minor"] = minor;
  return version;
}

void cmCTestMultiProcessHandler::PrintOutputAsJson()
{
  this->TestHandler->SetMaxIndex(this->FindMaxIndex());

  Json::Value result = Json::objectValue;
  result["kind"] = "ctestInfo";
  result["version"] = DumpVersion(1, 0);

  BacktraceData backtraceGraph;
  Json::Value tests = Json::arrayValue;
  for (auto& it : this->Properties) {
    cmCTestTestHandler::cmCTestTestProperties& p = *it.second;

    // Don't worry if this fails, we are only showing the test list, not
    // running the tests
    cmWorkingDirectory workdir(p.Directory);
    cmCTestRunTest testRun(*this);
    testRun.SetIndex(p.Index);
    testRun.SetTestProperties(&p);
    testRun.ComputeArguments();

    // Skip tests not available in this configuration.
    if (p.Args.size() >= 2 && p.Args[1] == "NOT_AVAILABLE") {
      continue;
    }

    Json::Value testInfo = DumpCTestInfo(testRun, p, backtraceGraph);
    tests.append(testInfo);
  }
  result["backtraceGraph"] = backtraceGraph.Dump();
  result["tests"] = std::move(tests);

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  ";
  std::unique_ptr<Json::StreamWriter> jout(builder.newStreamWriter());
  jout->write(result, &std::cout);
}

// For ShowOnly mode
void cmCTestMultiProcessHandler::PrintTestList()
{
  if (this->CTest->GetOutputAsJson()) {
    this->PrintOutputAsJson();
    return;
  }

  this->TestHandler->SetMaxIndex(this->FindMaxIndex());

  for (auto& it : this->Properties) {
    cmCTestTestHandler::cmCTestTestProperties& p = *it.second;

    // Don't worry if this fails, we are only showing the test list, not
    // running the tests
    cmWorkingDirectory workdir(p.Directory);

    cmCTestRunTest testRun(*this);
    testRun.SetIndex(p.Index);
    testRun.SetTestProperties(&p);
    testRun.ComputeArguments(); // logs the command in verbose mode

    if (!p.Labels.empty()) // print the labels
    {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "Labels:", this->Quiet);
    }
    for (std::string const& label : p.Labels) {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT, " " << label,
                         this->Quiet);
    }
    if (!p.Labels.empty()) // print the labels
    {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl,
                         this->Quiet);
    }

    if (this->TestHandler->MemCheck) {
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "  Memory Check",
                         this->Quiet);
    } else {
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "  Test", this->Quiet);
    }
    std::ostringstream indexStr;
    indexStr << " #" << p.Index << ":";
    cmCTestOptionalLog(
      this->CTest, HANDLER_OUTPUT,
      std::setw(3 + getNumWidth(this->TestHandler->GetMaxIndex()))
        << indexStr.str(),
      this->Quiet);
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, " " << p.Name,
                       this->Quiet);
    if (p.Disabled) {
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, " (Disabled)",
                         this->Quiet);
    }
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, std::endl, this->Quiet);
  }

  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     std::endl
                       << "Total Tests: " << this->Total << std::endl,
                     this->Quiet);
}

void cmCTestMultiProcessHandler::PrintLabels()
{
  std::set<std::string> allLabels;
  for (auto& it : this->Properties) {
    cmCTestTestHandler::cmCTestTestProperties& p = *it.second;
    allLabels.insert(p.Labels.begin(), p.Labels.end());
  }

  if (!allLabels.empty()) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "All Labels:" << std::endl,
                       this->Quiet);
  } else {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "No Labels Exist" << std::endl, this->Quiet);
  }
  for (std::string const& label : allLabels) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "  " << label << std::endl,
                       this->Quiet);
  }
}

void cmCTestMultiProcessHandler::CheckResume()
{
  std::string fname =
    this->CTest->GetBinaryDir() + "/Testing/Temporary/CTestCheckpoint.txt";
  if (this->CTest->GetFailover()) {
    if (cmSystemTools::FileExists(fname, true)) {
      *this->TestHandler->LogFile
        << "Resuming previously interrupted test set" << std::endl
        << "----------------------------------------------------------"
        << std::endl;

      cmsys::ifstream fin;
      fin.open(fname.c_str());
      std::string line;
      while (std::getline(fin, line)) {
        int index = atoi(line.c_str());
        this->RemoveTest(index);
      }
      fin.close();
    }
  } else if (cmSystemTools::FileExists(fname, true)) {
    cmSystemTools::RemoveFile(fname);
  }
}

void cmCTestMultiProcessHandler::RemoveTest(int index)
{
  this->EraseTest(index);
  this->Properties.erase(index);
  this->TestRunningMap[index] = false;
  this->TestFinishMap[index] = true;
  this->Completed++;
}

int cmCTestMultiProcessHandler::FindMaxIndex()
{
  int max = 0;
  for (auto const& i : this->Tests) {
    if (i.first > max) {
      max = i.first;
    }
  }
  return max;
}

// Returns true if no cycles exist in the dependency graph
bool cmCTestMultiProcessHandler::CheckCycles()
{
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Checking test dependency graph..." << std::endl,
                     this->Quiet);
  for (auto const& it : this->Tests) {
    // DFS from each element to itself
    int root = it.first;
    std::set<int> visited;
    std::stack<int> s;
    s.push(root);
    while (!s.empty()) {
      int test = s.top();
      s.pop();
      if (visited.insert(test).second) {
        for (auto const& d : this->Tests[test]) {
          if (d == root) {
            // cycle exists
            cmCTestLog(
              this->CTest, ERROR_MESSAGE,
              "Error: a cycle exists in the test dependency graph "
              "for the test \""
                << this->Properties[root]->Name
                << "\".\nPlease fix the cycle and run ctest again.\n");
            return false;
          }
          s.push(d);
        }
      }
    }
  }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Checking test dependency graph end" << std::endl,
                     this->Quiet);
  return true;
}

bool cmCTestMultiProcessHandler::CheckGeneratedResourceSpec()
{
  for (auto& test : this->Properties) {
    if (!test.second->GeneratedResourceSpecFile.empty()) {
      if (this->ResourceSpecSetupTest) {
        cmCTestLog(
          this->CTest, ERROR_MESSAGE,
          "Only one test may define the GENERATED_RESOURCE_SPEC_FILE property"
            << std::endl);
        return false;
      }

      if (test.second->FixturesSetup.size() != 1) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Test that defines GENERATED_RESOURCE_SPEC_FILE must have "
                   "exactly one FIXTURES_SETUP"
                     << std::endl);
        return false;
      }

      if (!cmSystemTools::FileIsFullPath(
            test.second->GeneratedResourceSpecFile)) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "GENERATED_RESOURCE_SPEC_FILE must be an absolute path"
                     << std::endl);
        return false;
      }

      this->ResourceSpecSetupTest = test.first;
      this->ResourceSpecSetupFixture = *test.second->FixturesSetup.begin();
    }
  }

  if (!this->ResourceSpecSetupFixture.empty()) {
    for (auto& test : this->Properties) {
      if (!test.second->ResourceGroups.empty() &&
          !test.second->FixturesRequired.count(
            this->ResourceSpecSetupFixture)) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "All tests that have RESOURCE_GROUPS must include the "
                   "resource spec generator fixture in their FIXTURES_REQUIRED"
                     << std::endl);
        return false;
      }
    }
  }

  if (!this->ResourceSpecFile.empty()) {
    if (this->ResourceSpecSetupTest) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "GENERATED_RESOURCE_SPEC_FILE test property cannot be used "
                 "in conjunction with ResourceSpecFile option"
                   << std::endl);
      return false;
    }
    std::string error;
    if (!this->InitResourceAllocator(error)) {
      cmCTestLog(this->CTest, ERROR_MESSAGE, error << std::endl);
      return false;
    }
  }

  return true;
}

bool cmCTestMultiProcessHandler::InitResourceAllocator(std::string& error)
{
  if (!this->ResourceSpec.ReadFromJSONFile(this->ResourceSpecFile)) {
    error = cmStrCat("Could not read/parse resource spec file ",
                     this->ResourceSpecFile, ": ",
                     this->ResourceSpec.parseState.GetErrorMessage());
    return false;
  }
  this->UseResourceSpec = true;
  this->ResourceAllocator.InitializeFromResourceSpec(this->ResourceSpec);
  return true;
}
