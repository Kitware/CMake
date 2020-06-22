/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmStandardLevelResolver.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <vector>

#include <cm/iterator>
#include <cmext/algorithm>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmProperty.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmake.h"

#define FEATURE_STRING(F) , #F
static const char* const C_FEATURES[] = { nullptr FOR_EACH_C_FEATURE(
  FEATURE_STRING) };

static const char* const CXX_FEATURES[] = { nullptr FOR_EACH_CXX_FEATURE(
  FEATURE_STRING) };

static const char* const CUDA_FEATURES[] = { nullptr FOR_EACH_CUDA_FEATURE(
  FEATURE_STRING) };
#undef FEATURE_STRING

static const char* const C_STANDARDS[] = { "90", "99", "11" };
static const char* const CXX_STANDARDS[] = { "98", "11", "14", "17", "20" };
static const char* const CUDA_STANDARDS[] = { "03", "11", "14", "17", "20" };

bool cmStandardLevelResolver::AddRequiredTargetFeature(
  cmTarget* target, const std::string& feature, std::string* error) const
{
  if (cmGeneratorExpression::Find(feature) != std::string::npos) {
    target->AppendProperty("COMPILE_FEATURES", feature);
    return true;
  }

  std::string lang;
  if (!this->CheckCompileFeaturesAvailable(target->GetName(), feature, lang,
                                           error)) {
    return false;
  }

  target->AppendProperty("COMPILE_FEATURES", feature);

  // FIXME: Add a policy to avoid updating the <LANG>_STANDARD target
  // property due to COMPILE_FEATURES.  The language standard selection
  // should be done purely at generate time based on whatever the project
  // code put in these properties explicitly.  That is mostly true now,
  // but for compatibility we need to continue updating the property here.
  if (lang == "C" || lang == "OBJC") {
    return this->AddRequiredTargetCFeature(target, feature, lang, error);
  }
  if (lang == "CUDA") {
    return this->AddRequiredTargetCudaFeature(target, feature, lang, error);
  }
  return this->AddRequiredTargetCxxFeature(target, feature, lang, error);
}

