/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmStandardLevelResolver.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmFileSet.h"
#include "cmFileSetMetadata.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmStandardLevel.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

#define FEATURE_STRING(F) , #F
char const* const C_FEATURES[] = { nullptr FOR_EACH_C_FEATURE(
  FEATURE_STRING) };

char const* const CXX_FEATURES[] = { nullptr FOR_EACH_CXX_FEATURE(
  FEATURE_STRING) };

char const* const CUDA_FEATURES[] = { nullptr FOR_EACH_CUDA_FEATURE(
  FEATURE_STRING) };

char const* const HIP_FEATURES[] = { nullptr FOR_EACH_HIP_FEATURE(
  FEATURE_STRING) };

char const* const SYCL_FEATURES[] = { nullptr FOR_EACH_SYCL_FEATURE(
  FEATURE_STRING) };
#undef FEATURE_STRING

bool TargetUsesSYCLCompileFeatures(cmMakefile* makefile,
                                   std::string const& targetName)
{
  auto* globalGenerator = makefile->GetGlobalGenerator();
  if (!globalGenerator->GetLanguageEnabled("SYCL")) {
    return false;
  }
  if (!globalGenerator->GetLanguageEnabled("CXX")) {
    return true;
  }

  cmTarget* target = makefile->FindTargetToUse(targetName);
  if (!target) {
    return false;
  }

  if (cmGeneratorTarget* generatorTarget =
        globalGenerator->FindGeneratorTarget(targetName)) {
    std::set<std::string> const languages =
      generatorTarget->GetAllConfigCompileLanguages();
    return languages.find("SYCL") != languages.end() &&
      languages.find("CXX") == languages.end();
  }

  bool hasCxxSource = false;
  bool hasSyclSource = false;
  bool hasSyclHeaders = false;
  auto recordLanguage = [&hasCxxSource, &hasSyclSource](cm::string_view lang) {
    hasCxxSource = hasCxxSource || lang == "CXX"_s;
    hasSyclSource = hasSyclSource || lang == "SYCL"_s;
  };
  for (auto const& name : target->GetAllPrivateFileSets()) {
    cmFileSet const* fileSet = target->GetFileSet(name);
    if (fileSet->GetType() == cm::FileSetMetadata::HEADERS) {
      hasSyclHeaders =
        hasSyclHeaders || fileSet->GetProperty("LANGUAGE") == "SYCL"_s;
      continue;
    }
    if (fileSet->GetType() != cm::FileSetMetadata::SOURCES ||
        fileSet->GetFileEntries().empty()) {
      continue;
    }
    cmValue const language = fileSet->GetProperty("LANGUAGE");
    if (!language.IsEmpty()) {
      recordLanguage(*language);
      continue;
    }
    for (auto const& entry : fileSet->GetFileEntries()) {
      for (auto const& source : cmList{ entry.Value }) {
        if (cmGeneratorExpression::Find(source) == std::string::npos) {
          recordLanguage(target->GetMakefile()
                           ->GetOrCreateSource(source)
                           ->GetOrDetermineLanguage());
        }
      }
    }
  }
  for (auto const& entry : target->GetSourceEntries()) {
    cmList sources{ entry.Value };
    for (auto const& source : sources) {
      if (cmGeneratorExpression::Find(source) != std::string::npos) {
        continue;
      }
      std::string const& language = target->GetMakefile()
                                      ->GetOrCreateSource(source)
                                      ->GetOrDetermineLanguage();
      recordLanguage(language);
    }
  }
  if (hasSyclHeaders && hasCxxSource) {
    hasCxxSource = false;
    hasSyclSource = true;
  }
  return hasSyclSource && !hasCxxSource;
}

int ParseStd(std::string const& level)
{
  try {
    return std::stoi(level);
  } catch (std::invalid_argument&) {
    // Fall through to use an invalid value.
  }
  return -1;
}

struct StandardLevelComputer
{
  explicit StandardLevelComputer(std::string lang, std::vector<int> levels,
                                 std::vector<std::string> levelsStr)
    : Language(std::move(lang))
    , Levels(std::move(levels))
    , LevelsAsStrings(std::move(levelsStr))
  {
    assert(this->Levels.size() == this->LevelsAsStrings.size());
  }

