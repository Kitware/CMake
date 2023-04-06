/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePresetsGraph.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmCMakePresetErrors.h"
#include "cmCMakePresetsGraphInternal.h"
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

using ConfigurePreset = cmCMakePresetsGraph::ConfigurePreset;
using BuildPreset = cmCMakePresetsGraph::BuildPreset;
using TestPreset = cmCMakePresetsGraph::TestPreset;
using PackagePreset = cmCMakePresetsGraph::PackagePreset;
using WorkflowPreset = cmCMakePresetsGraph::WorkflowPreset;
template <typename T>
using PresetPair = cmCMakePresetsGraph::PresetPair<T>;
using ExpandMacroResult = cmCMakePresetsGraphInternal::ExpandMacroResult;
using MacroExpander = cmCMakePresetsGraphInternal::MacroExpander;
using cmCMakePresetsGraphInternal::ExpandMacros;

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
bool VisitPreset(
  T& preset,
  std::map<std::string, cmCMakePresetsGraph::PresetPair<T>>& presets,
  std::map<std::string, CycleStatus> cycleStatus, cmCMakePresetsGraph& graph)
{
  switch (cycleStatus[preset.Name]) {
    case CycleStatus::InProgress:
      cmCMakePresetErrors::CYCLIC_PRESET_INHERITANCE(preset.Name,
                                                     &graph.parseState);
      return false;
    case CycleStatus::Verified:
      return true;
    default:
      break;
  }

  cycleStatus[preset.Name] = CycleStatus::InProgress;

  if (preset.Environment.count("") != 0) {
    cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name, &graph.parseState);
    return false;
  }

  bool result = preset.VisitPresetBeforeInherit();
  if (!result) {
    cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name, &graph.parseState);
    return false;
  }

  for (auto const& i : preset.Inherits) {
    auto parent = presets.find(i);
    if (parent == presets.end()) {
      cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name,
                                                &graph.parseState);
      return false;
    }

    auto& parentPreset = parent->second.Unexpanded;
    if (!preset.OriginFile->ReachableFiles.count(parentPreset.OriginFile)) {
      cmCMakePresetErrors::INHERITED_PRESET_UNREACHABLE_FROM_FILE(
        preset.Name, &graph.parseState);
      return false;
    }

    if (!VisitPreset(parentPreset, presets, cycleStatus, graph)) {
      return false;
    }

    result = preset.VisitPresetInherit(parentPreset);
    if (!result) {
      cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name,
                                                &graph.parseState);
      return false;
    }

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

  result = preset.VisitPresetAfterInherit(graph.GetVersion(preset),
                                          &graph.parseState);
  if (!result) {
    cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name, &graph.parseState);
    return false;
  }

  cycleStatus[preset.Name] = CycleStatus::Verified;
  return true;
}

