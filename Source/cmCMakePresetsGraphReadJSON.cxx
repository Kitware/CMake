/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"

#include "cmCMakePresetsGraph.h"
#include "cmCMakePresetsGraphInternal.h"
#include "cmJSONHelpers.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

namespace {
using ReadFileResult = cmCMakePresetsGraph::ReadFileResult;
using CacheVariable = cmCMakePresetsGraph::CacheVariable;
using ConfigurePreset = cmCMakePresetsGraph::ConfigurePreset;
using BuildPreset = cmCMakePresetsGraph::BuildPreset;
using TestPreset = cmCMakePresetsGraph::TestPreset;
using ArchToolsetStrategy = cmCMakePresetsGraph::ArchToolsetStrategy;
using JSONHelperBuilder = cmJSONHelperBuilder<ReadFileResult>;

constexpr int MIN_VERSION = 1;
constexpr int MAX_VERSION = 5;

struct CMakeVersion
{
  unsigned int Major = 0;
  unsigned int Minor = 0;
  unsigned int Patch = 0;
};

struct RootPresets
{
  CMakeVersion CMakeMinimumRequired;
  std::vector<cmCMakePresetsGraph::ConfigurePreset> ConfigurePresets;
  std::vector<cmCMakePresetsGraph::BuildPreset> BuildPresets;
  std::vector<cmCMakePresetsGraph::TestPreset> TestPresets;
  std::vector<std::string> Include;
};

std::unique_ptr<cmCMakePresetsGraphInternal::NotCondition> InvertCondition(
  std::unique_ptr<cmCMakePresetsGraph::Condition> condition)
{
  auto retval = cm::make_unique<cmCMakePresetsGraphInternal::NotCondition>();
  retval->SubCondition = std::move(condition);
  return retval;
}

auto const ConditionStringHelper = JSONHelperBuilder::String(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION);

auto const ConditionBoolHelper = JSONHelperBuilder::Bool(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION);

auto const ConditionStringListHelper = JSONHelperBuilder::Vector<std::string>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION,
  ConditionStringHelper);

auto const ConstConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::ConstCondition>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("value"_s, &cmCMakePresetsGraphInternal::ConstCondition::Value,
          ConditionBoolHelper, true);

auto const EqualsConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::EqualsCondition>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("lhs"_s, &cmCMakePresetsGraphInternal::EqualsCondition::Lhs,
          ConditionStringHelper, true)
    .Bind("rhs"_s, &cmCMakePresetsGraphInternal::EqualsCondition::Rhs,
          ConditionStringHelper, true);

auto const InListConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::InListCondition>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmCMakePresetsGraphInternal::InListCondition::String,
          ConditionStringHelper, true)
    .Bind("list"_s, &cmCMakePresetsGraphInternal::InListCondition::List,
          ConditionStringListHelper, true);

auto const MatchesConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::MatchesCondition>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("string"_s, &cmCMakePresetsGraphInternal::MatchesCondition::String,
          ConditionStringHelper, true)
    .Bind("regex"_s, &cmCMakePresetsGraphInternal::MatchesCondition::Regex,
          ConditionStringHelper, true);

ReadFileResult SubConditionHelper(
  std::unique_ptr<cmCMakePresetsGraph::Condition>& out,
  const Json::Value* value);

auto const ListConditionVectorHelper =
  JSONHelperBuilder::Vector<std::unique_ptr<cmCMakePresetsGraph::Condition>>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION,
    SubConditionHelper);
auto const AnyAllOfConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::AnyAllOfCondition>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("conditions"_s,
          &cmCMakePresetsGraphInternal::AnyAllOfCondition::Conditions,
          ListConditionVectorHelper);

auto const NotConditionHelper =
  JSONHelperBuilder::Object<cmCMakePresetsGraphInternal::NotCondition>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CONDITION, false)
    .Bind<std::string>("type"_s, nullptr, ConditionStringHelper, true)
    .Bind("condition"_s,
          &cmCMakePresetsGraphInternal::NotCondition::SubCondition,
          SubConditionHelper);

