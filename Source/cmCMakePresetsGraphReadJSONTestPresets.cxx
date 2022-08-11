/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"

#include "CTest/cmCTestTypes.h"

namespace {
using ReadFileResult = cmCMakePresetsGraph::ReadFileResult;
using TestPreset = cmCMakePresetsGraph::TestPreset;
using JSONHelperBuilder = cmJSONHelperBuilder<ReadFileResult>;

ReadFileResult TestPresetOutputVerbosityHelper(
  TestPreset::OutputOptions::VerbosityEnum& out, const Json::Value* value)
{
  if (!value) {
    out = TestPreset::OutputOptions::VerbosityEnum::Default;
    return ReadFileResult::READ_OK;
  }

  if (!value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "default") {
    out = TestPreset::OutputOptions::VerbosityEnum::Default;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "verbose") {
    out = TestPreset::OutputOptions::VerbosityEnum::Verbose;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "extra") {
    out = TestPreset::OutputOptions::VerbosityEnum::Extra;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const TestPresetOptionalOutputVerbosityHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions::VerbosityEnum>(
    ReadFileResult::READ_OK, TestPresetOutputVerbosityHelper);

ReadFileResult TestPresetOutputTruncationHelper(
  cm::optional<cmCTestTypes::TruncationMode>& out, const Json::Value* value)
{
  if (!value) {
    out = cm::nullopt;
    return ReadFileResult::READ_OK;
  }

  if (!value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "tail") {
    out = cmCTestTypes::TruncationMode::Tail;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "middle") {
    out = cmCTestTypes::TruncationMode::Middle;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "head") {
    out = cmCTestTypes::TruncationMode::Head;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const TestPresetOptionalOutputHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::OutputOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
      .Bind("shortProgress"_s, &TestPreset::OutputOptions::ShortProgress,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("verbosity"_s, &TestPreset::OutputOptions::Verbosity,
            TestPresetOptionalOutputVerbosityHelper, false)
      .Bind("debug"_s, &TestPreset::OutputOptions::Debug,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("outputOnFailure"_s, &TestPreset::OutputOptions::OutputOnFailure,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("quiet"_s, &TestPreset::OutputOptions::Quiet,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("outputLogFile"_s, &TestPreset::OutputOptions::OutputLogFile,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("labelSummary"_s, &TestPreset::OutputOptions::LabelSummary,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("subprojectSummary"_s,
            &TestPreset::OutputOptions::SubprojectSummary,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("maxPassedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxPassedTestOutputSize,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("maxFailedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxFailedTestOutputSize,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("testOutputTruncation"_s,
            &TestPreset::OutputOptions::TestOutputTruncation,
            TestPresetOutputTruncationHelper, false)
      .Bind("maxTestNameWidth"_s, &TestPreset::OutputOptions::MaxTestNameWidth,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false));

auto const TestPresetOptionalFilterIncludeIndexObjectHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions::IndexOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::IncludeOptions::IndexOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("start"_s, &TestPreset::IncludeOptions::IndexOptions::Start,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("end"_s, &TestPreset::IncludeOptions::IndexOptions::End,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("stride"_s, &TestPreset::IncludeOptions::IndexOptions::Stride,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("specificTests"_s,
            &TestPreset::IncludeOptions::IndexOptions::SpecificTests,
            cmCMakePresetsGraphInternal::PresetVectorIntHelper, false));

ReadFileResult TestPresetOptionalFilterIncludeIndexHelper(
  cm::optional<TestPreset::IncludeOptions::IndexOptions>& out,
  const Json::Value* value)
{
  if (!value) {
    out = cm::nullopt;
    return ReadFileResult::READ_OK;
  }

  if (value->isString()) {
    out.emplace();
    out->IndexFile = value->asString();
    return ReadFileResult::READ_OK;
  }

  if (value->isObject()) {
    return TestPresetOptionalFilterIncludeIndexObjectHelper(out, value);
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const TestPresetOptionalFilterIncludeHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::IncludeOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("name"_s, &TestPreset::IncludeOptions::Name,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::IncludeOptions::Label,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("index"_s, &TestPreset::IncludeOptions::Index,
            TestPresetOptionalFilterIncludeIndexHelper, false)
      .Bind("useUnion"_s, &TestPreset::IncludeOptions::UseUnion,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false));

auto const TestPresetOptionalFilterExcludeFixturesHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions::FixturesOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions::FixturesOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("any"_s, &TestPreset::ExcludeOptions::FixturesOptions::Any,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("setup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Setup,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("cleanup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Cleanup,
            cmCMakePresetsGraphInternal::PresetStringHelper, false));

auto const TestPresetOptionalFilterExcludeHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("name"_s, &TestPreset::ExcludeOptions::Name,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::ExcludeOptions::Label,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("fixtures"_s, &TestPreset::ExcludeOptions::Fixtures,
            TestPresetOptionalFilterExcludeFixturesHelper, false));

ReadFileResult TestPresetExecutionShowOnlyHelper(
  TestPreset::ExecutionOptions::ShowOnlyEnum& out, const Json::Value* value)
{
  if (!value || !value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "human") {
    out = TestPreset::ExecutionOptions::ShowOnlyEnum::Human;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "json-v1") {
    out = TestPreset::ExecutionOptions::ShowOnlyEnum::JsonV1;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const TestPresetOptionalExecutionShowOnlyHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::ShowOnlyEnum>(
    ReadFileResult::READ_OK, TestPresetExecutionShowOnlyHelper);

ReadFileResult TestPresetExecutionModeHelper(
  TestPreset::ExecutionOptions::RepeatOptions::ModeEnum& out,
  const Json::Value* value)
{
  if (!value) {
    return ReadFileResult::READ_OK;
  }

  if (!value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "until-fail") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::UntilFail;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "until-pass") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::UntilPass;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "after-timeout") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::AfterTimeout;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const TestPresetOptionalExecutionRepeatHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::RepeatOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions::RepeatOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("mode"_s, &TestPreset::ExecutionOptions::RepeatOptions::Mode,
            TestPresetExecutionModeHelper, true)
      .Bind("count"_s, &TestPreset::ExecutionOptions::RepeatOptions::Count,
            cmCMakePresetsGraphInternal::PresetIntHelper, true));

ReadFileResult TestPresetExecutionNoTestsActionHelper(
  TestPreset::ExecutionOptions::NoTestsActionEnum& out,
  const Json::Value* value)
{
  if (!value) {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return ReadFileResult::READ_OK;
  }

  if (!value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "default") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "error") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Error;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "ignore") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Ignore;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const TestPresetOptionalExecutionNoTestsActionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::NoTestsActionEnum>(
    ReadFileResult::READ_OK, TestPresetExecutionNoTestsActionHelper);

auto const TestPresetExecutionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("stopOnFailure"_s, &TestPreset::ExecutionOptions::StopOnFailure,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("enableFailover"_s, &TestPreset::ExecutionOptions::EnableFailover,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("jobs"_s, &TestPreset::ExecutionOptions::Jobs,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("resourceSpecFile"_s,
            &TestPreset::ExecutionOptions::ResourceSpecFile,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("testLoad"_s, &TestPreset::ExecutionOptions::TestLoad,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("showOnly"_s, &TestPreset::ExecutionOptions::ShowOnly,
            TestPresetOptionalExecutionShowOnlyHelper, false)
      .Bind("repeat"_s, &TestPreset::ExecutionOptions::Repeat,
            TestPresetOptionalExecutionRepeatHelper, false)
      .Bind("interactiveDebugging"_s,
            &TestPreset::ExecutionOptions::InteractiveDebugging,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("scheduleRandom"_s, &TestPreset::ExecutionOptions::ScheduleRandom,
            cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
      .Bind("timeout"_s, &TestPreset::ExecutionOptions::Timeout,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("noTestsAction"_s, &TestPreset::ExecutionOptions::NoTestsAction,
            TestPresetOptionalExecutionNoTestsActionHelper, false));

auto const TestPresetFilterHelper =
  JSONHelperBuilder::Optional<TestPreset::FilterOptions>(
    ReadFileResult::READ_OK,
    JSONHelperBuilder::Object<TestPreset::FilterOptions>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("include"_s, &TestPreset::FilterOptions::Include,
            TestPresetOptionalFilterIncludeHelper, false)
      .Bind("exclude"_s, &TestPreset::FilterOptions::Exclude,
            TestPresetOptionalFilterExcludeHelper, false));

auto const TestPresetHelper =
  JSONHelperBuilder::Object<TestPreset>(ReadFileResult::READ_OK,
                                        ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &TestPreset::Name,
          cmCMakePresetsGraphInternal::PresetStringHelper)
    .Bind("inherits"_s, &TestPreset::Inherits,
          cmCMakePresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("hidden"_s, &TestPreset::Hidden,
          cmCMakePresetsGraphInternal::PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            ReadFileResult::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &TestPreset::DisplayName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &TestPreset::Description,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("environment"_s, &TestPreset::Environment,
          cmCMakePresetsGraphInternal::EnvironmentMapHelper, false)
    .Bind("configurePreset"_s, &TestPreset::ConfigurePreset,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("inheritConfigureEnvironment"_s,
          &TestPreset::InheritConfigureEnvironment,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("configuration"_s, &TestPreset::Configuration,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("overwriteConfigurationFile"_s,
          &TestPreset::OverwriteConfigurationFile,
          cmCMakePresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("output"_s, &TestPreset::Output, TestPresetOptionalOutputHelper,
          false)
    .Bind("filter"_s, &TestPreset::Filter, TestPresetFilterHelper, false)
    .Bind("execution"_s, &TestPreset::Execution, TestPresetExecutionHelper,
          false)
    .Bind("condition"_s, &TestPreset::ConditionEvaluator,
          cmCMakePresetsGraphInternal::PresetConditionHelper, false);
}

namespace cmCMakePresetsGraphInternal {
cmCMakePresetsGraph::ReadFileResult TestPresetsHelper(
  std::vector<cmCMakePresetsGraph::TestPreset>& out, const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Vector<TestPreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS,
    TestPresetHelper);

  return helper(out, value);
}
}
