/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCxxModuleUsageEffects.h"

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmCryptoHash.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"

namespace {

bool IsMSVCWarning(cm::string_view flag)
{
  if (flag.size() < 2) {
    return false;
  }

  if (flag.front() != '/' && flag.front() != '-') {
    return false;
  }

  flag.remove_prefix(1);

  if (flag.front() == 'w' || flag.front() == 'W') {
    return true;
  }

  if (flag.substr(0, 9) == "external:") {
    return true;
  }

  return false;
}

bool IsGNUWarning(cm::string_view flag)
{
  if (flag.size() < 2) {
    return false;
  }

  if (flag.front() != '-') {
    return false;
  }

  flag.remove_prefix(1);

  if (flag.front() == 'w' || flag.front() == 'W') {
    return true;
  }

  if (flag.front() == '-') {
    flag.remove_prefix(1);
  }

  static std::set<cm::string_view> const long_names{
    "pedantic", "pedantic-errors", "all-warnings", "extra-warnings",
    "no-warnings"
  };

  return long_names.find(flag) != long_names.end();
}

void AppendUsage(std::string& usageHashInput, std::string const& entry)
{
  usageHashInput += entry;
  usageHashInput.push_back('\0');
}

template <typename Range>
void AppendUsageEntries(std::string& usageHashInput, Range const& entries)
{
  std::vector<std::string> entryValues;
  entryValues.reserve(entries.size());
  for (auto const& entry : entries) {
    entryValues.push_back(entry.Value);
  }
  std::sort(entryValues.begin(), entryValues.end());

  for (auto const& entryValue : entryValues) {
    usageHashInput += entryValue;
    usageHashInput.push_back('\n');
  }

  usageHashInput.push_back('\0');
}

enum class CompileOptionUsage
{
  Hash,
  Ignore,
  SavePreprocessor,
};

struct CompileOptionClassification
{
  CompileOptionClassification(
    CompileOptionUsage usage = CompileOptionUsage::Hash,
    bool saveFollowingArgument = false)
    : Usage(usage)
    , SaveFollowingArgument(saveFollowingArgument)
  {
  }

  CompileOptionUsage Usage = CompileOptionUsage::Hash;
  bool SaveFollowingArgument = false;
};

CompileOptionClassification SaveIfStartsWith(cm::string_view flag,
                                             cm::string_view prefix)
{
  if (flag.rfind(prefix, 0) == 0) {
    return { CompileOptionUsage::SavePreprocessor,
             flag.size() == prefix.size() };
  }

  return {};
}

CompileOptionClassification ClassifyGNUCompileOption(cm::string_view flag)
{
  if (IsGNUWarning(flag)) {
    return { CompileOptionUsage::Ignore, false };
  }

  if (flag.size() < 2 || flag.front() != '-') {
    return {};
  }

  flag.remove_prefix(1);
  switch (flag.front()) {
    case 'D':
    case 'I':
    case 'U':
      return { CompileOptionUsage::SavePreprocessor, flag.size() == 1 };
    case 'i': {
      CompileOptionClassification classification =
        SaveIfStartsWith(flag, "isystem"_s);
      if (classification.Usage != CompileOptionUsage::Hash) {
        return classification;
      }
    }
      {
        CompileOptionClassification classification =
          SaveIfStartsWith(flag, "iquote"_s);
        if (classification.Usage != CompileOptionUsage::Hash) {
          return classification;
        }
      }
      {
        CompileOptionClassification classification =
          SaveIfStartsWith(flag, "idirafter"_s);
        if (classification.Usage != CompileOptionUsage::Hash) {
          return classification;
        }
      }
      {
        CompileOptionClassification classification =
          SaveIfStartsWith(flag, "include"_s);
        if (classification.Usage != CompileOptionUsage::Hash) {
          return classification;
        }
      }
      {
        CompileOptionClassification classification =
          SaveIfStartsWith(flag, "imacros"_s);
        if (classification.Usage != CompileOptionUsage::Hash) {
          return classification;
        }
      }
      break;
    case '-':
      flag.remove_prefix(1);
      return SaveIfStartsWith(flag, "embed-dir"_s);
    default:
      break;
  }

  return {};
}

CompileOptionClassification ClassifyMSVCCompileOption(cm::string_view flag)
{
  if (IsMSVCWarning(flag)) {
    return { CompileOptionUsage::Ignore, false };
  }

  if (flag.size() < 2 || (flag.front() != '/' && flag.front() != '-')) {
    return {};
  }

  flag.remove_prefix(1);
  switch (flag.front()) {
    case 'D':
    case 'I':
    case 'U':
      return { CompileOptionUsage::SavePreprocessor, flag.size() == 1 };
    default:
      break;
  }

  return {};
}

CompileOptionClassification ClassifyUnknownCompileOption(cm::string_view flag)
{
  static_cast<void>(flag);
  return {};
}

using CompileOptionClassifier =
  CompileOptionClassification (*)(cm::string_view flag);

CompileOptionClassifier GetCompileOptionClassifier(cmGeneratorTarget const* gt)
{
  static CompileOptionClassifier classifier = nullptr;
  if (classifier) {
    return classifier;
  }

  cmValue frontendVariant =
    gt->Makefile->GetDefinition("CMAKE_CXX_COMPILER_FRONTEND_VARIANT");

  if (frontendVariant == "GNU") {
    classifier = ClassifyGNUCompileOption;
  } else if (frontendVariant == "MSVC") {
    classifier = ClassifyMSVCCompileOption;
  } else {
    classifier = ClassifyUnknownCompileOption;
  }

  return classifier;
}

struct SplitCompileOptionsResult
{
  std::vector<BT<std::string>> HashEntries;
  std::vector<BT<std::string>> PreprocessorEntries;
};

template <typename Range>
SplitCompileOptionsResult SplitCompileOptions(Range const& entries,
                                              CompileOptionClassifier classify)
{
  SplitCompileOptionsResult result;

  for (auto it = entries.begin(); it != entries.end(); ++it) {
    BT<std::string> const& option = *it;
    auto const classification = classify(option.Value);

    switch (classification.Usage) {
      case CompileOptionUsage::Ignore:
        continue;
      case CompileOptionUsage::SavePreprocessor:
        result.PreprocessorEntries.push_back(option);
        if (classification.SaveFollowingArgument) {
          auto next = it;
          ++next;
          if (next != entries.end()) {
            result.PreprocessorEntries.push_back(*next);
            it = next;
          }
        }
        continue;
      case CompileOptionUsage::Hash:
        break;
    }

    result.HashEntries.push_back(option);
  }

  return result;
}
}