ReadFileResult ConditionHelper(
  std::unique_ptr<cmCMakePresetsGraph::Condition>& out,
  const Json::Value* value)
{
  if (!value) {
    out.reset();
    return ReadFileResult::READ_OK;
  }

  if (value->isBool()) {
    auto c = cm::make_unique<cmCMakePresetsGraphInternal::ConstCondition>();
    c->Value = value->asBool();
    out = std::move(c);
    return ReadFileResult::READ_OK;
  }

  if (value->isNull()) {
    out = cm::make_unique<cmCMakePresetsGraphInternal::NullCondition>();
    return ReadFileResult::READ_OK;
  }

  if (value->isObject()) {
    if (!value->isMember("type")) {
      return ReadFileResult::INVALID_CONDITION;
    }

    if (!(*value)["type"].isString()) {
      return ReadFileResult::INVALID_CONDITION;
    }
    auto type = (*value)["type"].asString();

    if (type == "const") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::ConstCondition>();
      CHECK_OK(ConstConditionHelper(*c, value));
      out = std::move(c);
      return ReadFileResult::READ_OK;
    }

    if (type == "equals" || type == "notEquals") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::EqualsCondition>();
      CHECK_OK(EqualsConditionHelper(*c, value));
      out = std::move(c);
      if (type == "notEquals") {
        out = InvertCondition(std::move(out));
      }
      return ReadFileResult::READ_OK;
    }

    if (type == "inList" || type == "notInList") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::InListCondition>();
      CHECK_OK(InListConditionHelper(*c, value));
      out = std::move(c);
      if (type == "notInList") {
        out = InvertCondition(std::move(out));
      }
      return ReadFileResult::READ_OK;
    }

    if (type == "matches" || type == "notMatches") {
      auto c =
        cm::make_unique<cmCMakePresetsGraphInternal::MatchesCondition>();
      CHECK_OK(MatchesConditionHelper(*c, value));
      out = std::move(c);
      if (type == "notMatches") {
        out = InvertCondition(std::move(out));
      }
      return ReadFileResult::READ_OK;
    }

    if (type == "anyOf" || type == "allOf") {
      auto c =
        cm::make_unique<cmCMakePresetsGraphInternal::AnyAllOfCondition>();
      c->StopValue = (type == "anyOf");
      CHECK_OK(AnyAllOfConditionHelper(*c, value));
      out = std::move(c);
      return ReadFileResult::READ_OK;
    }

    if (type == "not") {
      auto c = cm::make_unique<cmCMakePresetsGraphInternal::NotCondition>();
      CHECK_OK(NotConditionHelper(*c, value));
      out = std::move(c);
      return ReadFileResult::READ_OK;
    }
  }

  return ReadFileResult::INVALID_CONDITION;
}

ReadFileResult SubConditionHelper(
  std::unique_ptr<cmCMakePresetsGraph::Condition>& out,
  const Json::Value* value)
{
  std::unique_ptr<cmCMakePresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value);
  if (ptr && ptr->IsNull()) {
    return ReadFileResult::INVALID_CONDITION;
  }
  out = std::move(ptr);
  return result;
}

ReadFileResult EnvironmentHelper(cm::optional<std::string>& out,
                                 const Json::Value* value)
{
  if (!value || value->isNull()) {
    out = cm::nullopt;
    return ReadFileResult::READ_OK;
  }
  if (value->isString()) {
    out = value->asString();
    return ReadFileResult::READ_OK;
  }
  return ReadFileResult::INVALID_PRESET;
}

auto const VersionIntHelper = JSONHelperBuilder::Int(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_VERSION);

auto const VersionHelper = JSONHelperBuilder::Required<int>(
  ReadFileResult::NO_VERSION, VersionIntHelper);

auto const RootVersionHelper =
  JSONHelperBuilder::Object<int>(ReadFileResult::READ_OK,
                                 ReadFileResult::INVALID_ROOT)
    .Bind("version"_s, VersionHelper, false);

auto const CMakeVersionUIntHelper = JSONHelperBuilder::UInt(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_VERSION);

auto const CMakeVersionHelper =
  JSONHelperBuilder::Object<CMakeVersion>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_CMAKE_VERSION, false)
    .Bind("major"_s, &CMakeVersion::Major, CMakeVersionUIntHelper, false)
    .Bind("minor"_s, &CMakeVersion::Minor, CMakeVersionUIntHelper, false)
    .Bind("patch"_s, &CMakeVersion::Patch, CMakeVersionUIntHelper, false);

auto const IncludeHelper = JSONHelperBuilder::String(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_INCLUDE);

auto const IncludeVectorHelper = JSONHelperBuilder::Vector<std::string>(
  ReadFileResult::READ_OK, ReadFileResult::INVALID_INCLUDE, IncludeHelper);

auto const RootPresetsHelper =
  JSONHelperBuilder::Object<RootPresets>(ReadFileResult::READ_OK,
                                         ReadFileResult::INVALID_ROOT, false)
    .Bind<int>("version"_s, nullptr, VersionHelper)
    .Bind("configurePresets"_s, &RootPresets::ConfigurePresets,
          cmCMakePresetsGraphInternal::ConfigurePresetsHelper, false)
    .Bind("buildPresets"_s, &RootPresets::BuildPresets,
          cmCMakePresetsGraphInternal::BuildPresetsHelper, false)
    .Bind("testPresets"_s, &RootPresets::TestPresets,
          cmCMakePresetsGraphInternal::TestPresetsHelper, false)
    .Bind("cmakeMinimumRequired"_s, &RootPresets::CMakeMinimumRequired,
          CMakeVersionHelper, false)
    .Bind("include"_s, &RootPresets::Include, IncludeVectorHelper, false)
    .Bind<std::nullptr_t>(
      "vendor"_s, nullptr,
      cmCMakePresetsGraphInternal::VendorHelper(ReadFileResult::INVALID_ROOT),
      false);
}

