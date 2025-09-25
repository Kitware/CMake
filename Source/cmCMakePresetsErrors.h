/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"

class cmJSONState;

namespace cmCMakePresetsErrors {
Json::Value const* getPreset(cmJSONState* state);

std::string getPresetName(cmJSONState* state);

std::string getVariableName(cmJSONState* state);

void FILE_NOT_FOUND(std::string const& filename, cmJSONState* state);

void INVALID_ROOT(Json::Value const* value, cmJSONState* state);

void NO_VERSION(Json::Value const* value, cmJSONState* state);

void INVALID_VERSION(Json::Value const* value, cmJSONState* state);

void UNRECOGNIZED_VERSION(Json::Value const* value, cmJSONState* state);

void INVALID_PRESETS(Json::Value const* value, cmJSONState* state);

void INVALID_PRESET(Json::Value const* value, cmJSONState* state);

void INVALID_PRESET_NAMED(std::string const& presetName, cmJSONState* state);

void INVALID_VARIABLE(Json::Value const* value, cmJSONState* state);

void DUPLICATE_PRESETS(std::string const& presetName, cmJSONState* state);

void CYCLIC_PRESET_INHERITANCE(std::string const& presetName,
                               cmJSONState* state);

void INHERITED_PRESET_UNREACHABLE_FROM_FILE(std::string const& presetName,
                                            cmJSONState* state);

void CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(std::string const& presetName,
                                            cmJSONState* state);

void INVALID_MACRO_EXPANSION(std::string const& presetName,
                             cmJSONState* state);

void BUILD_TEST_PRESETS_UNSUPPORTED(Json::Value const*, cmJSONState* state);

void PACKAGE_PRESETS_UNSUPPORTED(Json::Value const*, cmJSONState* state);

void WORKFLOW_PRESETS_UNSUPPORTED(Json::Value const*, cmJSONState* state);

void INCLUDE_UNSUPPORTED(Json::Value const*, cmJSONState* state);

void INVALID_INCLUDE(Json::Value const* value, cmJSONState* state);

void INVALID_CONFIGURE_PRESET(std::string const& presetName,
                              cmJSONState* state);

void INSTALL_PREFIX_UNSUPPORTED(Json::Value const* value, cmJSONState* state);

void CONDITION_UNSUPPORTED(cmJSONState* state);

void TOOLCHAIN_FILE_UNSUPPORTED(cmJSONState* state);

void GRAPHVIZ_FILE_UNSUPPORTED(cmJSONState* state);

void CYCLIC_INCLUDE(std::string const& file, cmJSONState* state);

void TEST_OUTPUT_TRUNCATION_UNSUPPORTED(cmJSONState* state);

void INVALID_WORKFLOW_STEPS(std::string const& workflowStep,
                            cmJSONState* state);

void NO_WORKFLOW_STEPS(std::string const& presetName, cmJSONState* state);

void FIRST_WORKFLOW_STEP_NOT_CONFIGURE(std::string const& stepName,
                                       cmJSONState* state);

void CONFIGURE_WORKFLOW_STEP_NOT_FIRST(std::string const& stepName,
                                       cmJSONState* state);

void WORKFLOW_STEP_UNREACHABLE_FROM_FILE(std::string const& workflowStep,
                                         cmJSONState* state);

void CTEST_JUNIT_UNSUPPORTED(cmJSONState* state);

void TRACE_UNSUPPORTED(cmJSONState* state);

JsonErrors::ErrorGenerator UNRECOGNIZED_VERSION_RANGE(int min, int max);

JsonErrors::ErrorGenerator UNRECOGNIZED_CMAKE_VERSION(
  std::string const& version, int current, int required);

void INVALID_PRESET_NAME(Json::Value const* value, cmJSONState* state);

void INVALID_CONDITION(Json::Value const* value, cmJSONState* state);

JsonErrors::ErrorGenerator INVALID_CONDITION_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields);

JsonErrors::ErrorGenerator INVALID_VARIABLE_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields);

JsonErrors::ErrorGenerator INVALID_PRESET_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields);

JsonErrors::ErrorGenerator INVALID_ROOT_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields);

void PRESET_MISSING_FIELD(std::string const& presetName,
                          std::string const& missingField, cmJSONState* state);

void SCHEMA_UNSUPPORTED(cmJSONState* state);
}