  // Note that the logic here is shadowed in `GetEffectiveStandard`; if one is
  // changed, the other needs changed as well.
  std::string GetCompileOptionDef(cmMakefile* makefile,
                                  cmGeneratorTarget const* target,
                                  std::string const& config) const
  {

    auto const& stds = this->Levels;
    auto const& stdsStrings = this->LevelsAsStrings;

    cmValue defaultStd = makefile->GetDefinition(
      cmStrCat("CMAKE_"_s, this->Language, "_STANDARD_DEFAULT"_s));
    if (!cmNonempty(defaultStd)) {
      // this compiler has no notion of language standard levels
      return std::string{};
    }

    cmPolicies::PolicyStatus const cmp0128{ makefile->GetPolicyStatus(
      cmPolicies::CMP0128) };
    bool const defaultExt{ makefile
                             ->GetDefinition(cmStrCat("CMAKE_"_s,
                                                      this->Language,
                                                      "_EXTENSIONS_DEFAULT"_s))
                             .IsOn() };
    bool ext = true;

    if (cmp0128 == cmPolicies::NEW) {
      ext = defaultExt;
    }

    if (cmValue extPropValue = target->GetLanguageExtensions(this->Language)) {
      ext = extPropValue.IsOn();
    }

    std::string const type{ ext ? "EXTENSION" : "STANDARD" };

    auto const strictPermissiveOption = [&](std::string const& stdNum) {
      if (target->GetPolicyStatusCMP0231() != cmPolicies::NEW) {
        return std::string{};
      }
      std::string option =
        cmStrCat("CMAKE_"_s, this->Language, stdNum,
                 ext ? "_EXTENSION_COMPILE_OPTION_PERMISSIVE"_s
                     : "_STANDARD_COMPILE_OPTION_PERMISSIVE"_s);
      if (makefile->GetDefinition(option).IsEmpty()) {
        return std::string{};
      }
      return option;
    };

    cmValue standardProp = target->GetLanguageStandard(this->Language, config);
    if (!standardProp) {
      if (cmp0128 == cmPolicies::NEW) {
        // Add extension flag if compiler's default doesn't match.
        if (ext != defaultExt) {
          return cmStrCat("CMAKE_"_s, this->Language, *defaultStd, '_', type,
                          "_COMPILE_OPTION"_s);
        }
      } else {
        if (cmp0128 == cmPolicies::WARN &&
            makefile->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0128") &&
            ext != defaultExt) {
          char const* state{};
          if (ext) {
            if (!makefile->GetDefinition(
                  cmStrCat("CMAKE_"_s, this->Language,
                           "_EXTENSION_COMPILE_OPTION"_s))) {
              state = "enabled";
            }
          } else {
            state = "disabled";
          }
          if (state) {
            makefile->IssuePolicyWarning(
              cmPolicies::CMP0128, {},
              cmStrCat("For compatibility with older versions of CMake, "
                       "compiler extensions won't be "_s,
                       state, '.'));
          }
        }

        if (ext) {
          return cmStrCat("CMAKE_"_s, this->Language,
                          "_EXTENSION_COMPILE_OPTION"_s);
        }
      }
      if (cmp0128 == cmPolicies::NEW) {
        return strictPermissiveOption(*defaultStd);
      }
      return std::string{};
    }

    if (target->GetLanguageStandardRequired(this->Language)) {
      std::string option_flag =
        cmStrCat("CMAKE_"_s, this->Language, *standardProp, '_', type,
                 "_COMPILE_OPTION"_s);

      cmValue opt = target->Target->GetMakefile()->GetDefinition(option_flag);
      if (!opt) {
        makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Target \""_s, target->GetName(),
                   "\" requires the language dialect \""_s, this->Language,
                   *standardProp, "\" "_s,
                   ext ? "(with compiler extensions)" : "",
                   ". But the current compiler \""_s,
                   makefile->GetSafeDefinition(
                     cmStrCat("CMAKE_"_s, this->Language, "_COMPILER_ID"_s)),
                   "\" does not support this, or "
                   "CMake does not know the flags to enable it."_s));
      }
      return option_flag;
    }

    // If the request matches the compiler's defaults we don't need to add
    // anything.
    if (*standardProp == *defaultStd && ext == defaultExt) {
      if (cmp0128 == cmPolicies::NEW) {
        return strictPermissiveOption(*standardProp);
      }

      if (cmp0128 == cmPolicies::WARN &&
          makefile->PolicyOptionalWarningEnabled(
            "CMAKE_POLICY_WARNING_CMP0128")) {
        makefile->IssuePolicyWarning(
          cmPolicies::CMP0128, {},
          "For compatibility with older versions of CMake, unnecessary flags "
          "for language standard or compiler extensions may be added."_s);
      }
    }

    std::string standardStr(*standardProp);
    if (this->Language == "CUDA"_s && standardStr == "98"_s) {
      standardStr = "03";
    }

    auto stdIt =
      std::find(std::cbegin(stds), std::cend(stds), ParseStd(standardStr));
    if (stdIt == std::cend(stds)) {
      std::string e = cmStrCat(
        cmStandardLevelResolver::GetStandardPropertyName(this->Language),
        " is set to invalid value '"_s, standardStr, '\'');
      makefile->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e,
                                                 target->GetBacktrace());
      return std::string{};
    }

    auto defaultStdIt =
      std::find(std::cbegin(stds), std::cend(stds), ParseStd(*defaultStd));
    if (defaultStdIt == std::cend(stds)) {
      std::string e = cmStrCat("CMAKE_"_s, this->Language,
                               "_STANDARD_DEFAULT is set to invalid value '"_s,
                               *defaultStd, '\'');
      makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
      return std::string{};
    }

    // If the standard requested is older than the compiler's default or the
    // extension mode doesn't match then we need to use a flag.
    if ((cmp0128 != cmPolicies::NEW && stdIt <= defaultStdIt) ||
        (cmp0128 == cmPolicies::NEW &&
         (stdIt < defaultStdIt || ext != defaultExt))) {
      auto offset = std::distance(std::cbegin(stds), stdIt);
      return cmStrCat("CMAKE_"_s, this->Language, stdsStrings[offset], '_',
                      type, "_COMPILE_OPTION"_s);
    }

    // The compiler's default is at least as new as the requested standard,
    // and the requested standard is not required.  Decay to the newest
    // standard for which a flag is defined.
    for (; defaultStdIt < stdIt; --stdIt) {
      auto offset = std::distance(std::cbegin(stds), stdIt);
      std::string option_flag =
        cmStrCat("CMAKE_"_s, this->Language, stdsStrings[offset], '_', type,
                 "_COMPILE_OPTION"_s);
      if (target->Target->GetMakefile()->GetDefinition(option_flag)) {
        return option_flag;
      }
    }

    return std::string{};
  }

  std::string GetEffectiveStandard(cmMakefile* makefile,
                                   cmGeneratorTarget const* target,
                                   std::string const& config) const
  {
    auto const& stds = this->Levels;
    auto const& stdsStrings = this->LevelsAsStrings;

    cmValue defaultStd = makefile->GetDefinition(
      cmStrCat("CMAKE_"_s, this->Language, "_STANDARD_DEFAULT"_s));
    if (!cmNonempty(defaultStd)) {
      // this compiler has no notion of language standard levels
      return std::string{};
    }

    cmPolicies::PolicyStatus const cmp0128{ makefile->GetPolicyStatus(
      cmPolicies::CMP0128) };
    bool const defaultExt{ makefile
                             ->GetDefinition(cmStrCat("CMAKE_"_s,
                                                      this->Language,
                                                      "_EXTENSIONS_DEFAULT"_s))
                             .IsOn() };
    bool ext = true;

    if (cmp0128 == cmPolicies::NEW) {
      ext = defaultExt;
    }

    if (cmValue extPropValue = target->GetLanguageExtensions(this->Language)) {
      ext = extPropValue.IsOn();
    }

    std::string const type{ ext ? "EXTENSION" : "STANDARD" };

    cmValue standardProp = target->GetLanguageStandard(this->Language, config);
    if (!standardProp) {
      if (cmp0128 == cmPolicies::NEW) {
        // Add extension flag if compiler's default doesn't match.
        if (ext != defaultExt) {
          return *defaultStd;
        }
      } else {
        if (ext) {
          return *defaultStd;
        }
      }
      return std::string{};
    }

    if (target->GetLanguageStandardRequired(this->Language)) {
      return *standardProp;
    }

    // If the request matches the compiler's defaults we don't need to add
    // anything.
    if (*standardProp == *defaultStd && ext == defaultExt) {
      if (cmp0128 == cmPolicies::NEW) {
        return std::string{};
      }
    }

    std::string standardStr(*standardProp);
    if (this->Language == "CUDA"_s && standardStr == "98"_s) {
      standardStr = "03";
    }

    auto stdIt =
      std::find(std::cbegin(stds), std::cend(stds), ParseStd(standardStr));
    if (stdIt == std::cend(stds)) {
      return std::string{};
    }

    auto defaultStdIt =
      std::find(std::cbegin(stds), std::cend(stds), ParseStd(*defaultStd));
    if (defaultStdIt == std::cend(stds)) {
      return std::string{};
    }

    // If the standard requested is older than the compiler's default or the
    // extension mode doesn't match then we need to use a flag.
    if ((cmp0128 != cmPolicies::NEW && stdIt <= defaultStdIt) ||
        (cmp0128 == cmPolicies::NEW &&
         (stdIt < defaultStdIt || ext != defaultExt))) {
      auto offset = std::distance(std::cbegin(stds), stdIt);
      return stdsStrings[offset];
    }

    // The compiler's default is at least as new as the requested standard,
    // and the requested standard is not required.  Decay to the newest
    // standard for which a flag is defined.
    for (; defaultStdIt < stdIt; --stdIt) {
      auto offset = std::distance(std::cbegin(stds), stdIt);
      std::string option_flag =
        cmStrCat("CMAKE_"_s, this->Language, stdsStrings[offset], '_', type,
                 "_COMPILE_OPTION"_s);
      if (target->Target->GetMakefile()->GetDefinition(option_flag)) {
        return stdsStrings[offset];
      }
    }

    return std::string{};
  }

  bool GetNewRequiredStandard(cmMakefile* makefile,
                              std::string const& targetName,
                              cm::optional<cmStandardLevel> featureLevel,
                              cmValue currentLangStandardValue,
                              std::string& newRequiredStandard,
                              std::string* error) const
  {
    if (currentLangStandardValue) {
      newRequiredStandard = *currentLangStandardValue;
    } else {
      newRequiredStandard.clear();
    }

    cmValue existingStandard = currentLangStandardValue;
    if (!existingStandard) {
      cmValue defaultStandard = makefile->GetDefinition(
        cmStrCat("CMAKE_"_s, this->Language, "_STANDARD_DEFAULT"_s));
      if (cmNonempty(defaultStandard)) {
        existingStandard = defaultStandard;
      }
    }

    auto existingLevelIter = std::cend(this->Levels);
    if (existingStandard) {
      existingLevelIter =
        std::find(std::cbegin(this->Levels), std::cend(this->Levels),
                  ParseStd(*existingStandard));
      if (existingLevelIter == std::cend(this->Levels)) {
        std::string const e = cmStrCat(
          "The "_s,
          cmStandardLevelResolver::GetStandardPropertyName(this->Language),
          " property on target \""_s, targetName,
          "\" contained an invalid value: \""_s, *existingStandard, "\"."_s);
        if (error) {
          *error = e;
        } else {
          makefile->IssueMessage(MessageType::FATAL_ERROR, e);
        }
        return false;
      }
    }

    if (featureLevel) {
      // Ensure the C++ language level is high enough to support
      // the needed C++ features.
      if (existingLevelIter == std::cend(this->Levels) ||
          existingLevelIter < this->Levels.begin() + featureLevel->Index()) {
        newRequiredStandard = this->LevelsAsStrings[featureLevel->Index()];
      }
    }

    return true;
  }

  bool HaveStandardAvailable(cmMakefile* makefile,
                             cmGeneratorTarget const* target,
                             std::string const& config,
                             std::string const& feature) const
  {
    cmValue defaultStandard = makefile->GetDefinition(
      cmStrCat("CMAKE_"_s, this->Language, "_STANDARD_DEFAULT"_s));
    if (!defaultStandard) {
      makefile->IssueMessage(
        MessageType::INTERNAL_ERROR,
        cmStrCat("CMAKE_"_s, this->Language,
                 "_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
                 "not fully configured for this compiler."_s));
      // Return true so the caller does not try to lookup the default standard.
      return true;
    }
    // convert defaultStandard to an integer
    if (std::find(std::cbegin(this->Levels), std::cend(this->Levels),
                  ParseStd(*defaultStandard)) == std::cend(this->Levels)) {
      std::string const e = cmStrCat("The CMAKE_"_s, this->Language,
                                     "_STANDARD_DEFAULT variable contains an "
                                     "invalid value: \""_s,
                                     *defaultStandard, "\"."_s);
      makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
      return false;
    }

    cmValue existingStandard =
      target->GetLanguageStandard(this->Language, config);
    if (!existingStandard) {
      existingStandard = defaultStandard;
    }

    auto existingLevelIter =
      std::find(std::cbegin(this->Levels), std::cend(this->Levels),
                ParseStd(*existingStandard));
    if (existingLevelIter == std::cend(this->Levels)) {
      std::string const e = cmStrCat(
        "The "_s,
        cmStandardLevelResolver::GetStandardPropertyName(this->Language),
        " property on target \""_s, target->GetName(),
        "\" contained an invalid value: \""_s, *existingStandard, "\"."_s);
      makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      return false;
    }

    cm::optional<cmStandardLevel> needed =
      this->CompileFeatureStandardLevel(makefile, feature);

    return !needed ||
      (this->Levels.begin() + needed->Index()) <= existingLevelIter;
  }

  cm::optional<cmStandardLevel> CompileFeatureStandardLevel(
    cmMakefile* makefile, std::string const& feature) const
  {
    std::string prefix = cmStrCat("CMAKE_"_s, this->Language);
    cm::optional<cmStandardLevel> maxLevel;
    for (size_t i = 0; i < this->Levels.size(); ++i) {
      if (cmValue prop = makefile->GetDefinition(cmStrCat(
            prefix, this->LevelsAsStrings[i], "_COMPILE_FEATURES"_s))) {
        cmList props{ *prop };
        if (cm::contains(props, feature)) {
          maxLevel = cmStandardLevel(i);
        }
      }
    }
    return maxLevel;
  }

  cm::optional<cmStandardLevel> LanguageStandardLevel(
    std::string const& standardStr) const
  {
    cm::optional<cmStandardLevel> langLevel;
    auto const& stds = this->Levels;
    auto stdIt =
      std::find(std::cbegin(stds), std::cend(stds), ParseStd(standardStr));
    if (stdIt != std::cend(stds)) {
      langLevel = cmStandardLevel(std::distance(std::cbegin(stds), stdIt));
    }
    return langLevel;
  }

  bool IsLaterStandard(int lhs, int rhs) const
  {
    auto rhsIt =
      std::find(std::cbegin(this->Levels), std::cend(this->Levels), rhs);

    return std::find(rhsIt, std::cend(this->Levels), lhs) !=
      std::cend(this->Levels);
  }

  std::string Language;
  std::vector<int> Levels;
  std::vector<std::string> LevelsAsStrings;
};

