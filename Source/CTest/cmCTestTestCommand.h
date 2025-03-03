/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"

class cmExecutionStatus;
class cmCTestGenericHandler;
class cmCTestTestHandler;

class cmCTestTestCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  struct TestArguments : HandlerArguments
  {
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

  template <typename Args>
  static auto MakeTestParser() -> cmArgumentParser<Args>
  {
    static_assert(std::is_base_of<TestArguments, Args>::value, "");
    return cmArgumentParser<Args>{ MakeHandlerParser<Args>() }
      .Bind("START"_s, &TestArguments::Start)
      .Bind("END"_s, &TestArguments::End)
      .Bind("STRIDE"_s, &TestArguments::Stride)
      .Bind("EXCLUDE"_s, &TestArguments::Exclude)
      .Bind("INCLUDE"_s, &TestArguments::Include)
      .Bind("EXCLUDE_LABEL"_s, &TestArguments::ExcludeLabel)
      .Bind("INCLUDE_LABEL"_s, &TestArguments::IncludeLabel)
      .Bind("EXCLUDE_FROM_FILE"_s, &TestArguments::ExcludeTestsFromFile)
      .Bind("INCLUDE_FROM_FILE"_s, &TestArguments::IncludeTestsFromFile)
      .Bind("EXCLUDE_FIXTURE"_s, &TestArguments::ExcludeFixture)
      .Bind("EXCLUDE_FIXTURE_SETUP"_s, &TestArguments::ExcludeFixtureSetup)
      .Bind("EXCLUDE_FIXTURE_CLEANUP"_s, &TestArguments::ExcludeFixtureCleanup)
      .Bind("PARALLEL_LEVEL"_s, &TestArguments::ParallelLevel)
      .Bind("REPEAT"_s, &TestArguments::Repeat)
      .Bind("SCHEDULE_RANDOM"_s, &TestArguments::ScheduleRandom)
      .Bind("STOP_TIME"_s, &TestArguments::StopTime)
      .Bind("TEST_LOAD"_s, &TestArguments::TestLoad)
      .Bind("RESOURCE_SPEC_FILE"_s, &TestArguments::ResourceSpecFile)
      .Bind("STOP_ON_FAILURE"_s, &TestArguments::StopOnFailure)
      .Bind("OUTPUT_JUNIT"_s, &TestArguments::OutputJUnit);
  }

private:
  std::string GetName() const override { return "ctest_test"; }

  virtual std::unique_ptr<cmCTestTestHandler> InitializeActualHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) const;

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) const override;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) const override;
};
