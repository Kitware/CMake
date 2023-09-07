/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"

class cmJSONState;

namespace cmCMakePresetsErrors {
const Json::Value* getPreset(cmJSONState* state);

std::string getPresetName(cmJSONState* state);

std::string getVariableName(cmJSONState* state);

void FILE_NOT_FOUND(const std::string& filename, cmJSONState* state);

void INVALID_ROOT(const Json::Value* value, cmJSONState* state);

void NO_VERSION(const Json::Value* value, cmJSONState* state);

void INVALID_VERSION(const Json::Value* value, cmJSONState* state);

void UNRECOGNIZED_VERSION(const Json::Value* value, cmJSONState* state);

void INVALID_PRESETS(const Json::Value* value, cmJSONState* state);

void INVALID_PRESET(const Json::Value* value, cmJSONState* state);

void INVALID_PRESET_NAMED(const std::string& presetName, cmJSONState* state);

void INVALID_VARIABLE(const Json::Value* value, cmJSONState* state);

void DUPLICATE_PRESETS(const std::string& presetName, cmJSONState* state);

void CYCLIC_PRESET_INHERITANCE(const std::string& presetName,
                               cmJSONState* state);

void INHERITED_PRESET_UNREACHABLE_FROM_FILE(const std::string& presetName,
                                            cmJSONState* state);

void CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(const std::string& presetName,
                                            cmJSONState* state);

void INVALID_MACRO_EXPANSION(const std::string& presetName,
                             cmJSONState* state);

void BUILD_TEST_PRESETS_UNSUPPORTED(const Json::Value*, cmJSONState* state);

void PACKAGE_PRESETS_UNSUPPORTED(const Json::Value*, cmJSONState* state);

void WORKFLOW_PRESETS_UNSUPPORTED(const Json::Value*, cmJSONState* state);

void INCLUDE_UNSUPPORTED(const Json::Value*, cmJSONState* state);

void INVALID_INCLUDE(const Json::Value* value, cmJSONState* state);

void INVALID_CONFIGURE_PRESET(const std::string& presetName,
                              cmJSONState* state);

void INSTALL_PREFIX_UNSUPPORTED(const Json::Value* value, cmJSONState* state);

void CONDITION_UNSUPPORTED(cmJSONState* state);

void TOOLCHAIN_FILE_UNSUPPORTED(cmJSONState* state);

void CYCLIC_INCLUDE(const std::string& file, cmJSONState* state);

void TEST_OUTPUT_TRUNCATION_UNSUPPORTED(cmJSONState* state);

void INVALID_WORKFLOW_STEPS(const std::string& workflowStep,
                            cmJSONState* state);

void NO_WORKFLOW_STEPS(const std::string& presetName, cmJSONState* state);

void FIRST_WORKFLOW_STEP_NOT_CONFIGURE(const std::string& stepName,
                                       cmJSONState* state);

void CONFIGURE_WORKFLOW_STEP_NOT_FIRST(const std::string& stepName,
                                       cmJSONState* state);

void WORKFLOW_STEP_UNREACHABLE_FROM_FILE(const std::string& workflowStep,
                                         cmJSONState* state);

void CTEST_JUNIT_UNSUPPORTED(cmJSONState* state);

void TRACE_UNSUPPORTED(cmJSONState* state);

JsonErrors::ErrorGenerator UNRECOGNIZED_CMAKE_VERSION(
  const std::string& version, int current, int required);

void INVALID_PRESET_NAME(const Json::Value* value, cmJSONState* state);

void INVALID_CONDITION(const Json::Value* value, cmJSONState* state);

JsonErrors::ErrorGenerator INVALID_CONDITION_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields);

JsonErrors::ErrorGenerator INVALID_VARIABLE_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields);

JsonErrors::ErrorGenerator INVALID_PRESET_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields);

JsonErrors::ErrorGenerator INVALID_ROOT_OBJECT(
  JsonErrors::ObjectError errorType, const Json::Value::Members& extraFields);

void PRESET_MISSING_FIELD(const std::string& presetName,
                          const std::string& missingField, cmJSONState* state);

void SCHEMA_UNSUPPORTED(cmJSONState* state);
}
