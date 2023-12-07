/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmStandardLevelResolver.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/iterator>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStandardLevel.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

#define FEATURE_STRING(F) , #F
const char* const C_FEATURES[] = { nullptr FOR_EACH_C_FEATURE(
  FEATURE_STRING) };

const char* const CXX_FEATURES[] = { nullptr FOR_EACH_CXX_FEATURE(
  FEATURE_STRING) };

const char* const CUDA_FEATURES[] = { nullptr FOR_EACH_CUDA_FEATURE(
  FEATURE_STRING) };

const char* const HIP_FEATURES[] = { nullptr FOR_EACH_HIP_FEATURE(
  FEATURE_STRING) };
#undef FEATURE_STRING

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

    const auto& stds = this->Levels;
    const auto& stdsStrings = this->LevelsAsStrings;

    cmValue defaultStd = makefile->GetDefinition(
      cmStrCat("CMAKE_", this->Language, "_STANDARD_DEFAULT"));
    if (!cmNonempty(defaultStd)) {
      // this compiler has no notion of language standard levels
      return std::string{};
    }

    cmPolicies::PolicyStatus const cmp0128{ makefile->GetPolicyStatus(
      cmPolicies::CMP0128) };
    bool const defaultExt{ cmIsOn(*makefile->GetDefinition(
      cmStrCat("CMAKE_", this->Language, "_EXTENSIONS_DEFAULT"))) };
    bool ext = true;

    if (cmp0128 == cmPolicies::NEW) {
      ext = defaultExt;
    }

    if (cmValue extPropValue = target->GetLanguageExtensions(this->Language)) {
      ext = cmIsOn(*extPropValue);
    }

    std::string const type{ ext ? "EXTENSION" : "STANDARD" };

    cmValue standardProp = target->GetLanguageStandard(this->Language, config);
    if (!standardProp) {
      if (cmp0128 == cmPolicies::NEW) {
        // Add extension flag if compiler's default doesn't match.
        if (ext != defaultExt) {
          return cmStrCat("CMAKE_", this->Language, *defaultStd, '_', type,
                          "_COMPILE_OPTION");
        }
      } else {
        if (cmp0128 == cmPolicies::WARN &&
            makefile->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0128") &&
            ext != defaultExt) {
          const char* state{};
          if (ext) {
            if (!makefile->GetDefinition(cmStrCat(
                  "CMAKE_", this->Language, "_EXTENSION_COMPILE_OPTION"))) {
              state = "enabled";
            }
          } else {
            state = "disabled";
          }
          if (state) {
            makefile->IssueMessage(
              MessageType::AUTHOR_WARNING,
              cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0128),
                       "\nFor compatibility with older versions of CMake, "
                       "compiler extensions won't be ",
                       state, '.'));
          }
        }

        if (ext) {
          return cmStrCat("CMAKE_", this->Language,
                          "_EXTENSION_COMPILE_OPTION");
        }
      }
      return std::string{};
    }

    if (target->GetLanguageStandardRequired(this->Language)) {
      std::string option_flag = cmStrCat(
        "CMAKE_", this->Language, *standardProp, '_', type, "_COMPILE_OPTION");

      cmValue opt = target->Target->GetMakefile()->GetDefinition(option_flag);
      if (!opt) {
        std::ostringstream e;
        e << "Target \"" << target->GetName()
          << "\" requires the language "
             "dialect \""
          << this->Language << *standardProp << "\" "
          << (ext ? "(with compiler extensions)" : "")
          << ". But the current compiler \""
          << makefile->GetSafeDefinition(
               cmStrCat("CMAKE_", this->Language, "_COMPILER_ID"))
          << "\" does not support this, or "
             "CMake does not know the flags to enable it.";

        makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      }
      return option_flag;
    }

    // If the request matches the compiler's defaults we don't need to add
    // anything.
    if (*standardProp == *defaultStd && ext == defaultExt) {
      if (cmp0128 == cmPolicies::NEW) {
        return std::string{};
      }

      if (cmp0128 == cmPolicies::WARN &&
          makefile->PolicyOptionalWarningEnabled(
            "CMAKE_POLICY_WARNING_CMP0128")) {
        makefile->IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0128),
                   "\nFor compatibility with older versions of CMake, "
                   "unnecessary flags for language standard or compiler "
                   "extensions may be added."));
      }
    }

    std::string standardStr(*standardProp);
    if (this->Language == "CUDA"_s && standardStr == "98"_s) {
      standardStr = "03";
    }

    auto stdIt =
      std::find(cm::cbegin(stds), cm::cend(stds), ParseStd(standardStr));
    if (stdIt == cm::cend(stds)) {
      std::string e =
        cmStrCat(this->Language, "_STANDARD is set to invalid value '",
                 standardStr, '\'');
      makefile->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e,
                                                 target->GetBacktrace());
      return std::string{};
    }

    auto defaultStdIt =
      std::find(cm::cbegin(stds), cm::cend(stds), ParseStd(*defaultStd));
    if (defaultStdIt == cm::cend(stds)) {
      std::string e = cmStrCat("CMAKE_", this->Language,
                               "_STANDARD_DEFAULT is set to invalid value '",
                               *defaultStd, '\'');
      makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
      return std::string{};
    }

    // If the standard requested is older than the compiler's default or the
    // extension mode doesn't match then we need to use a flag.
    if ((cmp0128 != cmPolicies::NEW && stdIt <= defaultStdIt) ||
        (cmp0128 == cmPolicies::NEW &&
         (stdIt < defaultStdIt || ext != defaultExt))) {
      auto offset = std::distance(cm::cbegin(stds), stdIt);
      return cmStrCat("CMAKE_", this->Language, stdsStrings[offset], '_', type,
                      "_COMPILE_OPTION");
    }

    // The compiler's default is at least as new as the requested standard,
    // and the requested standard is not required.  Decay to the newest
    // standard for which a flag is defined.
    for (; defaultStdIt < stdIt; --stdIt) {
      auto offset = std::distance(cm::cbegin(stds), stdIt);
      std::string option_flag =
        cmStrCat("CMAKE_", this->Language, stdsStrings[offset], '_', type,
                 "_COMPILE_OPTION");
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
    const auto& stds = this->Levels;
    const auto& stdsStrings = this->LevelsAsStrings;

    cmValue defaultStd = makefile->GetDefinition(
      cmStrCat("CMAKE_", this->Language, "_STANDARD_DEFAULT"));
    if (!cmNonempty(defaultStd)) {
      // this compiler has no notion of language standard levels
      return std::string{};
    }

    cmPolicies::PolicyStatus const cmp0128{ makefile->GetPolicyStatus(
      cmPolicies::CMP0128) };
    bool const defaultExt{ cmIsOn(*makefile->GetDefinition(
      cmStrCat("CMAKE_", this->Language, "_EXTENSIONS_DEFAULT"))) };
    bool ext = true;

    if (cmp0128 == cmPolicies::NEW) {
      ext = defaultExt;
    }

    if (cmValue extPropValue = target->GetLanguageExtensions(this->Language)) {
      ext = cmIsOn(*extPropValue);
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
      std::find(cm::cbegin(stds), cm::cend(stds), ParseStd(standardStr));
    if (stdIt == cm::cend(stds)) {
      return std::string{};
    }

    auto defaultStdIt =
      std::find(cm::cbegin(stds), cm::cend(stds), ParseStd(*defaultStd));
    if (defaultStdIt == cm::cend(stds)) {
      return std::string{};
    }

    // If the standard requested is older than the compiler's default or the
    // extension mode doesn't match then we need to use a flag.
    if ((cmp0128 != cmPolicies::NEW && stdIt <= defaultStdIt) ||
        (cmp0128 == cmPolicies::NEW &&
         (stdIt < defaultStdIt || ext != defaultExt))) {
      auto offset = std::distance(cm::cbegin(stds), stdIt);
      return stdsStrings[offset];
    }

    // The compiler's default is at least as new as the requested standard,
    // and the requested standard is not required.  Decay to the newest
    // standard for which a flag is defined.
    for (; defaultStdIt < stdIt; --stdIt) {
      auto offset = std::distance(cm::cbegin(stds), stdIt);
      std::string option_flag =
        cmStrCat("CMAKE_", this->Language, stdsStrings[offset], '_', type,
                 "_COMPILE_OPTION");
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
        cmStrCat("CMAKE_", this->Language, "_STANDARD_DEFAULT"));
      if (cmNonempty(defaultStandard)) {
        existingStandard = defaultStandard;
      }
    }

    auto existingLevelIter = cm::cend(this->Levels);
    if (existingStandard) {
      existingLevelIter =
        std::find(cm::cbegin(this->Levels), cm::cend(this->Levels),
                  ParseStd(*existingStandard));
      if (existingLevelIter == cm::cend(this->Levels)) {
        const std::string e =
          cmStrCat("The ", this->Language, "_STANDARD property on target \"",
                   targetName, "\" contained an invalid value: \"",
                   *existingStandard, "\".");
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
      if (existingLevelIter == cm::cend(this->Levels) ||
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
      cmStrCat("CMAKE_", this->Language, "_STANDARD_DEFAULT"));
    if (!defaultStandard) {
      makefile->IssueMessage(
        MessageType::INTERNAL_ERROR,
        cmStrCat("CMAKE_", this->Language,
                 "_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
                 "not fully configured for this compiler."));
      // Return true so the caller does not try to lookup the default standard.
      return true;
    }
    // convert defaultStandard to an integer
    if (std::find(cm::cbegin(this->Levels), cm::cend(this->Levels),
                  ParseStd(*defaultStandard)) == cm::cend(this->Levels)) {
      const std::string e = cmStrCat("The CMAKE_", this->Language,
                                     "_STANDARD_DEFAULT variable contains an "
                                     "invalid value: \"",
                                     *defaultStandard, "\".");
      makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
      return false;
    }

    cmValue existingStandard =
      target->GetLanguageStandard(this->Language, config);
    if (!existingStandard) {
      existingStandard = defaultStandard;
    }

    auto existingLevelIter =
      std::find(cm::cbegin(this->Levels), cm::cend(this->Levels),
                ParseStd(*existingStandard));
    if (existingLevelIter == cm::cend(this->Levels)) {
      const std::string e =
        cmStrCat("The ", this->Language, "_STANDARD property on target \"",
                 target->GetName(), "\" contained an invalid value: \"",
                 *existingStandard, "\".");
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
    std::string prefix = cmStrCat("CMAKE_", this->Language);
    cm::optional<cmStandardLevel> maxLevel;
    for (size_t i = 0; i < this->Levels.size(); ++i) {
      if (cmValue prop = makefile->GetDefinition(
            cmStrCat(prefix, this->LevelsAsStrings[i], "_COMPILE_FEATURES"))) {
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
      std::find(cm::cbegin(stds), cm::cend(stds), ParseStd(standardStr));
    if (stdIt != cm::cend(stds)) {
      langLevel = cmStandardLevel(std::distance(cm::cbegin(stds), stdIt));
    }
    return langLevel;
  }

  bool IsLaterStandard(int lhs, int rhs) const
  {
    auto rhsIt =
      std::find(cm::cbegin(this->Levels), cm::cend(this->Levels), rhs);

    return std::find(rhsIt, cm::cend(this->Levels), lhs) !=
      cm::cend(this->Levels);
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
      std::vector<std::string>{ "98", "11", "14", "17", "20", "23", "26" } } }
};
}

std::string cmStandardLevelResolver::GetCompileOptionDef(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config) const
{
  const auto& mapping = StandardComputerMapping.find(lang);
  if (mapping == cm::cend(StandardComputerMapping)) {
    return std::string{};
  }

  return mapping->second.GetCompileOptionDef(this->Makefile, target, config);
}

std::string cmStandardLevelResolver::GetEffectiveStandard(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config) const
{
  const auto& mapping = StandardComputerMapping.find(lang);
  if (mapping == cm::cend(StandardComputerMapping)) {
    return std::string{};
  }

  return mapping->second.GetEffectiveStandard(this->Makefile, target, config);
}

bool cmStandardLevelResolver::AddRequiredTargetFeature(
  cmTarget* target, const std::string& feature, std::string* error) const
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
    target->GetProperty(cmStrCat(lang, "_STANDARD")), featureLevel,
    newRequiredStandard, error);
  if (!newRequiredStandard.empty()) {
    target->SetProperty(cmStrCat(lang, "_STANDARD"), newRequiredStandard);
  }
  return succeeded;
}

