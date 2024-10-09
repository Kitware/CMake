/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include "cmConfigure.h"

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmEvaluatedTargetProperty.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

enum class OptionsParse
{
  None,
  Shell
};

namespace {
const auto DL_BEGIN = "<DEVICE_LINK>"_s;
const auto DL_END = "</DEVICE_LINK>"_s;

void processOptions(cmGeneratorTarget const* tgt,
                    EvaluatedTargetPropertyEntries const& entries,
                    std::vector<BT<std::string>>& options,
                    std::unordered_set<std::string>& uniqueOptions,
                    bool debugOptions, const char* logName, OptionsParse parse,
                    bool processDeviceOptions = false)
{
  bool splitOption = !processDeviceOptions;
  for (EvaluatedTargetPropertyEntry const& entry : entries.Entries) {
    std::string usedOptions;
    for (std::string const& opt : entry.Values) {
      if (processDeviceOptions && (opt == DL_BEGIN || opt == DL_END)) {
        options.emplace_back(opt, entry.Backtrace);
        splitOption = opt == DL_BEGIN;
        continue;
      }

      if (uniqueOptions.insert(opt).second) {
        if (parse == OptionsParse::Shell &&
            cmHasLiteralPrefix(opt, "SHELL:")) {
          if (splitOption) {
            std::vector<std::string> tmp;
            cmSystemTools::ParseUnixCommandLine(opt.c_str() + 6, tmp);
            for (std::string& o : tmp) {
              options.emplace_back(std::move(o), entry.Backtrace);
            }
          } else {
            options.emplace_back(std::string(opt.c_str() + 6),
                                 entry.Backtrace);
          }
        } else {
          options.emplace_back(opt, entry.Backtrace);
        }
        if (debugOptions) {
          usedOptions += " * " + opt + "\n";
        }
      }
    }
    if (!usedOptions.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used ") + logName + std::string(" for target ") +
          tgt->GetName() + ":\n" + usedOptions,
        entry.Backtrace);
    }
  }
}

std::vector<BT<std::string>> wrapOptions(
  std::vector<std::string>& options, const cmListFileBacktrace& bt,
  const std::vector<std::string>& wrapperFlag, const std::string& wrapperSep,
  bool concatFlagAndArgs)
{
  std::vector<BT<std::string>> result;

  if (options.empty()) {
    return result;
  }

  if (wrapperFlag.empty()) {
    // nothing specified, insert elements as is
    result.reserve(options.size());
    for (std::string& o : options) {
      result.emplace_back(std::move(o), bt);
    }
    return result;
  }

  for (std::vector<std::string>::size_type index = 0; index < options.size();
       index++) {
    if (cmHasLiteralPrefix(options[index], "LINKER:")) {
      // LINKER wrapper specified, insert elements as is
      result.emplace_back(std::move(options[index]), bt);
      continue;
    }
    if (cmHasLiteralPrefix(options[index], "-Wl,")) {
      // replace option by LINKER wrapper
      result.emplace_back(options[index].replace(0, 4, "LINKER:"), bt);
      continue;
    }
    if (cmHasLiteralPrefix(options[index], "-Xlinker=")) {
      // replace option by LINKER wrapper
      result.emplace_back(options[index].replace(0, 9, "LINKER:"), bt);
      continue;
    }
    if (options[index] == "-Xlinker") {
      // replace option by LINKER wrapper
      if (index + 1 < options.size()) {
        result.emplace_back("LINKER:" + options[++index], bt);
      } else {
        result.emplace_back(std::move(options[index]), bt);
      }
      continue;
    }

    // collect all options which must be transformed
    std::vector<std::string> opts;
    while (index < options.size()) {
      if (!cmHasLiteralPrefix(options[index], "LINKER:") &&
          !cmHasLiteralPrefix(options[index], "-Wl,") &&
          !cmHasLiteralPrefix(options[index], "-Xlinker")) {
        opts.emplace_back(std::move(options[index++]));
      } else {
        --index;
        break;
      }
    }
    if (opts.empty()) {
      continue;
    }

    if (!wrapperSep.empty()) {
      if (concatFlagAndArgs) {
        // insert flag elements except last one
        for (auto i = wrapperFlag.begin(); i != wrapperFlag.end() - 1; ++i) {
          result.emplace_back(*i, bt);
        }
        // concatenate last flag element and all list values
        // in one option
        result.emplace_back(wrapperFlag.back() + cmJoin(opts, wrapperSep), bt);
      } else {
        for (std::string const& i : wrapperFlag) {
          result.emplace_back(i, bt);
        }
        // concatenate all list values in one option
        result.emplace_back(cmJoin(opts, wrapperSep), bt);
      }
    } else {
      // prefix each element of list with wrapper
      if (concatFlagAndArgs) {
        std::transform(opts.begin(), opts.end(), opts.begin(),
                       [&wrapperFlag](std::string const& o) -> std::string {
                         return wrapperFlag.back() + o;
                       });
      }
      for (std::string& o : opts) {
        for (auto i = wrapperFlag.begin(),
                  e = concatFlagAndArgs ? wrapperFlag.end() - 1
                                        : wrapperFlag.end();
             i != e; ++i) {
          result.emplace_back(*i, bt);
        }
        result.emplace_back(std::move(o), bt);
      }
    }
  }
  return result;
}
}

