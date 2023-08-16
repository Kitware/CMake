/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmCMakePresetsErrors.h"
#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

namespace {
using CacheVariable = cmCMakePresetsGraph::CacheVariable;
using ConfigurePreset = cmCMakePresetsGraph::ConfigurePreset;
using BuildPreset = cmCMakePresetsGraph::BuildPreset;
using TestPreset = cmCMakePresetsGraph::TestPreset;
using PackagePreset = cmCMakePresetsGraph::PackagePreset;
using WorkflowPreset = cmCMakePresetsGraph::WorkflowPreset;
using ArchToolsetStrategy = cmCMakePresetsGraph::ArchToolsetStrategy;
using JSONHelperBuilder = cmJSONHelperBuilder;
using ExpandMacroResult = cmCMakePresetsGraphInternal::ExpandMacroResult;
using MacroExpander = cmCMakePresetsGraphInternal::MacroExpander;
using cmCMakePresetsGraphInternal::ExpandMacros;

constexpr int MIN_VERSION = 1;
constexpr int MAX_VERSION = 7;

struct CMakeVersion
{
  unsigned int Major = 0;
  unsigned int Minor = 0;
  unsigned int Patch = 0;
};

struct RootPresets
{
  CMakeVersion CMakeMinimumRequired;
  std::vector<ConfigurePreset> ConfigurePresets;
  std::vector<BuildPreset> BuildPresets;
  std::vector<TestPreset> TestPresets;
  std::vector<PackagePreset> PackagePresets;
  std::vector<WorkflowPreset> WorkflowPresets;
  std::vector<std::string> Include;
};

std::unique_ptr<cmCMakePresetsGraphInternal::NotCondition> InvertCondition(
  std::unique_ptr<cmCMakePresetsGraph::Condition> condition)
{
  auto retval = cm::make_unique<cmCMakePresetsGraphInternal::NotCondition>();
  retval->SubCondition = std::move(condition);
  return retval;
}

auto const ConditionStringHelper = JSONHelperBuilder::String();

auto const ConditionBoolHelper = JSONHelperBuilder::Bool();

auto const ConditionStringListHelper = JSONHelperBuilder::Vector<std::string>(
  cmCMakePresetsErrors::INVALID_CONDITION, ConditionStringHelper);

auto const ConstConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::ConstCondition>(
    cmCMakePresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("value"_s, &cmCMakePresetsGraphInternal::ConstCondition::Value,
          ConditionBoolHelper, true);

auto const EqualsConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::EqualsCondition>(
    cmCMakePresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("lhs"_s, &cmCMakePresetsGraphInternal::EqualsCondition::Lhs,
          ConditionStringHelper, true)
    .Bind("rhs"_s, &cmCMakePresetsGraphInternal::EqualsCondition::Rhs,
          ConditionStringHelper, true);

auto const InListConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::InListCondition>(
    cmCMakePresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmCMakePresetsGraphInternal::InListCondition::String,
          ConditionStringHelper, true)
    .Bind("list"_s, &cmCMakePresetsGraphInternal::InListCondition::List,
          ConditionStringListHelper, true);

auto const MatchesConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::MatchesCondition>(
    cmCMakePresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmCMakePresetsGraphInternal::MatchesCondition::String,
          ConditionStringHelper, true)
    .Bind("regex"_s, &cmCMakePresetsGraphInternal::MatchesCondition::Regex,
          ConditionStringHelper, true);

bool SubConditionHelper(std::unique_ptr<cmCMakePresetsGraph::Condition>& out,
                        const Json::Value* value, cmJSONState* state);

auto const ListConditionVectorHelper =
  JSONHelperBuilder::Vector<std::unique_ptr<cmCMakePresetsGraph::Condition>>(
    cmCMakePresetsErrors::INVALID_CONDITION, SubConditionHelper);
auto const AnyAllOfConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::AnyAllOfCondition>(
    cmCMakePresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("conditions"_s,
          &cmCMakePresetsGraphInternal::AnyAllOfCondition::Conditions,
          ListConditionVectorHelper);

auto const NotConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::NotCondition>(
    cmCMakePresetsErrors::INVALID_CONDITION_OBJECT, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("condition"_s,
          &cmCMakePresetsGraphInternal::NotCondition::SubCondition,
          SubConditionHelper);

bool ConditionHelper(std::unique_ptr<cmCMakePresetsGraph::Condition>& out,
                     const Json::Value* value, cmJSONState* state)
{
  if (!value) {
    out.reset();
    return true;
  }

  if (value->isBool()) {
    auto c = cm::make_unique<cmCMakePresetsGraphInternal::ConstCondition>();
    c->Value = value->asBool();
    out = std::move(c);
    return true;
  }

  if (value->isNull()) {
    out = cm::make_unique<cmCMakePresetsGraphInternal::NullCondition>();
    return true;
  }

  if (value->isObject()) {
    if (!value->isMember("type")) {
      cmCMakePresetsErrors::INVALID_CONDITION(value, state);
      return false;
    }

    if (!(*value)["type"].isString()) {
      cmCMakePresetsErrors::INVALID_CONDITION(value, state);
      return false;
    }
    auto type = (*value)["type"].asString();

    if (type == "const") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::ConstCondition>();
      CHECK_OK(ConstConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }

    if (type == "equals" || type == "notEquals") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::EqualsCondition>();
      CHECK_OK(EqualsConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notEquals") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "inList" || type == "notInList") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::InListCondition>();
      CHECK_OK(InListConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notInList") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "matches" || type == "notMatches") {
      auto c =
        cm::make_unique<cmCMakePresetsGraphInternal::MatchesCondition>();
      CHECK_OK(MatchesConditionHelper(*c, value, state));
      out = std::move(c);
      if (type == "notMatches") {
        out = InvertCondition(std::move(out));
      }
      return true;
    }

    if (type == "anyOf" || type == "allOf") {
      auto c =
        cm::make_unique<cmCMakePresetsGraphInternal::AnyAllOfCondition>();
      c->StopValue = (type == "anyOf");
      CHECK_OK(AnyAllOfConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }

    if (type == "not") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::NotCondition>();
      CHECK_OK(NotConditionHelper(*c, value, state));
      out = std::move(c);
      return true;
    }
  }

  cmCMakePresetsErrors::INVALID_CONDITION(value, state);
  return false;
}

bool SubConditionHelper(std::unique_ptr<cmCMakePresetsGraph::Condition>& out,
                        const Json::Value* value, cmJSONState* state)
{
  std::unique_ptr<cmCMakePresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value, state);
  if (ptr && ptr->IsNull()) {
    cmCMakePresetsErrors::INVALID_CONDITION(value, state);
    return false;
  }
  out = std::move(ptr);
  return result;
}

bool EnvironmentHelper(cm::optional<std::string>& out,
                       const Json::Value* value, cmJSONState* state)
{
  if (!value || value->isNull()) {
    out = cm::nullopt;
    return true;
  }
  if (value->isString()) {
    out = value->asString();
    return true;
  }
  cmCMakePresetsErrors::INVALID_PRESET(value, state);
  return false;
}

auto const VersionIntHelper =
  JSONHelperBuilder::Int(cmCMakePresetsErrors::INVALID_VERSION);

auto const VersionHelper = JSONHelperBuilder::Required<int>(
  cmCMakePresetsErrors::NO_VERSION, VersionIntHelper);

auto const RootVersionHelper =
  JSONHelperBuilder::Object<int>(cmCMakePresetsErrors::INVALID_ROOT_OBJECT)
    .Bind("version"_s, VersionHelper, false);

auto const CMakeVersionUIntHelper =
  JSONHelperBuilder::UInt(cmCMakePresetsErrors::INVALID_VERSION);

auto const CMakeVersionHelper =
  JSONHelperBuilder::Object<CMakeVersion>(JsonErrors::INVALID_NAMED_OBJECT_KEY,
                                          false)
    .Bind("major"_s, &CMakeVersion::Major, CMakeVersionUIntHelper, false)
    .Bind("minor"_s, &CMakeVersion::Minor, CMakeVersionUIntHelper, false)
    .Bind("patch"_s, &CMakeVersion::Patch, CMakeVersionUIntHelper, false);

auto const IncludeHelper =
  JSONHelperBuilder::String(cmCMakePresetsErrors::INVALID_INCLUDE);

auto const IncludeVectorHelper = JSONHelperBuilder::Vector<std::string>(
  cmCMakePresetsErrors::INVALID_INCLUDE, IncludeHelper);

auto const RootPresetsHelper =
  JSONHelperBuilder::Object<RootPresets>(
    cmCMakePresetsErrors::INVALID_ROOT_OBJECT, false)
    .Bind<int>("version"_s, nullptr, VersionHelper)
    .Bind("configurePresets"_s, &RootPresets::ConfigurePresets,
          cmCMakePresetsGraphInternal::ConfigurePresetsHelper, false)
    .Bind("buildPresets"_s, &RootPresets::BuildPresets,
          cmCMakePresetsGraphInternal::BuildPresetsHelper, false)
    .Bind("testPresets"_s, &RootPresets::TestPresets,
          cmCMakePresetsGraphInternal::TestPresetsHelper, false)
    .Bind("packagePresets"_s, &RootPresets::PackagePresets,
          cmCMakePresetsGraphInternal::PackagePresetsHelper, false)
    .Bind("workflowPresets"_s, &RootPresets::WorkflowPresets,
          cmCMakePresetsGraphInternal::WorkflowPresetsHelper, false)
    .Bind("cmakeMinimumRequired"_s, &RootPresets::CMakeMinimumRequired,
          CMakeVersionHelper, false)
    .Bind("include"_s, &RootPresets::Include, IncludeVectorHelper, false)
    .Bind<std::nullptr_t>("vendor"_s, nullptr,
                          cmCMakePresetsGraphInternal::VendorHelper(
                            cmCMakePresetsErrors::INVALID_ROOT),
                          false);
}

namespace cmCMakePresetsGraphInternal {
bool PresetStringHelper(std::string& out, const Json::Value* value,
                        cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::String();
  return helper(out, value, state);
}

bool PresetNameHelper(std::string& out, const Json::Value* value,
                      cmJSONState* state)
{
  if (!value || !value->isString() || value->asString().empty()) {
    cmCMakePresetsErrors::INVALID_PRESET_NAME(value, state);
    return false;
  }
  out = value->asString();
  return true;
}

bool PresetVectorStringHelper(std::vector<std::string>& out,
                              const Json::Value* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<std::string>(
    cmCMakePresetsErrors::INVALID_PRESET,
    cmCMakePresetsGraphInternal::PresetStringHelper);
  return helper(out, value, state);
}

bool PresetBoolHelper(bool& out, const Json::Value* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Bool();
  return helper(out, value, state);
}

bool PresetOptionalBoolHelper(cm::optional<bool>& out,
                              const Json::Value* value, cmJSONState* state)
{
  static auto const helper =
    JSONHelperBuilder::Optional<bool>(PresetBoolHelper);
  return helper(out, value, state);
}

bool PresetIntHelper(int& out, const Json::Value* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Int();
  return helper(out, value, state);
}

bool PresetOptionalIntHelper(cm::optional<int>& out, const Json::Value* value,
                             cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Optional<int>(PresetIntHelper);
  return helper(out, value, state);
}

bool PresetVectorIntHelper(std::vector<int>& out, const Json::Value* value,
                           cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Vector<int>(
    cmCMakePresetsErrors::INVALID_PRESET, PresetIntHelper);
  return helper(out, value, state);
}

cmJSONHelper<std::nullptr_t> VendorHelper(const ErrorGenerator& error)
{
  return [error](std::nullptr_t& /*out*/, const Json::Value* value,
                 cmJSONState* state) -> bool {
    if (!value) {
      return true;
    }

    if (!value->isObject()) {
      error(value, state);
      return false;
    }

    return true;
  };
}

bool PresetConditionHelper(
  std::shared_ptr<cmCMakePresetsGraph::Condition>& out,
  const Json::Value* value, cmJSONState* state)
{
  std::unique_ptr<cmCMakePresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value, state);
  out = std::move(ptr);
  return result;
}

bool PresetVectorOneOrMoreStringHelper(std::vector<std::string>& out,
                                       const Json::Value* value,
                                       cmJSONState* state)
{
  out.clear();
  if (!value) {
    return true;
  }

  if (value->isString()) {
    out.push_back(value->asString());
    return true;
  }

  return PresetVectorStringHelper(out, value, state);
}

bool EnvironmentMapHelper(
  std::map<std::string, cm::optional<std::string>>& out,
  const Json::Value* value, cmJSONState* state)
{
  static auto const helper = JSONHelperBuilder::Map<cm::optional<std::string>>(
    cmCMakePresetsErrors::INVALID_PRESET, EnvironmentHelper);

  return helper(out, value, state);
}
}

bool cmCMakePresetsGraph::ReadJSONFile(const std::string& filename,
                                       RootType rootType,
                                       ReadReason readReason,
                                       std::vector<File*>& inProgressFiles,
                                       File*& file, std::string& errMsg)
{
  bool result;

  for (auto const& f : this->Files) {
    if (cmSystemTools::SameFile(filename, f->Filename)) {
      file = f.get();
      auto fileIt =
        std::find(inProgressFiles.begin(), inProgressFiles.end(), file);
      if (fileIt != inProgressFiles.end()) {
        cmCMakePresetsErrors::CYCLIC_INCLUDE(filename, &this->parseState);
        return false;
      }

      return true;
    }
  }

  Json::Value root;
  this->parseState = cmJSONState(filename, &root);
  if (!this->parseState.errors.empty()) {
    return false;
  }

  int v = 0;
  if ((result = RootVersionHelper(v, &root, &parseState)) != true) {
    return result;
  }
  if (v < MIN_VERSION || v > MAX_VERSION) {
    cmCMakePresetsErrors::UNRECOGNIZED_VERSION(&root["version"],
                                               &this->parseState);
    return false;
  }

  // Support for build and test presets added in version 2.
  if (v < 2) {
    if (root.isMember("buildPresets")) {
      cmCMakePresetsErrors::BUILD_TEST_PRESETS_UNSUPPORTED(
        &root["buildPresets"], &this->parseState);
      return false;
    }
    if (root.isMember("testPresets")) {
      cmCMakePresetsErrors::BUILD_TEST_PRESETS_UNSUPPORTED(
        &root["testPresets"], &this->parseState);
      return false;
    }
  }

  // Support for package presets added in version 6.
  if (v < 6 && root.isMember("packagePresets")) {
    cmCMakePresetsErrors::PACKAGE_PRESETS_UNSUPPORTED(&root["packagePresets"],
                                                      &this->parseState);
    return false;
  }

  // Support for workflow presets added in version 6.
  if (v < 6 && root.isMember("workflowPresets")) {
    cmCMakePresetsErrors::WORKFLOW_PRESETS_UNSUPPORTED(
      &root["workflowPresets"], &this->parseState);
    return false;
  }

  // Support for include added in version 4.
  if (v < 4 && root.isMember("include")) {
    cmCMakePresetsErrors::INCLUDE_UNSUPPORTED(&root["include"],
                                              &this->parseState);
    return false;
  }

  RootPresets presets;
  if ((result = RootPresetsHelper(presets, &root, &parseState)) != true) {
    return result;
  }

  unsigned int currentMajor = cmVersion::GetMajorVersion();
  unsigned int currentMinor = cmVersion::GetMinorVersion();
  unsigned int currentPatch = cmVersion::GetPatchVersion();
  auto const& required = presets.CMakeMinimumRequired;
  if (required.Major > currentMajor) {
    ErrorGenerator error = cmCMakePresetsErrors::UNRECOGNIZED_CMAKE_VERSION(
      "major", currentMajor, required.Major);
    error(&root["cmakeMinimumRequired"]["major"], &this->parseState);
    return false;
  }
  if (required.Major == currentMajor) {
    if (required.Minor > currentMinor) {
      ErrorGenerator error = cmCMakePresetsErrors::UNRECOGNIZED_CMAKE_VERSION(
        "minor", currentMinor, required.Minor);
      error(&root["cmakeMinimumRequired"]["minor"], &this->parseState);
      return false;
    }
    if (required.Minor == currentMinor && required.Patch > currentPatch) {
      ErrorGenerator error = cmCMakePresetsErrors::UNRECOGNIZED_CMAKE_VERSION(
        "patch", currentPatch, required.Patch);
      error(&root["cmakeMinimumRequired"]["patch"], &this->parseState);
      return false;
    }
  }

  auto filePtr = cm::make_unique<File>();
  file = filePtr.get();
  this->Files.emplace_back(std::move(filePtr));
  inProgressFiles.emplace_back(file);
  file->Filename = filename;
  file->Version = v;
  file->ReachableFiles.insert(file);

  for (auto& preset : presets.ConfigurePresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      // No error, already handled by PresetNameHelper
      return false;
    }

    PresetPair<ConfigurePreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->ConfigurePresets.emplace(preset.Name, presetPair).second) {
      cmCMakePresetsErrors::DUPLICATE_PRESETS(preset.Name, &this->parseState);
      return false;
    }

    // Support for installDir presets added in version 3.
    if (v < 3 && !preset.InstallDir.empty()) {
      cmCMakePresetsErrors::INSTALL_PREFIX_UNSUPPORTED(&root["installDir"],
                                                       &this->parseState);
      return false;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmCMakePresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for toolchainFile presets added in version 3.
    if (v < 3 && !preset.ToolchainFile.empty()) {
      cmCMakePresetsErrors::TOOLCHAIN_FILE_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for trace presets added in version 7.
    if (v < 7 &&
        (preset.TraceMode.has_value() || preset.TraceFormat.has_value() ||
         !preset.TraceRedirect.empty() || !preset.TraceSource.empty())) {
      cmCMakePresetsErrors::TRACE_UNSUPPORTED(&this->parseState);
      return false;
    }

    this->ConfigurePresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.BuildPresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      // No error, already handled by PresetNameHelper
      return false;
    }

    PresetPair<BuildPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->BuildPresets.emplace(preset.Name, presetPair).second) {
      cmCMakePresetsErrors::DUPLICATE_PRESETS(preset.Name, &this->parseState);
      return false;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmCMakePresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    this->BuildPresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.TestPresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      // No error, already handled by PresetNameHelper
      return false;
    }

    PresetPair<TestPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->TestPresets.emplace(preset.Name, presetPair).second) {
      cmCMakePresetsErrors::DUPLICATE_PRESETS(preset.Name, &this->parseState);
      return false;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      cmCMakePresetsErrors::CONDITION_UNSUPPORTED(&this->parseState);
      return false;
    }

    // Support for TestOutputTruncation added in version 5.
    if (v < 5 && preset.Output && preset.Output->TestOutputTruncation) {
      cmCMakePresetsErrors::TEST_OUTPUT_TRUNCATION_UNSUPPORTED(
        &this->parseState);
      return false;
    }

    // Support for outputJUnitFile added in version 6.
    if (v < 6 && preset.Output && !preset.Output->OutputJUnitFile.empty()) {
      cmCMakePresetsErrors::CTEST_JUNIT_UNSUPPORTED(&this->parseState);
      return false;
    }

    this->TestPresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.PackagePresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      // No error, already handled by PresetNameHelper
      return false;
    }

