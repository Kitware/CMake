/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCMakePresetsErrors.h"

#include <functional>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"

namespace cmCMakePresetsErrors {
namespace {
std::string CapitalizePresetKind(std::string kind)
{
  if (!kind.empty() && kind.front() >= 'a' && kind.front() <= 'z') {
    kind.front() = static_cast<char>(kind.front() - 'a' + 'A');
  }
  return kind;
}
}

Json::Value const* getPreset(cmJSONState* state)
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
  Json::Value const* preset = getPreset(state);
  if (preset && preset->isMember("name")) {
    Json::Value const& name = preset->operator[]("name");
    if (name.isConvertibleTo(Json::ValueType::stringValue)) {
      return name.asString();
    }
  }
  return "";
}

std::string getVariableName(cmJSONState* state)
{
  std::string var = state->key_after("cacheVariables");
  if (var.empty()) {
    var = state->key_after("variables");
  }
  std::string errMsg =
    var.empty() ? "variable" : cmStrCat("variable \""_s, var, '"');
  errMsg =
    cmStrCat(std::move(errMsg), " for preset \""_s, getPresetName(state), '"');
  return errMsg;
}

void FILE_NOT_FOUND(std::string const& filename, cmJSONState* state)
{
  state->AddError(cmStrCat("File not found: "_s, filename));
}

void INVALID_ROOT(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid root object", value);
}

void NO_VERSION(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue("No \"version\" field", value);
}

void INVALID_VERSION(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid \"version\" field", value);
}

void UNRECOGNIZED_VERSION(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue("Unrecognized \"version\" field", value);
}

void INVALID_PRESETS(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue(cmStrCat("Invalid \""_s, state->key(), "\" field"_s),
                         value);
}

void INVALID_PRESET(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid preset", value);
}

void INVALID_PRESET_NAMED(std::string const& presetName,
                          std::string const& kind, cmJSONState* state,
                          std::string const& detail)
{
  std::string err_msg =
    cmStrCat("Invalid "_s, kind, " preset: \""_s, presetName, '"');
  if (!detail.empty()) {
    err_msg = cmStrCat(std::move(err_msg), ": "_s, detail);
  }
  state->AddError(err_msg);
}

void INVALID_VARIABLE(Json::Value const* value, cmJSONState* state)
{
  std::string var = cmCMakePresetsErrors::getVariableName(state);
  state->AddErrorAtValue(cmStrCat("Invalid CMake "_s, var), value);
}

void DUPLICATE_PRESETS(std::string const& presetName, cmJSONState* state)
{
  state->AddError(cmStrCat("Duplicate preset: \""_s, presetName, '"'));
}

void CYCLIC_PRESET_INHERITANCE(std::string const& presetName,
                               std::string const& kind, cmJSONState* state)

{
  state->AddError(cmStrCat("Cyclic preset inheritance for "_s, kind,
                           " preset \""_s, presetName, '"'));
}

void INHERITED_PRESET_UNREACHABLE_FROM_FILE(std::string const& presetName,
                                            std::string const& inheritedPreset,
                                            std::string const& kind,
                                            cmJSONState* state)
{
  state->AddError(cmStrCat(CapitalizePresetKind(kind), " preset \""_s,
                           presetName, "\" inherits preset \""_s,
                           inheritedPreset, "\", but \""_s, inheritedPreset,
                           "\" is not reachable from the preset's file"_s));
}

void CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(std::string const& presetName,
                                            std::string const& kind,
                                            std::string const& configurePreset,
                                            cmJSONState* state)
{
  state->AddError(cmStrCat(CapitalizePresetKind(kind), " preset \""_s,
                           presetName, "\" references configure preset \""_s,
                           configurePreset, "\", but \""_s, configurePreset,
                           "\" is not reachable from the preset's file"_s));
}

void INVALID_MACRO_EXPANSION(std::string const& presetName, cmJSONState* state)
{
  state->AddError(
    cmStrCat("Invalid macro expansion in \""_s, presetName, '"'));
}

void BUILD_TEST_PRESETS_UNSUPPORTED(Json::Value const*, cmJSONState* state)
{
  state->AddError("File version must be 2 or higher for build and test preset "
                  "support");
}

void PACKAGE_PRESETS_UNSUPPORTED(Json::Value const*, cmJSONState* state)
{
  state->AddError(
    "File version must be 6 or higher for package preset support");
}

void WORKFLOW_PRESETS_UNSUPPORTED(Json::Value const*, cmJSONState* state)
{
  state->AddError(
    "File version must be 6 or higher for workflow preset support");
}

void INCLUDE_UNSUPPORTED(Json::Value const*, cmJSONState* state)
{
  state->AddError("File version must be 4 or higher for include support");
}

void INVALID_INCLUDE(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue("Invalid \"include\" field", value);
}

void CONFIGURE_PRESET_NOT_FOUND(std::string const& presetName,
                                std::string const& kind,
                                std::string const& configurePreset,
                                cmJSONState* state)
{
  state->AddError(cmStrCat(
    CapitalizePresetKind(kind), " preset \""_s, presetName,
    "\" references unknown configure preset \""_s, configurePreset, '"'));
}

void INSTALL_PREFIX_UNSUPPORTED(Json::Value const* value, cmJSONState* state)
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

void GRAPHVIZ_FILE_UNSUPPORTED(cmJSONState* state)
{
  state->AddError(
    "File version must be 10 or higher for graphviz preset support");
}

void JOBS_PROC_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 11 or higher for "
                  "processor-count-based jobs preset support");
}

void PASSTHROUGH_ARGS_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 12 or higher for "
                  "testPassthroughArguments preset support");
}

void CYCLIC_INCLUDE(std::string const& file, cmJSONState* state)
{
  state->AddError(cmStrCat("Cyclic include among presets files: "_s, file));
}

