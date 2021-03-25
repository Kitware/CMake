/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePresetsFile.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"

#include "cmJSONHelpers.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

#define CHECK_OK(expr)                                                        \
  {                                                                           \
    auto _result = expr;                                                      \
    if (_result != ReadFileResult::READ_OK)                                   \
      return _result;                                                         \
  }

#define CHECK_EXPAND(out, field, expanders)                                   \
  {                                                                           \
    switch (ExpandMacros(field, expanders)) {                                 \
      case ExpandMacroResult::Error:                                          \
        return false;                                                         \
      case ExpandMacroResult::Ignore:                                         \
        out.reset();                                                          \
        return true;                                                          \
      case ExpandMacroResult::Ok:                                             \
        break;                                                                \
    }                                                                         \
  }

namespace {
enum class CycleStatus
{
  Unvisited,
  InProgress,
  Verified,
};

using ReadFileResult = cmCMakePresetsFile::ReadFileResult;
using CacheVariable = cmCMakePresetsFile::CacheVariable;
using ConfigurePreset = cmCMakePresetsFile::ConfigurePreset;
using BuildPreset = cmCMakePresetsFile::BuildPreset;
using TestPreset = cmCMakePresetsFile::TestPreset;
using ArchToolsetStrategy = cmCMakePresetsFile::ArchToolsetStrategy;

constexpr int MIN_VERSION = 1;
constexpr int MAX_VERSION = 2;

struct CMakeVersion
{
  unsigned int Major = 0;
  unsigned int Minor = 0;
  unsigned int Patch = 0;
};

struct RootPresets
{
  CMakeVersion CMakeMinimumRequired;
  std::vector<cmCMakePresetsFile::ConfigurePreset> ConfigurePresets;
  std::vector<cmCMakePresetsFile::BuildPreset> BuildPresets;
  std::vector<cmCMakePresetsFile::TestPreset> TestPresets;
};

cmJSONHelper<std::nullptr_t, ReadFileResult> VendorHelper(ReadFileResult error)
{
  return [error](std::nullptr_t& /*out*/,
                 const Json::Value* value) -> ReadFileResult {
    if (!value) {
      return ReadFileResult::READ_OK;
    }

    if (!value->isObject()) {
      return error;
    }

    return ReadFileResult::READ_OK;
  };
}

auto const VersionIntHelper = cmJSONIntHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_VERSION);

auto const VersionHelper = cmJSONRequiredHelper<int, ReadFileResult>(
  ReadFileResult::NO_VERSION, VersionIntHelper);

auto const RootVersionHelper =
  cmJSONObjectHelper<int, ReadFileResult>(ReadFileResult::READ_OK,
                                          ReadFileResult::INVALID_ROOT)
    .Bind("version"_s, VersionHelper, false);

auto const VariableStringHelper = cmJSONStringHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_VARIABLE);

ReadFileResult VariableValueHelper(std::string& out, const Json::Value* value)
{
  if (!value) {
    out.clear();
    return ReadFileResult::READ_OK;
  }

  if (value->isBool()) {
    out = value->asBool() ? "TRUE" : "FALSE";
    return ReadFileResult::READ_OK;
  }

  return VariableStringHelper(out, value);
}

auto const VariableObjectHelper =
  cmJSONObjectHelper<CacheVariable, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_VARIABLE, false)
    .Bind("type"_s, &CacheVariable::Type, VariableStringHelper, false)
    .Bind("value"_s, &CacheVariable::Value, VariableValueHelper);

ReadFileResult VariableHelper(cm::optional<CacheVariable>& out,
                              const Json::Value* value)
{
  if (value->isBool()) {
    out = CacheVariable{
      /*Type=*/"BOOL",
      /*Value=*/value->asBool() ? "TRUE" : "FALSE",
    };
    return ReadFileResult::READ_OK;
  }
  if (value->isString()) {
    out = CacheVariable{
      /*Type=*/"",
      /*Value=*/value->asString(),
    };
    return ReadFileResult::READ_OK;
  }
  if (value->isObject()) {
    out.emplace();
    return VariableObjectHelper(*out, value);
  }
  if (value->isNull()) {
    out = cm::nullopt;
    return ReadFileResult::READ_OK;
  }
  return ReadFileResult::INVALID_VARIABLE;
}

auto const VariablesHelper =
  cmJSONMapHelper<cm::optional<CacheVariable>, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, VariableHelper);

auto const PresetStringHelper = cmJSONStringHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET);

ReadFileResult EnvironmentHelper(cm::optional<std::string>& out,
                                 const Json::Value* value)
{
  if (!value || value->isNull()) {
    out = cm::nullopt;
    return ReadFileResult::READ_OK;
  }
  if (value->isString()) {
    out = value->asString();
    return ReadFileResult::READ_OK;
  }
  return ReadFileResult::INVALID_PRESET;
}

auto const EnvironmentMapHelper =
  cmJSONMapHelper<cm::optional<std::string>, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET,
    EnvironmentHelper);

auto const PresetVectorStringHelper =
  cmJSONVectorHelper<std::string, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET,
    PresetStringHelper);

ReadFileResult PresetInheritsHelper(std::vector<std::string>& out,
                                    const Json::Value* value)
{
  out.clear();
  if (!value) {
    return ReadFileResult::READ_OK;
  }

  if (value->isString()) {
    out.push_back(value->asString());
    return ReadFileResult::READ_OK;
  }

  return PresetVectorStringHelper(out, value);
}

auto const PresetBoolHelper = cmJSONBoolHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET);

auto const PresetOptionalBoolHelper =
  cmJSONOptionalHelper<bool, ReadFileResult>(ReadFileResult::READ_OK,
                                             PresetBoolHelper);

auto const PresetIntHelper = cmJSONIntHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET);

auto const PresetOptionalIntHelper = cmJSONOptionalHelper<int, ReadFileResult>(
  ReadFileResult::READ_OK, PresetIntHelper);

auto const PresetVectorIntHelper = cmJSONVectorHelper<int, ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, PresetIntHelper);

auto const PresetWarningsHelper =
  cmJSONObjectHelper<ConfigurePreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("dev"_s, &ConfigurePreset::WarnDev, PresetOptionalBoolHelper, false)
    .Bind("deprecated"_s, &ConfigurePreset::WarnDeprecated,
          PresetOptionalBoolHelper, false)
    .Bind("uninitialized"_s, &ConfigurePreset::WarnUninitialized,
          PresetOptionalBoolHelper, false)
    .Bind("unusedCli"_s, &ConfigurePreset::WarnUnusedCli,
          PresetOptionalBoolHelper, false)
    .Bind("systemVars"_s, &ConfigurePreset::WarnSystemVars,
          PresetOptionalBoolHelper, false);

