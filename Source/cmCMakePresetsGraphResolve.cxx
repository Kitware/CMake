/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <cassert>
#include <map>
#include <string>

#include <cm/optional>

#include "cmCMakePresetsGraph.h"
#include "cmStringAlgorithms.h"

template <class T>
cmCMakePresetsGraph::PresetResolveResult<T> cmCMakePresetsGraph::ResolvePreset(
  std::string const& presetName,
  std::map<std::string, PresetPair<T>> const& presets) const
{
  using Result = PresetResolveResult<T>;
  Result result;

  auto presetPair = presets.find(presetName);
  if (presetPair == presets.end()) {
    result.StatusCode = Result::Status::NotFound;
    result.ErrorPresetName = presetName;
    return result;
  }
  if (presetPair->second.Unexpanded.Hidden) {
    result.StatusCode = Result::Status::Hidden;
    result.ErrorPresetName = presetName;
    return result;
  }
  if (!presetPair->second.Expanded) {
    result.StatusCode = Result::Status::InvalidMacroExpansion;
    result.ErrorPresetName = presetName;
    return result;
  }
  if (!presetPair->second.Expanded->ConditionResult) {
    result.StatusCode = Result::Status::Disabled;
    result.ErrorPresetName = presetName;
    return result;
  }

  result.Preset = &*presetPair->second.Expanded;
  return result;
}

// Explicit template instantiations
template cmCMakePresetsGraph::PresetResolveResult<
  cmCMakePresetsGraph::ConfigurePreset>
cmCMakePresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string,
           PresetPair<cmCMakePresetsGraph::ConfigurePreset>> const&) const;

template cmCMakePresetsGraph::PresetResolveResult<
  cmCMakePresetsGraph::BuildPreset>
cmCMakePresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmCMakePresetsGraph::BuildPreset>> const&)
  const;

template cmCMakePresetsGraph::PresetResolveResult<
  cmCMakePresetsGraph::TestPreset>
cmCMakePresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmCMakePresetsGraph::TestPreset>> const&)
  const;

template cmCMakePresetsGraph::PresetResolveResult<
  cmCMakePresetsGraph::PackagePreset>
cmCMakePresetsGraph::ResolvePreset(
  std::string const&,
  std::map<std::string, PresetPair<cmCMakePresetsGraph::PackagePreset>> const&)
  const;

template <class T>
cm::optional<std::string> cmCMakePresetsGraph::FormatPresetError(
  PresetResolveStatus status, std::string const& errorPresetName,
  std::string const& directory)
{
  using Status = PresetResolveStatus;
  switch (status) {
    case Status::NotFound:
      return cmStrCat("No such ", T::kind(), " preset in ", directory, ": \"",
                      errorPresetName, '"');
    case Status::Hidden:
      return cmStrCat("Cannot use hidden ", T::kind(), " preset in ",
                      directory, ": \"", errorPresetName, '"');
    case Status::InvalidMacroExpansion:
      return cmStrCat("Could not evaluate ", T::kind(), " preset \"",
                      errorPresetName, "\": Invalid macro expansion");
    case Status::Disabled:
      return cmStrCat("Cannot use disabled ", T::kind(), " preset in ",
                      directory, ": \"", errorPresetName, '"');
    case Status::Success:
      return cm::nullopt;
  }
  assert(false && "Unreachable.");
  return cm::nullopt;
}

// Explicit template instantiations for FormatPresetError
template cm::optional<std::string>
cmCMakePresetsGraph::FormatPresetError<cmCMakePresetsGraph::ConfigurePreset>(
  PresetResolveStatus, std::string const&, std::string const&);

template cm::optional<std::string>
cmCMakePresetsGraph::FormatPresetError<cmCMakePresetsGraph::BuildPreset>(
  PresetResolveStatus, std::string const&, std::string const&);

template cm::optional<std::string>
cmCMakePresetsGraph::FormatPresetError<cmCMakePresetsGraph::TestPreset>(
  PresetResolveStatus, std::string const&, std::string const&);

template cm::optional<std::string>
cmCMakePresetsGraph::FormatPresetError<cmCMakePresetsGraph::PackagePreset>(
  PresetResolveStatus, std::string const&, std::string const&);