namespace cmCMakePresetsGraphInternal {
cmCMakePresetsGraph::ReadFileResult PresetStringHelper(
  std::string& out, const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::String(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET);

  return helper(out, value);
}

cmCMakePresetsGraph::ReadFileResult PresetVectorStringHelper(
  std::vector<std::string>& out, const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Vector<std::string>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET,
    cmCMakePresetsGraphInternal::PresetStringHelper);

  return helper(out, value);
}

cmCMakePresetsGraph::ReadFileResult PresetBoolHelper(bool& out,
                                                     const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Bool(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET);

  return helper(out, value);
}

cmCMakePresetsGraph::ReadFileResult PresetOptionalBoolHelper(
  cm::optional<bool>& out, const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Optional<bool>(
    ReadFileResult::READ_OK, PresetBoolHelper);

  return helper(out, value);
}

cmCMakePresetsGraph::ReadFileResult PresetIntHelper(int& out,
                                                    const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Int(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET);

  return helper(out, value);
}

cmCMakePresetsGraph::ReadFileResult PresetOptionalIntHelper(
  cm::optional<int>& out, const Json::Value* value)
{
  static auto const helper =
    JSONHelperBuilder::Optional<int>(ReadFileResult::READ_OK, PresetIntHelper);

  return helper(out, value);
}

cmCMakePresetsGraph::ReadFileResult PresetVectorIntHelper(
  std::vector<int>& out, const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Vector<int>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET, PresetIntHelper);

  return helper(out, value);
}

cmJSONHelper<std::nullptr_t, ReadFileResult> VendorHelper(ReadFileResult error)
{
  return [error](std::nullptr_t& /*out*/,
                 const Json::Value* value) -> ReadFileResult {
    if (!value) {
      return ReadFileResult::READ_OK;
    }

    if (!value->isObject()) {
      return error;
    }

    return ReadFileResult::READ_OK;
  };
}

ReadFileResult PresetConditionHelper(
  std::shared_ptr<cmCMakePresetsGraph::Condition>& out,
  const Json::Value* value)
{
  std::unique_ptr<cmCMakePresetsGraph::Condition> ptr;
  auto result = ConditionHelper(ptr, value);
  out = std::move(ptr);
  return result;
}

ReadFileResult PresetVectorOneOrMoreStringHelper(std::vector<std::string>& out,
                                                 const Json::Value* value)
{
  out.clear();
  if (!value) {
    return ReadFileResult::READ_OK;
  }

  if (value->isString()) {
    out.push_back(value->asString());
    return ReadFileResult::READ_OK;
  }

  return PresetVectorStringHelper(out, value);
}

cmCMakePresetsGraph::ReadFileResult EnvironmentMapHelper(
  std::map<std::string, cm::optional<std::string>>& out,
  const Json::Value* value)
{
  static auto const helper = JSONHelperBuilder::Map<cm::optional<std::string>>(
    ReadFileResult::READ_OK, ReadFileResult::INVALID_PRESET,
    EnvironmentHelper);

  return helper(out, value);
}
}