auto const PresetErrorsHelper =
  cmJSONObjectHelper<ConfigurePreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("dev"_s, &ConfigurePreset::ErrorDev, PresetOptionalBoolHelper, false)
    .Bind("deprecated"_s, &ConfigurePreset::ErrorDeprecated,
          PresetOptionalBoolHelper, false);

auto const PresetDebugHelper =
  cmJSONObjectHelper<ConfigurePreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("output"_s, &ConfigurePreset::DebugOutput, PresetOptionalBoolHelper,
          false)
    .Bind("tryCompile"_s, &ConfigurePreset::DebugTryCompile,
          PresetOptionalBoolHelper, false)
    .Bind("find"_s, &ConfigurePreset::DebugFind, PresetOptionalBoolHelper,
          false);

ReadFileResult ArchToolsetStrategyHelper(
  cm::optional<ArchToolsetStrategy>& out, const Json::Value* value)
{
  if (!value) {
    out = cm::nullopt;
    return ReadFileResult::READ_OK;
  }

  if (!value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "set") {
    out = ArchToolsetStrategy::Set;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "external") {
    out = ArchToolsetStrategy::External;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

std::function<ReadFileResult(ConfigurePreset&, const Json::Value*)>
ArchToolsetHelper(
  std::string ConfigurePreset::*valueField,
  cm::optional<ArchToolsetStrategy> ConfigurePreset::*strategyField)
{
  auto const objectHelper =
    cmJSONObjectHelper<ConfigurePreset, ReadFileResult>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
      .Bind("value", valueField, PresetStringHelper, false)
      .Bind("strategy", strategyField, ArchToolsetStrategyHelper, false);
  return [valueField, strategyField, objectHelper](
           ConfigurePreset& out, const Json::Value* value) -> ReadFileResult {
    if (!value) {
      (out.*valueField).clear();
      out.*strategyField = cm::nullopt;
      return ReadFileResult::READ_OK;
    }

    if (value->isString()) {
      out.*valueField = value->asString();
      out.*strategyField = cm::nullopt;
      return ReadFileResult::READ_OK;
    }

    if (value->isObject()) {
      return objectHelper(out, value);
    }

    return ReadFileResult::INVALID_PRESET;
  };
}

auto const ArchitectureHelper = ArchToolsetHelper(
  &ConfigurePreset::Architecture, &ConfigurePreset::ArchitectureStrategy);
auto const ToolsetHelper = ArchToolsetHelper(
  &ConfigurePreset::Toolset, &ConfigurePreset::ToolsetStrategy);

auto const ConfigurePresetHelper =
  cmJSONObjectHelper<ConfigurePreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &ConfigurePreset::Name, PresetStringHelper)
    .Bind("inherits"_s, &ConfigurePreset::Inherits, PresetInheritsHelper,
          false)
    .Bind("hidden"_s, &ConfigurePreset::Hidden, PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          VendorHelper(ReadFileResult::INVALID_PRESET), false)
    .Bind("displayName"_s, &ConfigurePreset::DisplayName, PresetStringHelper,
          false)
    .Bind("description"_s, &ConfigurePreset::Description, PresetStringHelper,
          false)
    .Bind("generator"_s, &ConfigurePreset::Generator, PresetStringHelper,
          false)
    .Bind("architecture"_s, ArchitectureHelper, false)
    .Bind("toolset"_s, ToolsetHelper, false)
    .Bind("binaryDir"_s, &ConfigurePreset::BinaryDir, PresetStringHelper,
          false)
    .Bind<std::string>("cmakeExecutable"_s, nullptr, PresetStringHelper, false)
    .Bind("cacheVariables"_s, &ConfigurePreset::CacheVariables,
          VariablesHelper, false)
    .Bind("environment"_s, &ConfigurePreset::Environment, EnvironmentMapHelper,
          false)
    .Bind("warnings"_s, PresetWarningsHelper, false)
    .Bind("errors"_s, PresetErrorsHelper, false)
    .Bind("debug"_s, PresetDebugHelper, false);

auto const BuildPresetHelper =
  cmJSONObjectHelper<BuildPreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &BuildPreset::Name, PresetStringHelper)
    .Bind("inherits"_s, &BuildPreset::Inherits, PresetInheritsHelper, false)
    .Bind("hidden"_s, &BuildPreset::Hidden, PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          VendorHelper(ReadFileResult::INVALID_PRESET), false)
    .Bind("displayName"_s, &BuildPreset::DisplayName, PresetStringHelper,
          false)
    .Bind("description"_s, &BuildPreset::Description, PresetStringHelper,
          false)
    .Bind("environment"_s, &BuildPreset::Environment, EnvironmentMapHelper,
          false)
    .Bind("configurePreset"_s, &BuildPreset::ConfigurePreset,
          PresetStringHelper, false)
    .Bind("inheritConfigureEnvironment"_s,
          &BuildPreset::InheritConfigureEnvironment, PresetOptionalBoolHelper,
          false)
    .Bind("jobs"_s, &BuildPreset::Jobs, PresetOptionalIntHelper, false)
    .Bind("targets"_s, &BuildPreset::Targets, PresetVectorStringHelper, false)
    .Bind("configuration"_s, &BuildPreset::Configuration, PresetStringHelper,
          false)
    .Bind("cleanFirst"_s, &BuildPreset::CleanFirst, PresetOptionalBoolHelper,
          false)
    .Bind("verbose"_s, &BuildPreset::Verbose, PresetOptionalBoolHelper, false)
    .Bind("nativeToolOptions"_s, &BuildPreset::NativeToolOptions,
          PresetVectorStringHelper, false);

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
  cmJSONOptionalHelper<TestPreset::OutputOptions::VerbosityEnum,
                       ReadFileResult>(ReadFileResult::READ_OK,
                                       TestPresetOutputVerbosityHelper);

