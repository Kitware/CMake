/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"

#include "cmCMakePresetsFile.h"
#include "cmCMakePresetsFileInternal.h"
#include "cmJSONHelpers.h"
#include "cmVersion.h"

namespace {
using ReadFileResult = cmCMakePresetsFile::ReadFileResult;
using CacheVariable = cmCMakePresetsFile::CacheVariable;
using ConfigurePreset = cmCMakePresetsFile::ConfigurePreset;
using BuildPreset = cmCMakePresetsFile::BuildPreset;
using TestPreset = cmCMakePresetsFile::TestPreset;
using ArchToolsetStrategy = cmCMakePresetsFile::ArchToolsetStrategy;

constexpr int MIN_VERSION = 1;
constexpr int MAX_VERSION = 3;

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

std::unique_ptr<cmCMakePresetsFileInternal::NotCondition> InvertCondition(
  std::unique_ptr<cmCMakePresetsFile::Condition> condition)
{
  auto retval = cm::make_unique<cmCMakePresetsFileInternal::NotCondition>();
  retval->SubCondition = std::move(condition);
  return retval;
}

auto const ConditionStringHelper = cmJSONStringHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION);

auto const ConditionBoolHelper = cmJSONBoolHelper<ReadFileResult>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION);

auto const ConditionStringListHelper =
  cmJSONVectorHelper<std::string, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION,
    ConditionStringHelper);