bool cmStandardLevelResolver::CheckCompileFeaturesAvailable(
  const std::string& targetName, const std::string& feature, std::string& lang,
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
    std::ostringstream e;
    e << "The compiler feature \"" << feature << "\" is not known to " << lang
      << " compiler\n\""
      << this->Makefile->GetSafeDefinition(
           cmStrCat("CMAKE_", lang, "_COMPILER_ID"))
      << "\"\nversion "
      << this->Makefile->GetSafeDefinition(
           cmStrCat("CMAKE_", lang, "_COMPILER_VERSION"))
      << '.';
    if (error) {
      *error = e.str();
    } else {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    }
    return false;
  }

  return true;
}

bool cmStandardLevelResolver::CompileFeatureKnown(
  const std::string& targetName, const std::string& feature, std::string& lang,
  std::string* error) const
{
  assert(cmGeneratorExpression::Find(feature) == std::string::npos);

  bool isCFeature =
    std::find_if(cm::cbegin(C_FEATURES) + 1, cm::cend(C_FEATURES),
                 cmStrCmp(feature)) != cm::cend(C_FEATURES);
  if (isCFeature) {
    lang = "C";
    return true;
  }
  bool isCxxFeature =
    std::find_if(cm::cbegin(CXX_FEATURES) + 1, cm::cend(CXX_FEATURES),
                 cmStrCmp(feature)) != cm::cend(CXX_FEATURES);
  if (isCxxFeature) {
    lang = "CXX";
    return true;
  }
  bool isCudaFeature =
    std::find_if(cm::cbegin(CUDA_FEATURES) + 1, cm::cend(CUDA_FEATURES),
                 cmStrCmp(feature)) != cm::cend(CUDA_FEATURES);
  if (isCudaFeature) {
    lang = "CUDA";
    return true;
  }
  bool isHIPFeature =
    std::find_if(cm::cbegin(HIP_FEATURES) + 1, cm::cend(HIP_FEATURES),
                 cmStrCmp(feature)) != cm::cend(HIP_FEATURES);
  if (isHIPFeature) {
    lang = "HIP";
    return true;
  }
  std::ostringstream e;
  if (error) {
    e << "specified";
  } else {
    e << "Specified";
  }
  e << " unknown feature \"" << feature
    << "\" for "
       "target \""
    << targetName << "\".";
  if (error) {
    *error = e.str();
  } else {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
  return false;
}

cm::optional<cmStandardLevel>
cmStandardLevelResolver::CompileFeatureStandardLevel(
  std::string const& lang, std::string const& feature) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping == cm::cend(StandardComputerMapping)) {
    return cm::nullopt;
  }
  return mapping->second.CompileFeatureStandardLevel(this->Makefile, feature);
}