template <class T>
bool ComputePresetInheritance(
  std::map<std::string, cmCMakePresetsGraph::PresetPair<T>>& presets,
  cmCMakePresetsGraph& graph)
{
  std::map<std::string, CycleStatus> cycleStatus;
  for (auto const& it : presets) {
    cycleStatus[it.first] = CycleStatus::Unvisited;
  }

  for (auto& it : presets) {
    auto& preset = it.second.Unexpanded;
    if (!VisitPreset<T>(preset, presets, cycleStatus, graph)) {
      return false;
    }
  }

  return true;
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

bool ExpandMacros(const cmCMakePresetsGraph& graph,
                  const ConfigurePreset& preset,
                  cm::optional<ConfigurePreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  std::string binaryDir = preset.BinaryDir;
  CHECK_EXPAND(out, binaryDir, macroExpanders, graph.GetVersion(preset));

  if (!binaryDir.empty()) {
    if (!cmSystemTools::FileIsFullPath(binaryDir)) {
      binaryDir = cmStrCat(graph.SourceDir, '/', binaryDir);
    }
    out->BinaryDir = cmSystemTools::CollapseFullPath(binaryDir);
    cmSystemTools::ConvertToUnixSlashes(out->BinaryDir);
  }

  if (!preset.InstallDir.empty()) {
    std::string installDir = preset.InstallDir;
    CHECK_EXPAND(out, installDir, macroExpanders, graph.GetVersion(preset));

    if (!cmSystemTools::FileIsFullPath(installDir)) {
      installDir = cmStrCat(graph.SourceDir, '/', installDir);
    }
    out->InstallDir = cmSystemTools::CollapseFullPath(installDir);
    cmSystemTools::ConvertToUnixSlashes(out->InstallDir);
  }

  if (!preset.ToolchainFile.empty()) {
    std::string toolchain = preset.ToolchainFile;
    CHECK_EXPAND(out, toolchain, macroExpanders, graph.GetVersion(preset));
    out->ToolchainFile = toolchain;
  }

  for (auto& variable : out->CacheVariables) {
    if (variable.second) {
      CHECK_EXPAND(out, variable.second->Value, macroExpanders,
                   graph.GetVersion(preset));
    }
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsGraph& graph, const BuildPreset& preset,
                  cm::optional<BuildPreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& target : out->Targets) {
    CHECK_EXPAND(out, target, macroExpanders, graph.GetVersion(preset));
  }

  for (auto& nativeToolOption : out->NativeToolOptions) {
    CHECK_EXPAND(out, nativeToolOption, macroExpanders,
                 graph.GetVersion(preset));
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsGraph& graph, const TestPreset& preset,
                  cm::optional<TestPreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& overwrite : out->OverwriteConfigurationFile) {
    CHECK_EXPAND(out, overwrite, macroExpanders, graph.GetVersion(preset));
  }

  if (out->Output) {
    CHECK_EXPAND(out, out->Output->OutputLogFile, macroExpanders,
                 graph.GetVersion(preset));
    CHECK_EXPAND(out, out->Output->OutputJUnitFile, macroExpanders,
                 graph.GetVersion(preset));
  }

  if (out->Filter) {
    if (out->Filter->Include) {
      CHECK_EXPAND(out, out->Filter->Include->Name, macroExpanders,
                   graph.GetVersion(preset));
      CHECK_EXPAND(out, out->Filter->Include->Label, macroExpanders,
                   graph.GetVersion(preset));

      if (out->Filter->Include->Index) {
        CHECK_EXPAND(out, out->Filter->Include->Index->IndexFile,
                     macroExpanders, graph.GetVersion(preset));
      }
    }

    if (out->Filter->Exclude) {
      CHECK_EXPAND(out, out->Filter->Exclude->Name, macroExpanders,
                   graph.GetVersion(preset));
      CHECK_EXPAND(out, out->Filter->Exclude->Label, macroExpanders,
                   graph.GetVersion(preset));

      if (out->Filter->Exclude->Fixtures) {
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Any, macroExpanders,
                     graph.GetVersion(preset));
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Setup,
                     macroExpanders, graph.GetVersion(preset));
        CHECK_EXPAND(out, out->Filter->Exclude->Fixtures->Cleanup,
                     macroExpanders, graph.GetVersion(preset));
      }
    }
  }

  if (out->Execution) {
    CHECK_EXPAND(out, out->Execution->ResourceSpecFile, macroExpanders,
                 graph.GetVersion(preset));
  }

  return true;
}

bool ExpandMacros(const cmCMakePresetsGraph& graph,
                  const PackagePreset& preset,
                  cm::optional<PackagePreset>& out,
                  const std::vector<MacroExpander>& macroExpanders)
{
  for (auto& variable : out->Variables) {
    CHECK_EXPAND(out, variable.second, macroExpanders,
                 graph.GetVersion(preset));
  }

  CHECK_EXPAND(out, out->ConfigFile, macroExpanders, graph.GetVersion(preset));
  CHECK_EXPAND(out, out->PackageName, macroExpanders,
               graph.GetVersion(preset));
  CHECK_EXPAND(out, out->PackageVersion, macroExpanders,
               graph.GetVersion(preset));
  CHECK_EXPAND(out, out->PackageDirectory, macroExpanders,
               graph.GetVersion(preset));
  CHECK_EXPAND(out, out->VendorName, macroExpanders, graph.GetVersion(preset));

  return true;
}