void TEST_OUTPUT_TRUNCATION_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 5 or higher for testOutputTruncation "
                  "preset support");
}

namespace {
std::string WorkflowStepLabel(cm::string_view stepType,
                              std::string const& stepName)
{
  return cmStrCat("of type \""_s, stepType, "\" named \""_s, stepName, '"');
}
}

void INVALID_WORKFLOW_STEPS(cm::string_view stepType,
                            std::string const& stepName, cmJSONState* state)
{
  state->AddError(cmStrCat("Invalid workflow step "_s,
                           WorkflowStepLabel(stepType, stepName)));
}

void WORKFLOW_STEP_CONFIGURE_PRESET_MISMATCH(
  std::string const& kind, std::string const& workflowStep,
  std::string const& stepConfigurePreset,
  std::string const& workflowConfigurePreset, cmJSONState* state)
{
  state->AddError(cmStrCat(CapitalizePresetKind(kind), " workflow step \""_s,
                           workflowStep, "\" uses configure preset \""_s,
                           stepConfigurePreset,
                           "\", but workflow configure preset is \""_s,
                           workflowConfigurePreset, '"'));
}

void INVALID_TEST_FILTER_INCLUDE_INDEX(Json::Value const* value,
                                       cmJSONState* state)
{
  state->AddErrorAtValue(
    "Invalid \"filter.include.index\" field: expected a string or object",
    value);
}

void NO_WORKFLOW_STEPS(std::string const& presetName, cmJSONState* state)
{
  state->AddError(
    cmStrCat("No workflow steps specified for \""_s, presetName, '"'));
}

void FIRST_WORKFLOW_STEP_NOT_CONFIGURE(cm::string_view stepType,
                                       std::string const& stepName,
                                       cmJSONState* state)
{
  state->AddError(cmStrCat("First workflow step "_s,
                           WorkflowStepLabel(stepType, stepName),
                           " must be a configure step"_s));
}

void CONFIGURE_WORKFLOW_STEP_NOT_FIRST(cm::string_view stepType,
                                       std::string const& stepName,
                                       cmJSONState* state)
{
  state->AddError(cmStrCat("Workflow step "_s,
                           WorkflowStepLabel(stepType, stepName),
                           " must be the first step"_s));
}

void WORKFLOW_STEP_UNREACHABLE_FROM_FILE(cm::string_view stepType,
                                         std::string const& stepName,
                                         cmJSONState* state)
{
  state->AddError(cmStrCat("Workflow step "_s,
                           WorkflowStepLabel(stepType, stepName),
                           " is unreachable from preset's file"_s));
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

JsonErrors::ErrorGenerator UNRECOGNIZED_VERSION_RANGE(int min, int max)
{
  return [min, max](Json::Value const* value, cmJSONState* state) -> void {
    state->AddErrorAtValue(cmStrCat("Unrecognized \"version\" "_s,
                                    value->asString(), ": must be >="_s, min,
                                    " and <="_s, max),
                           value);
  };
}

JsonErrors::ErrorGenerator UNRECOGNIZED_CMAKE_VERSION(
  std::string const& version, int current, int required)
{
  return [version, current, required](Json::Value const* value,
                                      cmJSONState* state) -> void {
    state->AddErrorAtValue(cmStrCat("\"cmakeMinimumRequired\" "_s, version,
                                    " version "_s, required,
                                    " must be less than "_s, current),
                           value);
  };
}

void INVALID_PRESET_NAME(Json::Value const* value, cmJSONState* state)
{
  std::string errMsg = "Invalid Preset Name";
  if (value && value->isConvertibleTo(Json::ValueType::stringValue) &&
      !value->asString().empty()) {
    errMsg = cmStrCat(std::move(errMsg), ": "_s, value->asString());
  }
  state->AddErrorAtValue(errMsg, value);
}

void INVALID_CONDITION(Json::Value const* value, cmJSONState* state)
{
  state->AddErrorAtValue(
    cmStrCat("Invalid condition for preset \""_s, getPresetName(state), '"'),
    value);
}

JsonErrors::ErrorGenerator INVALID_CONDITION_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState* state) -> std::string {
      return cmStrCat(" condition for preset \""_s, getPresetName(state), '"');
    })(errorType, extraFields);
}

JsonErrors::ErrorGenerator INVALID_VARIABLE_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState* state) -> std::string {
      return getVariableName(state);
    })(errorType, extraFields);
}

JsonErrors::ErrorGenerator INVALID_PRESET_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState*) -> std::string { return "Preset"; })(
    errorType, extraFields);
}

JsonErrors::ErrorGenerator INVALID_ROOT_OBJECT(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState*) -> std::string {
      return "root object";
    })(errorType, extraFields);
}

void PRESET_MISSING_FIELD(std::string const& presetName,
                          std::string const& missingField, cmJSONState* state)
{
  state->AddError(cmStrCat("Preset \""_s, presetName, "\" missing field \""_s,
                           missingField, '"'));
}

void SCHEMA_UNSUPPORTED(cmJSONState* state)
{
  state->AddError("File version must be 8 or higher for $schema support");
}

void DIAGNOSTIC_UNSUPPORTED(cm::string_view name, cm::string_view context,
                            int version, cmJSONState* state)
{
  state->AddError(cmStrCat("File version must be "_s, std::to_string(version),
                           " or higher for "_s, context, '.', name,
                           " support"_s));
}

void DIAGNOSTIC_REMOVED(cm::string_view name, cm::string_view context,
                        int version, cmJSONState* state)
{
  state->AddError(cmStrCat("File version must be "_s, std::to_string(version),
                           " or lower for "_s, context, '.', name,
                           " support"_s));
}
}
