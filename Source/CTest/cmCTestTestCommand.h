/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include <cm/optional>

#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"

class cmCTestGenericHandler;
class cmCTestTestHandler;
class cmCommand;

class cmCTestTestCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  void BindArguments() override;
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

private:
  std::unique_ptr<cmCommand> Clone() override;

  std::string GetName() const override { return "ctest_test"; }

  virtual std::unique_ptr<cmCTestTestHandler> InitializeActualHandler();

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler() override;
};
