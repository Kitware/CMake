/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestTestCommand.h"

#include <chrono>
#include <cstdlib>
#include <ratio>
#include <sstream>
#include <vector>

#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestTestHandler.h"
#include "cmDuration.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

void cmCTestTestCommand::BindArguments()
{
  this->cmCTestHandlerCommand::BindArguments();
  this->Bind("START"_s, this->Start);
  this->Bind("END"_s, this->End);
  this->Bind("STRIDE"_s, this->Stride);
  this->Bind("EXCLUDE"_s, this->Exclude);
  this->Bind("INCLUDE"_s, this->Include);
  this->Bind("EXCLUDE_LABEL"_s, this->ExcludeLabel);
  this->Bind("INCLUDE_LABEL"_s, this->IncludeLabel);
  this->Bind("EXCLUDE_FROM_FILE"_s, this->ExcludeTestsFromFile);
  this->Bind("INCLUDE_FROM_FILE"_s, this->IncludeTestsFromFile);
  this->Bind("EXCLUDE_FIXTURE"_s, this->ExcludeFixture);
  this->Bind("EXCLUDE_FIXTURE_SETUP"_s, this->ExcludeFixtureSetup);
  this->Bind("EXCLUDE_FIXTURE_CLEANUP"_s, this->ExcludeFixtureCleanup);
  this->Bind("PARALLEL_LEVEL"_s, this->ParallelLevel);
  this->Bind("REPEAT"_s, this->Repeat);
  this->Bind("SCHEDULE_RANDOM"_s, this->ScheduleRandom);
  this->Bind("STOP_TIME"_s, this->StopTime);
  this->Bind("TEST_LOAD"_s, this->TestLoad);
  this->Bind("RESOURCE_SPEC_FILE"_s, this->ResourceSpecFile);
  this->Bind("STOP_ON_FAILURE"_s, this->StopOnFailure);
  this->Bind("OUTPUT_JUNIT"_s, this->OutputJUnit);
}

std::unique_ptr<cmCTestGenericHandler> cmCTestTestCommand::InitializeHandler()
{
  cmValue ctestTimeout = this->Makefile->GetDefinition("CTEST_TEST_TIMEOUT");

  cmDuration timeout;
  if (ctestTimeout) {
    timeout = cmDuration(atof(ctestTimeout->c_str()));
  } else {
    timeout = this->CTest->GetTimeOut();
    if (timeout <= cmDuration::zero()) {
      // By default use timeout of 10 minutes
      timeout = std::chrono::minutes(10);
    }
  }
  this->CTest->SetTimeOut(timeout);

  cmValue resourceSpecFile =
    this->Makefile->GetDefinition("CTEST_RESOURCE_SPEC_FILE");
  if (this->ResourceSpecFile.empty() && resourceSpecFile) {
    this->ResourceSpecFile = *resourceSpecFile;
  }

  auto handler = this->InitializeActualHandler();
  if (!this->Start.empty() || !this->End.empty() || !this->Stride.empty()) {
    handler->TestOptions.TestsToRunInformation =
      cmStrCat(this->Start, ',', this->End, ',', this->Stride);
  }
  if (!this->Exclude.empty()) {
    handler->TestOptions.ExcludeRegularExpression = this->Exclude;
  }
  if (!this->Include.empty()) {
    handler->TestOptions.IncludeRegularExpression = this->Include;
  }
  if (!this->ExcludeLabel.empty()) {
    handler->TestOptions.ExcludeLabelRegularExpression.push_back(
      this->ExcludeLabel);
  }
  if (!this->IncludeLabel.empty()) {
    handler->TestOptions.LabelRegularExpression.push_back(this->IncludeLabel);
  }

  if (!this->ExcludeTestsFromFile.empty()) {
    handler->TestOptions.ExcludeTestListFile = this->ExcludeTestsFromFile;
  }
  if (!this->IncludeTestsFromFile.empty()) {
    handler->TestOptions.TestListFile = this->IncludeTestsFromFile;
  }

  if (!this->ExcludeFixture.empty()) {
    handler->TestOptions.ExcludeFixtureRegularExpression =
      this->ExcludeFixture;
  }
  if (!this->ExcludeFixtureSetup.empty()) {
    handler->TestOptions.ExcludeFixtureSetupRegularExpression =
      this->ExcludeFixtureSetup;
  }
  if (!this->ExcludeFixtureCleanup.empty()) {
    handler->TestOptions.ExcludeFixtureCleanupRegularExpression =
      this->ExcludeFixtureCleanup;
  }
  if (this->StopOnFailure) {
    handler->TestOptions.StopOnFailure = true;
  }
  if (this->ParallelLevel) {
    handler->ParallelLevel = *this->ParallelLevel;
  }
  if (!this->Repeat.empty()) {
    handler->Repeat = this->Repeat;
  }
  if (!this->ScheduleRandom.empty()) {
    handler->TestOptions.ScheduleRandom = cmValue(this->ScheduleRandom).IsOn();
  }
  if (!this->ResourceSpecFile.empty()) {
    handler->TestOptions.ResourceSpecFile = this->ResourceSpecFile;
  }
  if (!this->StopTime.empty()) {
    this->CTest->SetStopTime(this->StopTime);
  }

  // Test load is determined by: TEST_LOAD argument,
  // or CTEST_TEST_LOAD script variable, or ctest --test-load
  // command line argument... in that order.
  unsigned long testLoad;
  cmValue ctestTestLoad = this->Makefile->GetDefinition("CTEST_TEST_LOAD");
  if (!this->TestLoad.empty()) {
    if (!cmStrToULong(this->TestLoad, &testLoad)) {
      testLoad = 0;
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'TEST_LOAD' : " << this->TestLoad
                                                    << std::endl);
    }
  } else if (cmNonempty(ctestTestLoad)) {
    if (!cmStrToULong(*ctestTestLoad, &testLoad)) {
      testLoad = 0;
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'CTEST_TEST_LOAD' : " << *ctestTestLoad
                                                          << std::endl);
    }
  } else {
    testLoad = this->CTest->GetTestLoad();
  }
  handler->SetTestLoad(testLoad);

  if (cmValue labelsForSubprojects =
        this->Makefile->GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, this->Quiet);
  }

  if (!this->OutputJUnit.empty()) {
    handler->SetJUnitXMLFileName(this->OutputJUnit);
  }

  handler->SetQuiet(this->Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

std::unique_ptr<cmCTestTestHandler>
cmCTestTestCommand::InitializeActualHandler()
{
  return cm::make_unique<cmCTestTestHandler>(this->CTest);
}
