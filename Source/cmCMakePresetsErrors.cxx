/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCMakePresetsErrors.h"

#include <functional>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"

namespace cmCMakePresetsErrors {
const Json::Value* getPreset(cmJSONState* state)
{
  if (state->parseStack.size() < 2) {
    return nullptr;
  }
  std::string firstKey = state->parseStack[0].first;
  if (firstKey == "configurePresets" || firstKey == "packagePresets" ||
      firstKey == "buildPresets" || firstKey == "testPresets") {
    return state->parseStack[1].second;
  }
  return nullptr;
}

std::string getPresetName(cmJSONState* state)
{
  const Json::Value* preset = getPreset(state);
  if (preset != nullptr && preset->isMember("name")) {
    return preset->operator[]("name").asString();
  }
  return "";
}

std::string getVariableName(cmJSONState* state)
{
  std::string var = state->key_after("cacheVariables");
  std::string errMsg = cmStrCat("variable \"", var, "\"");
  errMsg = cmStrCat(errMsg, " for preset \"", getPresetName(state), "\"");
  return errMsg;
}

void FILE_NOT_FOUND(const std::string& filename, cmJSONState* state)
{
  state->AddError(cmStrCat("File not found: ", filename));
}

void INVALID_ROOT(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid root object", value);
}

void NO_VERSION(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("No \"version\" field", value);
}

void INVALID_VERSION(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid \"version\" field", value);
}

void UNRECOGNIZED_VERSION(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("Unrecognized \"version\" field", value);
}

void INVALID_PRESETS(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid \"configurePresets\" field", value);
}

void INVALID_PRESET(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid preset", value);
}

void INVALID_PRESET_NAMED(const std::string& presetName, cmJSONState* state)
{
  state->AddError(cmStrCat("Invalid preset: \"", presetName, "\""));
}

void INVALID_VARIABLE(const Json::Value* value, cmJSONState* state)
{
  std::string var = cmCMakePresetsErrors::getVariableName(state);
  state->AddErrorAtValue(cmStrCat("Invalid CMake ", var), value);
}

void DUPLICATE_PRESETS(const std::string& presetName, cmJSONState* state)
{
  state->AddError(cmStrCat("Duplicate preset: \"", presetName, "\""));
}

void CYCLIC_PRESET_INHERITANCE(const std::string& presetName,
                               cmJSONState* state)

{
  state->AddError(
    cmStrCat("Cyclic preset inheritance for preset \"", presetName, "\""));
}

void INHERITED_PRESET_UNREACHABLE_FROM_FILE(const std::string& presetName,
                                            cmJSONState* state)
{
  state->AddError(cmStrCat("Inherited preset \"", presetName,
                           "\" is unreachable from preset's file"));
}

void CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(const std::string& presetName,
                                            cmJSONState* state)
{
  state->AddError(cmStrCat("Configure preset \"", presetName,
                           "\" is unreachable from preset's file"));
}

void INVALID_MACRO_EXPANSION(const std::string& presetName, cmJSONState* state)
{
  state->AddError(cmStrCat("Invalid macro expansion in \"", presetName, "\""));
}

void BUILD_TEST_PRESETS_UNSUPPORTED(const Json::Value*, cmJSONState* state)
{
  state->AddError("File version must be 2 or higher for build and test preset "
                  "support");
}

void PACKAGE_PRESETS_UNSUPPORTED(const Json::Value*, cmJSONState* state)
{
  state->AddError(
    "File version must be 6 or higher for package preset support");
}

void WORKFLOW_PRESETS_UNSUPPORTED(const Json::Value*, cmJSONState* state)
{
  state->AddError(
    "File version must be 6 or higher for workflow preset support");
}

void INCLUDE_UNSUPPORTED(const Json::Value*, cmJSONState* state)
{
  state->AddError("File version must be 4 or higher for include support");
}

void INVALID_INCLUDE(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid \"include\" field", value);
}

void INVALID_CONFIGURE_PRESET(const std::string& presetName,
                              cmJSONState* state)
{
  state->AddError(
    cmStrCat(R"(Invalid "configurePreset": ")", presetName, "\""));
}