std::unordered_map<std::string,
                   StandardLevelComputer> const StandardComputerMapping = {
  { "C",
    StandardLevelComputer{
      "C", std::vector<int>{ 90, 99, 11, 17, 23 },
      std::vector<std::string>{ "90", "99", "11", "17", "23" } } },
  { "CXX",
    StandardLevelComputer{
      "CXX", std::vector<int>{ 98, 11, 14, 17, 20, 23, 26 },
      std::vector<std::string>{ "98", "11", "14", "17", "20", "23", "26" } } },
  { "CUDA",
    StandardLevelComputer{
      "CUDA", std::vector<int>{ 03, 11, 14, 17, 20, 23, 26 },
      std::vector<std::string>{ "03", "11", "14", "17", "20", "23", "26" } } },
  { "OBJC",
    StandardLevelComputer{
      "OBJC", std::vector<int>{ 90, 99, 11, 17, 23 },
      std::vector<std::string>{ "90", "99", "11", "17", "23" } } },
  { "OBJCXX",
    StandardLevelComputer{
      "OBJCXX", std::vector<int>{ 98, 11, 14, 17, 20, 23, 26 },
      std::vector<std::string>{ "98", "11", "14", "17", "20", "23", "26" } } },
  { "HIP",
    StandardLevelComputer{
      "HIP", std::vector<int>{ 98, 11, 14, 17, 20, 23, 26 },
      std::vector<std::string>{ "98", "11", "14", "17", "20", "23", "26" } } },
  { "SYCL",
    StandardLevelComputer{
      "SYCL", std::vector<int>{ 98, 11, 14, 17, 20, 23, 26 },
      std::vector<std::string>{ "98", "11", "14", "17", "20", "23", "26" } } }
};
}