auto const TestPresetOptionalOutputHelper =
  cmJSONOptionalHelper<TestPreset::OutputOptions, ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::OutputOptions, ReadFileResult>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
      .Bind("shortProgress"_s, &TestPreset::OutputOptions::ShortProgress,
            PresetOptionalBoolHelper, false)
      .Bind("verbosity"_s, &TestPreset::OutputOptions::Verbosity,
            TestPresetOptionalOutputVerbosityHelper, false)
      .Bind("debug"_s, &TestPreset::OutputOptions::Debug,
            PresetOptionalBoolHelper, false)
      .Bind("outputOnFailure"_s, &TestPreset::OutputOptions::OutputOnFailure,
            PresetOptionalBoolHelper, false)
      .Bind("quiet"_s, &TestPreset::OutputOptions::Quiet,
            PresetOptionalBoolHelper, false)
      .Bind("outputLogFile"_s, &TestPreset::OutputOptions::OutputLogFile,
            PresetStringHelper, false)
      .Bind("labelSummary"_s, &TestPreset::OutputOptions::LabelSummary,
            PresetOptionalBoolHelper, false)
      .Bind("subprojectSummary"_s,
            &TestPreset::OutputOptions::SubprojectSummary,
            PresetOptionalBoolHelper, false)
      .Bind("maxPassedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxPassedTestOutputSize,
            PresetOptionalIntHelper, false)
      .Bind("maxFailedTestOutputSize"_s,
            &TestPreset::OutputOptions::MaxFailedTestOutputSize,
            PresetOptionalIntHelper, false)
      .Bind("maxTestNameWidth"_s, &TestPreset::OutputOptions::MaxTestNameWidth,
            PresetOptionalIntHelper, false));

auto const TestPresetOptionalFilterIncludeIndexObjectHelper =
  cmJSONOptionalHelper<TestPreset::IncludeOptions::IndexOptions,
                       ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::IncludeOptions::IndexOptions,
                       ReadFileResult>(ReadFileResult::READ_OK,
                                       ReadFileResult::INVALID_PRESET)
      .Bind("start"_s, &TestPreset::IncludeOptions::IndexOptions::Start,
            PresetOptionalIntHelper, false)
      .Bind("end"_s, &TestPreset::IncludeOptions::IndexOptions::End,
            PresetOptionalIntHelper, false)
      .Bind("stride"_s, &TestPreset::IncludeOptions::IndexOptions::Stride,
            PresetOptionalIntHelper, false)
      .Bind("specificTests"_s,
            &TestPreset::IncludeOptions::IndexOptions::SpecificTests,
            PresetVectorIntHelper, false));

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
  cmJSONOptionalHelper<TestPreset::IncludeOptions, ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::IncludeOptions, ReadFileResult>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("name"_s, &TestPreset::IncludeOptions::Name, PresetStringHelper,
            false)
      .Bind("label"_s, &TestPreset::IncludeOptions::Label, PresetStringHelper,
            false)
      .Bind("index"_s, &TestPreset::IncludeOptions::Index,
            TestPresetOptionalFilterIncludeIndexHelper, false)
      .Bind("useUnion"_s, &TestPreset::IncludeOptions::UseUnion,
            PresetOptionalBoolHelper, false));

auto const TestPresetOptionalFilterExcludeFixturesHelper =
  cmJSONOptionalHelper<TestPreset::ExcludeOptions::FixturesOptions,
                       ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::ExcludeOptions::FixturesOptions,
                       ReadFileResult>(ReadFileResult::READ_OK,
                                       ReadFileResult::INVALID_PRESET)
      .Bind("any"_s, &TestPreset::ExcludeOptions::FixturesOptions::Any,
            PresetStringHelper, false)
      .Bind("setup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Setup,
            PresetStringHelper, false)
      .Bind("cleanup"_s, &TestPreset::ExcludeOptions::FixturesOptions::Cleanup,
            PresetStringHelper, false));

auto const TestPresetOptionalFilterExcludeHelper =
  cmJSONOptionalHelper<TestPreset::ExcludeOptions, ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::ExcludeOptions, ReadFileResult>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("name"_s, &TestPreset::ExcludeOptions::Name, PresetStringHelper,
            false)
      .Bind("label"_s, &TestPreset::ExcludeOptions::Label, PresetStringHelper,
            false)
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
  cmJSONOptionalHelper<TestPreset::ExecutionOptions::ShowOnlyEnum,
                       ReadFileResult>(ReadFileResult::READ_OK,
                                       TestPresetExecutionShowOnlyHelper);

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
  cmJSONOptionalHelper<TestPreset::ExecutionOptions::RepeatOptions,
                       ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::ExecutionOptions::RepeatOptions,
                       ReadFileResult>(ReadFileResult::READ_OK,
                                       ReadFileResult::INVALID_PRESET)
      .Bind("mode"_s, &TestPreset::ExecutionOptions::RepeatOptions::Mode,
            TestPresetExecutionModeHelper, true)
      .Bind("count"_s, &TestPreset::ExecutionOptions::RepeatOptions::Count,
            PresetIntHelper, true));

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
  cmJSONOptionalHelper<TestPreset::ExecutionOptions::NoTestsActionEnum,
                       ReadFileResult>(ReadFileResult::READ_OK,
                                       TestPresetExecutionNoTestsActionHelper);

auto const TestPresetExecutionHelper =
  cmJSONOptionalHelper<TestPreset::ExecutionOptions, ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::ExecutionOptions, ReadFileResult>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("stopOnFailure"_s, &TestPreset::ExecutionOptions::StopOnFailure,
            PresetOptionalBoolHelper, false)
      .Bind("enableFailover"_s, &TestPreset::ExecutionOptions::EnableFailover,
            PresetOptionalBoolHelper, false)
      .Bind("jobs"_s, &TestPreset::ExecutionOptions::Jobs,
            PresetOptionalIntHelper, false)
      .Bind("resourceSpecFile"_s,
            &TestPreset::ExecutionOptions::ResourceSpecFile,
            PresetStringHelper, false)
      .Bind("testLoad"_s, &TestPreset::ExecutionOptions::TestLoad,
            PresetOptionalIntHelper, false)
      .Bind("showOnly"_s, &TestPreset::ExecutionOptions::ShowOnly,
            TestPresetOptionalExecutionShowOnlyHelper, false)
      .Bind("repeat"_s, &TestPreset::ExecutionOptions::Repeat,
            TestPresetOptionalExecutionRepeatHelper, false)
      .Bind("interactiveDebugging"_s,
            &TestPreset::ExecutionOptions::InteractiveDebugging,
            PresetOptionalBoolHelper, false)
      .Bind("scheduleRandom"_s, &TestPreset::ExecutionOptions::ScheduleRandom,
            PresetOptionalBoolHelper, false)
      .Bind("timeout"_s, &TestPreset::ExecutionOptions::Timeout,
            PresetOptionalIntHelper, false)
      .Bind("noTestsAction"_s, &TestPreset::ExecutionOptions::NoTestsAction,
            TestPresetOptionalExecutionNoTestsActionHelper, false));