void cmGeneratorTarget::GetCompileOptions(std::vector<std::string>& result,
                                          const std::string& config,
                                          const std::string& language) const
{
  std::vector<BT<std::string>> tmp = this->GetCompileOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileOptions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->CompileOptionsCache.find(cacheKey);
    if (it != this->CompileOptionsCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(
    this, "COMPILE_OPTIONS", nullptr, nullptr, this->LocalGenerator, config);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugOptions = !this->DebugCompileOptionsDone &&
    cm::contains(debugProperties, "COMPILE_OPTIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileOptionsDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->CompileOptionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_OPTIONS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  processOptions(this, entries, result, uniqueOptions, debugOptions,
                 "compile options", OptionsParse::Shell);

  CompileOptionsCache.emplace(cacheKey, result);
  return result;
}

void cmGeneratorTarget::GetCompileFeatures(std::vector<std::string>& result,
                                           const std::string& config) const
{
  std::vector<BT<std::string>> tmp = this->GetCompileFeatures(config);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileFeatures(
  std::string const& config) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueFeatures;

  cmGeneratorExpressionDAGChecker dagChecker(
    this, "COMPILE_FEATURES", nullptr, nullptr, this->LocalGenerator, config);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugFeatures = !this->DebugCompileFeaturesDone &&
    cm::contains(debugProperties, "COMPILE_FEATURES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileFeaturesDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, std::string(), &dagChecker, this->CompileFeaturesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_FEATURES",
                      std::string(), &dagChecker, entries,
                      IncludeRuntimeInterface::Yes);

  processOptions(this, entries, result, uniqueFeatures, debugFeatures,
                 "compile features", OptionsParse::None);

  return result;
}

void cmGeneratorTarget::GetCompileDefinitions(
  std::vector<std::string>& result, const std::string& config,
  const std::string& language) const
{
  std::vector<BT<std::string>> tmp =
    this->GetCompileDefinitions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileDefinitions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->CompileDefinitionsCache.find(cacheKey);
    if (it != this->CompileDefinitionsCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> list;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "COMPILE_DEFINITIONS",
                                             nullptr, nullptr,
                                             this->LocalGenerator, config);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugDefines = !this->DebugCompileDefinitionsDone &&
    cm::contains(debugProperties, "COMPILE_DEFINITIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileDefinitionsDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->CompileDefinitionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_DEFINITIONS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  if (!config.empty()) {
    std::string configPropName =
      "COMPILE_DEFINITIONS_" + cmSystemTools::UpperCase(config);
    cmValue configProp = this->GetProperty(configPropName);
    if (configProp) {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0043)) {
        case cmPolicies::WARN: {
          this->LocalGenerator->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmPolicies::GetPolicyWarning(cmPolicies::CMP0043));
          CM_FALLTHROUGH;
        }
        case cmPolicies::OLD: {
          std::unique_ptr<TargetPropertyEntry> entry =
            TargetPropertyEntry::Create(
              *this->LocalGenerator->GetCMakeInstance(), *configProp);
          entries.Entries.emplace_back(EvaluateTargetPropertyEntry(
            this, config, language, &dagChecker, *entry));
        } break;
        case cmPolicies::NEW:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::REQUIRED_IF_USED:
          break;
      }
    }
  }

  processOptions(this, entries, list, uniqueOptions, debugDefines,
                 "compile definitions", OptionsParse::None);

  this->CompileDefinitionsCache.emplace(cacheKey, list);
  return list;
}

