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

namespace {
using ReadFileResult = cmCMakePresetsGraph::ReadFileResult;
using CacheVariable = cmCMakePresetsGraph::CacheVariable;
using ConfigurePreset = cmCMakePresetsGraph::ConfigurePreset;
using ArchToolsetStrategy = cmCMakePresetsGraph::ArchToolsetStrategy;
using JSONHelperBuilder = cmJSONHelperBuilder<ReadFileResult>;

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
    JSONHelperBuilder::Object<ConfigurePreset>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
      .Bind("value", valueField,
            cmCMakePresetsGraphInternal::PresetStringHelper, false)
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

auto const VariableStringHelper = JSONHelperBuilder::String(
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
  JSONHelperBuilder::Object<CacheVariable>(
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
  JSONHelperBuilder::Map<cm::optional<CacheVariable>>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, VariableHelper);

auto const PresetWarningsHelper =
  JSONHelperBuilder::Object<ConfigurePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("dev"_s, &ConfigurePreset::WarnDev,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("deprecated"_s, &ConfigurePreset::WarnDeprecated,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("uninitialized"_s, &ConfigurePreset::WarnUninitialized,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("unusedCli"_s, &ConfigurePreset::WarnUnusedCli,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("systemVars"_s, &ConfigurePreset::WarnSystemVars,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const PresetErrorsHelper =
  JSONHelperBuilder::Object<ConfigurePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("dev"_s, &ConfigurePreset::ErrorDev,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("deprecated"_s, &ConfigurePreset::ErrorDeprecated,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const PresetDebugHelper =
  JSONHelperBuilder::Object<ConfigurePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("output"_s, &ConfigurePreset::DebugOutput,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("tryCompile"_s, &ConfigurePreset::DebugTryCompile,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("find"_s, &ConfigurePreset::DebugFind,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const ConfigurePresetHelper =
  JSONHelperBuilder::Object<ConfigurePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &ConfigurePreset::Name,
          cmCMakePresetsGraphInternal::PresetStringHelper)
    .Bind("inherits"_s, &ConfigurePreset::Inherits,
          cmCMakePresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("hidden"_s, &ConfigurePreset::Hidden,
          cmCMakePresetsGraphInternal::PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            ReadFileResult::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &ConfigurePreset::DisplayName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &ConfigurePreset::Description,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("generator"_s, &ConfigurePreset::Generator,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("architecture"_s, ArchitectureHelper, false)
    .Bind("toolset"_s, ToolsetHelper, false)
    .Bind("toolchainFile"_s, &ConfigurePreset::ToolchainFile,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("binaryDir"_s, &ConfigurePreset::BinaryDir,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("installDir"_s, &ConfigurePreset::InstallDir,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind<std::string>("cmakeExecutable"_s, nullptr,
                       cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("cacheVariables"_s, &ConfigurePreset::CacheVariables,
          VariablesHelper, false)
    .Bind("environment"_s, &ConfigurePreset::Environment,
          cmCMakePresetsGraphInternal::EnvironmentMapHelper, false)
    .Bind("warnings"_s, PresetWarningsHelper, false)
    .Bind("errors"_s, PresetErrorsHelper, false)
    .Bind("debug"_s, PresetDebugHelper, false)
    .Bind("condition"_s, &ConfigurePreset::ConditionEvaluator,
          cmCMakePresetsGraphInternal::PresetConditionHelper, false);
}

namespace cmCMakePresetsGraphInternal {
ReadFileResult ConfigurePresetsHelper(std::vector<ConfigurePreset>& out,
                                      const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Vector<ConfigurePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS,
    ConfigurePresetHelper);

  return helper(out, value);
}
}