auto const TestPresetFilterHelper =
  cmJSONOptionalHelper<TestPreset::FilterOptions, ReadFileResult>(
    ReadFileResult::READ_OK,
    cmJSONObjectHelper<TestPreset::FilterOptions, ReadFileResult>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET)
      .Bind("include"_s, &TestPreset::FilterOptions::Include,
            TestPresetOptionalFilterIncludeHelper, false)
      .Bind("exclude"_s, &TestPreset::FilterOptions::Exclude,
            TestPresetOptionalFilterExcludeHelper, false));

auto const TestPresetHelper =
  cmJSONObjectHelper<TestPreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &TestPreset::Name, PresetStringHelper)
    .Bind("inherits"_s, &TestPreset::Inherits, PresetInheritsHelper, false)
    .Bind("hidden"_s, &TestPreset::Hidden, PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          VendorHelper(ReadFileResult::INVALID_PRESET), false)
    .Bind("displayName"_s, &TestPreset::DisplayName, PresetStringHelper, false)
    .Bind("description"_s, &TestPreset::Description, PresetStringHelper, false)
    .Bind("environment"_s, &TestPreset::Environment, EnvironmentMapHelper,
          false)
    .Bind("configurePreset"_s, &TestPreset::ConfigurePreset,
          PresetStringHelper, false)
    .Bind("inheritConfigureEnvironment"_s,
          &TestPreset::InheritConfigureEnvironment, PresetOptionalBoolHelper,
          false)
    .Bind("configuration"_s, &TestPreset::Configuration, PresetStringHelper,
          false)
    .Bind("overwriteConfigurationFile"_s,
          &TestPreset::OverwriteConfigurationFile, PresetVectorStringHelper,
          false)
    .Bind("output"_s, &TestPreset::Output, TestPresetOptionalOutputHelper,
          false)
    .Bind("filter"_s, &TestPreset::Filter, TestPresetFilterHelper, false)
    .Bind("execution"_s, &TestPreset::Execution, TestPresetExecutionHelper,
          false);

auto const ConfigurePresetsHelper =
  cmJSONVectorHelper<ConfigurePreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS,
    ConfigurePresetHelper);

auto const BuildPresetsHelper =
  cmJSONVectorHelper<BuildPreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS,
    BuildPresetHelper);

auto const TestPresetsHelper = cmJSONVectorHelper<TestPreset, ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS, TestPresetHelper);

auto const CMakeVersionUIntHelper = cmJSONUIntHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_VERSION);

auto const CMakeVersionHelper =
  cmJSONObjectHelper<CMakeVersion, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CMAKE_VERSION, false)
    .Bind("major"_s, &CMakeVersion::Major, CMakeVersionUIntHelper, false)
    .Bind("minor"_s, &CMakeVersion::Minor, CMakeVersionUIntHelper, false)
    .Bind("patch"_s, &CMakeVersion::Patch, CMakeVersionUIntHelper, false);

auto const RootPresetsHelper =
  cmJSONObjectHelper<RootPresets, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_ROOT, false)
    .Bind<int>("version"_s, nullptr, VersionHelper)
    .Bind("configurePresets"_s, &RootPresets::ConfigurePresets,
          ConfigurePresetsHelper, false)
    .Bind("buildPresets"_s, &RootPresets::BuildPresets, BuildPresetsHelper,
          false)
    .Bind("testPresets"_s, &RootPresets::TestPresets, TestPresetsHelper, false)
    .Bind("cmakeMinimumRequired"_s, &RootPresets::CMakeMinimumRequired,
          CMakeVersionHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          VendorHelper(ReadFileResult::INVALID_ROOT), false);

void InheritString(std::string& child, const std::string& parent)
{
  if (child.empty()) {
    child = parent;
  }
}

template <typename T>
void InheritOptionalValue(cm::optional<T>& child,
                          const cm::optional<T>& parent)
{
  if (!child) {
    child = parent;
  }
}

template <typename T>
void InheritVector(std::vector<T>& child, const std::vector<T>& parent)
{
  if (child.empty()) {
    child = parent;
  }
}

/**
 * Check preset inheritance for cycles (using a DAG check algorithm) while
 * also bubbling up fields through the inheritance hierarchy, then verify
 * that each preset has the required fields, either directly or through
 * inheritance.
 */
template <class T>
ReadFileResult VisitPreset(
  T& preset, std::map<std::string, cmCMakePresetsFile::PresetPair<T>>& presets,
  std::map<std::string, CycleStatus> cycleStatus)
{
  switch (cycleStatus[preset.Name]) {
    case CycleStatus::InProgress:
      return ReadFileResult::CYCLIC_PRESET_INHERITANCE;
    case CycleStatus::Verified:
      return ReadFileResult::READ_OK;
    default:
      break;
  }

  cycleStatus[preset.Name] = CycleStatus::InProgress;

  if (preset.Environment.count("") != 0) {
    return ReadFileResult::INVALID_PRESET;
  }

  CHECK_OK(preset.VisitPresetBeforeInherit())

  for (auto const& i : preset.Inherits) {
    auto parent = presets.find(i);
    if (parent == presets.end()) {
      return ReadFileResult::INVALID_PRESET;
    }

    auto& parentPreset = parent->second.Unexpanded;
    if (!preset.User && parentPreset.User) {
      return ReadFileResult::USER_PRESET_INHERITANCE;
    }

    auto result = VisitPreset(parentPreset, presets, cycleStatus);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }

    CHECK_OK(preset.VisitPresetInherit(parentPreset))

    for (auto const& v : parentPreset.Environment) {
      preset.Environment.insert(v);
    }
  }

  CHECK_OK(preset.VisitPresetAfterInherit())

  cycleStatus[preset.Name] = CycleStatus::Verified;
  return ReadFileResult::READ_OK;
}

template <class T>
ReadFileResult ComputePresetInheritance(
  std::map<std::string, cmCMakePresetsFile::PresetPair<T>>& presets)
{
  std::map<std::string, CycleStatus> cycleStatus;
  for (auto const& it : presets) {
    cycleStatus[it.first] = CycleStatus::Unvisited;
  }

  for (auto& it : presets) {
    auto result = VisitPreset<T>(it.second.Unexpanded, presets, cycleStatus);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }
  }

  return ReadFileResult::READ_OK;
}

constexpr const char* ValidPrefixes[] = {
  "",
  "env",
  "penv",
  "vendor",
};