std::vector<BT<std::string>> cmGeneratorTarget::GetPrecompileHeaders(
  const std::string& config, const std::string& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->PrecompileHeadersCache.find(cacheKey);
    if (it != this->PrecompileHeadersCache.end()) {
      return it->second;
    }
  }
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "PRECOMPILE_HEADERS",
                                             nullptr, nullptr,
                                             this->LocalGenerator, config);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugDefines = !this->DebugPrecompileHeadersDone &&
    std::find(debugProperties.begin(), debugProperties.end(),
              "PRECOMPILE_HEADERS") != debugProperties.end();

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugPrecompileHeadersDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->PrecompileHeadersEntries);

  AddInterfaceEntries(this, config, "INTERFACE_PRECOMPILE_HEADERS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  std::vector<BT<std::string>> list;
  processOptions(this, entries, list, uniqueOptions, debugDefines,
                 "precompile headers", OptionsParse::None);

  this->PrecompileHeadersCache.emplace(cacheKey, list);
  return list;
}

void cmGeneratorTarget::GetLinkOptions(std::vector<std::string>& result,
                                       const std::string& config,
                                       const std::string& language) const
{
  if (this->IsDeviceLink() &&
      this->GetPolicyStatusCMP0105() != cmPolicies::NEW) {
    // link options are not propagated to the device link step
    return;
  }

  std::vector<BT<std::string>> tmp = this->GetLinkOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkOptions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(
    config, cmStrCat(language, this->IsDeviceLink() ? "-device" : ""));
  {
    auto it = this->LinkOptionsCache.find(cacheKey);
    if (it != this->LinkOptionsCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(
    this, "LINK_OPTIONS", nullptr, nullptr, this->LocalGenerator, config);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugOptions = !this->DebugLinkOptionsDone &&
    cm::contains(debugProperties, "LINK_OPTIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugLinkOptionsDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->LinkOptionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_LINK_OPTIONS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes,
                      this->GetPolicyStatusCMP0099() == cmPolicies::NEW
                        ? UseTo::Link
                        : UseTo::Compile);

  processOptions(this, entries, result, uniqueOptions, debugOptions,
                 "link options", OptionsParse::Shell, this->IsDeviceLink());

  if (this->IsDeviceLink()) {
    // wrap host link options
    const std::string wrapper(this->Makefile->GetSafeDefinition(
      "CMAKE_" + language + "_DEVICE_COMPILER_WRAPPER_FLAG"));
    cmList wrapperFlag{ wrapper };
    const std::string wrapperSep(this->Makefile->GetSafeDefinition(
      "CMAKE_" + language + "_DEVICE_COMPILER_WRAPPER_FLAG_SEP"));
    bool concatFlagAndArgs = true;
    if (!wrapperFlag.empty() && wrapperFlag.back() == " ") {
      concatFlagAndArgs = false;
      wrapperFlag.pop_back();
    }

    auto it = result.begin();
    while (it != result.end()) {
      if (it->Value == DL_BEGIN) {
        // device link options, no treatment
        it = result.erase(it);
        it = std::find_if(it, result.end(), [](const BT<std::string>& item) {
          return item.Value == DL_END;
        });
        if (it != result.end()) {
          it = result.erase(it);
        }
      } else {
        // host link options must be wrapped
        std::vector<std::string> options;
        cmSystemTools::ParseUnixCommandLine(it->Value.c_str(), options);
        auto hostOptions = wrapOptions(options, it->Backtrace, wrapperFlag,
                                       wrapperSep, concatFlagAndArgs);
        it = result.erase(it);
        // some compilers (like gcc 4.8 or Intel 19.0 or XLC 16) do not respect
        // C++11 standard: 'std::vector::insert()' do not returns an iterator,
        // so need to recompute the iterator after insertion.
        if (it == result.end()) {
          cm::append(result, hostOptions);
          it = result.end();
        } else {
          auto index = it - result.begin();
          result.insert(it, hostOptions.begin(), hostOptions.end());
          it = result.begin() + index + hostOptions.size();
        }
      }
    }
  }

  // Last step: replace "LINKER:" prefixed elements by
  // actual linker wrapper
  result = this->ResolveLinkerWrapper(result, language);

  this->LinkOptionsCache.emplace(cacheKey, result);
  return result;
}

std::vector<BT<std::string>>& cmGeneratorTarget::ResolveLinkerWrapper(
  std::vector<BT<std::string>>& result, const std::string& language,
  bool joinItems) const
{
  // replace "LINKER:" prefixed elements by actual linker wrapper
  const std::string wrapper(this->Makefile->GetSafeDefinition(
    "CMAKE_" + language +
    (this->IsDeviceLink() ? "_DEVICE_LINKER_WRAPPER_FLAG"
                          : "_LINKER_WRAPPER_FLAG")));
  cmList wrapperFlag{ wrapper };
  const std::string wrapperSep(this->Makefile->GetSafeDefinition(
    "CMAKE_" + language +
    (this->IsDeviceLink() ? "_DEVICE_LINKER_WRAPPER_FLAG_SEP"
                          : "_LINKER_WRAPPER_FLAG_SEP")));
  bool concatFlagAndArgs = true;
  if (!wrapperFlag.empty() && wrapperFlag.back() == " ") {
    concatFlagAndArgs = false;
    wrapperFlag.pop_back();
  }

  const std::string LINKER{ "LINKER:" };
  const std::string SHELL{ "SHELL:" };
  const std::string LINKER_SHELL = LINKER + SHELL;

  std::vector<BT<std::string>>::iterator entry;
  while ((entry = std::find_if(result.begin(), result.end(),
                               [&LINKER](BT<std::string> const& item) -> bool {
                                 return item.Value.compare(0, LINKER.length(),
                                                           LINKER) == 0;
                               })) != result.end()) {
    std::string value = std::move(entry->Value);
    cmListFileBacktrace bt = std::move(entry->Backtrace);
    entry = result.erase(entry);

    std::vector<std::string> linkerOptions;
    if (value.compare(0, LINKER_SHELL.length(), LINKER_SHELL) == 0) {
      cmSystemTools::ParseUnixCommandLine(
        value.c_str() + LINKER_SHELL.length(), linkerOptions);
    } else {
      linkerOptions = cmTokenize(value.substr(LINKER.length()), ",");
    }

    if (linkerOptions.empty() ||
        (linkerOptions.size() == 1 && linkerOptions.front().empty())) {
      continue;
    }

    // for now, raise an error if prefix SHELL: is part of arguments
    if (std::find_if(linkerOptions.begin(), linkerOptions.end(),
                     [&SHELL](const std::string& item) -> bool {
                       return item.find(SHELL) != std::string::npos;
                     }) != linkerOptions.end()) {
      this->LocalGenerator->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR,
        "'SHELL:' prefix is not supported as part of 'LINKER:' arguments.",
        this->GetBacktrace());
      return result;
    }

    std::vector<BT<std::string>> options = wrapOptions(
      linkerOptions, bt, wrapperFlag, wrapperSep, concatFlagAndArgs);
    if (joinItems) {
      result.insert(entry,
                    cmJoin(cmRange<decltype(options.cbegin())>(
                             options.cbegin(), options.cend()),
                           " "_s));
    } else {
      result.insert(entry, options.begin(), options.end());
    }
  }
  return result;
}

void cmGeneratorTarget::GetStaticLibraryLinkOptions(
  std::vector<std::string>& result, const std::string& config,
  const std::string& language) const
{
  std::vector<BT<std::string>> tmp =
    this->GetStaticLibraryLinkOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetStaticLibraryLinkOptions(
  std::string const& config, std::string const& language) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "STATIC_LIBRARY_OPTIONS",
                                             nullptr, nullptr,
                                             this->LocalGenerator, config);

  EvaluatedTargetPropertyEntries entries;
  if (cmValue linkOptions = this->GetProperty("STATIC_LIBRARY_OPTIONS")) {
    std::unique_ptr<TargetPropertyEntry> entry = TargetPropertyEntry::Create(
      *this->LocalGenerator->GetCMakeInstance(), *linkOptions);
    entries.Entries.emplace_back(EvaluateTargetPropertyEntry(
      this, config, language, &dagChecker, *entry));
  }
  processOptions(this, entries, result, uniqueOptions, false,
                 "static library link options", OptionsParse::Shell);

  return result;
}

