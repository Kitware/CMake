/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePresetsFile.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmCMakePresetsFileInternal.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#define CHECK_EXPAND(out, field, expanders, version)                          \
  do {                                                                        \
    switch (ExpandMacros(field, expanders, version)) {                        \
      case ExpandMacroResult::Error:                                          \
        return false;                                                         \
      case ExpandMacroResult::Ignore:                                         \
        out.reset();                                                          \
        return true;                                                          \
      case ExpandMacroResult::Ok:                                             \
        break;                                                                \
    }                                                                         \
  } while (false)

namespace {
enum class CycleStatus
{
  Unvisited,
  InProgress,
  Verified,
};

using ReadFileResult = cmCMakePresetsFile::ReadFileResult;
using ConfigurePreset = cmCMakePresetsFile::ConfigurePreset;
using BuildPreset = cmCMakePresetsFile::BuildPreset;
using TestPreset = cmCMakePresetsFile::TestPreset;
using ExpandMacroResult = cmCMakePresetsFileInternal::ExpandMacroResult;
using MacroExpander = cmCMakePresetsFileInternal::MacroExpander;

void InheritString(std::string& child, const std::string& parent)
{
  if (child.empty()) {
    child = parent;
  }
}

template <typename T>
void InheritOptionalValue(cm::optional<T>& child,
                          const cm::optional<T>& parent)
{
  if (!child) {
    child = parent;
  }
}

template <typename T>
void InheritVector(std::vector<T>& child, const std::vector<T>& parent)
{
  if (child.empty()) {
    child = parent;
  }
}

/**
 * Check preset inheritance for cycles (using a DAG check algorithm) while
 * also bubbling up fields through the inheritance hierarchy, then verify
 * that each preset has the required fields, either directly or through
 * inheritance.
 */
template <class T>
ReadFileResult VisitPreset(
  T& preset, std::map<std::string, cmCMakePresetsFile::PresetPair<T>>& presets,
  std::map<std::string, CycleStatus> cycleStatus,
  const cmCMakePresetsFile& file)
{
  switch (cycleStatus[preset.Name]) {
    case CycleStatus::InProgress:
      return ReadFileResult::CYCLIC_PRESET_INHERITANCE;
    case CycleStatus::Verified:
      return ReadFileResult::READ_OK;
    default:
      break;
  }

  cycleStatus[preset.Name] = CycleStatus::InProgress;

  if (preset.Environment.count("") != 0) {
    return ReadFileResult::INVALID_PRESET;
  }

  CHECK_OK(preset.VisitPresetBeforeInherit());

  for (auto const& i : preset.Inherits) {
    auto parent = presets.find(i);
    if (parent == presets.end()) {
      return ReadFileResult::INVALID_PRESET;
    }

    auto& parentPreset = parent->second.Unexpanded;
    if (!preset.User && parentPreset.User) {
      return ReadFileResult::USER_PRESET_INHERITANCE;
    }

    auto result = VisitPreset(parentPreset, presets, cycleStatus, file);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }

    CHECK_OK(preset.VisitPresetInherit(parentPreset));

    for (auto const& v : parentPreset.Environment) {
      preset.Environment.insert(v);
    }

    if (!preset.ConditionEvaluator) {
      preset.ConditionEvaluator = parentPreset.ConditionEvaluator;
    }
  }

  if (preset.ConditionEvaluator && preset.ConditionEvaluator->IsNull()) {
    preset.ConditionEvaluator.reset();
  }

  CHECK_OK(preset.VisitPresetAfterInherit(file.GetVersion(preset)));

  cycleStatus[preset.Name] = CycleStatus::Verified;
  return ReadFileResult::READ_OK;
}

template <class T>
ReadFileResult ComputePresetInheritance(
  std::map<std::string, cmCMakePresetsFile::PresetPair<T>>& presets,
  const cmCMakePresetsFile& file)
{
  std::map<std::string, CycleStatus> cycleStatus;
  for (auto const& it : presets) {
    cycleStatus[it.first] = CycleStatus::Unvisited;
  }

  for (auto& it : presets) {
    auto& preset = it.second.Unexpanded;
    auto result = VisitPreset<T>(preset, presets, cycleStatus, file);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }
  }

  return ReadFileResult::READ_OK;
}