bool ExpandMacros(const cmCMakePresetsGraph& /*graph*/,
                  const WorkflowPreset& /*preset*/,
                  cm::optional<WorkflowPreset>& /*out*/,
                  const std::vector<MacroExpander>& /*macroExpanders*/)
{
  return true;
}

template <class T>
bool ExpandMacros(cmCMakePresetsGraph& graph, const T& preset,
                  cm::optional<T>& out)
{
  out.emplace(preset);

  std::map<std::string, CycleStatus> envCycles;
  for (auto const& v : out->Environment) {
    envCycles[v.first] = CycleStatus::Unvisited;
  }

  std::vector<MacroExpander> macroExpanders;

  MacroExpander defaultMacroExpander =
    [&graph, &preset](const std::string& macroNamespace,
                      const std::string& macroName, std::string& macroOut,
                      int version) -> ExpandMacroResult {
    if (macroNamespace.empty()) {
      if (macroName == "sourceDir") {
        macroOut += graph.SourceDir;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "sourceParentDir") {
        macroOut += cmSystemTools::GetParentDirectory(graph.SourceDir);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "sourceDirName") {
        macroOut += cmSystemTools::GetFilenameName(graph.SourceDir);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "presetName") {
        macroOut += preset.Name;
        return ExpandMacroResult::Ok;
      }
      if (macroName == "generator") {
        // Generator only makes sense if preset is not hidden.
        if (!preset.Hidden) {
          macroOut += graph.GetGeneratorForPreset(preset.Name);
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
      if (macroName == "fileDir") {
        if (version < 4) {
          return ExpandMacroResult::Error;
        }
        macroOut +=
          cmSystemTools::GetParentDirectory(preset.OriginFile->Filename);
        return ExpandMacroResult::Ok;
      }
      if (macroName == "pathListSep") {
        if (version < 5) {
          return ExpandMacroResult::Error;
        }
        macroOut += cmSystemTools::GetSystemPathlistSeparator();
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
      if (cm::optional<std::string> value =
            cmSystemTools::GetEnvVar(macroName)) {
        result += *value;
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
                       graph.GetVersion(preset))) {
        case ExpandMacroResult::Error:
          cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name,
                                                    &graph.parseState);
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
          macroExpanders, graph.GetVersion(preset), result)) {
      cmCMakePresetErrors::INVALID_PRESET_NAMED(preset.Name,
                                                &graph.parseState);
      return false;
    }
    if (!result) {
      out.reset();
      return true;
    }
    out->ConditionResult = *result;
  }

  return ExpandMacros(graph, preset, out, macroExpanders);
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
}

ExpandMacroResult cmCMakePresetsGraphInternal::ExpandMacros(
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

ExpandMacroResult cmCMakePresetsGraphInternal::ExpandMacro(
  std::string& out, const std::string& macroNamespace,
  const std::string& macroName,
  const std::vector<MacroExpander>& macroExpanders, int version)
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

namespace {
template <typename T>
bool SetupWorkflowConfigurePreset(const T& preset,
                                  const ConfigurePreset*& configurePreset,
                                  cmJSONState* state)
{
  if (preset.ConfigurePreset != configurePreset->Name) {
    cmCMakePresetErrors::INVALID_WORKFLOW_STEPS(configurePreset->Name, state);
    return false;
  }
  return true;
}

template <>
bool SetupWorkflowConfigurePreset<ConfigurePreset>(
  const ConfigurePreset& preset, const ConfigurePreset*& configurePreset,
  cmJSONState*)
{
  configurePreset = &preset;
  return true;
}

template <typename T>
bool TryReachPresetFromWorkflow(
  const WorkflowPreset& origin,
  const std::map<std::string, PresetPair<T>>& presets, const std::string& name,
  const ConfigurePreset*& configurePreset, cmJSONState* state)
{
  auto it = presets.find(name);
  if (it == presets.end()) {
    cmCMakePresetErrors::INVALID_WORKFLOW_STEPS(name, state);
    return false;
  }
  if (!origin.OriginFile->ReachableFiles.count(
        it->second.Unexpanded.OriginFile)) {
    cmCMakePresetErrors::WORKFLOW_STEP_UNREACHABLE_FROM_FILE(name, state);
    return false;
  }
  return SetupWorkflowConfigurePreset<T>(it->second.Unexpanded,
                                         configurePreset, state);
}
}

bool cmCMakePresetsGraphInternal::EqualsCondition::Evaluate(
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

bool cmCMakePresetsGraphInternal::InListCondition::Evaluate(
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

bool cmCMakePresetsGraphInternal::MatchesCondition::Evaluate(
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

bool cmCMakePresetsGraphInternal::AnyAllOfCondition::Evaluate(
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

bool cmCMakePresetsGraphInternal::NotCondition::Evaluate(
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

bool cmCMakePresetsGraph::ConfigurePreset::VisitPresetInherit(
  const cmCMakePresetsGraph::Preset& parentPreset)
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

  return true;
}

bool cmCMakePresetsGraph::ConfigurePreset::VisitPresetBeforeInherit()
{
  auto& preset = *this;
  if (preset.Environment.count("") != 0) {
    return false;
  }

  return true;
}

bool cmCMakePresetsGraph::ConfigurePreset::VisitPresetAfterInherit(
  int version, cmJSONState* state)
{
  auto& preset = *this;
  if (!preset.Hidden) {
    if (version < 3) {
      if (preset.Generator.empty()) {
        cmCMakePresetErrors::PRESET_MISSING_FIELD(preset.Name, "generator",
                                                  state);
        return false;
      }
      if (preset.BinaryDir.empty()) {
        cmCMakePresetErrors::PRESET_MISSING_FIELD(preset.Name, "binaryDir",
                                                  state);
        return false;
      }
    }

    if (preset.WarnDev == false && preset.ErrorDev == true) {
      return false;
    }
    if (preset.WarnDeprecated == false && preset.ErrorDeprecated == true) {
      return false;
    }
    if (preset.CacheVariables.count("") != 0) {
      return false;
    }
  }

  return true;
}

bool cmCMakePresetsGraph::BuildPreset::VisitPresetInherit(
  const cmCMakePresetsGraph::Preset& parentPreset)
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
  if (!preset.ResolvePackageReferences) {
    preset.ResolvePackageReferences = parent.ResolvePackageReferences;
  }

  return true;
}

bool cmCMakePresetsGraph::BuildPreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*stat*/)
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return false;
  }
  return true;
}

bool cmCMakePresetsGraph::TestPreset::VisitPresetInherit(
  const cmCMakePresetsGraph::Preset& parentPreset)
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
      InheritString(output.OutputJUnitFile, parentOutput.OutputJUnitFile);
      InheritOptionalValue(output.LabelSummary, parentOutput.LabelSummary);
      InheritOptionalValue(output.SubprojectSummary,
                           parentOutput.SubprojectSummary);
      InheritOptionalValue(output.MaxPassedTestOutputSize,
                           parentOutput.MaxPassedTestOutputSize);
      InheritOptionalValue(output.MaxFailedTestOutputSize,
                           parentOutput.MaxFailedTestOutputSize);
      InheritOptionalValue(output.TestOutputTruncation,
                           parentOutput.TestOutputTruncation);
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

  return true;
}

bool cmCMakePresetsGraph::TestPreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*state*/)
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return false;
  }
  return true;
}

