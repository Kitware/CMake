/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <algorithm>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cmEvaluatedTargetProperty.h"
#include "cmGenExContext.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmUnreachable.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

enum class WrapperType
{
  Compile,
  Link,
};

std::string WrapperTypeToString(WrapperType type)
{
  switch (type) {
    case WrapperType::Compile:
      return "compile";
    case WrapperType::Link:
      return "link";
  }
  CM_UNREACHABLE;
}

void ProcessWrappers(cmGeneratorTarget const* tgt,
                     cm::EvaluatedTargetPropertyEntries const& entries,
                     std::vector<BT<std::string>>& wrappers,
                     std::unordered_set<std::string>& uniqueWrappers,
                     WrapperType wrapperType, bool debugWrapper)
{
  for (cm::EvaluatedTargetPropertyEntry const& entry : entries.Entries) {
    std::string usedOptions;
    for (std::string const& entryOption : entry.Values) {
      if (uniqueWrappers.insert(entryOption).second) {
        wrappers.emplace_back(entryOption, entry.Backtrace);
        if (debugWrapper) {
          usedOptions =
            cmStrCat(std::move(usedOptions), " * ", entryOption, '\n');
        }
      }
    }
    if (!usedOptions.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        cmStrCat("Used ", WrapperTypeToString(wrapperType),
                 " wrappers for target ", tgt->GetName(), ":\n", usedOptions),
        entry.Backtrace);
    }
  }

  // Order wrappers lexicographically based on their name.
  std::sort(wrappers.begin(), wrappers.end(),
            [](BT<std::string> const& a, BT<std::string> const& b) {
              return a.Value < b.Value;
            });
}
} // namespace

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkWrappers(
  std::string const& config, std::string const& lang) const
{
  ConfigAndLanguage cacheKey(config, lang);
  {
    auto it = this->LinkWrappersCache.find(cacheKey);
    if (it != this->LinkWrappersCache.end()) {
      return it->second;
    }
  }

  std::vector<BT<std::string>> wrappers;
  std::unordered_set<std::string> uniqueWrappers;
  cm::GenEx::Context context(this->LocalGenerator, config, lang);
  cmGeneratorExpressionDAGChecker dagChecker{
    this, "LINK_WRAPPERS", nullptr, nullptr, context,
  };

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugWrappers = !this->DebugLinkWrappersDone &&
    cm::contains(debugProperties, "LINK_WRAPPERS");
  this->DebugLinkWrappersDone = true;

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this, context, &dagChecker,
                                      this->LinkWrappersEntries);
  AddInterfaceEntries(this, "INTERFACE_LINK_WRAPPERS", context, &dagChecker,
                      entries, cm::IncludeRuntimeInterface::Yes, UseTo::Link);

  ProcessWrappers(this, entries, wrappers, uniqueWrappers, WrapperType::Link,
                  debugWrappers);

  this->LinkWrappersCache.emplace(cacheKey, wrappers);
  return wrappers;
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileWrappers(
  std::string const& config, std::string const& lang) const
{
  ConfigAndLanguage cacheKey(config, lang);
  {
    auto it = this->CompileWrappersCache.find(cacheKey);
    if (it != this->CompileWrappersCache.end()) {
      return it->second;
    }
  }

  std::vector<BT<std::string>> wrappers;
  std::unordered_set<std::string> uniqueWrappers;
  cm::GenEx::Context context(this->LocalGenerator, config, lang);
  cmGeneratorExpressionDAGChecker dagChecker{
    this, "COMPILE_WRAPPERS", nullptr, nullptr, context,
  };

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugWrappers = !this->DebugCompileWrappersDone &&
    cm::contains(debugProperties, "COMPILE_WRAPPERS");
  this->DebugCompileWrappersDone = true;

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this, context, &dagChecker,
                                      this->CompileWrappersEntries);
  AddInterfaceEntries(this, "INTERFACE_COMPILE_WRAPPERS", context, &dagChecker,
                      entries, cm::IncludeRuntimeInterface::Yes,
                      UseTo::Compile);

  ProcessWrappers(this, entries, wrappers, uniqueWrappers,
                  WrapperType::Compile, debugWrappers);

  this->CompileWrappersCache.emplace(cacheKey, wrappers);
  return wrappers;
}