cmCxxModuleUsageEffects::cmCxxModuleUsageEffects(cmGeneratorTarget const* gt,
                                                 std::string const& config)
{
  auto const* tgt = gt->Target;
  auto const classify = GetCompileOptionClassifier(gt);

  std::string usageHashInput;
  if (tgt->IsImported()) {
    AppendUsageEntries(usageHashInput,
                       tgt->GetImportedCxxModulesCompileFeaturesEntries());
    auto compileOptions = SplitCompileOptions(
      tgt->GetImportedCxxModulesCompileOptionsEntries(), classify);
    this->PreprocessorCompileOptions =
      std::move(compileOptions.PreprocessorEntries);
    AppendUsageEntries(usageHashInput, compileOptions.HashEntries);
  } else {
    auto compileOptions =
      SplitCompileOptions(gt->GetCompileOptions(config, "CXX"), classify);
    this->PreprocessorCompileOptions =
      std::move(compileOptions.PreprocessorEntries);
    AppendUsageEntries(usageHashInput, compileOptions.HashEntries);

    cmValue langStd = gt->GetLanguageStandard("CXX", config);
    if (!langStd) {
      langStd = gt->Makefile->GetDefinition("CMAKE_CXX_STANDARD_DEFAULT");
    }
    AppendUsage(usageHashInput,
                cmStrCat("CXX_STANDARD:", langStd ? *langStd : "20"));
    AppendUsage(
      usageHashInput,
      cmStrCat("CXX_EXTENSIONS:", *gt->GetLanguageExtensions("CXX")));
    AppendUsage(
      usageHashInput,
      cmStrCat("CXX_STANDARD_REQUIRED:",
               gt->GetLanguageStandardRequired("CXX") ? "TRUE" : "FALSE"));
  }

  cmValue runtimeDefault =
    gt->Makefile->GetDefinition("CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT");
  if (cmNonempty(runtimeDefault)) {
    cmValue runtime = gt->GetProperty("MSVC_RUNTIME_LIBRARY");
    this->MsvcRuntimeLibrary =
      cmGeneratorExpression::Evaluate(runtime ? *runtime : *runtimeDefault,
                                      gt->GetLocalGenerator(), config, gt);
    AppendUsage(usageHashInput,
                cmStrCat("MSVC_RUNTIME_LIBRARY:", this->MsvcRuntimeLibrary));
  }

  cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_512);
  this->Hash = hasher.HashString(usageHashInput);
}

std::string const& cmCxxModuleUsageEffects::GetHash() const
{
  return this->Hash;
}

std::string const& cmCxxModuleUsageEffects::GetMsvcRuntimeLibrary() const
{
  return this->MsvcRuntimeLibrary;
}

std::vector<BT<std::string>> const&
cmCxxModuleUsageEffects::GetPreprocessorCompileOptions() const
{
  return this->PreprocessorCompileOptions;
}