bool cmCMakePresetsGraph::PackagePreset::VisitPresetInherit(
  const cmCMakePresetsGraph::Preset& parentPreset)
{
  auto& preset = *this;
  const PackagePreset& parent =
    static_cast<const PackagePreset&>(parentPreset);

  InheritString(preset.ConfigurePreset, parent.ConfigurePreset);
  InheritOptionalValue(preset.InheritConfigureEnvironment,
                       parent.InheritConfigureEnvironment);
  InheritVector(preset.Generators, parent.Generators);
  InheritVector(preset.Configurations, parent.Configurations);

  for (auto const& v : parent.Variables) {
    preset.Variables.insert(v);
  }

  InheritOptionalValue(preset.DebugOutput, parent.DebugOutput);
  InheritOptionalValue(preset.VerboseOutput, parent.VerboseOutput);
  InheritString(preset.PackageName, parent.PackageName);
  InheritString(preset.PackageVersion, parent.PackageVersion);
  InheritString(preset.PackageDirectory, parent.PackageDirectory);
  InheritString(preset.VendorName, parent.VendorName);

  return true;
}

bool cmCMakePresetsGraph::PackagePreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*state*/)
{
  auto& preset = *this;
  if (!preset.Hidden && preset.ConfigurePreset.empty()) {
    return false;
  }
  return true;
}

