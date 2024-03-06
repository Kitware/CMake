/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>

#include <cm/memory>
#include <cm/optional>

#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"
#include "cmCommand.h"

class cmCTestGenericHandler;
class cmCTestTestHandler;

/** \class cmCTestTest
 * \brief Run a ctest script
 *
 * cmCTestTestCommand defineds the command to test the project.
 */
class cmCTestTestCommand : public cmCTestHandlerCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    auto ni = cm::make_unique<cmCTestTestCommand>();
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return std::unique_ptr<cmCommand>(std::move(ni));
  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const override { return "ctest_test"; }

protected:
  void BindArguments() override;
  virtual cmCTestTestHandler* InitializeActualHandler();
  cmCTestGenericHandler* InitializeHandler() override;

  std::string Start;
  std::string End;
  std::string Stride;
  std::string Exclude;
  std::string Include;
  std::string ExcludeLabel;
  std::string IncludeLabel;
  std::string IncludeTestsFromFile;
  std::string ExcludeTestsFromFile;
  std::string ExcludeFixture;
  std::string ExcludeFixtureSetup;
  std::string ExcludeFixtureCleanup;
  cm::optional<ArgumentParser::Maybe<std::string>> ParallelLevel;
  std::string Repeat;
  std::string ScheduleRandom;
  std::string StopTime;
  std::string TestLoad;
  std::string ResourceSpecFile;
  std::string OutputJUnit;
  bool StopOnFailure = false;
};