constexpr const char* ValidPrefixes[] = {
  "",
  "env",
  "penv",
  "vendor",
};

bool PrefixesValidMacroNamespace(const std::string& str)
{
  return std::any_of(
    std::begin(ValidPrefixes), std::end(ValidPrefixes),
    [&str](const char* prefix) -> bool { return cmHasPrefix(prefix, str); });
}

bool IsValidMacroNamespace(const std::string& str)
{
  return std::any_of(
    std::begin(ValidPrefixes), std::end(ValidPrefixes),
    [&str](const char* prefix) -> bool { return str == prefix; });
}

ExpandMacroResult VisitEnv(std::string& value, CycleStatus& status,
                           const std::vector<MacroExpander>& macroExpanders,
                           int version);
ExpandMacroResult ExpandMacros(
  std::string& out, const std::vector<MacroExpander>& macroExpanders,
  int version);
ExpandMacroResult ExpandMacro(std::string& out,
                              const std::string& macroNamespace,
                              const std::string& macroName,
                              const std::vector<MacroExpander>& macroExpanders,
                              int version);

bool ExpandMacros(const cmCMakePresetsFile& file,
                  const ConfigurePreset& preset,
                  cm::optional<ConfigurePreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  std::string binaryDir = preset.BinaryDir;
  CHECK_EXPAND(out, binaryDir, macroExpanders, file.GetVersion(preset));

  if (!binaryDir.empty()) {
    if (!cmSystemTools::FileIsFullPath(binaryDir)) {
      binaryDir = cmStrCat(file.SourceDir, '/', binaryDir);
    }
    out->BinaryDir = cmSystemTools::CollapseFullPath(binaryDir);
    cmSystemTools::ConvertToUnixSlashes(out->BinaryDir);
  }

  if (!preset.InstallDir.empty()) {
    std::string installDir = preset.InstallDir;
    CHECK_EXPAND(out, installDir, macroExpanders, file.GetVersion(preset));

    if (!cmSystemTools::FileIsFullPath(installDir)) {
      installDir = cmStrCat(file.SourceDir, '/', installDir);
    }
    out->InstallDir = cmSystemTools::CollapseFullPath(installDir);
    cmSystemTools::ConvertToUnixSlashes(out->InstallDir);
  }

  if (!preset.ToolchainFile.empty()) {
    std::string toolchain = preset.ToolchainFile;
    CHECK_EXPAND(out, toolchain, macroExpanders, file.GetVersion(preset));
    out->ToolchainFile = toolchain;
  }

  for (auto& variable : out->CacheVariables) {
    if (variable.second) {
      CHECK_EXPAND(out, variable.second->Value, macroExpanders,
                   file.GetVersion(preset));
    }
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsFile& file, const BuildPreset& preset,
                  cm::optional<BuildPreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& target : out->Targets) {
    CHECK_EXPAND(out, target, macroExpanders, file.GetVersion(preset));
  }

  for (auto& nativeToolOption : out->NativeToolOptions) {
    CHECK_EXPAND(out, nativeToolOption, macroExpanders,
                 file.GetVersion(preset));
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsFile& file, const TestPreset& preset,
                  cm::optional<TestPreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& overwrite : out->OverwriteConfigurationFile) {
    CHECK_EXPAND(out, overwrite, macroExpanders, file.GetVersion(preset));
  }

  if (out->Output) {
    CHECK_EXPAND(out, out->Output->OutputLogFile, macroExpanders,
                 file.GetVersion(preset));
  }

  if (out->Filter) {
    if (out->Filter->Include) {
      CHECK_EXPAND(out, out->Filter->Include->Name, macroExpanders,
                   file.GetVersion(preset));
      CHECK_EXPAND(out, out->Filter->Include->Label, macroExpanders,
                   file.GetVersion(preset));

      if (out->Filter->Include->Index) {
        CHECK_EXPAND(out, out->Filter->Include->Index->IndexFile,
                     macroExpanders, file.GetVersion(preset));
      }
    }

    if (out->Filter->Exclude) {
      CHECK_EXPAND(out, out->Filter->Exclude->Name, macroExpanders,
                   file.GetVersion(preset));
      CHECK_EXPAND(out, out->Filter->Exclude->Label, macroExpanders,
                   file.GetVersion(preset));

      if (out->Filter->Exclude->Fixtures) {
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Any, macroExpanders,
                     file.GetVersion(preset));
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Setup,
                     macroExpanders, file.GetVersion(preset));
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Cleanup,
                     macroExpanders, file.GetVersion(preset));
      }
    }
  }

  if (out->Execution) {
    CHECK_EXPAND(out, out->Execution->ResourceSpecFile, macroExpanders,
                 file.GetVersion(preset));
  }

  return true;
}

