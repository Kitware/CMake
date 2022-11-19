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
using PackagePreset = cmCMakePresetsGraph::PackagePreset;

auto const OutputHelper =
  cmJSONHelperBuilder<ReadFileResult>::Object<PackagePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("debug"_s, &PackagePreset::DebugOutput,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("verbose"_s, &PackagePreset::VerboseOutput,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const VariableHelper = cmJSONHelperBuilder<ReadFileResult>::String(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_VARIABLE);

auto const VariablesHelper =
  cmJSONHelperBuilder<ReadFileResult>::Map<std::string>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, VariableHelper);

auto const PackagePresetHelper =
  cmJSONHelperBuilder<ReadFileResult>::Object<PackagePreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &PackagePreset::Name,
          cmCMakePresetsGraphInternal::PresetStringHelper)
    .Bind("inherits"_s, &PackagePreset::Inherits,
          cmCMakePresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("hidden"_s, &PackagePreset::Hidden,
          cmCMakePresetsGraphInternal::PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            ReadFileResult::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &PackagePreset::DisplayName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &PackagePreset::Description,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("environment"_s, &PackagePreset::Environment,
          cmCMakePresetsGraphInternal::EnvironmentMapHelper, false)
    .Bind("configurePreset"_s, &PackagePreset::ConfigurePreset,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("inheritConfigureEnvironment"_s,
          &PackagePreset::InheritConfigureEnvironment,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("generators"_s, &PackagePreset::Generators,
          cmCMakePresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("configurations"_s, &PackagePreset::Configurations,
          cmCMakePresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("variables"_s, &PackagePreset::Variables, VariablesHelper, false)
    .Bind("configFile"_s, &PackagePreset::ConfigFile,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("output"_s, OutputHelper, false)
    .Bind("packageName"_s, &PackagePreset::PackageName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("packageVersion"_s, &PackagePreset::PackageVersion,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("packageDirectory"_s, &PackagePreset::PackageDirectory,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("vendorName"_s, &PackagePreset::VendorName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("condition"_s, &PackagePreset::ConditionEvaluator,
          cmCMakePresetsGraphInternal::PresetConditionHelper, false);
}

namespace cmCMakePresetsGraphInternal {
cmCMakePresetsGraph::ReadFileResult PackagePresetsHelper(
  std::vector<cmCMakePresetsGraph::PackagePreset>& out,
  const Json::Value* value)
{
  static auto const helper =
    cmJSONHelperBuilder<ReadFileResult>::Vector<PackagePreset>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS,
      PackagePresetHelper);

  return helper(out, value);
}
}
