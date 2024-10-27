/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestTestCommand.h"

#include <chrono>
#include <cstdlib>
#include <ratio>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCommand.h"
#include "cmDuration.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

class cmExecutionStatus;

std::unique_ptr<cmCommand> cmCTestTestCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestTestCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

std::unique_ptr<cmCTestGenericHandler> cmCTestTestCommand::InitializeHandler(
  HandlerArguments& arguments)
{
  auto& args = static_cast<TestArguments&>(arguments);
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
  if (args.ResourceSpecFile.empty() && resourceSpecFile) {
    args.ResourceSpecFile = *resourceSpecFile;
  }

  auto handler = this->InitializeActualHandler(args);
  if (!args.Start.empty() || !args.End.empty() || !args.Stride.empty()) {
    handler->TestOptions.TestsToRunInformation =
      cmStrCat(args.Start, ',', args.End, ',', args.Stride);
  }
  if (!args.Exclude.empty()) {
    handler->TestOptions.ExcludeRegularExpression = args.Exclude;
  }
  if (!args.Include.empty()) {
    handler->TestOptions.IncludeRegularExpression = args.Include;
  }
  if (!args.ExcludeLabel.empty()) {
    handler->TestOptions.ExcludeLabelRegularExpression.push_back(
      args.ExcludeLabel);
  }
  if (!args.IncludeLabel.empty()) {
    handler->TestOptions.LabelRegularExpression.push_back(args.IncludeLabel);
  }

  if (!args.ExcludeTestsFromFile.empty()) {
    handler->TestOptions.ExcludeTestListFile = args.ExcludeTestsFromFile;
  }
  if (!args.IncludeTestsFromFile.empty()) {
    handler->TestOptions.TestListFile = args.IncludeTestsFromFile;
  }

  if (!args.ExcludeFixture.empty()) {
    handler->TestOptions.ExcludeFixtureRegularExpression = args.ExcludeFixture;
  }
  if (!args.ExcludeFixtureSetup.empty()) {
    handler->TestOptions.ExcludeFixtureSetupRegularExpression =
      args.ExcludeFixtureSetup;
  }
  if (!args.ExcludeFixtureCleanup.empty()) {
    handler->TestOptions.ExcludeFixtureCleanupRegularExpression =
      args.ExcludeFixtureCleanup;
  }
  if (args.StopOnFailure) {
    handler->TestOptions.StopOnFailure = true;
  }
  if (args.ParallelLevel) {
    handler->ParallelLevel = *args.ParallelLevel;
  }
  if (!args.Repeat.empty()) {
    handler->Repeat = args.Repeat;
  }
  if (!args.ScheduleRandom.empty()) {
    handler->TestOptions.ScheduleRandom = cmValue(args.ScheduleRandom).IsOn();
  }
  if (!args.ResourceSpecFile.empty()) {
    handler->TestOptions.ResourceSpecFile = args.ResourceSpecFile;
  }
  if (!args.StopTime.empty()) {
    this->CTest->SetStopTime(args.StopTime);
  }

  // Test load is determined by: TEST_LOAD argument,
  // or CTEST_TEST_LOAD script variable, or ctest --test-load
  // command line argument... in that order.
  unsigned long testLoad;
  cmValue ctestTestLoad = this->Makefile->GetDefinition("CTEST_TEST_LOAD");
  if (!args.TestLoad.empty()) {
    if (!cmStrToULong(args.TestLoad, &testLoad)) {
      testLoad = 0;
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'TEST_LOAD' : " << args.TestLoad
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
                                       *labelsForSubprojects, args.Quiet);
  }

  if (!args.OutputJUnit.empty()) {
    handler->SetJUnitXMLFileName(args.OutputJUnit);
  }

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

std::unique_ptr<cmCTestTestHandler>
cmCTestTestCommand::InitializeActualHandler(HandlerArguments&)
{
  return cm::make_unique<cmCTestTestHandler>(this->CTest);
}

bool cmCTestTestCommand::InitialPass(std::vector<std::string> const& args,
                                     cmExecutionStatus& status)
{
  static auto const parser = MakeTestParser<TestArguments>();

  std::vector<std::string> unparsedArguments;
  TestArguments arguments = parser.Parse(args, &unparsedArguments);
  return this->ExecuteHandlerCommand(arguments, unparsedArguments, status);
}