void cmGeneratorTarget::GetLinkDepends(std::vector<std::string>& result,
                                       const std::string& config,
                                       const std::string& language) const
{
  std::vector<BT<std::string>> tmp = this->GetLinkDepends(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkDepends(
  std::string const& config, std::string const& language) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;
  cmGeneratorExpressionDAGChecker dagChecker(
    this, "LINK_DEPENDS", nullptr, nullptr, this->LocalGenerator, config);

  EvaluatedTargetPropertyEntries entries;
  if (cmValue linkDepends = this->GetProperty("LINK_DEPENDS")) {
    cmList depends{ *linkDepends };
    for (const auto& depend : depends) {
      std::unique_ptr<TargetPropertyEntry> entry = TargetPropertyEntry::Create(
        *this->LocalGenerator->GetCMakeInstance(), depend);
      entries.Entries.emplace_back(EvaluateTargetPropertyEntry(
        this, config, language, &dagChecker, *entry));
    }
  }
  AddInterfaceEntries(this, config, "INTERFACE_LINK_DEPENDS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes,
                      this->GetPolicyStatusCMP0099() == cmPolicies::NEW
                        ? UseTo::Link
                        : UseTo::Compile);

  processOptions(this, entries, result, uniqueOptions, false, "link depends",
                 OptionsParse::None);

  return result;
}