template <class T>
bool ExpandMacros(const cmCMakePresetsFile& file, const T& preset,
                  cm::optional<T>& out)
{
  out.emplace(preset);

  std::map<std::string, CycleStatus> envCycles;
  for (auto const& v : out->Environment) {
    envCycles[v.first] = CycleStatus::Unvisited;
  }

  std::vector<MacroExpander> macroExpanders;

  MacroExpander defaultMacroExpander =
    [&file, &preset](const std::string& macroNamespace,
                     const std::string& macroName, std::string& macroOut,
                     int version) -> ExpandMacroResult {
    if (macroNamespace.empty()) {
      if (macroName == "sourceDir") {
        macroOut += file.SourceDir;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "sourceParentDir") {
        macroOut += cmSystemTools::GetParentDirectory(file.SourceDir);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "sourceDirName") {
        macroOut += cmSystemTools::GetFilenameName(file.SourceDir);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "presetName") {
        macroOut += preset.Name;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "generator") {
        // Generator only makes sense if preset is not hidden.
        if (!preset.Hidden) {
          macroOut += file.GetGeneratorForPreset(preset.Name);
        }
        return ExpandMacroResult::Ok;
      }
      if (macroName == "dollar") {
        macroOut += '$';
        return ExpandMacroResult::Ok;
      }
      if (macroName == "hostSystemName") {
        if (version < 3) {
          return ExpandMacroResult::Error;
        }
        macroOut += cmSystemTools::GetSystemName();
        return ExpandMacroResult::Ok;
      }
    }

    return ExpandMacroResult::Ignore;
  };

  MacroExpander environmentMacroExpander =
    [&macroExpanders, &out, &envCycles](
      const std::string& macroNamespace, const std::string& macroName,
      std::string& result, int version) -> ExpandMacroResult {
    if (macroNamespace == "env" && !macroName.empty() && out) {
      auto v = out->Environment.find(macroName);
      if (v != out->Environment.end() && v->second) {
        auto e =
          VisitEnv(*v->second, envCycles[macroName], macroExpanders, version);
        if (e != ExpandMacroResult::Ok) {
          return e;
        }
        result += *v->second;
        return ExpandMacroResult::Ok;
      }
    }

    if (macroNamespace == "env" || macroNamespace == "penv") {
      if (macroName.empty()) {
        return ExpandMacroResult::Error;
      }
      const char* value = std::getenv(macroName.c_str());
      if (value) {
        result += value;
      }
      return ExpandMacroResult::Ok;
    }

    return ExpandMacroResult::Ignore;
  };

  macroExpanders.push_back(defaultMacroExpander);
  macroExpanders.push_back(environmentMacroExpander);

  for (auto& v : out->Environment) {
    if (v.second) {
      switch (VisitEnv(*v.second, envCycles[v.first], macroExpanders,
                       file.GetVersion(preset))) {
        case ExpandMacroResult::Error:
          return false;
        case ExpandMacroResult::Ignore:
          out.reset();
          return true;
        case ExpandMacroResult::Ok:
          break;
      }
    }
  }

  if (preset.ConditionEvaluator) {
    cm::optional<bool> result;
    if (!preset.ConditionEvaluator->Evaluate(
          macroExpanders, file.GetVersion(preset), result)) {
      return false;
    }
    if (!result) {
      out.reset();
      return true;
    }
    out->ConditionResult = *result;
  }

  return ExpandMacros(file, preset, out, macroExpanders);
}