cm::optional<cmStandardLevel> cmStandardLevelResolver::LanguageStandardLevel(
  std::string const& lang, std::string const& standardStr) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping == cm::cend(StandardComputerMapping)) {
    return cm::nullopt;
  }
  return mapping->second.LanguageStandardLevel(standardStr);
}

cmValue cmStandardLevelResolver::CompileFeaturesAvailable(
  const std::string& lang, std::string* error) const
{
  if (!this->Makefile->GetGlobalGenerator()->GetLanguageEnabled(lang)) {
    std::ostringstream e;
    if (error) {
      e << "cannot";
    } else {
      e << "Cannot";
    }
    e << " use features from non-enabled language " << lang;
    if (error) {
      *error = e.str();
    } else {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    }
    return nullptr;
  }

  cmValue featuresKnown = this->Makefile->GetDefinition(
    cmStrCat("CMAKE_", lang, "_COMPILE_FEATURES"));

  if (!cmNonempty(featuresKnown)) {
    std::ostringstream e;
    if (error) {
      e << "no";
    } else {
      e << "No";
    }
    e << " known features for " << lang << " compiler\n\""
      << this->Makefile->GetSafeDefinition(
           cmStrCat("CMAKE_", lang, "_COMPILER_ID"))
      << "\"\nversion "
      << this->Makefile->GetSafeDefinition(
           cmStrCat("CMAKE_", lang, "_COMPILER_VERSION"))
      << '.';
    if (error) {
      *error = e.str();
    } else {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    }
    return nullptr;
  }
  return featuresKnown;
}

bool cmStandardLevelResolver::GetNewRequiredStandard(
  const std::string& targetName, const std::string& feature,
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
  if (mapping != cm::cend(StandardComputerMapping)) {
    return mapping->second.GetNewRequiredStandard(
      this->Makefile, targetName, featureLevel, currentLangStandardValue,
      newRequiredStandard, error);
  }
  return false;
}

bool cmStandardLevelResolver::HaveStandardAvailable(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, const std::string& feature) const
{
  auto mapping = StandardComputerMapping.find(lang);
  if (mapping != cm::cend(StandardComputerMapping)) {
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
  if (mapping != cm::cend(StandardComputerMapping)) {
    return mapping->second.IsLaterStandard(std::stoi(lhs), std::stoi(rhs));
  }
  return false;
}