std::string cmStandardLevelResolver::GetStandardPropertyName(
  std::string const& lang)
{
  return lang == "SYCL" ? "SYCL_CXX_STANDARD" : cmStrCat(lang, "_STANDARD"_s);
}

std::string cmStandardLevelResolver::GetCompileOptionDef(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config) const
{
  auto const& mapping = StandardComputerMapping.find(lang);
  if (mapping == std::cend(StandardComputerMapping)) {
    return std::string{};
  }

  return mapping->second.GetCompileOptionDef(this->Makefile, target, config);
}

std::string cmStandardLevelResolver::GetPermissiveCompileOption(
  cmGeneratorTarget const* target, std::string const& optionFlagDef) const
{
  if (optionFlagDef.empty() ||
      target->GetPolicyStatusCMP0231() != cmPolicies::NEW) {
    return {};
  }
  return this->Makefile->GetSafeDefinition(
    cmStrCat(optionFlagDef, "_PERMISSIVE"_s));
}

std::string cmStandardLevelResolver::GetMsvcStdcCompileOption(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config) const
{
  // CMP0233: the compiler modules define a companion of the C standard
  // option variable for the standards that support /Zc:__STDC__.  Only the
  // STANDARD (not EXTENSION) variables have one, so it is found only when
  // extensions are off.
  if (target->GetPolicyStatusCMP0233() != cmPolicies::NEW) {
    return {};
  }
  bool const isC = lang == "C"_s;
  if (!isC) {
    if (lang != "CXX"_s) {
      return {};
    }
    // The Visual Studio generators compile the C and C++ sources of a
    // target with one set of options, so pass the flag with the C++
    // sources of targets that have C sources too in every generator.
    if (!target->IsLanguageUsed("C", config)) {
      return {};
    }
  }
  std::string const cOptionDef =
    this->GetCompileOptionDef(target, "C", config);
  if (cOptionDef.empty() ||
      this->Makefile->GetDefinition(cmStrCat(cOptionDef, "_MSVC_STDC"_s))
        .IsEmpty()) {
    return {};
  }
  if (isC) {
    return this->Makefile->GetSafeDefinition(
      cmStrCat(cOptionDef, "_MSVC_STDC"_s));
  }
  return this->Makefile->GetSafeDefinition(
    "CMAKE_CXX_COMPILE_OPTION_MSVC_STDC");
}