bool PrefixesValidMacroNamespace(const std::string& str)
{
  return std::any_of(
    std::begin(ValidPrefixes), std::end(ValidPrefixes),
    [&str](const char* prefix) -> bool { return cmHasPrefix(prefix, str); });
}

bool IsValidMacroNamespace(const std::string& str)
{
  return std::any_of(
    std::begin(ValidPrefixes), std::end(ValidPrefixes),
    [&str](const char* prefix) -> bool { return str == prefix; });
}

enum class ExpandMacroResult
{
  Ok,
  Ignore,
  Error,
};

using MacroExpander = std::function<ExpandMacroResult(
  const std::string&, const std::string&, std::string&)>;

ExpandMacroResult VisitEnv(std::string& value, CycleStatus& status,
                           const std::vector<MacroExpander>& macroExpanders);
ExpandMacroResult ExpandMacros(
  std::string& out, const std::vector<MacroExpander>& macroExpanders);
ExpandMacroResult ExpandMacro(
  std::string& out, const std::string& macroNamespace,
  const std::string& macroName,
  const std::vector<MacroExpander>& macroExpanders);

bool ExpandMacros(const cmCMakePresetsFile& file,
                  const ConfigurePreset& preset,
                  cm::optional<ConfigurePreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  std::string binaryDir = preset.BinaryDir;
  CHECK_EXPAND(out, binaryDir, macroExpanders)

  if (!cmSystemTools::FileIsFullPath(binaryDir)) {
    binaryDir = cmStrCat(file.SourceDir, '/', binaryDir);
  }
  out->BinaryDir = cmSystemTools::CollapseFullPath(binaryDir);
  cmSystemTools::ConvertToUnixSlashes(out->BinaryDir);

  for (auto& variable : out->CacheVariables) {
    if (variable.second) {
      CHECK_EXPAND(out, variable.second->Value, macroExpanders)
    }
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsFile&, const BuildPreset&,
                  cm::optional<BuildPreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& target : out->Targets) {
    CHECK_EXPAND(out, target, macroExpanders)
  }

  for (auto& nativeToolOption : out->NativeToolOptions) {
    CHECK_EXPAND(out, nativeToolOption, macroExpanders)
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsFile&, const TestPreset&,
                  cm::optional<TestPreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& overwrite : out->OverwriteConfigurationFile) {
    CHECK_EXPAND(out, overwrite, macroExpanders);
  }

  if (out->Output) {
    CHECK_EXPAND(out, out->Output->OutputLogFile, macroExpanders)
  }

  if (out->Filter) {
    if (out->Filter->Include) {
      CHECK_EXPAND(out, out->Filter->Include->Name, macroExpanders)
      CHECK_EXPAND(out, out->Filter->Include->Label, macroExpanders)

      if (out->Filter->Include->Index) {
        CHECK_EXPAND(out, out->Filter->Include->Index->IndexFile,
                     macroExpanders);
      }
    }

    if (out->Filter->Exclude) {
      CHECK_EXPAND(out, out->Filter->Exclude->Name, macroExpanders)
      CHECK_EXPAND(out, out->Filter->Exclude->Label, macroExpanders)

      if (out->Filter->Exclude->Fixtures) {
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Any, macroExpanders)
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Setup,
                     macroExpanders)
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Cleanup,
                     macroExpanders)
      }
    }
  }

  if (out->Execution) {
    CHECK_EXPAND(out, out->Execution->ResourceSpecFile, macroExpanders)
  }

  return true;
}

template <class T>
bool ExpandMacros(const cmCMakePresetsFile& file, const T& preset,
                  cm::optional<T>& out)
{
  out.emplace(preset);

  std::map<std::string, CycleStatus> envCycles;
  for (auto const& v : out->Environment) {
    envCycles[v.first] = CycleStatus::Unvisited;
  }

  std::vector<MacroExpander> macroExpanders;

  MacroExpander defaultMacroExpander =
    [&file, &preset](const std::string& macroNamespace,
                     const std::string& macroName,
                     std::string& macroOut) -> ExpandMacroResult {
    if (macroNamespace.empty()) {
      if (macroName == "sourceDir") {
        macroOut += file.SourceDir;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "sourceParentDir") {
        macroOut += cmSystemTools::GetParentDirectory(file.SourceDir);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "sourceDirName") {
        macroOut += cmSystemTools::GetFilenameName(file.SourceDir);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "presetName") {
        macroOut += preset.Name;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "generator") {
        // Generator only makes sense if preset is not hidden.
        if (!preset.Hidden) {
          macroOut += file.GetGeneratorForPreset(preset.Name);
        }
        return ExpandMacroResult::Ok;
      }
      if (macroName == "dollar") {
        macroOut += '$';
        return ExpandMacroResult::Ok;
      }
    }

    return ExpandMacroResult::Ignore;
  };

  MacroExpander environmentMacroExpander =
    [&macroExpanders, &out, &envCycles](
      const std::string& macroNamespace, const std::string& macroName,
      std::string& result) -> ExpandMacroResult {
    if (macroNamespace == "env" && !macroName.empty() && out) {
      auto v = out->Environment.find(macroName);
      if (v != out->Environment.end() && v->second) {
        auto e = VisitEnv(*v->second, envCycles[macroName], macroExpanders);
        if (e != ExpandMacroResult::Ok) {
          return e;
        }
        result += *v->second;
        return ExpandMacroResult::Ok;
      }
    }

    if (macroNamespace == "env" || macroNamespace == "penv") {
      if (macroName.empty()) {
        return ExpandMacroResult::Error;
      }
      const char* value = std::getenv(macroName.c_str());
      if (value) {
        result += value;
      }
      return ExpandMacroResult::Ok;
    }

    return ExpandMacroResult::Ignore;
  };

  macroExpanders.push_back(defaultMacroExpander);
  macroExpanders.push_back(environmentMacroExpander);

  for (auto& v : out->Environment) {
    if (v.second) {
      switch (VisitEnv(*v.second, envCycles[v.first], macroExpanders)) {
        case ExpandMacroResult::Error:
          return false;
        case ExpandMacroResult::Ignore:
          out.reset();
          return true;
        case ExpandMacroResult::Ok:
          break;
      }
    }
  }

  return ExpandMacros(file, preset, out, macroExpanders);
}

ExpandMacroResult VisitEnv(std::string& value, CycleStatus& status,
                           const std::vector<MacroExpander>& macroExpanders)
{
  if (status == CycleStatus::Verified) {
    return ExpandMacroResult::Ok;
  }
  if (status == CycleStatus::InProgress) {
    return ExpandMacroResult::Error;
  }

  status = CycleStatus::InProgress;
  auto e = ExpandMacros(value, macroExpanders);
  if (e != ExpandMacroResult::Ok) {
    return e;
  }
  status = CycleStatus::Verified;
  return ExpandMacroResult::Ok;
}

