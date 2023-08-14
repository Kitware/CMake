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

#include "cmCMakePresetErrors.h"
#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"

#include "CTest/cmCTestTypes.h"

class cmJSONState;

namespace {
using TestPreset = cmCMakePresetsGraph::TestPreset;
using JSONHelperBuilder = cmJSONHelperBuilder;

bool TestPresetOutputVerbosityHelper(
  TestPreset::OutputOptions::VerbosityEnum& out, const Json::Value* value,
  cmJSONState* state)
{
  if (!value) {
    out = TestPreset::OutputOptions::VerbosityEnum::Default;
    return true;
  }

  if (!value->isString()) {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "default") {
    out = TestPreset::OutputOptions::VerbosityEnum::Default;
    return true;
  }

  if (value->asString() == "verbose") {
    out = TestPreset::OutputOptions::VerbosityEnum::Verbose;
    return true;
  }

  if (value->asString() == "extra") {
    out = TestPreset::OutputOptions::VerbosityEnum::Extra;
    return true;
  }

  cmCMakePresetErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalOutputVerbosityHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions::VerbosityEnum>(
    TestPresetOutputVerbosityHelper);

bool TestPresetOutputTruncationHelper(
  cm::optional<cmCTestTypes::TruncationMode>& out, const Json::Value* value,
  cmJSONState* state)
{
  if (!value) {
    out = cm::nullopt;
    return true;
  }

  if (!value->isString()) {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "tail") {
    out = cmCTestTypes::TruncationMode::Tail;
    return true;
  }

  if (value->asString() == "middle") {
    out = cmCTestTypes::TruncationMode::Middle;
    return true;
  }

  if (value->asString() == "head") {
    out = cmCTestTypes::TruncationMode::Head;
    return true;
  }

  cmCMakePresetErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalOutputHelper =
  JSONHelperBuilder::Optional<TestPreset::OutputOptions>(
    JSONHelperBuilder::Object<TestPreset::OutputOptions>(
      JsonErrors::INVALID_OBJECT, false)
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
      .Bind("outputJUnitFile"_s, &TestPreset::OutputOptions::OutputJUnitFile,
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
    JSONHelperBuilder::Object<TestPreset::IncludeOptions::IndexOptions>()
      .Bind("start"_s, &TestPreset::IncludeOptions::IndexOptions::Start,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("end"_s, &TestPreset::IncludeOptions::IndexOptions::End,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("stride"_s, &TestPreset::IncludeOptions::IndexOptions::Stride,
            cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
      .Bind("specificTests"_s,
            &TestPreset::IncludeOptions::IndexOptions::SpecificTests,
            cmCMakePresetsGraphInternal::PresetVectorIntHelper, false));

bool TestPresetOptionalFilterIncludeIndexHelper(
  cm::optional<TestPreset::IncludeOptions::IndexOptions>& out,
  const Json::Value* value, cmJSONState* state)
{
  if (!value) {
    out = cm::nullopt;
    return true;
  }

  if (value->isString()) {
    out.emplace();
    out->IndexFile = value->asString();
    return true;
  }

  if (value->isObject()) {
    return TestPresetOptionalFilterIncludeIndexObjectHelper(out, value, state);
  }

  return false;
}

auto const TestPresetOptionalFilterIncludeHelper =
  JSONHelperBuilder::Optional<TestPreset::IncludeOptions>(
    JSONHelperBuilder::Object<TestPreset::IncludeOptions>()
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
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions::FixturesOptions>()
      .Bind("any"_s, &TestPreset::ExcludeOptions::FixturesOptions::Any,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("setup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Setup,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("cleanup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Cleanup,
            cmCMakePresetsGraphInternal::PresetStringHelper, false));

auto const TestPresetOptionalFilterExcludeHelper =
  JSONHelperBuilder::Optional<TestPreset::ExcludeOptions>(
    JSONHelperBuilder::Object<TestPreset::ExcludeOptions>()
      .Bind("name"_s, &TestPreset::ExcludeOptions::Name,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("label"_s, &TestPreset::ExcludeOptions::Label,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
      .Bind("fixtures"_s, &TestPreset::ExcludeOptions::Fixtures,
            TestPresetOptionalFilterExcludeFixturesHelper, false));

bool TestPresetExecutionShowOnlyHelper(
  TestPreset::ExecutionOptions::ShowOnlyEnum& out, const Json::Value* value,
  cmJSONState* state)
{
  if (!value || !value->isString()) {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "human") {
    out = TestPreset::ExecutionOptions::ShowOnlyEnum::Human;
    return true;
  }

  if (value->asString() == "json-v1") {
    out = TestPreset::ExecutionOptions::ShowOnlyEnum::JsonV1;
    return true;
  }

  cmCMakePresetErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionShowOnlyHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::ShowOnlyEnum>(
    TestPresetExecutionShowOnlyHelper);

bool TestPresetExecutionModeHelper(
  TestPreset::ExecutionOptions::RepeatOptions::ModeEnum& out,
  const Json::Value* value, cmJSONState* state)
{
  if (!value) {
    return true;
  }

  if (!value->isString()) {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "until-fail") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::UntilFail;
    return true;
  }

  if (value->asString() == "until-pass") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::UntilPass;
    return true;
  }

  if (value->asString() == "after-timeout") {
    out = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum::AfterTimeout;
    return true;
  }

  cmCMakePresetErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionRepeatHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::RepeatOptions>(
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions::RepeatOptions>()
      .Bind("mode"_s, &TestPreset::ExecutionOptions::RepeatOptions::Mode,
            TestPresetExecutionModeHelper, true)
      .Bind("count"_s, &TestPreset::ExecutionOptions::RepeatOptions::Count,
            cmCMakePresetsGraphInternal::PresetIntHelper, true));

bool TestPresetExecutionNoTestsActionHelper(
  TestPreset::ExecutionOptions::NoTestsActionEnum& out,
  const Json::Value* value, cmJSONState* state)
{
  if (!value) {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return true;
  }

  if (!value->isString()) {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "default") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Default;
    return true;
  }

  if (value->asString() == "error") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Error;
    return true;
  }

  if (value->asString() == "ignore") {
    out = TestPreset::ExecutionOptions::NoTestsActionEnum::Ignore;
    return true;
  }

  cmCMakePresetErrors::INVALID_PRESET(value, state);
  return false;
}

auto const TestPresetOptionalExecutionNoTestsActionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions::NoTestsActionEnum>(
    TestPresetExecutionNoTestsActionHelper);

auto const TestPresetExecutionHelper =
  JSONHelperBuilder::Optional<TestPreset::ExecutionOptions>(
    JSONHelperBuilder::Object<TestPreset::ExecutionOptions>()
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
    JSONHelperBuilder::Object<TestPreset::FilterOptions>()
      .Bind("include"_s, &TestPreset::FilterOptions::Include,
            TestPresetOptionalFilterIncludeHelper, false)
      .Bind("exclude"_s, &TestPreset::FilterOptions::Exclude,
            TestPresetOptionalFilterExcludeHelper, false));

auto const TestPresetHelper =
  JSONHelperBuilder::Object<TestPreset>(
    cmCMakePresetErrors::INVALID_PRESET_OBJECT, false)
    .Bind("name"_s, &TestPreset::Name,
          cmCMakePresetsGraphInternal::PresetNameHelper)
    .Bind("inherits"_s, &TestPreset::Inherits,
          cmCMakePresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("hidden"_s, &TestPreset::Hidden,
          cmCMakePresetsGraphInternal::PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            cmCMakePresetErrors::INVALID_PRESET),
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
bool TestPresetsHelper(std::vector<cmCMakePresetsGraph::TestPreset>& out,
                       const Json::Value* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<TestPreset>(
    cmCMakePresetErrors::INVALID_PRESETS, TestPresetHelper);

  return helper(out, value, state);
}
}