std::string cmStandardLevelResolver::GetEffectiveStandard(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config) const
{
  auto const& mapping = StandardComputerMapping.find(lang);
  if (mapping == std::cend(StandardComputerMapping)) {
    return std::string{};
  }

  return mapping->second.GetEffectiveStandard(this->Makefile, target, config);
}

std::string cmStandardLevelResolver::GetLevelString(
  std::string const& lang, cmStandardLevel level) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping == StandardComputerMapping.end()) {
    return {};
  }

  if (mapping->second.LevelsAsStrings.size() <= level.Index()) {
    return {};
  }

  return mapping->second.LevelsAsStrings[level.Index()];
}

bool cmStandardLevelResolver::AddRequiredTargetFeature(
  cmTarget* target, std::string const& feature, std::string* error) const
{
  if (cmGeneratorExpression::Find(feature) != std::string::npos) {
    target->AppendProperty("COMPILE_FEATURES", feature,
                           this->Makefile->GetBacktrace());
    return true;
  }

  std::string lang;
  if (!this->CheckCompileFeaturesAvailable(target->GetName(), feature, lang,
                                           error)) {
    return false;
  }

  target->AppendProperty("COMPILE_FEATURES", feature,
                         this->Makefile->GetBacktrace());

  // FIXME: Add a policy to avoid updating the <LANG>_STANDARD target
  // property due to COMPILE_FEATURES.  The language standard selection
  // should be done purely at generate time based on whatever the project
  // code put in these properties explicitly.  That is mostly true now,
  // but for compatibility we need to continue updating the property here.
  cm::optional<cmStandardLevel> featureLevel;
  std::string newRequiredStandard;
  bool succeeded = this->GetNewRequiredStandard(
    target->GetName(), feature,
    target->GetProperty(GetStandardPropertyName(lang)), featureLevel,
    newRequiredStandard, error);
  if (!newRequiredStandard.empty()) {
    target->SetProperty(GetStandardPropertyName(lang), newRequiredStandard);
  }
  return succeeded;
}