ExpandMacroResult ExpandMacros(
  std::string& out, const std::vector<MacroExpander>& macroExpanders)
{
  std::string result;
  std::string macroNamespace;
  std::string macroName;

  enum class State
  {
    Default,
    MacroNamespace,
    MacroName,
  } state = State::Default;

  for (auto c : out) {
    switch (state) {
      case State::Default:
        if (c == '$') {
          state = State::MacroNamespace;
        } else {
          result += c;
        }
        break;

      case State::MacroNamespace:
        if (c == '{') {
          if (IsValidMacroNamespace(macroNamespace)) {
            state = State::MacroName;
          } else {
            result += '$';
            result += macroNamespace;
            result += '{';
            macroNamespace.clear();
            state = State::Default;
          }
        } else {
          macroNamespace += c;
          if (!PrefixesValidMacroNamespace(macroNamespace)) {
            result += '$';
            result += macroNamespace;
            macroNamespace.clear();
            state = State::Default;
          }
        }
        break;

      case State::MacroName:
        if (c == '}') {
          auto e =
            ExpandMacro(result, macroNamespace, macroName, macroExpanders);
          if (e != ExpandMacroResult::Ok) {
            return e;
          }
          macroNamespace.clear();
          macroName.clear();
          state = State::Default;
        } else {
          macroName += c;
        }
        break;
    }
  }

  switch (state) {
    case State::Default:
      break;
    case State::MacroNamespace:
      result += '$';
      result += macroNamespace;
      break;
    case State::MacroName:
      return ExpandMacroResult::Error;
  }

  out = std::move(result);
  return ExpandMacroResult::Ok;
}

