/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"

namespace cmCMakePresetErrors {
const auto getPreset = [](cmJSONState* state) -> const Json::Value* {
  if (state->parseStack.size() < 2) {
    return nullptr;
  }
  std::string firstKey = state->parseStack[0].first;
  if (firstKey == "configurePresets" || firstKey == "packagePresets" ||
      firstKey == "buildPresets" || firstKey == "testPresets") {
    return state->parseStack[1].second;
  }
  return nullptr;
};
const auto getPresetName = [](cmJSONState* state) -> std::string {
#if !defined(CMAKE_BOOTSTRAP)
  const Json::Value* preset = getPreset(state);
  if (preset != nullptr && preset->isMember("name")) {
    return preset->operator[]("name").asString();
  }
#endif
  return "";
};
const auto getVariableName = [](cmJSONState* state) -> std::string {
  std::string var = state->key_after("cacheVariables");
  std::string errMsg = cmStrCat("variable \"", var, "\"");
  errMsg = cmStrCat(errMsg, " for preset \"", getPresetName(state), "\"");
  return errMsg;
};
const auto FILE_NOT_FOUND = [](const std::string& filename,
                               cmJSONState* state) -> void {
  state->AddError(cmStrCat("File not found: ", filename));
};
const auto INVALID_ROOT = [](const Json::Value* value,
                             cmJSONState* state) -> void {
  state->AddErrorAtValue("Invalid root object", value);
};
const auto NO_VERSION = [](const Json::Value* value,
                           cmJSONState* state) -> void {
  state->AddErrorAtValue("No \"version\" field", value);
};
const auto INVALID_VERSION = [](const Json::Value* value,
                                cmJSONState* state) -> void {
  state->AddErrorAtValue("Invalid \"version\" field", value);
};
const auto UNRECOGNIZED_VERSION = [](const Json::Value* value,
                                     cmJSONState* state) -> void {
  state->AddErrorAtValue("Unrecognized \"version\" field", value);
};
const auto INVALID_PRESETS = [](const Json::Value* value,
                                cmJSONState* state) -> void {
  state->AddErrorAtValue("Invalid \"configurePresets\" field", value);
};
const auto INVALID_PRESET = [](const Json::Value* value,
                               cmJSONState* state) -> void {
  state->AddErrorAtValue("Invalid preset", value);
};
const auto INVALID_PRESET_NAMED = [](const std::string& presetName,
                                     cmJSONState* state) -> void {
  state->AddError(cmStrCat("Invalid preset: \"", presetName, "\""));
};
const auto INVALID_VARIABLE = [](const Json::Value* value,
                                 cmJSONState* state) -> void {
  std::string var = cmCMakePresetErrors::getVariableName(state);
  state->AddErrorAtValue(cmStrCat("Invalid CMake ", var), value);
};
const auto DUPLICATE_PRESETS = [](const std::string& presetName,
                                  cmJSONState* state) -> void {
  state->AddError(cmStrCat("Duplicate preset: \"", presetName, "\""));
};
const auto CYCLIC_PRESET_INHERITANCE = [](const std::string& presetName,
                                          cmJSONState* state) -> void {
  state->AddError(
    cmStrCat("Cyclic preset inheritance for preset \"", presetName, "\""));
};
const auto INHERITED_PRESET_UNREACHABLE_FROM_FILE =
  [](const std::string& presetName, cmJSONState* state) -> void {
  state->AddError(cmStrCat("Inherited preset \"", presetName,
                           "\" is unreachable from preset's file"));
};
const auto CONFIGURE_PRESET_UNREACHABLE_FROM_FILE =
  [](const std::string& presetName, cmJSONState* state) -> void {
  state->AddError(cmStrCat("Configure preset \"", presetName,
                           "\" is unreachable from preset's file"));
};
const auto INVALID_MACRO_EXPANSION = [](const std::string& presetName,
                                        cmJSONState* state) -> void {
  state->AddError(cmStrCat("Invalid macro expansion in \"", presetName, "\""));
};
const auto BUILD_TEST_PRESETS_UNSUPPORTED = [](const Json::Value*,
                                               cmJSONState* state) -> void {
  state->AddError("File version must be 2 or higher for build and test preset "
                  "support");
};
const auto PACKAGE_PRESETS_UNSUPPORTED = [](const Json::Value*,
                                            cmJSONState* state) -> void {
  state->AddError(
    "File version must be 6 or higher for package preset support");
};
const auto WORKFLOW_PRESETS_UNSUPPORTED = [](const Json::Value*,
                                             cmJSONState* state) -> void {
  state->AddError(
    "File version must be 6 or higher for workflow preset support");
};
const auto INCLUDE_UNSUPPORTED = [](const Json::Value*,
                                    cmJSONState* state) -> void {
  state->AddError("File version must be 4 or higher for include support");
};
const auto INVALID_INCLUDE = [](const Json::Value* value,
                                cmJSONState* state) -> void {
  state->AddErrorAtValue("Invalid \"include\" field", value);
};
const auto INVALID_CONFIGURE_PRESET = [](const std::string& presetName,
                                         cmJSONState* state) -> void {
  state->AddError(
    cmStrCat(R"(Invalid "configurePreset": ")", presetName, "\""));
};
const auto INSTALL_PREFIX_UNSUPPORTED = [](const Json::Value* value,
                                           cmJSONState* state) -> void {
  state->AddErrorAtValue(
    "File version must be 3 or higher for installDir preset "
    "support",
    value);
};
const auto CONDITION_UNSUPPORTED = [](cmJSONState* state) -> void {
  state->AddError("File version must be 3 or higher for condition support");
};
const auto TOOLCHAIN_FILE_UNSUPPORTED = [](cmJSONState* state) -> void {
  state->AddError("File version must be 3 or higher for toolchainFile preset "
                  "support");
};
const auto CYCLIC_INCLUDE = [](const std::string& file,
                               cmJSONState* state) -> void {
  state->AddError(cmStrCat("Cyclic include among preset files: ", file));
};
const auto TEST_OUTPUT_TRUNCATION_UNSUPPORTED =
  [](cmJSONState* state) -> void {
  state->AddError("File version must be 5 or higher for testOutputTruncation "
                  "preset support");
};
const auto INVALID_WORKFLOW_STEPS = [](const std::string& workflowStep,
                                       cmJSONState* state) -> void {
  state->AddError(cmStrCat("Invalid workflow step \"", workflowStep, "\""));
};
const auto NO_WORKFLOW_STEPS = [](const std::string& presetName,
                                  cmJSONState* state) -> void {
  state->AddError(
    cmStrCat("No workflow steps specified for \"", presetName, "\""));
};
const auto FIRST_WORKFLOW_STEP_NOT_CONFIGURE = [](const std::string& stepName,
                                                  cmJSONState* state) -> void {
  state->AddError(cmStrCat("First workflow step \"", stepName,
                           "\" must be a configure step"));
};
const auto CONFIGURE_WORKFLOW_STEP_NOT_FIRST = [](const std::string& stepName,
                                                  cmJSONState* state) -> void {
  state->AddError(cmStrCat("Configure workflow step \"", stepName,
                           "\" must be the first step"));
};
const auto WORKFLOW_STEP_UNREACHABLE_FROM_FILE =
  [](const std::string& workflowStep, cmJSONState* state) -> void {
  state->AddError(cmStrCat("Workflow step \"", workflowStep,
                           "\" is unreachable from preset's file"));
};
const auto CTEST_JUNIT_UNSUPPORTED = [](cmJSONState* state) -> void {
  state->AddError(
    "File version must be 6 or higher for CTest JUnit output support");
};
const auto TRACE_UNSUPPORTED = [](cmJSONState* state) -> void {
  state->AddError("File version must be 7 or higher for trace preset support");
};
const auto UNRECOGNIZED_CMAKE_VERSION = [](const std::string& version,
                                           int current, int required) {
  return [version, current, required](const Json::Value* value,
                                      cmJSONState* state) -> void {
    state->AddErrorAtValue(cmStrCat("\"cmakeMinimumRequired\" ", version,
                                    " version ", required,
                                    " must be less than ", current),
                           value);
  };
};
const auto INVALID_PRESET_NAME = [](const Json::Value* value,
                                    cmJSONState* state) -> void {
  std::string errMsg = "Invalid Preset Name";
  if (value && value->isConvertibleTo(Json::ValueType::stringValue) &&
      !value->asString().empty()) {
    errMsg = cmStrCat(errMsg, ": ", value->asString());
  }
  state->AddErrorAtValue(errMsg, value);
};
const auto INVALID_CONDITION = [](const Json::Value* value,
                                  cmJSONState* state) -> void {
  state->AddErrorAtValue(
    cmStrCat("Invalid condition for preset \"", getPresetName(state), "\""),
    value);
};
const auto INVALID_CONDITION_OBJECT =
  [](JsonErrors::ObjectError errorType,
     const Json::Value::Members& extraFields) {
    return JsonErrors::INVALID_NAMED_OBJECT(
      [](const Json::Value*, cmJSONState* state) -> std::string {
        return cmStrCat(" condition for preset \"", getPresetName(state),
                        "\"");
      })(errorType, extraFields);
  };
const auto INVALID_VARIABLE_OBJECT =
  [](JsonErrors::ObjectError errorType,
     const Json::Value::Members& extraFields) {
    return JsonErrors::INVALID_NAMED_OBJECT(
      [](const Json::Value*, cmJSONState* state) -> std::string {
        return getVariableName(state);
      })(errorType, extraFields);
  };
const auto INVALID_PRESET_OBJECT =
  [](JsonErrors::ObjectError errorType,
     const Json::Value::Members& extraFields) {
    return JsonErrors::INVALID_NAMED_OBJECT(
      [](const Json::Value*, cmJSONState*) -> std::string {
        return "Preset";
      })(errorType, extraFields);
  };
const auto INVALID_ROOT_OBJECT = [](JsonErrors::ObjectError errorType,
                                    const Json::Value::Members& extraFields) {
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState*) -> std::string {
      return "root object";
    })(errorType, extraFields);
};
const auto PRESET_MISSING_FIELD = [](const std::string& presetName,
                                     const std::string& missingField,
                                     cmJSONState* state) {
  state->AddError(cmStrCat("Preset \"", presetName, "\" missing field \"",
                           missingField, "\""));
};
}
