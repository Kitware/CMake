/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"

namespace {
using ReadFileResult = cmCMakePresetsGraph::ReadFileResult;
using WorkflowPreset = cmCMakePresetsGraph::WorkflowPreset;

ReadFileResult WorkflowStepTypeHelper(WorkflowPreset::WorkflowStep::Type& out,
                                      const Json::Value* value)
{
  if (!value) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (!value->isString()) {
    return ReadFileResult::INVALID_PRESET;
  }

  if (value->asString() == "configure") {
    out = WorkflowPreset::WorkflowStep::Type::Configure;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "build") {
    out = WorkflowPreset::WorkflowStep::Type::Build;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "test") {
    out = WorkflowPreset::WorkflowStep::Type::Test;
    return ReadFileResult::READ_OK;
  }

  if (value->asString() == "package") {
    out = WorkflowPreset::WorkflowStep::Type::Package;
    return ReadFileResult::READ_OK;
  }

  return ReadFileResult::INVALID_PRESET;
}

auto const WorkflowStepHelper =
  cmJSONHelperBuilder<ReadFileResult>::Object<WorkflowPreset::WorkflowStep>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("type"_s, &WorkflowPreset::WorkflowStep::PresetType,
          WorkflowStepTypeHelper)
    .Bind("name"_s, &WorkflowPreset::WorkflowStep::PresetName,
          cmCMakePresetsGraphInternal::PresetStringHelper);

auto const WorkflowStepsHelper =
  cmJSONHelperBuilder<ReadFileResult>::Vector<WorkflowPreset::WorkflowStep>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET,
    WorkflowStepHelper);

auto const WorkflowPresetHelper =
  cmJSONHelperBuilder<ReadFileResult>::Object<WorkflowPreset>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, false)
    .Bind("name"_s, &WorkflowPreset::Name,
          cmCMakePresetsGraphInternal::PresetStringHelper)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            ReadFileResult::INVALID_PRESET),
                          false)
    .Bind("displayName"_s, &WorkflowPreset::DisplayName,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("description"_s, &WorkflowPreset::Description,
          cmCMakePresetsGraphInternal::PresetStringHelper, false)
    .Bind("steps"_s, &WorkflowPreset::Steps, WorkflowStepsHelper);
}

namespace cmCMakePresetsGraphInternal {
cmCMakePresetsGraph::ReadFileResult WorkflowPresetsHelper(
  std::vector<cmCMakePresetsGraph::WorkflowPreset>& out,
  const Json::Value* value)
{
  static auto const helper =
    cmJSONHelperBuilder<ReadFileResult>::Vector<WorkflowPreset>(
      ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESETS,
      WorkflowPresetHelper);

  return helper(out, value);
}
}