bool cmCMakePresetsGraph::WorkflowPreset::VisitPresetInherit(
  const cmCMakePresetsGraph::Preset& /*parentPreset*/)
{
  return true;
}

bool cmCMakePresetsGraph::WorkflowPreset::VisitPresetAfterInherit(
  int /* version */, cmJSONState* /*state*/)
{
  return true;
}

std::string cmCMakePresetsGraph::GetFilename(const std::string& sourceDir)
{
  return cmStrCat(sourceDir, "/CMakePresets.json");
}

std::string cmCMakePresetsGraph::GetUserFilename(const std::string& sourceDir)
{
  return cmStrCat(sourceDir, "/CMakeUserPresets.json");
}

bool cmCMakePresetsGraph::ReadProjectPresets(const std::string& sourceDir,
                                             bool allowNoFiles)
{
  this->SourceDir = sourceDir;
  this->ClearPresets();

  if (!this->ReadProjectPresetsInternal(allowNoFiles)) {
    this->ClearPresets();
    return false;
  }

  return true;
}

bool cmCMakePresetsGraph::ReadProjectPresetsInternal(bool allowNoFiles)
{
  bool haveOneFile = false;

  File* file;
  std::string filename = GetUserFilename(this->SourceDir);
  std::vector<File*> inProgressFiles;
  if (cmSystemTools::FileExists(filename)) {
    if (!this->ReadJSONFile(filename, RootType::User, ReadReason::Root,
                            inProgressFiles, file, this->errors)) {
      return false;
    }
    haveOneFile = true;
  } else {
    filename = GetFilename(this->SourceDir);
    if (cmSystemTools::FileExists(filename)) {
      if (!this->ReadJSONFile(filename, RootType::Project, ReadReason::Root,
                              inProgressFiles, file, this->errors)) {
        return false;
      }
      haveOneFile = true;
    }
  }
  assert(inProgressFiles.empty());

  if (!haveOneFile) {
    if (allowNoFiles) {
      return true;
    }
    cmCMakePresetErrors::FILE_NOT_FOUND(filename, &this->parseState);
    return false;
  }

  bool result = ComputePresetInheritance(this->ConfigurePresets, *this) &&
    ComputePresetInheritance(this->ConfigurePresets, *this) &&
    ComputePresetInheritance(this->BuildPresets, *this) &&
    ComputePresetInheritance(this->TestPresets, *this) &&
    ComputePresetInheritance(this->PackagePresets, *this) &&
    ComputePresetInheritance(this->WorkflowPresets, *this);
  if (!result) {
    return false;
  }

  for (auto& it : this->ConfigurePresets) {
    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      cmCMakePresetErrors::INVALID_MACRO_EXPANSION(it.first,
                                                   &this->parseState);
      return false;
    }
  }

  for (auto& it : this->BuildPresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        cmCMakePresetErrors::INVALID_CONFIGURE_PRESET(it.first,
                                                      &this->parseState);
        return false;
      }
      if (!it.second.Unexpanded.OriginFile->ReachableFiles.count(
            configurePreset->second.Unexpanded.OriginFile)) {
        cmCMakePresetErrors::CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(
          it.first, &this->parseState);
        return false;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      cmCMakePresetErrors::INVALID_MACRO_EXPANSION(it.first,
                                                   &this->parseState);
      return false;
    }
  }

  for (auto& it : this->TestPresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        cmCMakePresetErrors::INVALID_CONFIGURE_PRESET(it.first,
                                                      &this->parseState);
        return false;
      }
      if (!it.second.Unexpanded.OriginFile->ReachableFiles.count(
            configurePreset->second.Unexpanded.OriginFile)) {
        cmCMakePresetErrors::CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(
          it.first, &this->parseState);
        return false;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      cmCMakePresetErrors::INVALID_MACRO_EXPANSION(it.first,
                                                   &this->parseState);
      return false;
    }
  }

  for (auto& it : this->PackagePresets) {
    if (!it.second.Unexpanded.Hidden) {
      const auto configurePreset =
        this->ConfigurePresets.find(it.second.Unexpanded.ConfigurePreset);
      if (configurePreset == this->ConfigurePresets.end()) {
        cmCMakePresetErrors::INVALID_CONFIGURE_PRESET(it.first,
                                                      &this->parseState);
        return false;
      }
      if (!it.second.Unexpanded.OriginFile->ReachableFiles.count(
            configurePreset->second.Unexpanded.OriginFile)) {
        cmCMakePresetErrors::CONFIGURE_PRESET_UNREACHABLE_FROM_FILE(
          it.first, &this->parseState);
        return false;
      }

      if (it.second.Unexpanded.InheritConfigureEnvironment.value_or(true)) {
        it.second.Unexpanded.Environment.insert(
          configurePreset->second.Unexpanded.Environment.begin(),
          configurePreset->second.Unexpanded.Environment.end());
      }
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      cmCMakePresetErrors::INVALID_MACRO_EXPANSION(it.first,
                                                   &this->parseState);
      return false;
    }
  }

  for (auto& it : this->WorkflowPresets) {
    using Type = WorkflowPreset::WorkflowStep::Type;

    const ConfigurePreset* configurePreset = nullptr;
    for (auto const& step : it.second.Unexpanded.Steps) {
      if (configurePreset == nullptr && step.PresetType != Type::Configure) {
        cmCMakePresetErrors::FIRST_WORKFLOW_STEP_NOT_CONFIGURE(
          step.PresetName, &this->parseState);
        return false;
      }
      if (configurePreset != nullptr && step.PresetType == Type::Configure) {
        cmCMakePresetErrors::CONFIGURE_WORKFLOW_STEP_NOT_FIRST(
          step.PresetName, &this->parseState);
        return false;
      }

      switch (step.PresetType) {
        case Type::Configure:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->ConfigurePresets, step.PresetName,
            configurePreset, &this->parseState);
          break;
        case Type::Build:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->BuildPresets, step.PresetName,
            configurePreset, &this->parseState);
          break;
        case Type::Test:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->TestPresets, step.PresetName,
            configurePreset, &this->parseState);
          break;
        case Type::Package:
          result = TryReachPresetFromWorkflow(
            it.second.Unexpanded, this->PackagePresets, step.PresetName,
            configurePreset, &this->parseState);
          break;
      }
      if (!result) {
        return false;
      }
    }

    if (configurePreset == nullptr) {
      cmCMakePresetErrors::NO_WORKFLOW_STEPS(it.first, &this->parseState);
      return false;
    }

    if (!ExpandMacros(*this, it.second.Unexpanded, it.second.Expanded)) {
      cmCMakePresetErrors::INVALID_MACRO_EXPANSION(it.first,
                                                   &this->parseState);
      return false;
    }
  }

  return true;
}