bool cmStandardLevelResolver::CheckCompileFeaturesAvailable(
  std::string const& targetName, std::string const& feature, std::string& lang,
  std::string* error) const
{
  if (!this->CompileFeatureKnown(targetName, feature, lang, error)) {
    return false;
  }

  if (!this->Makefile->GetGlobalGenerator()->GetLanguageEnabled(lang)) {
    return true;
  }

  cmValue features = this->CompileFeaturesAvailable(lang, error);
  if (!features) {
    return false;
  }

  cmList availableFeatures{ features };
  if (!cm::contains(availableFeatures, feature)) {
    std::string e = cmStrCat("The compiler feature \""_s, feature,
                             "\" is not known to "_s, lang, " compiler\n\""_s,
                             this->Makefile->GetSafeDefinition(
                               cmStrCat("CMAKE_"_s, lang, "_COMPILER_ID"_s)),
                             "\"\nversion "_s,
                             this->Makefile->GetSafeDefinition(cmStrCat(
                               "CMAKE_"_s, lang, "_COMPILER_VERSION"_s)),
                             '.');
    if (error) {
      *error = std::move(e);
    } else {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
    }
    return false;
  }

  return true;
}

bool cmStandardLevelResolver::CompileFeatureKnown(
  std::string const& targetName, std::string const& feature, std::string& lang,
  std::string* error) const
{
  assert(cmGeneratorExpression::Find(feature) == std::string::npos);

  bool isCFeature =
    std::find_if(std::cbegin(C_FEATURES) + 1, std::cend(C_FEATURES),
                 cmStrCmp(feature)) != std::cend(C_FEATURES);
  if (isCFeature) {
    lang = "C";
    return true;
  }
  bool isCxxFeature =
    std::find_if(std::cbegin(CXX_FEATURES) + 1, std::cend(CXX_FEATURES),
                 cmStrCmp(feature)) != std::cend(CXX_FEATURES);
  bool isSYCLFeature =
    std::find_if(std::cbegin(SYCL_FEATURES) + 1, std::cend(SYCL_FEATURES),
                 cmStrCmp(feature)) != std::cend(SYCL_FEATURES);
  if (isSYCLFeature &&
      (!isCxxFeature ||
       TargetUsesSYCLCompileFeatures(this->Makefile, targetName))) {
    lang = "SYCL";
    return true;
  }
  if (isCxxFeature) {
    lang = "CXX";
    return true;
  }
  bool isCudaFeature =
    std::find_if(std::cbegin(CUDA_FEATURES) + 1, std::cend(CUDA_FEATURES),
                 cmStrCmp(feature)) != std::cend(CUDA_FEATURES);
  if (isCudaFeature) {
    lang = "CUDA";
    return true;
  }
  bool isHIPFeature =
    std::find_if(std::cbegin(HIP_FEATURES) + 1, std::cend(HIP_FEATURES),
                 cmStrCmp(feature)) != std::cend(HIP_FEATURES);
  if (isHIPFeature) {
    lang = "HIP";
    return true;
  }
  std::string e =
    cmStrCat(error ? "specified" : "Specified", " unknown feature \""_s,
             feature, "\" for target \""_s, targetName, "\"."_s);
  if (error) {
    *error = std::move(e);
  } else {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
  }
  return false;
}

