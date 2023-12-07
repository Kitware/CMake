/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <cstddef>
#include <string>
#include <vector>

#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmCMakePresetsErrors.h"
#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"

class cmJSONState;

namespace {
using WorkflowPreset = cmCMakePresetsGraph::WorkflowPreset;

bool WorkflowStepTypeHelper(WorkflowPreset::WorkflowStep::Type& out,
                            const Json::Value* value, cmJSONState* state)
{
  if (!value) {
    cmCMakePresetsErrors::INVALID_PRESET(value, state);
    return false;
  }

  if (!value->isString()) {
    return false;
  }

  if (value->asString() == "configure") {
    out = WorkflowPreset::WorkflowStep::Type::Configure;
    return true;
  }

  if (value->asString() == "build") {
    out = WorkflowPreset::WorkflowStep::Type::Build;
    return true;
  }

  if (value->asString() == "test") {
    out = WorkflowPreset::WorkflowStep::Type::Test;
    return true;
  }

  if (value->asString() == "package") {
    out = WorkflowPreset::WorkflowStep::Type::Package;
    return true;
  }

  cmCMakePresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const WorkflowStepHelper =
  cmJSONHelperBuilder::Object<WorkflowPreset::WorkflowStep>(
    JsonErrors::INVALID_OBJECT, false)
    .Bind("type"_s, &WorkflowPreset::WorkflowStep::PresetType,
          WorkflowStepTypeHelper)
    .Bind("name"_s, &WorkflowPreset::WorkflowStep::PresetName,
          cmCMakePresetsGraphInternal::PresetStringHelper);

auto const WorkflowStepsHelper =
  cmJSONHelperBuilder::Vector<WorkflowPreset::WorkflowStep>(
    cmCMakePresetsErrors::INVALID_PRESET, WorkflowStepHelper);

auto const WorkflowPresetHelper =
  cmJSONHelperBuilder::Object<WorkflowPreset>(
    cmCMakePresetsErrors::INVALID_PRESET_OBJECT, false)
    .Bind("name"_s, &WorkflowPreset::Name,
          cmCMakePresetsGraphInternal::PresetNameHelper)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            cmCMakePresetsErrors::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &WorkflowPreset::DisplayName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &WorkflowPreset::Description,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("steps"_s, &WorkflowPreset::Steps, WorkflowStepsHelper);
}

namespace cmCMakePresetsGraphInternal {
bool WorkflowPresetsHelper(
  std::vector<cmCMakePresetsGraph::WorkflowPreset>& out,
  const Json::Value* value, cmJSONState* state)
{
  static auto const helper = cmJSONHelperBuilder::Vector<WorkflowPreset>(
    cmCMakePresetsErrors::INVALID_PRESETS, WorkflowPresetHelper);

  return helper(out, value, state);
}
}