cmCMakePresetsGraph::ReadFileResult cmCMakePresetsGraph::ReadJSONFile(
  const std::string& filename, RootType rootType, ReadReason readReason,
  std::vector<File*>& inProgressFiles, File*& file)
{
  ReadFileResult result;

  for (auto const& f : this->Files) {
    if (cmSystemTools::SameFile(filename, f->Filename)) {
      file = f.get();
      auto fileIt =
        std::find(inProgressFiles.begin(), inProgressFiles.end(), file);
      if (fileIt != inProgressFiles.end()) {
        return cmCMakePresetsGraph::ReadFileResult::CYCLIC_INCLUDE;
      }

      return cmCMakePresetsGraph::ReadFileResult::READ_OK;
    }
  }

  cmsys::ifstream fin(filename.c_str());
  if (!fin) {
    return ReadFileResult::FILE_NOT_FOUND;
  }
  // If there's a BOM, toss it.
  cmsys::FStream::ReadBOM(fin);

  Json::Value root;
  Json::CharReaderBuilder builder;
  Json::CharReaderBuilder::strictMode(&builder.settings_);
  if (!Json::parseFromStream(builder, fin, &root, nullptr)) {
    return ReadFileResult::JSON_PARSE_ERROR;
  }

  int v = 0;
  if ((result = RootVersionHelper(v, &root)) != ReadFileResult::READ_OK) {
    return result;
  }
  if (v < MIN_VERSION || v > MAX_VERSION) {
    return ReadFileResult::UNRECOGNIZED_VERSION;
  }

  // Support for build and test presets added in version 2.
  if (v < 2 &&
      (root.isMember("buildPresets") || root.isMember("testPresets"))) {
    return ReadFileResult::BUILD_TEST_PRESETS_UNSUPPORTED;
  }

  // Support for include added in version 4.
  if (v < 4 && root.isMember("include")) {
    return ReadFileResult::INCLUDE_UNSUPPORTED;
  }

  RootPresets presets;
  if ((result = RootPresetsHelper(presets, &root)) !=
      ReadFileResult::READ_OK) {
    return result;
  }

  unsigned int currentMajor = cmVersion::GetMajorVersion();
  unsigned int currentMinor = cmVersion::GetMinorVersion();
  unsigned int currentPatch = cmVersion::GetPatchVersion();
  auto const& required = presets.CMakeMinimumRequired;
  if (required.Major > currentMajor ||
      (required.Major == currentMajor &&
       (required.Minor > currentMinor ||
        (required.Minor == currentMinor &&
         (required.Patch > currentPatch))))) {
    return ReadFileResult::UNRECOGNIZED_CMAKE_VERSION;
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
      return ReadFileResult::INVALID_PRESET;
    }

    PresetPair<ConfigurePreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->ConfigurePresets
           .emplace(std::make_pair(preset.Name, presetPair))
           .second) {
      return ReadFileResult::DUPLICATE_PRESETS;
    }

    // Support for installDir presets added in version 3.
    if (v < 3 && !preset.InstallDir.empty()) {
      return ReadFileResult::INSTALL_PREFIX_UNSUPPORTED;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      return ReadFileResult::CONDITION_UNSUPPORTED;
    }

    // Support for toolchainFile presets added in version 3.
    if (v < 3 && !preset.ToolchainFile.empty()) {
      return ReadFileResult::TOOLCHAIN_FILE_UNSUPPORTED;
    }

    this->ConfigurePresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.BuildPresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }

    PresetPair<BuildPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->BuildPresets.emplace(preset.Name, presetPair).second) {
      return ReadFileResult::DUPLICATE_PRESETS;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      return ReadFileResult::CONDITION_UNSUPPORTED;
    }

    this->BuildPresetOrder.push_back(preset.Name);
  }

  for (auto& preset : presets.TestPresets) {
    preset.OriginFile = file;
    if (preset.Name.empty()) {
      return ReadFileResult::INVALID_PRESET;
    }

    PresetPair<TestPreset> presetPair;
    presetPair.Unexpanded = preset;
    presetPair.Expanded = cm::nullopt;
    if (!this->TestPresets.emplace(preset.Name, presetPair).second) {
      return ReadFileResult::DUPLICATE_PRESETS;
    }

    // Support for conditions added in version 3.
    if (v < 3 && preset.ConditionEvaluator) {
      return ReadFileResult::CONDITION_UNSUPPORTED;
    }

    // Support for TestOutputTruncation added in version 5.
    if (v < 5 && preset.Output && preset.Output->TestOutputTruncation) {
      return ReadFileResult::TEST_OUTPUT_TRUNCATION_UNSUPPORTED;
    }

    this->TestPresetOrder.push_back(preset.Name);
  }

  auto const includeFile = [this, &inProgressFiles, file](
                             const std::string& include, RootType rootType2,
                             ReadReason readReason2) -> ReadFileResult {
    ReadFileResult r;
    File* includedFile;
    if ((r = this->ReadJSONFile(include, rootType2, readReason2,
                                inProgressFiles, includedFile)) !=
        ReadFileResult::READ_OK) {
      return r;
    }

    file->ReachableFiles.insert(includedFile->ReachableFiles.begin(),
                                includedFile->ReachableFiles.end());
    return ReadFileResult::READ_OK;
  };

  for (auto include : presets.Include) {
    if (!cmSystemTools::FileIsFullPath(include)) {
      auto directory = cmSystemTools::GetFilenamePath(filename);
      include = cmStrCat(directory, '/', include);
    }

    if ((result = includeFile(include, rootType, ReadReason::Included)) !=
        ReadFileResult::READ_OK) {
      return result;
    }
  }

  if (rootType == RootType::User && readReason == ReadReason::Root) {
    auto cmakePresetsFilename = GetFilename(this->SourceDir);
    if (cmSystemTools::FileExists(cmakePresetsFilename)) {
      if ((result = includeFile(cmakePresetsFilename, RootType::Project,
                                ReadReason::Root)) !=
          ReadFileResult::READ_OK) {
        return result;
      }
    }
  }

  inProgressFiles.pop_back();
  return ReadFileResult::READ_OK;
}