cm::optional<cmStandardLevel>
cmStandardLevelResolver::CompileFeatureStandardLevel(
  std::string const& lang, std::string const& feature) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping == std::cend(StandardComputerMapping)) {
    return cm::nullopt;
  }
  return mapping->second.CompileFeatureStandardLevel(this->Makefile, feature);
}

cm::optional<cmStandardLevel> cmStandardLevelResolver::LanguageStandardLevel(
  std::string const& lang, std::string const& standardStr) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping == std::cend(StandardComputerMapping)) {
    return cm::nullopt;
  }
  return mapping->second.LanguageStandardLevel(standardStr);
}

cmValue cmStandardLevelResolver::CompileFeaturesAvailable(
  std::string const& lang, std::string* error) const
{
  if (!this->Makefile->GetGlobalGenerator()->GetLanguageEnabled(lang)) {
    std::string e =
      cmStrCat(error ? "cannot"_s : "Cannot"_s,
               " use features from non-enabled language "_s, lang);
    if (error) {
      *error = std::move(e);
    } else {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
    }
    return nullptr;
  }

  cmValue featuresKnown = this->Makefile->GetDefinition(
    cmStrCat("CMAKE_"_s, lang, "_COMPILE_FEATURES"_s));

  if (!cmNonempty(featuresKnown)) {
    std::string e = cmStrCat(error ? "no"_s : "No"_s, " known features for "_s,
                             lang, " compiler\n\""_s,
                             this->Makefile->GetSafeDefinition(
                               cmStrCat("CMAKE_"_s, lang, "_COMPILER_ID"_s)),
                             "\"\nversion "_s,
                             this->Makefile->GetSafeDefinition(cmStrCat(
                               "CMAKE_"_s, lang, "_COMPILER_VERSION"_s)),
                             '.');
    if (error) {
      *error = std::move(e);
    } else {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
    }
    return nullptr;
  }
  return featuresKnown;
}