ExpandMacroResult ExpandMacro(std::string& out,
                              const std::string& macroNamespace,
                              const std::string& macroName,
                              const std::vector<MacroExpander>& macroExpanders)
{
  for (auto const& macroExpander : macroExpanders) {
    auto result = macroExpander(macroNamespace, macroName, out);
    if (result != ExpandMacroResult::Ignore) {
      return result;
    }
  }

  if (macroNamespace == "vendor") {
    return ExpandMacroResult::Ignore;
  }

  return ExpandMacroResult::Error;
}
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ConfigurePreset::VisitPresetInherit(
  const cmCMakePresetsFile::Preset& parentPreset)
{
  auto& preset = *this;
  const ConfigurePreset& parent =
    static_cast<const ConfigurePreset&>(parentPreset);
  InheritString(preset.Generator, parent.Generator);
  InheritString(preset.Architecture, parent.Architecture);
  InheritString(preset.Toolset, parent.Toolset);
  if (!preset.ArchitectureStrategy) {
    preset.ArchitectureStrategy = parent.ArchitectureStrategy;
  }
  if (!preset.ToolsetStrategy) {
    preset.ToolsetStrategy = parent.ToolsetStrategy;
  }
  InheritString(preset.BinaryDir, parent.BinaryDir);
  InheritOptionalValue(preset.WarnDev, parent.WarnDev);
  InheritOptionalValue(preset.ErrorDev, parent.ErrorDev);
  InheritOptionalValue(preset.WarnDeprecated, parent.WarnDeprecated);
  InheritOptionalValue(preset.ErrorDeprecated, parent.ErrorDeprecated);
  InheritOptionalValue(preset.WarnUninitialized, parent.WarnUninitialized);
  InheritOptionalValue(preset.WarnUnusedCli, parent.WarnUnusedCli);
  InheritOptionalValue(preset.WarnSystemVars, parent.WarnSystemVars);

  for (auto const& v : parent.CacheVariables) {
    preset.CacheVariables.insert(v);
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ConfigurePreset::VisitPresetBeforeInherit()
{
  auto& preset = *this;
  if (preset.Environment.count("") != 0) {
    return ReadFileResult::INVALID_PRESET;
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ConfigurePreset::VisitPresetAfterInherit()
{
  auto& preset = *this;
  if (!preset.Hidden) {
    if (preset.Generator.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }
    if (preset.BinaryDir.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }
    if (preset.WarnDev == false && preset.ErrorDev == true) {
      return ReadFileResult::INVALID_PRESET;
    }
    if (preset.WarnDeprecated == false && preset.ErrorDeprecated == true) {
      return ReadFileResult::INVALID_PRESET;
    }
    if (preset.CacheVariables.count("") != 0) {
      return ReadFileResult::INVALID_PRESET;
    }
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::BuildPreset::VisitPresetInherit(
  const cmCMakePresetsFile::Preset& parentPreset)
{
  auto& preset = *this;
  const BuildPreset& parent = static_cast<const BuildPreset&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritOptionalValue(preset.Jobs, parent.Jobs);
  InheritVector(preset.Targets, parent.Targets);
  InheritString(preset.Configuration, parent.Configuration);
  InheritOptionalValue(preset.CleanFirst, parent.CleanFirst);
  InheritOptionalValue(preset.Verbose, parent.Verbose);
  InheritVector(preset.NativeToolOptions, parent.NativeToolOptions);

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::BuildPreset::VisitPresetAfterInherit()
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return ReadFileResult::INVALID_PRESET;
  }
  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::TestPreset::VisitPresetInherit(
  const cmCMakePresetsFile::Preset& parentPreset)
{
  auto& preset = *this;
  const TestPreset& parent = static_cast<const TestPreset&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritString(preset.Configuration, parent.Configuration);
  InheritVector(preset.OverwriteConfigurationFile,
                parent.OverwriteConfigurationFile);

  if (parent.Output) {
    if (preset.Output) {
      auto& output = preset.Output.value();
      const auto& parentOutput = parent.Output.value();
      InheritOptionalValue(output.ShortProgress, parentOutput.ShortProgress);
      InheritOptionalValue(output.Verbosity, parentOutput.Verbosity);
      InheritOptionalValue(output.Debug, parentOutput.Debug);
      InheritOptionalValue(output.OutputOnFailure,
                           parentOutput.OutputOnFailure);
      InheritOptionalValue(output.Quiet, parentOutput.Quiet);
      InheritString(output.OutputLogFile, parentOutput.OutputLogFile);
      InheritOptionalValue(output.LabelSummary, parentOutput.LabelSummary);
      InheritOptionalValue(output.SubprojectSummary,
                           parentOutput.SubprojectSummary);
      InheritOptionalValue(output.MaxPassedTestOutputSize,
                           parentOutput.MaxPassedTestOutputSize);
      InheritOptionalValue(output.MaxFailedTestOutputSize,
                           parentOutput.MaxFailedTestOutputSize);
      InheritOptionalValue(output.MaxTestNameWidth,
                           parentOutput.MaxTestNameWidth);
    } else {
      preset.Output = parent.Output;
    }
  }

  if (parent.Filter) {
    if (parent.Filter->Include) {
      if (preset.Filter && preset.Filter->Include) {
        auto& include = *preset.Filter->Include;
        const auto& parentInclude = *parent.Filter->Include;
        InheritString(include.Name, parentInclude.Name);
        InheritString(include.Label, parentInclude.Label);
        InheritOptionalValue(include.Index, parentInclude.Index);
      } else {
        if (!preset.Filter) {
          preset.Filter.emplace();
        }
        preset.Filter->Include = parent.Filter->Include;
      }
    }

    if (parent.Filter->Exclude) {
      if (preset.Filter && preset.Filter->Exclude) {
        auto& exclude = *preset.Filter->Exclude;
        const auto& parentExclude = *parent.Filter->Exclude;
        InheritString(exclude.Name, parentExclude.Name);
        InheritString(exclude.Label, parentExclude.Label);
        InheritOptionalValue(exclude.Fixtures, parentExclude.Fixtures);
      } else {
        if (!preset.Filter) {
          preset.Filter.emplace();
        }
        preset.Filter->Exclude = parent.Filter->Exclude;
      }
    }
  }

  if (parent.Execution) {
    if (preset.Execution) {
      auto& execution = *preset.Execution;
      const auto& parentExecution = *parent.Execution;
      InheritOptionalValue(execution.StopOnFailure,
                           parentExecution.StopOnFailure);
      InheritOptionalValue(execution.EnableFailover,
                           parentExecution.EnableFailover);
      InheritOptionalValue(execution.Jobs, parentExecution.Jobs);
      InheritString(execution.ResourceSpecFile,
                    parentExecution.ResourceSpecFile);
      InheritOptionalValue(execution.TestLoad, parentExecution.TestLoad);
      InheritOptionalValue(execution.ShowOnly, parentExecution.ShowOnly);
      InheritOptionalValue(execution.Repeat, parentExecution.Repeat);
      InheritOptionalValue(execution.InteractiveDebugging,
                           parentExecution.InteractiveDebugging);
      InheritOptionalValue(execution.ScheduleRandom,
                           parentExecution.ScheduleRandom);
      InheritOptionalValue(execution.Timeout, parentExecution.Timeout);
      InheritOptionalValue(execution.NoTestsAction,
                           parentExecution.NoTestsAction);
    } else {
      preset.Execution = parent.Execution;
    }
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::TestPreset::VisitPresetAfterInherit()
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return ReadFileResult::INVALID_PRESET;
  }
  return ReadFileResult::READ_OK;
}

std::string cmCMakePresetsFile::GetFilename(const std::string& sourceDir)
{
  return cmStrCat(sourceDir, "/CMakePresets.json");
}

std::string cmCMakePresetsFile::GetUserFilename(const std::string& sourceDir)
{
  return cmStrCat(sourceDir, "/CMakeUserPresets.json");
}

cmCMakePresetsFile::ReadFileResult cmCMakePresetsFile::ReadProjectPresets(
  const std::string& sourceDir, bool allowNoFiles)
{
  this->SourceDir = sourceDir;
  this->ClearPresets();

  auto result = this->ReadProjectPresetsInternal(allowNoFiles);
  if (result != ReadFileResult::READ_OK) {
    this->ClearPresets();
  }

  return result;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ReadProjectPresetsInternal(bool allowNoFiles)
{
  bool haveOneFile = false;

  std::string filename = GetUserFilename(this->SourceDir);
  if (cmSystemTools::FileExists(filename)) {
    auto result = this->ReadJSONFile(filename, true);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }
    haveOneFile = true;
  }

  filename = GetFilename(this->SourceDir);
  if (cmSystemTools::FileExists(filename)) {
    auto result = this->ReadJSONFile(filename, false);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }
    haveOneFile = true;
  }

  if (!haveOneFile) {
    return allowNoFiles ? ReadFileResult::READ_OK
                        : ReadFileResult::FILE_NOT_FOUND;
  }

  CHECK_OK(ComputePresetInheritance(this->ConfigurePresets))
  CHECK_OK(ComputePresetInheritance(this->BuildPresets))
  CHECK_OK(ComputePresetInheritance(this->TestPresets))

  for (auto& it : this->ConfigurePresets) {
    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      return ReadFileResult::INVALID_MACRO_EXPANSION;
    }
  }

  for (auto& it : this->BuildPresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        return ReadFileResult::INVALID_CONFIGURE_PRESET;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      return ReadFileResult::INVALID_MACRO_EXPANSION;
    }
  }

  for (auto& it : this->TestPresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        return ReadFileResult::INVALID_CONFIGURE_PRESET;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      return ReadFileResult::INVALID_MACRO_EXPANSION;
    }
  }

  return ReadFileResult::READ_OK;
}

const char* cmCMakePresetsFile::ResultToString(ReadFileResult result)
{
  switch (result) {
    case ReadFileResult::READ_OK:
      return "OK";
    case ReadFileResult::FILE_NOT_FOUND:
      return "File not found";
    case ReadFileResult::JSON_PARSE_ERROR:
      return "JSON parse error";
    case ReadFileResult::INVALID_ROOT:
      return "Invalid root object";
    case ReadFileResult::NO_VERSION:
      return "No \"version\" field";
    case ReadFileResult::INVALID_VERSION:
      return "Invalid \"version\" field";
    case ReadFileResult::UNRECOGNIZED_VERSION:
      return "Unrecognized \"version\" field";
    case ReadFileResult::INVALID_CMAKE_VERSION:
      return "Invalid \"cmakeMinimumRequired\" field";
    case ReadFileResult::UNRECOGNIZED_CMAKE_VERSION:
      return "\"cmakeMinimumRequired\" version too new";
    case ReadFileResult::INVALID_PRESETS:
      return "Invalid \"configurePresets\" field";
    case ReadFileResult::INVALID_PRESET:
      return "Invalid preset";
    case ReadFileResult::INVALID_VARIABLE:
      return "Invalid CMake variable definition";
    case ReadFileResult::DUPLICATE_PRESETS:
      return "Duplicate presets";
    case ReadFileResult::CYCLIC_PRESET_INHERITANCE:
      return "Cyclic preset inheritance";
    case ReadFileResult::USER_PRESET_INHERITANCE:
      return "Project preset inherits from user preset";
    case ReadFileResult::INVALID_MACRO_EXPANSION:
      return "Invalid macro expansion";
    case ReadFileResult::BUILD_TEST_PRESETS_UNSUPPORTED:
      return "File version must be 2 or higher for build and test preset "
             "support.";
    case ReadFileResult::INVALID_CONFIGURE_PRESET:
      return "Invalid \"configurePreset\" field";
  }

  return "Unknown error";
}

cmCMakePresetsFile::ReadFileResult cmCMakePresetsFile::ReadJSONFile(
  const std::string& filename, bool user)
{
  cmsys::ifstream fin(filename.c_str());
  if (!fin) {
    return ReadFileResult::FILE_NOT_FOUND;
  }
  // If there's a BOM, toss it.
  cmsys::FStream::ReadBOM(fin);

  Json::Value root;
  Json::CharReaderBuilder builder;
  Json::CharReaderBuilder::strictMode(&builder.settings_);
  if (!Json::parseFromStream(builder, fin, &root, nullptr)) {
    return ReadFileResult::JSON_PARSE_ERROR;
  }

  int v = 0;
  auto result = RootVersionHelper(v, &root);
  if (result != ReadFileResult::READ_OK) {
    return result;
  }
  if (v < MIN_VERSION || v > MAX_VERSION) {
    return ReadFileResult::UNRECOGNIZED_VERSION;
  }

  // Support for build and test presets added in version 2.
  if (v < 2 &&
      (root.isMember("buildPresets") || root.isMember("testPresets"))) {
    return ReadFileResult::BUILD_TEST_PRESETS_UNSUPPORTED;
  }

  RootPresets presets;
  if ((result = RootPresetsHelper(presets, &root)) !=
      ReadFileResult::READ_OK) {
    return result;
  }

  unsigned int currentMajor = cmVersion::GetMajorVersion();
  unsigned int currentMinor = cmVersion::GetMinorVersion();
  unsigned int currentPatch = cmVersion::GetPatchVersion();
  auto const& required = presets.CMakeMinimumRequired;
  if (required.Major > currentMajor ||
      (required.Major == currentMajor &&
       (required.Minor > currentMinor ||
        (required.Minor == currentMinor &&
         (required.Patch > currentPatch))))) {
    return ReadFileResult::UNRECOGNIZED_CMAKE_VERSION;
  }

  for (auto& preset : presets.ConfigurePresets) {
    preset.User = user;
    if (preset.Name.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }

    PresetPair<ConfigurePreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->ConfigurePresets
           .emplace(std::make_pair(preset.Name, presetPair))
           .second) {
      return ReadFileResult::DUPLICATE_PRESETS;
    }
    this->ConfigurePresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.BuildPresets) {
    preset.User = user;
    if (preset.Name.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }

    PresetPair<BuildPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->BuildPresets.emplace(preset.Name, presetPair).second) {
      return ReadFileResult::DUPLICATE_PRESETS;
    }
    this->BuildPresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.TestPresets) {
    preset.User = user;
    if (preset.Name.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }

    PresetPair<TestPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->TestPresets.emplace(preset.Name, presetPair).second) {
      return ReadFileResult::DUPLICATE_PRESETS;
    }
    this->TestPresetOrder.push_back(preset.Name);
  }

  return ReadFileResult::READ_OK;
}