ExpandMacroResult VisitEnv(std::string& value, CycleStatus& status,
                           const std::vector<MacroExpander>& macroExpanders,
                           int version)
{
  if (status == CycleStatus::Verified) {
    return ExpandMacroResult::Ok;
  }
  if (status == CycleStatus::InProgress) {
    return ExpandMacroResult::Error;
  }

  status = CycleStatus::InProgress;
  auto e = ExpandMacros(value, macroExpanders, version);
  if (e != ExpandMacroResult::Ok) {
    return e;
  }
  status = CycleStatus::Verified;
  return ExpandMacroResult::Ok;
}

ExpandMacroResult ExpandMacros(
  std::string& out, const std::vector<MacroExpander>& macroExpanders,
  int version)
{
  std::string result;
  std::string macroNamespace;
  std::string macroName;

  enum class State
  {
    Default,
    MacroNamespace,
    MacroName,
  } state = State::Default;

  for (auto c : out) {
    switch (state) {
      case State::Default:
        if (c == '$') {
          state = State::MacroNamespace;
        } else {
          result += c;
        }
        break;

      case State::MacroNamespace:
        if (c == '{') {
          if (IsValidMacroNamespace(macroNamespace)) {
            state = State::MacroName;
          } else {
            result += '$';
            result += macroNamespace;
            result += '{';
            macroNamespace.clear();
            state = State::Default;
          }
        } else {
          macroNamespace += c;
          if (!PrefixesValidMacroNamespace(macroNamespace)) {
            result += '$';
            result += macroNamespace;
            macroNamespace.clear();
            state = State::Default;
          }
        }
        break;

      case State::MacroName:
        if (c == '}') {
          auto e = ExpandMacro(result, macroNamespace, macroName,
                               macroExpanders, version);
          if (e != ExpandMacroResult::Ok) {
            return e;
          }
          macroNamespace.clear();
          macroName.clear();
          state = State::Default;
        } else {
          macroName += c;
        }
        break;
    }
  }

  switch (state) {
    case State::Default:
      break;
    case State::MacroNamespace:
      result += '$';
      result += macroNamespace;
      break;
    case State::MacroName:
      return ExpandMacroResult::Error;
  }

  out = std::move(result);
  return ExpandMacroResult::Ok;
}

ExpandMacroResult ExpandMacro(std::string& out,
                              const std::string& macroNamespace,
                              const std::string& macroName,
                              const std::vector<MacroExpander>& macroExpanders,
                              int version)
{
  for (auto const& macroExpander : macroExpanders) {
    auto result = macroExpander(macroNamespace, macroName, out, version);
    if (result != ExpandMacroResult::Ignore) {
      return result;
    }
  }

  if (macroNamespace == "vendor") {
    return ExpandMacroResult::Ignore;
  }

  return ExpandMacroResult::Error;
}
}

bool cmCMakePresetsFileInternal::EqualsCondition::Evaluate(
  const std::vector<MacroExpander>& expanders, int version,
  cm::optional<bool>& out) const
{
  std::string lhs = this->Lhs;
  CHECK_EXPAND(out, lhs, expanders, version);

  std::string rhs = this->Rhs;
  CHECK_EXPAND(out, rhs, expanders, version);

  out = (lhs == rhs);
  return true;
}

bool cmCMakePresetsFileInternal::InListCondition::Evaluate(
  const std::vector<MacroExpander>& expanders, int version,
  cm::optional<bool>& out) const
{
  std::string str = this->String;
  CHECK_EXPAND(out, str, expanders, version);

  for (auto item : this->List) {
    CHECK_EXPAND(out, item, expanders, version);
    if (str == item) {
      out = true;
      return true;
    }
  }

  out = false;
  return true;
}

bool cmCMakePresetsFileInternal::MatchesCondition::Evaluate(
  const std::vector<MacroExpander>& expanders, int version,
  cm::optional<bool>& out) const
{
  std::string str = this->String;
  CHECK_EXPAND(out, str, expanders, version);
  std::string regexStr = this->Regex;
  CHECK_EXPAND(out, regexStr, expanders, version);

  cmsys::RegularExpression regex;
  if (!regex.compile(regexStr)) {
    return false;
  }

  out = regex.find(str);
  return true;
}