bool cmStandardLevelResolver::GetNewRequiredStandard(
  std::string const& targetName, std::string const& feature,
  cmValue currentLangStandardValue,
  cm::optional<cmStandardLevel>& featureLevel,
  std::string& newRequiredStandard, std::string* error) const
{
  std::string lang;
  if (!this->CheckCompileFeaturesAvailable(targetName, feature, lang, error)) {
    return false;
  }

  featureLevel = this->CompileFeatureStandardLevel(lang, feature);

  auto mapping = StandardComputerMapping.find(lang);
  if (mapping != std::cend(StandardComputerMapping)) {
    return mapping->second.GetNewRequiredStandard(
      this->Makefile, targetName, featureLevel, currentLangStandardValue,
      newRequiredStandard, error);
  }
  return false;
}

bool cmStandardLevelResolver::HaveStandardAvailable(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, std::string const& feature) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping != std::cend(StandardComputerMapping)) {
    return mapping->second.HaveStandardAvailable(this->Makefile, target,
                                                 config, feature);
  }
  return false;
}

bool cmStandardLevelResolver::IsLaterStandard(std::string const& lang,
                                              std::string const& lhs,
                                              std::string const& rhs) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping != std::cend(StandardComputerMapping)) {
    return mapping->second.IsLaterStandard(std::stoi(lhs), std::stoi(rhs));
  }
  return false;
}