void cmCMakePresetsFile::ClearPresets()
{
  this->ConfigurePresets.clear();
  this->BuildPresets.clear();
  this->TestPresets.clear();

  this->ConfigurePresetOrder.clear();
  this->BuildPresetOrder.clear();
  this->TestPresetOrder.clear();
}

void cmCMakePresetsFile::PrintPresets(
  const std::vector<const cmCMakePresetsFile::Preset*>& presets)
{
  if (presets.empty()) {
    return;
  }

  auto longestPresetName =
    std::max_element(presets.begin(), presets.end(),
                     [](const cmCMakePresetsFile::Preset* a,
                        const cmCMakePresetsFile::Preset* b) {
                       return a->Name.length() < b->Name.length();
                     });
  auto longestLength = (*longestPresetName)->Name.length();

  for (const auto* preset : presets) {
    std::cout << "  \"" << preset->Name << '"';
    const auto& description = preset->DisplayName;
    if (!description.empty()) {
      for (std::size_t i = 0; i < longestLength - preset->Name.length(); ++i) {
        std::cout << ' ';
      }
      std::cout << " - " << description;
    }
    std::cout << '\n';
  }
}

void cmCMakePresetsFile::PrintConfigurePresetList() const
{
  PrintConfigurePresetList([](const ConfigurePreset&) { return true; });
}

void cmCMakePresetsFile::PrintConfigurePresetList(
  const std::function<bool(const ConfigurePreset&)>& filter) const
{
  std::vector<const cmCMakePresetsFile::Preset*> presets;
  for (auto const& p : this->ConfigurePresetOrder) {
    auto const& preset = this->ConfigurePresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        filter(preset.Unexpanded)) {
      presets.push_back(
        static_cast<const cmCMakePresetsFile::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    std::cout << "Available configure presets:\n\n";
    cmCMakePresetsFile::PrintPresets(presets);
  }
}

void cmCMakePresetsFile::PrintBuildPresetList() const
{
  std::vector<const cmCMakePresetsFile::Preset*> presets;
  for (auto const& p : this->BuildPresetOrder) {
    auto const& preset = this->BuildPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded) {
      presets.push_back(
        static_cast<const cmCMakePresetsFile::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    std::cout << "Available build presets:\n\n";
    cmCMakePresetsFile::PrintPresets(presets);
  }
}

void cmCMakePresetsFile::PrintTestPresetList() const
{
  std::vector<const cmCMakePresetsFile::Preset*> presets;
  for (auto const& p : this->TestPresetOrder) {
    auto const& preset = this->TestPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded) {
      presets.push_back(
        static_cast<const cmCMakePresetsFile::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    std::cout << "Available test presets:\n\n";
    cmCMakePresetsFile::PrintPresets(presets);
  }
}

void cmCMakePresetsFile::PrintAllPresets() const
{
  this->PrintConfigurePresetList();
  std::cout << std::endl;
  this->PrintBuildPresetList();
  std::cout << std::endl;
  this->PrintTestPresetList();
}
