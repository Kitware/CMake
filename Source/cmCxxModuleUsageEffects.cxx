/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCxxModuleUsageEffects.h"

#include <algorithm>
#include <set>
#include <vector>

#include <cm/string_view>

#include "cmCryptoHash.h"
#include "cmGeneratorTarget.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
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

bool UnknownFilter(cm::string_view)
{
  return false;
}

using FlagFilter = bool (*)(cm::string_view flag);

FlagFilter GetFlagFilter(cmGeneratorTarget const* gt)
{
  static FlagFilter filter = nullptr;
  if (filter) {
    return filter;
  }

  cmValue frontendVariant =
    gt->Makefile->GetDefinition("CMAKE_CXX_COMPILER_FRONTEND_VARIANT");

  if (frontendVariant == "GNU") {
    filter = IsGNUWarning;
  } else if (frontendVariant == "MSVC") {
    filter = IsMSVCWarning;
  } else {
    filter = UnknownFilter;
  }

  return filter;
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
}

cmCxxModuleUsageEffects::cmCxxModuleUsageEffects(cmGeneratorTarget const* gt,
                                                 std::string const& config)
{
  auto const* tgt = gt->Target;
  auto const filter = GetFlagFilter(gt);

  std::string usageHashInput;
  if (tgt->IsImported()) {
    AppendUsageEntries(usageHashInput,
                       tgt->GetImportedCxxModulesCompileFeaturesEntries());
    AppendUsageEntries(
      usageHashInput,
      tgt->GetImportedCxxModulesCompileOptionsEntries().filter(
        [&](const BT<std::string>& flag) { return !filter(flag.Value); }));
  } else {
    AppendUsageEntries(usageHashInput,
                       cmMakeRange(gt->GetCompileOptions(config, "CXX"))
                         .filter([&](const BT<std::string>& flag) {
                           return !filter(flag.Value);
                         }));

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

  cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_512);
  this->Hash = hasher.HashString(usageHashInput);
}

std::string const& cmCxxModuleUsageEffects::GetHash() const
{
  return this->Hash;
}
