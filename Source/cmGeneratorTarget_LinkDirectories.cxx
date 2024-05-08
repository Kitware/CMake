/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cmEvaluatedTargetProperty.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
void processLinkDirectories(cmGeneratorTarget const* tgt,
                            EvaluatedTargetPropertyEntries& entries,
                            std::vector<BT<std::string>>& directories,
                            std::unordered_set<std::string>& uniqueDirectories,
                            bool debugDirectories)
{
  for (EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();

    std::string usedDirectories;
    for (std::string& entryDirectory : entry.Values) {
      if (!cmSystemTools::FileIsFullPath(entryDirectory)) {
        std::ostringstream e;
        bool noMessage = false;
        MessageType messageType = MessageType::FATAL_ERROR;
        if (!targetName.empty()) {
          /* clang-format off */
          e << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_LINK_DIRECTORIES:\n"
            "  \"" << entryDirectory << "\"";
          /* clang-format on */
        } else {
          switch (tgt->GetPolicyStatusCMP0081()) {
            case cmPolicies::WARN: {
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0081) << "\n";
              messageType = MessageType::AUTHOR_WARNING;
            } break;
            case cmPolicies::OLD:
              noMessage = true;
              break;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
          }
          e << "Found relative path while evaluating link directories of "
               "\""
            << tgt->GetName() << "\":\n  \"" << entryDirectory << "\"\n";
        }
        if (!noMessage) {
          tgt->GetLocalGenerator()->IssueMessage(messageType, e.str());
          if (messageType == MessageType::FATAL_ERROR) {
            return;
          }
        }
      }

      // Sanitize the path the same way the link_directories command does
      // in case projects set the LINK_DIRECTORIES property directly.
      cmSystemTools::ConvertToUnixSlashes(entryDirectory);
      if (uniqueDirectories.insert(entryDirectory).second) {
        directories.emplace_back(entryDirectory, entry.Backtrace);
        if (debugDirectories) {
          usedDirectories += " * " + entryDirectory + "\n";
        }
      }
    }
    if (!usedDirectories.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used link directories for target ") + tgt->GetName() +
          ":\n" + usedDirectories,
        entry.Backtrace);
    }
  }
}
}

void cmGeneratorTarget::GetLinkDirectories(std::vector<std::string>& result,
                                           const std::string& config,
                                           const std::string& language) const
{
  std::vector<BT<std::string>> tmp =
    this->GetLinkDirectories(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkDirectories(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(
    config, cmStrCat(language, this->IsDeviceLink() ? "-device" : ""));
  {
    auto it = this->LinkDirectoriesCache.find(cacheKey);
    if (it != this->LinkDirectoriesCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueDirectories;

  cmGeneratorExpressionDAGChecker dagChecker(
    this, "LINK_DIRECTORIES", nullptr, nullptr, this->LocalGenerator, config);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugDirectories = !this->DebugLinkDirectoriesDone &&
    cm::contains(debugProperties, "LINK_DIRECTORIES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugLinkDirectoriesDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->LinkDirectoriesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_LINK_DIRECTORIES", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes,
                      this->GetPolicyStatusCMP0099() == cmPolicies::NEW
                        ? UseTo::Link
                        : UseTo::Compile);

  processLinkDirectories(this, entries, result, uniqueDirectories,
                         debugDirectories);

  this->LinkDirectoriesCache.emplace(cacheKey, result);
  return result;
}
