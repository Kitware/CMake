/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cmCTest.h"
#include "cmCTestMultiProcessHandler.h"
#include "cmCTestTestHandler.h"
#include "cmProcess.h"

/** \class cmRunTest
 * \brief represents a single test to be run
 *
 * cmRunTest contains the information related to running a single test
 */
class cmCTestRunTest
{
public:
  explicit cmCTestRunTest(cmCTestMultiProcessHandler& multiHandler, int index);

  void SetNumberOfRuns(int n)
  {
    this->NumberOfRunsLeft = n;
    this->NumberOfRunsTotal = n;
  }

  void SetRepeatMode(cmCTest::Repeat r) { this->RepeatMode = r; }

  cmCTestTestHandler::cmCTestTestProperties* GetTestProperties()
  {
    return this->TestProperties;
  }

  int GetIndex() { return this->Index; }

  void AddFailedDependency(const std::string& failedTest)
  {
    this->FailedDependencies.insert(failedTest);
  }

  std::string GetProcessOutput() { return this->ProcessOutput; }

  cmCTestTestHandler::cmCTestTestResult GetTestResults()
  {
    return this->TestResult;
  }

  // Read and store output.  Returns true if it must be called again.
  void CheckOutput(std::string const& line);

  static void StartTest(std::unique_ptr<cmCTestRunTest> runner,
                        size_t completed, size_t total);
  static bool StartAgain(std::unique_ptr<cmCTestRunTest> runner,
                         size_t completed);

  static void StartFailure(std::unique_ptr<cmCTestRunTest> runner,
                           size_t total, std::string const& output,
                           std::string const& detail);

  struct EndTestResult
  {
    bool Passed = false;
    bool StopTimePassed = false;
  };

  // launch the test process, return whether it started correctly
  bool StartTest(size_t completed, size_t total);
  // capture and report the test results
  EndTestResult EndTest(size_t completed, size_t total, bool started);
  // Called by ctest -N to log the command string
  void ComputeArguments();

  void ComputeWeightedCost();

  void StartFailure(size_t total, std::string const& output,
                    std::string const& detail);

  cmCTest* GetCTest() const { return this->CTest; }

  std::string& GetActualCommand() { return this->ActualCommand; }

  const std::vector<std::string>& GetArguments() { return this->Arguments; }

  void FinalizeTest(bool started = true);

  void SetUseAllocatedResources(bool use)
  {
    this->UseAllocatedResources = use;
  }
  void SetAllocatedResources(
    const std::vector<
      std::map<std::string,
               std::vector<cmCTestMultiProcessHandler::ResourceAllocation>>>&
      resources)
  {
    this->AllocatedResources = resources;
  }

private:
  bool NeedsToRepeat();
  void ParseOutputForMeasurements();
  void ExeNotFound(std::string exe);
  bool ForkProcess();
  void WriteLogOutputTop(size_t completed, size_t total);
  // Run post processing of the process output for MemCheck
  void MemCheckPostProcess();

  void SetupResourcesEnvironment(std::vector<std::string>* log = nullptr);

  // Returns "completed/total Test #Index: "
  std::string GetTestPrefix(size_t completed, size_t total) const;

  cmCTestMultiProcessHandler& MultiTestHandler;
  int Index;
  cmCTest* CTest;
  cmCTestTestHandler* TestHandler;
  cmCTestTestHandler::cmCTestTestProperties* TestProperties;

  std::unique_ptr<cmProcess> TestProcess;
  std::string ProcessOutput;
  cmCTestTestHandler::cmCTestTestResult TestResult;
  std::set<std::string> FailedDependencies;
  std::string StartTime;
  std::string ActualCommand;
  std::vector<std::string> Arguments;
  bool UseAllocatedResources = false;
  std::vector<std::map<
    std::string, std::vector<cmCTestMultiProcessHandler::ResourceAllocation>>>
    AllocatedResources;
  cmCTest::Repeat RepeatMode = cmCTest::Repeat::Never;
  int NumberOfRunsLeft = 1;  // default to 1 run of the test
  int NumberOfRunsTotal = 1; // default to 1 run of the test
  bool RunAgain = false;     // default to not having to run again
  size_t TotalNumberOfTests;
};

inline int getNumWidth(size_t n)
{
  int w = 1;
  while (n >= 10) {
    n /= 10;
    ++w;
  }
  return w;
}