bool cmStandardLevelResolver::CheckCompileFeaturesAvailable(
  const std::string& targetName, const std::string& feature, std::string& lang,
  std::string* error) const
{
  if (!this->CompileFeatureKnown(targetName, feature, lang, error)) {
    return false;
  }

  const char* features = this->CompileFeaturesAvailable(lang, error);
  if (!features) {
    return false;
  }

  std::vector<std::string> availableFeatures = cmExpandedList(features);
  if (!cm::contains(availableFeatures, feature)) {
    std::ostringstream e;
    e << "The compiler feature \"" << feature << "\" is not known to " << lang
      << " compiler\n\""
      << this->Makefile->GetDefinition("CMAKE_" + lang + "_COMPILER_ID")
      << "\"\nversion "
      << this->Makefile->GetDefinition("CMAKE_" + lang + "_COMPILER_VERSION")
      << ".";
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

const char* cmStandardLevelResolver::CompileFeaturesAvailable(
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

  const char* featuresKnown =
    this->Makefile->GetDefinition("CMAKE_" + lang + "_COMPILE_FEATURES");

  if (!featuresKnown || !*featuresKnown) {
    std::ostringstream e;
    if (error) {
      e << "no";
    } else {
      e << "No";
    }
    e << " known features for " << lang << " compiler\n\""
      << this->Makefile->GetSafeDefinition("CMAKE_" + lang + "_COMPILER_ID")
      << "\"\nversion "
      << this->Makefile->GetSafeDefinition("CMAKE_" + lang +
                                           "_COMPILER_VERSION")
      << ".";
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
  cmProp currentLangStandardValue, std::string& newRequiredStandard,
  std::string* error) const
{
  std::string lang;
  if (!this->CheckCompileFeaturesAvailable(targetName, feature, lang, error)) {
    return false;
  }

  if (lang == "C" || lang == "OBJC") {
    return this->GetNewRequiredCStandard(targetName, feature, lang,
                                         currentLangStandardValue,
                                         newRequiredStandard, error);
  }
  if (lang == "CUDA") {
    return this->GetNewRequiredCudaStandard(targetName, feature, lang,
                                            currentLangStandardValue,
                                            newRequiredStandard, error);
  }
  return this->GetNewRequiredCxxStandard(targetName, feature, lang,
                                         currentLangStandardValue,
                                         newRequiredStandard, error);
}

bool cmStandardLevelResolver::HaveStandardAvailable(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, const std::string& feature) const
{
  if (lang == "C" || lang == "OBJC") {
    return this->HaveCStandardAvailable(target, lang, config, feature);
  }
  if (lang == "CUDA") {
    return this->HaveCudaStandardAvailable(target, lang, config, feature);
  }
  return this->HaveCxxStandardAvailable(target, lang, config, feature);
}

bool cmStandardLevelResolver::HaveCStandardAvailable(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, const std::string& feature) const
{
  cmProp defaultCStandard =
    this->Makefile->GetDef(cmStrCat("CMAKE_", lang, "_STANDARD_DEFAULT"));
  if (!defaultCStandard) {
    this->Makefile->IssueMessage(
      MessageType::INTERNAL_ERROR,
      cmStrCat("CMAKE_", lang,
               "_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
               "not fully configured for this compiler."));
    // Return true so the caller does not try to lookup the default standard.
    return true;
  }
  if (std::find_if(cm::cbegin(C_STANDARDS), cm::cend(C_STANDARDS),
                   cmStrCmp(*defaultCStandard)) == cm::cend(C_STANDARDS)) {
    const std::string e = cmStrCat("The CMAKE_", lang,
                                   "_STANDARD_DEFAULT variable contains an "
                                   "invalid value: \"",
                                   *defaultCStandard, "\".");
    this->Makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
    return false;
  }

  bool needC90 = false;
  bool needC99 = false;
  bool needC11 = false;

  this->CheckNeededCLanguage(feature, lang, needC90, needC99, needC11);

  cmProp existingCStandard = target->GetLanguageStandard(lang, config);
  if (!existingCStandard) {
    existingCStandard = defaultCStandard;
  }

  if (std::find_if(cm::cbegin(C_STANDARDS), cm::cend(C_STANDARDS),
                   cmStrCmp(*existingCStandard)) == cm::cend(C_STANDARDS)) {
    const std::string e = cmStrCat(
      "The ", lang, "_STANDARD property on target \"", target->GetName(),
      "\" contained an invalid value: \"", *existingCStandard, "\".");
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
    return false;
  }

  const char* const* existingCIt = existingCStandard
    ? std::find_if(cm::cbegin(C_STANDARDS), cm::cend(C_STANDARDS),
                   cmStrCmp(*existingCStandard))
    : cm::cend(C_STANDARDS);

  if (needC11 && existingCStandard &&
      existingCIt < std::find_if(cm::cbegin(C_STANDARDS),
                                 cm::cend(C_STANDARDS), cmStrCmp("11"))) {
    return false;
  }
  if (needC99 && existingCStandard &&
      existingCIt < std::find_if(cm::cbegin(C_STANDARDS),
                                 cm::cend(C_STANDARDS), cmStrCmp("99"))) {
    return false;
  }
  if (needC90 && existingCStandard &&
      existingCIt < std::find_if(cm::cbegin(C_STANDARDS),
                                 cm::cend(C_STANDARDS), cmStrCmp("90"))) {
    return false;
  }
  return true;
}

bool cmStandardLevelResolver::IsLaterStandard(std::string const& lang,
                                              std::string const& lhs,
                                              std::string const& rhs) const
{
  if (lang == "C" || lang == "OBJC") {
    const char* const* rhsIt = std::find_if(
      cm::cbegin(C_STANDARDS), cm::cend(C_STANDARDS), cmStrCmp(rhs));

    return std::find_if(rhsIt, cm::cend(C_STANDARDS), cmStrCmp(lhs)) !=
      cm::cend(C_STANDARDS);
  }
  if (lang == "CUDA") {
    const char* const* rhsIt = std::find_if(
      cm::cbegin(CUDA_STANDARDS), cm::cend(CUDA_STANDARDS), cmStrCmp(rhs));

    return std::find_if(rhsIt, cm::cend(CUDA_STANDARDS), cmStrCmp(lhs)) !=
      cm::cend(CUDA_STANDARDS);
  }

  const char* const* rhsIt = std::find_if(
    cm::cbegin(CXX_STANDARDS), cm::cend(CXX_STANDARDS), cmStrCmp(rhs));

  return std::find_if(rhsIt, cm::cend(CXX_STANDARDS), cmStrCmp(lhs)) !=
    cm::cend(CXX_STANDARDS);
}

bool cmStandardLevelResolver::HaveCxxStandardAvailable(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, const std::string& feature) const
{
  cmProp defaultCxxStandard =
    this->Makefile->GetDef(cmStrCat("CMAKE_", lang, "_STANDARD_DEFAULT"));
  if (!defaultCxxStandard) {
    this->Makefile->IssueMessage(
      MessageType::INTERNAL_ERROR,
      cmStrCat("CMAKE_", lang,
               "_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
               "not fully configured for this compiler."));
    // Return true so the caller does not try to lookup the default standard.
    return true;
  }
  if (std::find_if(cm::cbegin(CXX_STANDARDS), cm::cend(CXX_STANDARDS),
                   cmStrCmp(*defaultCxxStandard)) == cm::cend(CXX_STANDARDS)) {
    const std::string e =
      cmStrCat("The CMAKE_", lang, "_STANDARD_DEFAULT variable contains an ",
               "invalid value: \"", *defaultCxxStandard, "\".");
    this->Makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
    return false;
  }

  bool needCxx98 = false;
  bool needCxx11 = false;
  bool needCxx14 = false;
  bool needCxx17 = false;
  bool needCxx20 = false;
  this->CheckNeededCxxLanguage(feature, lang, needCxx98, needCxx11, needCxx14,
                               needCxx17, needCxx20);

  cmProp existingCxxStandard = target->GetLanguageStandard(lang, config);
  if (!existingCxxStandard) {
    existingCxxStandard = defaultCxxStandard;
  }

  const char* const* existingCxxLevel =
    std::find_if(cm::cbegin(CXX_STANDARDS), cm::cend(CXX_STANDARDS),
                 cmStrCmp(*existingCxxStandard));
  if (existingCxxLevel == cm::cend(CXX_STANDARDS)) {
    const std::string e = cmStrCat(
      "The ", lang, "_STANDARD property on target \"", target->GetName(),
      "\" contained an invalid value: \"", *existingCxxStandard, "\".");
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
    return false;
  }

  /* clang-format off */
  const char* const* needCxxLevel =
    needCxx20 ? &CXX_STANDARDS[4]
    : needCxx17 ? &CXX_STANDARDS[3]
    : needCxx14 ? &CXX_STANDARDS[2]
    : needCxx11 ? &CXX_STANDARDS[1]
    : needCxx98 ? &CXX_STANDARDS[0]
    : nullptr;
  /* clang-format on */

  return !needCxxLevel || needCxxLevel <= existingCxxLevel;
}

void cmStandardLevelResolver::CheckNeededCxxLanguage(
  const std::string& feature, std::string const& lang, bool& needCxx98,
  bool& needCxx11, bool& needCxx14, bool& needCxx17, bool& needCxx20) const
{
  if (const char* propCxx98 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "98_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCxx98);
    needCxx98 = cm::contains(props, feature);
  }
  if (const char* propCxx11 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "11_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCxx11);
    needCxx11 = cm::contains(props, feature);
  }
  if (const char* propCxx14 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "14_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCxx14);
    needCxx14 = cm::contains(props, feature);
  }
  if (const char* propCxx17 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "17_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCxx17);
    needCxx17 = cm::contains(props, feature);
  }
  if (const char* propCxx20 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "20_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCxx20);
    needCxx20 = cm::contains(props, feature);
  }
}

bool cmStandardLevelResolver::AddRequiredTargetCxxFeature(
  cmTarget* target, const std::string& feature, std::string const& lang,
  std::string* error) const
{
  std::string newRequiredStandard;
  if (this->GetNewRequiredCxxStandard(
        target->GetName(), feature, lang,
        target->GetProperty(cmStrCat(lang, "_STANDARD")), newRequiredStandard,
        error)) {
    if (!newRequiredStandard.empty()) {
      target->SetLanguageStandardProperty(lang, newRequiredStandard, feature);
    }
    return true;
  }

  return false;
}

bool cmStandardLevelResolver::GetNewRequiredCxxStandard(
  const std::string& targetName, const std::string& feature,
  std::string const& lang, cmProp currentLangStandardValue,
  std::string& newRequiredStandard, std::string* error) const
{
  newRequiredStandard.clear();

  bool needCxx98 = false;
  bool needCxx11 = false;
  bool needCxx14 = false;
  bool needCxx17 = false;
  bool needCxx20 = false;

  this->CheckNeededCxxLanguage(feature, lang, needCxx98, needCxx11, needCxx14,
                               needCxx17, needCxx20);

  cmProp existingCxxStandard = currentLangStandardValue;
  if (existingCxxStandard == nullptr) {
    cmProp defaultCxxStandard =
      this->Makefile->GetDef(cmStrCat("CMAKE_", lang, "_STANDARD_DEFAULT"));
    if (defaultCxxStandard && !defaultCxxStandard->empty()) {
      existingCxxStandard = defaultCxxStandard;
    }
  }
  const char* const* existingCxxLevel = nullptr;
  if (existingCxxStandard) {
    existingCxxLevel =
      std::find_if(cm::cbegin(CXX_STANDARDS), cm::cend(CXX_STANDARDS),
                   cmStrCmp(*existingCxxStandard));
    if (existingCxxLevel == cm::cend(CXX_STANDARDS)) {
      const std::string e = cmStrCat(
        "The ", lang, "_STANDARD property on target \"", targetName,
        "\" contained an invalid value: \"", *existingCxxStandard, "\".");
      if (error) {
        *error = e;
      } else {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      }
      return false;
    }
  }

  /* clang-format off */
  const char* const* needCxxLevel =
    needCxx20 ? &CXX_STANDARDS[4]
    : needCxx17 ? &CXX_STANDARDS[3]
    : needCxx14 ? &CXX_STANDARDS[2]
    : needCxx11 ? &CXX_STANDARDS[1]
    : needCxx98 ? &CXX_STANDARDS[0]
    : nullptr;
  /* clang-format on */

  if (needCxxLevel) {
    // Ensure the C++ language level is high enough to support
    // the needed C++ features.
    if (!existingCxxLevel || existingCxxLevel < needCxxLevel) {
      newRequiredStandard = *needCxxLevel;
    }
  }

  return true;
}

bool cmStandardLevelResolver::HaveCudaStandardAvailable(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, const std::string& feature) const
{
  cmProp defaultCudaStandard =
    this->Makefile->GetDef(cmStrCat("CMAKE_", lang, "_STANDARD_DEFAULT"));
  if (!defaultCudaStandard) {
    this->Makefile->IssueMessage(
      MessageType::INTERNAL_ERROR,
      cmStrCat("CMAKE_", lang,
               "_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
               "not fully configured for this compiler."));
    // Return true so the caller does not try to lookup the default standard.
    return true;
  }
  if (std::find_if(cm::cbegin(CUDA_STANDARDS), cm::cend(CUDA_STANDARDS),
                   cmStrCmp(*defaultCudaStandard)) ==
      cm::cend(CUDA_STANDARDS)) {
    const std::string e =
      cmStrCat("The CMAKE_", lang, "_STANDARD_DEFAULT variable contains an ",
               "invalid value: \"", *defaultCudaStandard, "\".");
    this->Makefile->IssueMessage(MessageType::INTERNAL_ERROR, e);
    return false;
  }

  bool needCuda03 = false;
  bool needCuda11 = false;
  bool needCuda14 = false;
  bool needCuda17 = false;
  bool needCuda20 = false;
  this->CheckNeededCudaLanguage(feature, lang, needCuda03, needCuda11,
                                needCuda14, needCuda17, needCuda20);

  cmProp existingCudaStandard = target->GetLanguageStandard(lang, config);
  if (!existingCudaStandard) {
    existingCudaStandard = defaultCudaStandard;
  }

  const char* const* existingCudaLevel =
    std::find_if(cm::cbegin(CUDA_STANDARDS), cm::cend(CUDA_STANDARDS),
                 cmStrCmp(*existingCudaStandard));
  if (existingCudaLevel == cm::cend(CUDA_STANDARDS)) {
    const std::string e = cmStrCat(
      "The ", lang, "_STANDARD property on target \"", target->GetName(),
      "\" contained an invalid value: \"", *existingCudaStandard, "\".");
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
    return false;
  }

  /* clang-format off */
  const char* const* needCudaLevel =
    needCuda20 ? &CUDA_STANDARDS[4]
    : needCuda17 ? &CUDA_STANDARDS[3]
    : needCuda14 ? &CUDA_STANDARDS[2]
    : needCuda11 ? &CUDA_STANDARDS[1]
    : needCuda03 ? &CUDA_STANDARDS[0]
    : nullptr;
  /* clang-format on */

  return !needCudaLevel || needCudaLevel <= existingCudaLevel;
}

void cmStandardLevelResolver::CheckNeededCudaLanguage(
  const std::string& feature, std::string const& lang, bool& needCuda03,
  bool& needCuda11, bool& needCuda14, bool& needCuda17, bool& needCuda20) const
{
  if (const char* propCuda03 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "03_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCuda03);
    needCuda03 = cm::contains(props, feature);
  }
  if (const char* propCuda11 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "11_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCuda11);
    needCuda11 = cm::contains(props, feature);
  }
  if (const char* propCuda14 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "14_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCuda14);
    needCuda14 = cm::contains(props, feature);
  }
  if (const char* propCuda17 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "17_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCuda17);
    needCuda17 = cm::contains(props, feature);
  }
  if (const char* propCuda20 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "20_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propCuda20);
    needCuda20 = cm::contains(props, feature);
  }
}

bool cmStandardLevelResolver::AddRequiredTargetCudaFeature(
  cmTarget* target, const std::string& feature, std::string const& lang,
  std::string* error) const
{
  std::string newRequiredStandard;
  if (this->GetNewRequiredCudaStandard(
        target->GetName(), feature, lang,
        target->GetProperty(cmStrCat(lang, "_STANDARD")), newRequiredStandard,
        error)) {
    if (!newRequiredStandard.empty()) {
      target->SetLanguageStandardProperty(lang, newRequiredStandard, feature);
    }
    return true;
  }
  return false;
}

bool cmStandardLevelResolver::GetNewRequiredCudaStandard(
  const std::string& targetName, const std::string& feature,
  std::string const& lang, cmProp currentLangStandardValue,
  std::string& newRequiredStandard, std::string* error) const
{
  newRequiredStandard.clear();

  bool needCuda03 = false;
  bool needCuda11 = false;
  bool needCuda14 = false;
  bool needCuda17 = false;
  bool needCuda20 = false;

  this->CheckNeededCudaLanguage(feature, lang, needCuda03, needCuda11,
                                needCuda14, needCuda17, needCuda20);

  cmProp existingCudaStandard = currentLangStandardValue;
  if (existingCudaStandard == nullptr) {
    cmProp defaultCudaStandard =
      this->Makefile->GetDef(cmStrCat("CMAKE_", lang, "_STANDARD_DEFAULT"));
    if (defaultCudaStandard && !defaultCudaStandard->empty()) {
      existingCudaStandard = defaultCudaStandard;
    }
  }
  const char* const* existingCudaLevel = nullptr;
  if (existingCudaStandard) {
    existingCudaLevel =
      std::find_if(cm::cbegin(CUDA_STANDARDS), cm::cend(CUDA_STANDARDS),
                   cmStrCmp(*existingCudaStandard));
    if (existingCudaLevel == cm::cend(CUDA_STANDARDS)) {
      const std::string e = cmStrCat(
        "The ", lang, "_STANDARD property on target \"", targetName,
        "\" contained an invalid value: \"", *existingCudaStandard, "\".");
      if (error) {
        *error = e;
      } else {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      }
      return false;
    }
  }

  /* clang-format off */
  const char* const* needCudaLevel =
    needCuda20 ? &CUDA_STANDARDS[4]
    : needCuda17 ? &CUDA_STANDARDS[3]
    : needCuda14 ? &CUDA_STANDARDS[2]
    : needCuda11 ? &CUDA_STANDARDS[1]
    : needCuda03 ? &CUDA_STANDARDS[0]
    : nullptr;
  /* clang-format on */

  if (needCudaLevel) {
    // Ensure the CUDA language level is high enough to support
    // the needed CUDA features.
    if (!existingCudaLevel || existingCudaLevel < needCudaLevel) {
      newRequiredStandard = *needCudaLevel;
    }
  }

  return true;
}

void cmStandardLevelResolver::CheckNeededCLanguage(const std::string& feature,
                                                   std::string const& lang,
                                                   bool& needC90,
                                                   bool& needC99,
                                                   bool& needC11) const
{
  if (const char* propC90 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "90_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propC90);
    needC90 = cm::contains(props, feature);
  }
  if (const char* propC99 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "99_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propC99);
    needC99 = cm::contains(props, feature);
  }
  if (const char* propC11 = this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "11_COMPILE_FEATURES"))) {
    std::vector<std::string> props = cmExpandedList(propC11);
    needC11 = cm::contains(props, feature);
  }
}

bool cmStandardLevelResolver::AddRequiredTargetCFeature(
  cmTarget* target, const std::string& feature, std::string const& lang,
  std::string* error) const
{
  std::string newRequiredStandard;
  if (this->GetNewRequiredCStandard(
        target->GetName(), feature, lang,
        target->GetProperty(cmStrCat(lang, "_STANDARD")), newRequiredStandard,
        error)) {
    if (!newRequiredStandard.empty()) {
      target->SetLanguageStandardProperty(lang, newRequiredStandard, feature);
    }
    return true;
  }

  return false;
}

bool cmStandardLevelResolver::GetNewRequiredCStandard(
  const std::string& targetName, const std::string& feature,
  std::string const& lang, cmProp currentLangStandardValue,
  std::string& newRequiredStandard, std::string* error) const
{
  newRequiredStandard.clear();

  bool needC90 = false;
  bool needC99 = false;
  bool needC11 = false;

  this->CheckNeededCLanguage(feature, lang, needC90, needC99, needC11);

  cmProp existingCStandard = currentLangStandardValue;
  if (existingCStandard == nullptr) {
    cmProp defaultCStandard =
      this->Makefile->GetDef(cmStrCat("CMAKE_", lang, "_STANDARD_DEFAULT"));
    if (defaultCStandard && !defaultCStandard->empty()) {
      existingCStandard = defaultCStandard;
    }
  }
  if (existingCStandard) {
    if (std::find_if(cm::cbegin(C_STANDARDS), cm::cend(C_STANDARDS),
                     cmStrCmp(*existingCStandard)) == cm::cend(C_STANDARDS)) {
      const std::string e = cmStrCat(
        "The ", lang, "_STANDARD property on target \"", targetName,
        "\" contained an invalid value: \"", *existingCStandard, "\".");
      if (error) {
        *error = e;
      } else {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      }
      return false;
    }
  }
  const char* const* existingCIt = existingCStandard
    ? std::find_if(cm::cbegin(C_STANDARDS), cm::cend(C_STANDARDS),
                   cmStrCmp(*existingCStandard))
    : cm::cend(C_STANDARDS);

  bool setC90 = needC90 && !existingCStandard;
  bool setC99 = needC99 && !existingCStandard;
  bool setC11 = needC11 && !existingCStandard;

  if (needC11 && existingCStandard &&
      existingCIt < std::find_if(cm::cbegin(C_STANDARDS),
                                 cm::cend(C_STANDARDS), cmStrCmp("11"))) {
    setC11 = true;
  } else if (needC99 && existingCStandard &&
             existingCIt < std::find_if(cm::cbegin(C_STANDARDS),
                                        cm::cend(C_STANDARDS),
                                        cmStrCmp("99"))) {
    setC99 = true;
  } else if (needC90 && existingCStandard &&
             existingCIt < std::find_if(cm::cbegin(C_STANDARDS),
                                        cm::cend(C_STANDARDS),
                                        cmStrCmp("90"))) {
    setC90 = true;
  }

  if (setC11) {
    newRequiredStandard = "11";
  } else if (setC99) {
    newRequiredStandard = "99";
  } else if (setC90) {
    newRequiredStandard = "90";
  }
  return true;
}