void INSTALL_PREFIX_UNSUPPORTED(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue(
    "File version must be 3 or higher for installDir preset "
    "support",
    value);
}

void CONDITION_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 3 or higher for condition support");
}

void TOOLCHAIN_FILE_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 3 or higher for toolchainFile preset "
                  "support");
}

void CYCLIC_INCLUDE(const std::string& file, cmJSONState* state)
{
  state->AddError(cmStrCat("Cyclic include among preset files: ", file));
}

void TEST_OUTPUT_TRUNCATION_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 5 or higher for testOutputTruncation "
                  "preset support");
}

void INVALID_WORKFLOW_STEPS(const std::string& workflowStep,
                            cmJSONState* state)
{
  state->AddError(cmStrCat("Invalid workflow step \"", workflowStep, "\""));
}

void NO_WORKFLOW_STEPS(const std::string& presetName, cmJSONState* state)
{
  state->AddError(
    cmStrCat("No workflow steps specified for \"", presetName, "\""));
}

void FIRST_WORKFLOW_STEP_NOT_CONFIGURE(const std::string& stepName,
                                       cmJSONState* state)
{
  state->AddError(cmStrCat("First workflow step \"", stepName,
                           "\" must be a configure step"));
}

void CONFIGURE_WORKFLOW_STEP_NOT_FIRST(const std::string& stepName,
                                       cmJSONState* state)
{
  state->AddError(cmStrCat("Configure workflow step \"", stepName,
                           "\" must be the first step"));
}

void WORKFLOW_STEP_UNREACHABLE_FROM_FILE(const std::string& workflowStep,
                                         cmJSONState* state)
{
  state->AddError(cmStrCat("Workflow step \"", workflowStep,
                           "\" is unreachable from preset's file"));
}

void CTEST_JUNIT_UNSUPPORTED(cmJSONState* state)
{
  state->AddError(
    "File version must be 6 or higher for CTest JUnit output support");
}

void TRACE_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 7 or higher for trace preset support");
}

JsonErrors::ErrorGenerator UNRECOGNIZED_CMAKE_VERSION(
  const std::string& version, int current, int required)
{
  return [version, current, required](const Json::Value* value,
                                      cmJSONState* state) -> void {
    state->AddErrorAtValue(cmStrCat("\"cmakeMinimumRequired\" ", version,
                                    " version ", required,
                                    " must be less than ", current),
                           value);
  };
}

void INVALID_PRESET_NAME(const Json::Value* value, cmJSONState* state)
{
  std::string errMsg = "Invalid Preset Name";
  if (value && value->isConvertibleTo(Json::ValueType::stringValue) &&
      !value->asString().empty()) {
    errMsg = cmStrCat(errMsg, ": ", value->asString());
  }
  state->AddErrorAtValue(errMsg, value);
}

void INVALID_CONDITION(const Json::Value* value, cmJSONState* state)
{
  state->AddErrorAtValue(
    cmStrCat("Invalid condition for preset \"", getPresetName(state), "\""),
    value);
}

JsonErrors::ErrorGenerator INVALID_CONDITION_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState* state) -> std::string {
      return cmStrCat(" condition for preset \"", getPresetName(state), "\"");
    })(errorType, extraFields);
}

JsonErrors::ErrorGenerator INVALID_VARIABLE_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState* state) -> std::string {
      return getVariableName(state);
    })(errorType, extraFields);
}

JsonErrors::ErrorGenerator INVALID_PRESET_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState*) -> std::string { return "Preset"; })(
    errorType, extraFields);
}

JsonErrors::ErrorGenerator INVALID_ROOT_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState*) -> std::string {
      return "root object";
    })(errorType, extraFields);
}

void PRESET_MISSING_FIELD(const std::string& presetName,
                          const std::string& missingField, cmJSONState* state)
{
  state->AddError(cmStrCat("Preset \"", presetName, "\" missing field \"",
                           missingField, "\""));
}

void SCHEMA_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 8 or higher for $schema support");
}
}