bool cmCMakePresetsFileInternal::AnyAllOfCondition::Evaluate(
  const std::vector<MacroExpander>& expanders, int version,
  cm::optional<bool>& out) const
{
  for (auto const& condition : this->Conditions) {
    cm::optional<bool> result;
    if (!condition->Evaluate(expanders, version, result)) {
      out.reset();
      return false;
    }

    if (!result) {
      out.reset();
      return true;
    }

    if (result == this->StopValue) {
      out = result;
      return true;
    }
  }

  out = !this->StopValue;
  return true;
}

bool cmCMakePresetsFileInternal::NotCondition::Evaluate(
  const std::vector<MacroExpander>& expanders, int version,
  cm::optional<bool>& out) const
{
  out.reset();
  if (!this->SubCondition->Evaluate(expanders, version, out)) {
    out.reset();
    return false;
  }
  if (out) {
    *out = !*out;
  }
  return true;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ConfigurePreset::VisitPresetInherit(
  const cmCMakePresetsFile::Preset& parentPreset)
{
  auto& preset = *this;
  const ConfigurePreset& parent =
    static_cast<const ConfigurePreset&>(parentPreset);
  InheritString(preset.Generator, parent.Generator);
  InheritString(preset.Architecture, parent.Architecture);
  InheritString(preset.Toolset, parent.Toolset);
  if (!preset.ArchitectureStrategy) {
    preset.ArchitectureStrategy = parent.ArchitectureStrategy;
  }
  if (!preset.ToolsetStrategy) {
    preset.ToolsetStrategy = parent.ToolsetStrategy;
  }
  InheritString(preset.BinaryDir, parent.BinaryDir);
  InheritString(preset.InstallDir, parent.InstallDir);
  InheritString(preset.ToolchainFile, parent.ToolchainFile);
  InheritOptionalValue(preset.WarnDev, parent.WarnDev);
  InheritOptionalValue(preset.ErrorDev, parent.ErrorDev);
  InheritOptionalValue(preset.WarnDeprecated, parent.WarnDeprecated);
  InheritOptionalValue(preset.ErrorDeprecated, parent.ErrorDeprecated);
  InheritOptionalValue(preset.WarnUninitialized, parent.WarnUninitialized);
  InheritOptionalValue(preset.WarnUnusedCli, parent.WarnUnusedCli);
  InheritOptionalValue(preset.WarnSystemVars, parent.WarnSystemVars);

  for (auto const& v : parent.CacheVariables) {
    preset.CacheVariables.insert(v);
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ConfigurePreset::VisitPresetBeforeInherit()
{
  auto& preset = *this;
  if (preset.Environment.count("") != 0) {
    return ReadFileResult::INVALID_PRESET;
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ConfigurePreset::VisitPresetAfterInherit(int version)
{
  auto& preset = *this;
  if (!preset.Hidden) {
    if (version < 3) {
      if (preset.Generator.empty()) {
        return ReadFileResult::INVALID_PRESET;
      }
      if (preset.BinaryDir.empty()) {
        return ReadFileResult::INVALID_PRESET;
      }
    }

    if (preset.WarnDev == false && preset.ErrorDev == true) {
      return ReadFileResult::INVALID_PRESET;
    }
    if (preset.WarnDeprecated == false && preset.ErrorDeprecated == true) {
      return ReadFileResult::INVALID_PRESET;
    }
    if (preset.CacheVariables.count("") != 0) {
      return ReadFileResult::INVALID_PRESET;
    }
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::BuildPreset::VisitPresetInherit(
  const cmCMakePresetsFile::Preset& parentPreset)
{
  auto& preset = *this;
  const BuildPreset& parent = static_cast<const BuildPreset&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritOptionalValue(preset.Jobs, parent.Jobs);
  InheritVector(preset.Targets, parent.Targets);
  InheritString(preset.Configuration, parent.Configuration);
  InheritOptionalValue(preset.CleanFirst, parent.CleanFirst);
  InheritOptionalValue(preset.Verbose, parent.Verbose);
  InheritVector(preset.NativeToolOptions, parent.NativeToolOptions);

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::BuildPreset::VisitPresetAfterInherit(int /* version */)
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return ReadFileResult::INVALID_PRESET;
  }
  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::TestPreset::VisitPresetInherit(
  const cmCMakePresetsFile::Preset& parentPreset)
{
  auto& preset = *this;
  const TestPreset& parent = static_cast<const TestPreset&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritString(preset.Configuration, parent.Configuration);
  InheritVector(preset.OverwriteConfigurationFile,
                parent.OverwriteConfigurationFile);

  if (parent.Output) {
    if (preset.Output) {
      auto& output = preset.Output.value();
      const auto& parentOutput = parent.Output.value();
      InheritOptionalValue(output.ShortProgress, parentOutput.ShortProgress);
      InheritOptionalValue(output.Verbosity, parentOutput.Verbosity);
      InheritOptionalValue(output.Debug, parentOutput.Debug);
      InheritOptionalValue(output.OutputOnFailure,
                           parentOutput.OutputOnFailure);
      InheritOptionalValue(output.Quiet, parentOutput.Quiet);
      InheritString(output.OutputLogFile, parentOutput.OutputLogFile);
      InheritOptionalValue(output.LabelSummary, parentOutput.LabelSummary);
      InheritOptionalValue(output.SubprojectSummary,
                           parentOutput.SubprojectSummary);
      InheritOptionalValue(output.MaxPassedTestOutputSize,
                           parentOutput.MaxPassedTestOutputSize);
      InheritOptionalValue(output.MaxFailedTestOutputSize,
                           parentOutput.MaxFailedTestOutputSize);
      InheritOptionalValue(output.MaxTestNameWidth,
                           parentOutput.MaxTestNameWidth);
    } else {
      preset.Output = parent.Output;
    }
  }

  if (parent.Filter) {
    if (parent.Filter->Include) {
      if (preset.Filter && preset.Filter->Include) {
        auto& include = *preset.Filter->Include;
        const auto& parentInclude = *parent.Filter->Include;
        InheritString(include.Name, parentInclude.Name);
        InheritString(include.Label, parentInclude.Label);
        InheritOptionalValue(include.Index, parentInclude.Index);
      } else {
        if (!preset.Filter) {
          preset.Filter.emplace();
        }
        preset.Filter->Include = parent.Filter->Include;
      }
    }

    if (parent.Filter->Exclude) {
      if (preset.Filter && preset.Filter->Exclude) {
        auto& exclude = *preset.Filter->Exclude;
        const auto& parentExclude = *parent.Filter->Exclude;
        InheritString(exclude.Name, parentExclude.Name);
        InheritString(exclude.Label, parentExclude.Label);
        InheritOptionalValue(exclude.Fixtures, parentExclude.Fixtures);
      } else {
        if (!preset.Filter) {
          preset.Filter.emplace();
        }
        preset.Filter->Exclude = parent.Filter->Exclude;
      }
    }
  }

  if (parent.Execution) {
    if (preset.Execution) {
      auto& execution = *preset.Execution;
      const auto& parentExecution = *parent.Execution;
      InheritOptionalValue(execution.StopOnFailure,
                           parentExecution.StopOnFailure);
      InheritOptionalValue(execution.EnableFailover,
                           parentExecution.EnableFailover);
      InheritOptionalValue(execution.Jobs, parentExecution.Jobs);
      InheritString(execution.ResourceSpecFile,
                    parentExecution.ResourceSpecFile);
      InheritOptionalValue(execution.TestLoad, parentExecution.TestLoad);
      InheritOptionalValue(execution.ShowOnly, parentExecution.ShowOnly);
      InheritOptionalValue(execution.Repeat, parentExecution.Repeat);
      InheritOptionalValue(execution.InteractiveDebugging,
                           parentExecution.InteractiveDebugging);
      InheritOptionalValue(execution.ScheduleRandom,
                           parentExecution.ScheduleRandom);
      InheritOptionalValue(execution.Timeout, parentExecution.Timeout);
      InheritOptionalValue(execution.NoTestsAction,
                           parentExecution.NoTestsAction);
    } else {
      preset.Execution = parent.Execution;
    }
  }

  return ReadFileResult::READ_OK;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::TestPreset::VisitPresetAfterInherit(int /* version */)
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return ReadFileResult::INVALID_PRESET;
  }
  return ReadFileResult::READ_OK;
}

std::string cmCMakePresetsFile::GetFilename(const std::string& sourceDir)
{
  return cmStrCat(sourceDir, "/CMakePresets.json");
}

std::string cmCMakePresetsFile::GetUserFilename(const std::string& sourceDir)
{
  return cmStrCat(sourceDir, "/CMakeUserPresets.json");
}

cmCMakePresetsFile::ReadFileResult cmCMakePresetsFile::ReadProjectPresets(
  const std::string& sourceDir, bool allowNoFiles)
{
  this->SourceDir = sourceDir;
  this->ClearPresets();

  auto result = this->ReadProjectPresetsInternal(allowNoFiles);
  if (result != ReadFileResult::READ_OK) {
    this->ClearPresets();
  }

  return result;
}

cmCMakePresetsFile::ReadFileResult
cmCMakePresetsFile::ReadProjectPresetsInternal(bool allowNoFiles)
{
  bool haveOneFile = false;

  std::string filename = GetUserFilename(this->SourceDir);
  if (cmSystemTools::FileExists(filename)) {
    auto result = this->ReadJSONFile(filename, true);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }
    haveOneFile = true;
  }

  filename = GetFilename(this->SourceDir);
  if (cmSystemTools::FileExists(filename)) {
    auto result = this->ReadJSONFile(filename, false);
    if (result != ReadFileResult::READ_OK) {
      return result;
    }
    haveOneFile = true;
  }

  if (!haveOneFile) {
    return allowNoFiles ? ReadFileResult::READ_OK
                        : ReadFileResult::FILE_NOT_FOUND;
  }

  CHECK_OK(ComputePresetInheritance(this->ConfigurePresets, *this));
  CHECK_OK(ComputePresetInheritance(this->BuildPresets, *this));
  CHECK_OK(ComputePresetInheritance(this->TestPresets, *this));

  for (auto& it : this->ConfigurePresets) {
    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      return ReadFileResult::INVALID_MACRO_EXPANSION;
    }
  }

  for (auto& it : this->BuildPresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        return ReadFileResult::INVALID_CONFIGURE_PRESET;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      return ReadFileResult::INVALID_MACRO_EXPANSION;
    }
  }

  for (auto& it : this->TestPresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        return ReadFileResult::INVALID_CONFIGURE_PRESET;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      return ReadFileResult::INVALID_MACRO_EXPANSION;
    }
  }

  return ReadFileResult::READ_OK;
}

const char* cmCMakePresetsFile::ResultToString(ReadFileResult result)
{
  switch (result) {
    case ReadFileResult::READ_OK:
      return "OK";
    case ReadFileResult::FILE_NOT_FOUND:
      return "File not found";
    case ReadFileResult::JSON_PARSE_ERROR:
      return "JSON parse error";
    case ReadFileResult::INVALID_ROOT:
      return "Invalid root object";
    case ReadFileResult::NO_VERSION:
      return "No \"version\" field";
    case ReadFileResult::INVALID_VERSION:
      return "Invalid \"version\" field";
    case ReadFileResult::UNRECOGNIZED_VERSION:
      return "Unrecognized \"version\" field";
    case ReadFileResult::INVALID_CMAKE_VERSION:
      return "Invalid \"cmakeMinimumRequired\" field";
    case ReadFileResult::UNRECOGNIZED_CMAKE_VERSION:
      return "\"cmakeMinimumRequired\" version too new";
    case ReadFileResult::INVALID_PRESETS:
      return "Invalid \"configurePresets\" field";
    case ReadFileResult::INVALID_PRESET:
      return "Invalid preset";
    case ReadFileResult::INVALID_VARIABLE:
      return "Invalid CMake variable definition";
    case ReadFileResult::DUPLICATE_PRESETS:
      return "Duplicate presets";
    case ReadFileResult::CYCLIC_PRESET_INHERITANCE:
      return "Cyclic preset inheritance";
    case ReadFileResult::USER_PRESET_INHERITANCE:
      return "Project preset inherits from user preset";
    case ReadFileResult::INVALID_MACRO_EXPANSION:
      return "Invalid macro expansion";
    case ReadFileResult::BUILD_TEST_PRESETS_UNSUPPORTED:
      return "File version must be 2 or higher for build and test preset "
             "support.";
    case ReadFileResult::INVALID_CONFIGURE_PRESET:
      return "Invalid \"configurePreset\" field";
    case ReadFileResult::INSTALL_PREFIX_UNSUPPORTED:
      return "File version must be 3 or higher for installDir preset "
             "support.";
    case ReadFileResult::INVALID_CONDITION:
      return "Invalid preset condition";
    case ReadFileResult::CONDITION_UNSUPPORTED:
      return "File version must be 3 or higher for condition support";
    case ReadFileResult::TOOLCHAIN_FILE_UNSUPPORTED:
      return "File version must be 3 or higher for toolchainFile preset "
             "support.";
  }

  return "Unknown error";
}

void cmCMakePresetsFile::ClearPresets()
{
  this->ConfigurePresets.clear();
  this->BuildPresets.clear();
  this->TestPresets.clear();

  this->ConfigurePresetOrder.clear();
  this->BuildPresetOrder.clear();
  this->TestPresetOrder.clear();
}

void cmCMakePresetsFile::PrintPresets(
  const std::vector<const cmCMakePresetsFile::Preset*>& presets)
{
  if (presets.empty()) {
    return;
  }

  auto longestPresetName =
    std::max_element(presets.begin(), presets.end(),
                     [](const cmCMakePresetsFile::Preset* a,
                        const cmCMakePresetsFile::Preset* b) {
                       return a->Name.length() < b->Name.length();
                     });
  auto longestLength = (*longestPresetName)->Name.length();

  for (const auto* preset : presets) {
    std::cout << "  \"" << preset->Name << '"';
    const auto& description = preset->DisplayName;
    if (!description.empty()) {
      for (std::size_t i = 0; i < longestLength - preset->Name.length(); ++i) {
        std::cout << ' ';
      }
      std::cout << " - " << description;
    }
    std::cout << '\n';
  }
}

void cmCMakePresetsFile::PrintConfigurePresetList() const
{
  PrintConfigurePresetList([](const ConfigurePreset&) { return true; });
}

void cmCMakePresetsFile::PrintConfigurePresetList(
  const std::function<bool(const ConfigurePreset&)>& filter) const
{
  std::vector<const cmCMakePresetsFile::Preset*> presets;
  for (auto const& p : this->ConfigurePresetOrder) {
    auto const& preset = this->ConfigurePresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult && filter(preset.Unexpanded)) {
      presets.push_back(
        static_cast<const cmCMakePresetsFile::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    std::cout << "Available configure presets:\n\n";
    cmCMakePresetsFile::PrintPresets(presets);
  }
}

void cmCMakePresetsFile::PrintBuildPresetList() const
{
  std::vector<const cmCMakePresetsFile::Preset*> presets;
  for (auto const& p : this->BuildPresetOrder) {
    auto const& preset = this->BuildPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult) {
      presets.push_back(
        static_cast<const cmCMakePresetsFile::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    std::cout << "Available build presets:\n\n";
    cmCMakePresetsFile::PrintPresets(presets);
  }
}

void cmCMakePresetsFile::PrintTestPresetList() const
{
  std::vector<const cmCMakePresetsFile::Preset*> presets;
  for (auto const& p : this->TestPresetOrder) {
    auto const& preset = this->TestPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult) {
      presets.push_back(
        static_cast<const cmCMakePresetsFile::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    std::cout << "Available test presets:\n\n";
    cmCMakePresetsFile::PrintPresets(presets);
  }
}

void cmCMakePresetsFile::PrintAllPresets() const
{
  this->PrintConfigurePresetList();
  std::cout << std::endl;
  this->PrintBuildPresetList();
  std::cout << std::endl;
  this->PrintTestPresetList();
}