auto const ConstConditionHelper =
  cmJSONObjectHelper<cmCMakePresetsFileInternal::ConstCondition,
                     ReadFileResult>(ReadFileResult::READ_OK,
                                     ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("value"_s, &cmCMakePresetsFileInternal::ConstCondition::Value,
          ConditionBoolHelper, true);

auto const EqualsConditionHelper =
  cmJSONObjectHelper<cmCMakePresetsFileInternal::EqualsCondition,
                     ReadFileResult>(ReadFileResult::READ_OK,
                                     ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("lhs"_s, &cmCMakePresetsFileInternal::EqualsCondition::Lhs,
          ConditionStringHelper, true)
    .Bind("rhs"_s, &cmCMakePresetsFileInternal::EqualsCondition::Rhs,
          ConditionStringHelper, true);

auto const InListConditionHelper =
  cmJSONObjectHelper<cmCMakePresetsFileInternal::InListCondition,
                     ReadFileResult>(ReadFileResult::READ_OK,
                                     ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmCMakePresetsFileInternal::InListCondition::String,
          ConditionStringHelper, true)
    .Bind("list"_s, &cmCMakePresetsFileInternal::InListCondition::List,
          ConditionStringListHelper, true);

auto const MatchesConditionHelper =
  cmJSONObjectHelper<cmCMakePresetsFileInternal::MatchesCondition,
                     ReadFileResult>(ReadFileResult::READ_OK,
                                     ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmCMakePresetsFileInternal::MatchesCondition::String,
          ConditionStringHelper, true)
    .Bind("regex"_s, &cmCMakePresetsFileInternal::MatchesCondition::Regex,
          ConditionStringHelper, true);

ReadFileResult SubConditionHelper(
  std::unique_ptr<cmCMakePresetsFile::Condition>& out,
  const Json::Value* value);

auto const ListConditionVectorHelper =
  cmJSONVectorHelper<std::unique_ptr<cmCMakePresetsFile::Condition>,
                     ReadFileResult>(ReadFileResult::READ_OK,
                                     ReadFileResult::INVALID_CONDITION,
                                     SubConditionHelper);
auto const AnyAllOfConditionHelper =
  cmJSONObjectHelper<cmCMakePresetsFileInternal::AnyAllOfCondition,
                     ReadFileResult>(ReadFileResult::READ_OK,
                                     ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("conditions"_s,
          &cmCMakePresetsFileInternal::AnyAllOfCondition::Conditions,
          ListConditionVectorHelper);

auto const NotConditionHelper =
  cmJSONObjectHelper<cmCMakePresetsFileInternal::NotCondition, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("condition"_s,
          &cmCMakePresetsFileInternal::NotCondition::SubCondition,
          SubConditionHelper);

ReadFileResult ConditionHelper(
  std::unique_ptr<cmCMakePresetsFile::Condition>& out,
  const Json::Value* value)
{
  if (!value) {
    out.reset();
    return ReadFileResult::READ_OK;
  }

  if (value->isBool()) {
    auto c = cm::make_unique<cmCMakePresetsFileInternal::ConstCondition>();
    c->Value = value->asBool();
    out = std::move(c);
    return ReadFileResult::READ_OK;
  }

  if (value->isNull()) {
    out = cm::make_unique<cmCMakePresetsFileInternal::NullCondition>();
    return ReadFileResult::READ_OK;
  }

  if (value->isObject()) {
    if (!value->isMember("type")) {
      return ReadFileResult::INVALID_CONDITION;
    }

    if (!(*value)["type"].isString()) {
      return ReadFileResult::INVALID_CONDITION;
    }
    auto type = (*value)["type"].asString();

    if (type == "const") {
      auto c = cm::make_unique<cmCMakePresetsFileInternal::ConstCondition>();
      CHECK_OK(ConstConditionHelper(*c, value));
      out = std::move(c);
      return ReadFileResult::READ_OK;
    }

    if (type == "equals" || type == "notEquals") {
      auto c = cm::make_unique<cmCMakePresetsFileInternal::EqualsCondition>();
      CHECK_OK(EqualsConditionHelper(*c, value));
      out = std::move(c);
      if (type == "notEquals") {
        out = InvertCondition(std::move(out));
      }
      return ReadFileResult::READ_OK;
    }

    if (type == "inList" || type == "notInList") {
      auto c = cm::make_unique<cmCMakePresetsFileInternal::InListCondition>();
      CHECK_OK(InListConditionHelper(*c, value));
      out = std::move(c);
      if (type == "notInList") {
        out = InvertCondition(std::move(out));
      }
      return ReadFileResult::READ_OK;
    }

    if (type == "matches" || type == "notMatches") {
      auto c = cm::make_unique<cmCMakePresetsFileInternal::MatchesCondition>();
      CHECK_OK(MatchesConditionHelper(*c, value));
      out = std::move(c);
      if (type == "notMatches") {
        out = InvertCondition(std::move(out));
      }
      return ReadFileResult::READ_OK;
    }

    if (type == "anyOf" || type == "allOf") {
      auto c =
        cm::make_unique<cmCMakePresetsFileInternal::AnyAllOfCondition>();
      c->StopValue = (type == "anyOf");
      CHECK_OK(AnyAllOfConditionHelper(*c, value));
      out = std::move(c);
      return ReadFileResult::READ_OK;
    }

    if (type == "not") {
      auto c = cm::make_unique<cmCMakePresetsFileInternal::NotCondition>();
      CHECK_OK(NotConditionHelper(*c, value));
      out = std::move(c);
      return ReadFileResult::READ_OK;
    }
  }

  return ReadFileResult::INVALID_CONDITION;
}

ReadFileResult PresetConditionHelper(
  std::shared_ptr<cmCMakePresetsFile::Condition>& out,
  const Json::Value* value)
{
  std::unique_ptr<cmCMakePresetsFile::Condition> ptr;
  auto result = ConditionHelper(ptr, value);
  out = std::move(ptr);
  return result;
}

ReadFileResult SubConditionHelper(
  std::unique_ptr<cmCMakePresetsFile::Condition>& out,
  const Json::Value* value)
{
  std::unique_ptr<cmCMakePresetsFile::Condition> ptr;
  auto result = ConditionHelper(ptr, value);
  if (ptr && ptr->IsNull()) {
    return ReadFileResult::INVALID_CONDITION;
  }
  out = std::move(ptr);
  return result;
}

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

ReadFileResult PresetVectorOneOrMoreStringHelper(std::vector<std::string>& out,
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
    .Bind("inherits"_s, &ConfigurePreset::Inherits,
          PresetVectorOneOrMoreStringHelper, false)
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
    .Bind("toolchainFile"_s, &ConfigurePreset::ToolchainFile,
          PresetStringHelper, false)
    .Bind("binaryDir"_s, &ConfigurePreset::BinaryDir, PresetStringHelper,
          false)
    .Bind("installDir"_s, &ConfigurePreset::InstallDir, PresetStringHelper,
          false)
    .Bind<std::string>("cmakeExecutable"_s, nullptr, PresetStringHelper, false)
    .Bind("cacheVariables"_s, &ConfigurePreset::CacheVariables,
          VariablesHelper, false)
    .Bind("environment"_s, &ConfigurePreset::Environment, EnvironmentMapHelper,
          false)
    .Bind("warnings"_s, PresetWarningsHelper, false)
    .Bind("errors"_s, PresetErrorsHelper, false)
    .Bind("debug"_s, PresetDebugHelper, false)
    .Bind("condition"_s, &ConfigurePreset::ConditionEvaluator,
          PresetConditionHelper, false);

auto const BuildPresetHelper =
  cmJSONObjectHelper<BuildPreset, ReadFileResult>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &BuildPreset::Name, PresetStringHelper)
    .Bind("inherits"_s, &BuildPreset::Inherits,
          PresetVectorOneOrMoreStringHelper, false)
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
    .Bind("targets"_s, &BuildPreset::Targets,
          PresetVectorOneOrMoreStringHelper, false)
    .Bind("configuration"_s, &BuildPreset::Configuration, PresetStringHelper,
          false)
    .Bind("cleanFirst"_s, &BuildPreset::CleanFirst, PresetOptionalBoolHelper,
          false)
    .Bind("verbose"_s, &BuildPreset::Verbose, PresetOptionalBoolHelper, false)
    .Bind("nativeToolOptions"_s, &BuildPreset::NativeToolOptions,
          PresetVectorStringHelper, false)
    .Bind("condition"_s, &BuildPreset::ConditionEvaluator,
          PresetConditionHelper, false);

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
    .Bind("inherits"_s, &TestPreset::Inherits,
          PresetVectorOneOrMoreStringHelper, false)
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
          false)
    .Bind("condition"_s, &TestPreset::ConditionEvaluator,
          PresetConditionHelper, false);

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
  if (user) {
    this->UserVersion = v;
  } else {
    this->Version = v;
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

    // Support for installDir presets added in version 3.
    if (v < 3 && !preset.InstallDir.empty()) {
      return ReadFileResult::INSTALL_PREFIX_UNSUPPORTED;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      return ReadFileResult::CONDITION_UNSUPPORTED;
    }

    // Support for toolchainFile presets added in version 3.
    if (v < 3 && !preset.ToolchainFile.empty()) {
      return ReadFileResult::TOOLCHAIN_FILE_UNSUPPORTED;
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

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      return ReadFileResult::CONDITION_UNSUPPORTED;
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

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      return ReadFileResult::CONDITION_UNSUPPORTED;
    }

    this->TestPresetOrder.push_back(preset.Name);
  }

  return ReadFileResult::READ_OK;
}