void cmCMakePresetsGraph::ClearPresets()
{
  this->ConfigurePresets.clear();
  this->BuildPresets.clear();
  this->TestPresets.clear();
  this->PackagePresets.clear();
  this->WorkflowPresets.clear();

  this->ConfigurePresetOrder.clear();
  this->BuildPresetOrder.clear();
  this->TestPresetOrder.clear();
  this->PackagePresetOrder.clear();
  this->WorkflowPresetOrder.clear();

  this->Files.clear();
}

void cmCMakePresetsGraph::printPrecedingNewline(PrintPrecedingNewline* newline)
{
  if (newline) {
    if (*newline == PrintPrecedingNewline::True) {
      std::cout << std::endl;
    }
    *newline = PrintPrecedingNewline::True;
  }
}

void cmCMakePresetsGraph::PrintPresets(
  const std::vector<const cmCMakePresetsGraph::Preset*>& presets)
{
  if (presets.empty()) {
    return;
  }

  auto longestPresetName =
    std::max_element(presets.begin(), presets.end(),
                     [](const cmCMakePresetsGraph::Preset* a,
                        const cmCMakePresetsGraph::Preset* b) {
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

void cmCMakePresetsGraph::PrintConfigurePresetList(
  PrintPrecedingNewline* newline) const
{
  PrintConfigurePresetList([](const ConfigurePreset&) { return true; },
                           newline);
}

void cmCMakePresetsGraph::PrintConfigurePresetList(
  const std::function<bool(const ConfigurePreset&)>& filter,
  PrintPrecedingNewline* newline) const
{
  std::vector<const cmCMakePresetsGraph::Preset*> presets;
  for (auto const& p : this->ConfigurePresetOrder) {
    auto const& preset = this->ConfigurePresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult && filter(preset.Unexpanded)) {
      presets.push_back(
        static_cast<const cmCMakePresetsGraph::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    printPrecedingNewline(newline);
    std::cout << "Available configure presets:\n\n";
    cmCMakePresetsGraph::PrintPresets(presets);
  }
}

void cmCMakePresetsGraph::PrintBuildPresetList(
  PrintPrecedingNewline* newline) const
{
  std::vector<const cmCMakePresetsGraph::Preset*> presets;
  for (auto const& p : this->BuildPresetOrder) {
    auto const& preset = this->BuildPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult) {
      presets.push_back(
        static_cast<const cmCMakePresetsGraph::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    printPrecedingNewline(newline);
    std::cout << "Available build presets:\n\n";
    cmCMakePresetsGraph::PrintPresets(presets);
  }
}

void cmCMakePresetsGraph::PrintTestPresetList(
  PrintPrecedingNewline* newline) const
{
  std::vector<const cmCMakePresetsGraph::Preset*> presets;
  for (auto const& p : this->TestPresetOrder) {
    auto const& preset = this->TestPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult) {
      presets.push_back(
        static_cast<const cmCMakePresetsGraph::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    printPrecedingNewline(newline);
    std::cout << "Available test presets:\n\n";
    cmCMakePresetsGraph::PrintPresets(presets);
  }
}

void cmCMakePresetsGraph::PrintPackagePresetList(
  PrintPrecedingNewline* newline) const
{
  this->PrintPackagePresetList([](const PackagePreset&) { return true; },
                               newline);
}

void cmCMakePresetsGraph::PrintPackagePresetList(
  const std::function<bool(const PackagePreset&)>& filter,
  PrintPrecedingNewline* newline) const
{
  std::vector<const cmCMakePresetsGraph::Preset*> presets;
  for (auto const& p : this->PackagePresetOrder) {
    auto const& preset = this->PackagePresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult && filter(preset.Unexpanded)) {
      presets.push_back(
        static_cast<const cmCMakePresetsGraph::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    printPrecedingNewline(newline);
    std::cout << "Available package presets:\n\n";
    cmCMakePresetsGraph::PrintPresets(presets);
  }
}

void cmCMakePresetsGraph::PrintWorkflowPresetList(
  PrintPrecedingNewline* newline) const
{
  std::vector<const cmCMakePresetsGraph::Preset*> presets;
  for (auto const& p : this->WorkflowPresetOrder) {
    auto const& preset = this->WorkflowPresets.at(p);
    if (!preset.Unexpanded.Hidden && preset.Expanded &&
        preset.Expanded->ConditionResult) {
      presets.push_back(
        static_cast<const cmCMakePresetsGraph::Preset*>(&preset.Unexpanded));
    }
  }

  if (!presets.empty()) {
    printPrecedingNewline(newline);
    std::cout << "Available workflow presets:\n\n";
    cmCMakePresetsGraph::PrintPresets(presets);
  }
}

void cmCMakePresetsGraph::PrintAllPresets() const
{
  PrintPrecedingNewline newline = PrintPrecedingNewline::False;
  this->PrintConfigurePresetList(&newline);
  this->PrintBuildPresetList(&newline);
  this->PrintTestPresetList(&newline);
  this->PrintPackagePresetList(&newline);
  this->PrintWorkflowPresetList(&newline);
}