    PresetPair<PackagePreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->PackagePresets.emplace(preset.Name, presetPair).second) {
      cmCMakePresetsErrors::DUPLICATE_PRESETS(preset.Name, &this->parseState);
      return false;
    }

    // Support for conditions added in version 3, but this requires version 5
    // already, so no action needed.

    this->PackagePresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.WorkflowPresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      // No error, already handled by PresetNameHelper
      return false;
    }

    PresetPair<WorkflowPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->WorkflowPresets.emplace(preset.Name, presetPair).second) {
      cmCMakePresetsErrors::DUPLICATE_PRESETS(preset.Name, &this->parseState);
      return false;
    }

    // Support for conditions added in version 3, but this requires version 6
    // already, so no action needed.

    this->WorkflowPresetOrder.push_back(preset.Name);
  }

  auto const includeFile = [this, &inProgressFiles,
                            file](const std::string& include,
                                  RootType rootType2, ReadReason readReason2,
                                  std::string& FailureMessage) -> bool {
    bool r;
    File* includedFile;
    if ((r =
           this->ReadJSONFile(include, rootType2, readReason2, inProgressFiles,
                              includedFile, FailureMessage)) != true) {
      return r;
    }

    file->ReachableFiles.insert(includedFile->ReachableFiles.begin(),
                                includedFile->ReachableFiles.end());
    return true;
  };

  std::vector<MacroExpander> macroExpanders;

  MacroExpander environmentMacroExpander =
    [](const std::string& macroNamespace, const std::string& macroName,
       std::string& expanded, int /*version*/) -> ExpandMacroResult {
    if (macroNamespace == "penv") {
      if (macroName.empty()) {
        return ExpandMacroResult::Error;
      }
      if (cm::optional<std::string> value =
            cmSystemTools::GetEnvVar(macroName)) {
        expanded += *value;
      }
      return ExpandMacroResult::Ok;
    }

    return ExpandMacroResult::Ignore;
  };

  macroExpanders.push_back(environmentMacroExpander);

  for (Json::ArrayIndex i = 0; i < presets.Include.size(); ++i) {
    auto include = presets.Include[i];

    // Support for macro expansion in includes added in version 7
    if (v >= 7) {
      if (ExpandMacros(include, macroExpanders, v) != ExpandMacroResult::Ok) {
        cmCMakePresetsErrors::INVALID_INCLUDE(&root["include"][i],
                                              &this->parseState);
        return false;
      }
    }

    if (!cmSystemTools::FileIsFullPath(include)) {
      auto directory = cmSystemTools::GetFilenamePath(filename);
      include = cmStrCat(directory, '/', include);
    }

    if ((result = includeFile(include, rootType, ReadReason::Included,
                              errMsg)) != true) {
      return result;
    }
  }

  if (rootType == RootType::User && readReason == ReadReason::Root) {
    auto cmakePresetsFilename = GetFilename(this->SourceDir);
    if (cmSystemTools::FileExists(cmakePresetsFilename)) {
      if ((result = includeFile(cmakePresetsFilename, RootType::Project,
                                ReadReason::Root, errMsg)) != true) {
        return result;
      }
    }
  }

  inProgressFiles.pop_back();
  return true;
}
