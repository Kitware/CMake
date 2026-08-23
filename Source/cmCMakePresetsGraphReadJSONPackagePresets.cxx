/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmCMakePresetsErrors.h"
#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmJSONState.h"

namespace {
using PackagePreset = cmCMakePresetsGraph::PackagePreset;

auto const OutputHelper =
  cmJSONHelperBuilder::Object<PackagePreset>(
    JsonErrors::INVALID_NAMED_OBJECT_KEY, false)
    .Bind("debug"_s, &PackagePreset::DebugOutput,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("verbose"_s, &PackagePreset::VerboseOutput,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false);

auto const VariableHelper =
  cmJSONHelperBuilder::String(cmCMakePresetsErrors::INVALID_VARIABLE);

auto const VariablesHelper = cmJSONHelperBuilder::Map<std::string>(
  cmCMakePresetsErrors::INVALID_VARIABLE, VariableHelper);

auto const PackagePresetHelper =
  cmCMakePresetsGraphInternal::BindDependentPresetFields(
    cmCMakePresetsGraphInternal::BindPresetIdentityFields(
      cmJSONHelperBuilder::Object<PackagePreset>(
        cmCMakePresetsErrors::INVALID_PRESET_OBJECT, false)))
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
          cmCMakePresetsGraphInternal::PresetStringHelper, false);
}

namespace cmCMakePresetsGraphInternal {
bool PackagePresetsHelper(std::vector<cmCMakePresetsGraph::PackagePreset>& out,
                          Json::Value const* value, cmJSONState* state)
{
  static auto const helper = cmJSONHelperBuilder::Vector<PackagePreset>(
    cmCMakePresetsErrors::INVALID_PRESETS, PackagePresetHelper);

  return helper(out, value, state);
}
}
