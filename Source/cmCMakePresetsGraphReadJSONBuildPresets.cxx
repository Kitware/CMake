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

#include "cmBuildOptions.h"
#include "cmCMakePresetErrors.h"
#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"

class cmJSONState;
namespace {
using BuildPreset = cmCMakePresetsGraph::BuildPreset;
using JSONHelperBuilder = cmJSONHelperBuilder;

bool PackageResolveModeHelper(cm::optional<PackageResolveMode>& out,
                              const Json::Value* value, cmJSONState* state)
{
  if (!value) {
    out = cm::nullopt;
    return true;
  }

  if (!value->isString()) {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (value->asString() == "on") {
    out = PackageResolveMode::Force;
  } else if (value->asString() == "off") {
    out = PackageResolveMode::Disable;
  } else if (value->asString() == "only") {
    out = PackageResolveMode::OnlyResolve;
  } else {
    cmCMakePresetErrors::INVALID_PRESET(value, state);
    return false;
  }

  return true;
}

std::function<bool(BuildPreset&, const Json::Value*, cmJSONState*)> const
  ResolvePackageReferencesHelper = [](BuildPreset& out,
                                      const Json::Value* value,
                                      cmJSONState* state) -> bool {
  return PackageResolveModeHelper(out.ResolvePackageReferences, value, state);
};

auto const BuildPresetHelper =
  JSONHelperBuilder::Object<BuildPreset>(
    cmCMakePresetErrors::INVALID_PRESET_OBJECT, false)
    .Bind("name"_s, &BuildPreset::Name,
          cmCMakePresetsGraphInternal::PresetNameHelper)
    .Bind("inherits"_s, &BuildPreset::Inherits,
          cmCMakePresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("hidden"_s, &BuildPreset::Hidden,
          cmCMakePresetsGraphInternal::PresetBoolHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            cmCMakePresetErrors::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &BuildPreset::DisplayName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &BuildPreset::Description,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("environment"_s, &BuildPreset::Environment,
          cmCMakePresetsGraphInternal::EnvironmentMapHelper, false)
    .Bind("configurePreset"_s, &BuildPreset::ConfigurePreset,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("inheritConfigureEnvironment"_s,
          &BuildPreset::InheritConfigureEnvironment,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("jobs"_s, &BuildPreset::Jobs,
          cmCMakePresetsGraphInternal::PresetOptionalIntHelper, false)
    .Bind("targets"_s, &BuildPreset::Targets,
          cmCMakePresetsGraphInternal::PresetVectorOneOrMoreStringHelper,
          false)
    .Bind("configuration"_s, &BuildPreset::Configuration,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("cleanFirst"_s, &BuildPreset::CleanFirst,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("verbose"_s, &BuildPreset::Verbose,
          cmCMakePresetsGraphInternal::PresetOptionalBoolHelper, false)
    .Bind("nativeToolOptions"_s, &BuildPreset::NativeToolOptions,
          cmCMakePresetsGraphInternal::PresetVectorStringHelper, false)
    .Bind("condition"_s, &BuildPreset::ConditionEvaluator,
          cmCMakePresetsGraphInternal::PresetConditionHelper, false)
    .Bind("resolvePackageReferences"_s, ResolvePackageReferencesHelper, false);
}

namespace cmCMakePresetsGraphInternal {
bool BuildPresetsHelper(std::vector<BuildPreset>& out,
                        const Json::Value* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<BuildPreset>(
    cmCMakePresetErrors::INVALID_PRESETS, BuildPresetHelper);

  return helper(out, value, state);
}
}
