/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include "cmConfigure.h"

#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/optional>
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
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
using UseTo = cmGeneratorTarget::UseTo;

enum class IncludeDirectoryFallBack
{
  BINARY,
  OBJECT
};

std::string AddLangSpecificInterfaceIncludeDirectories(
  cmGeneratorTarget const* root, cmGeneratorTarget const* target,
  std::string const& lang, std::string const& config,
  std::string const& propertyName, IncludeDirectoryFallBack mode,
  cmGeneratorExpressionDAGChecker* context)
{
  cmGeneratorExpressionDAGChecker dagChecker{
    target,
    propertyName,
    nullptr,
    context,
    target->GetLocalGenerator(),
    config,
    target->GetBacktrace(),
  };
  switch (dagChecker.Check()) {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      dagChecker.ReportError(
        nullptr, "$<TARGET_PROPERTY:" + target->GetName() + ",propertyName");
      CM_FALLTHROUGH;
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      // No error. We have already seen this transitive property.
      return "";
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
  }

  std::string directories;
  if (auto const* link_interface =
        target->GetLinkInterfaceLibraries(config, root, UseTo::Compile)) {
    for (cmLinkItem const& library : link_interface->Libraries) {
      if (cmGeneratorTarget const* dependency = library.Target) {
        if (cm::contains(dependency->GetAllConfigCompileLanguages(), lang)) {
          auto* lg = dependency->GetLocalGenerator();
          std::string value = dependency->GetSafeProperty(propertyName);
          if (value.empty()) {
            if (mode == IncludeDirectoryFallBack::BINARY) {
              value = lg->GetCurrentBinaryDirectory();
            } else if (mode == IncludeDirectoryFallBack::OBJECT) {
              value = cmStrCat(lg->GetCurrentBinaryDirectory(), '/',
                               lg->GetTargetDirectory(dependency));
            }
          }

          if (!directories.empty()) {
            directories += ";";
          }
          directories += value;
        }
      }
    }
  }
  return directories;
}

void AddLangSpecificImplicitIncludeDirectories(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, std::string const& propertyName,
  IncludeDirectoryFallBack mode, EvaluatedTargetPropertyEntries& entries)
{
  if (auto const* libraries =
        target->GetLinkImplementationLibraries(config, UseTo::Compile)) {
    cmGeneratorExpressionDAGChecker dagChecker{
      target,
      propertyName,
      nullptr,
      nullptr,
      target->GetLocalGenerator(),
      config,
      target->GetBacktrace(),
    };

    for (cmLinkImplItem const& library : libraries->Libraries) {
      if (cmGeneratorTarget const* dependency = library.Target) {
        if (!dependency->IsInBuildSystem()) {
          continue;
        }
        if (cm::contains(dependency->GetAllConfigCompileLanguages(), lang)) {
          auto* lg = dependency->GetLocalGenerator();
          EvaluatedTargetPropertyEntry entry{ library, library.Backtrace };

          if (lang == "Swift") {
            entry.Values.emplace_back(
              dependency->GetSwiftModuleDirectory(config));
          } else if (cmValue val = dependency->GetProperty(propertyName)) {
            entry.Values.emplace_back(*val);
          } else {
            if (mode == IncludeDirectoryFallBack::BINARY) {
              entry.Values.emplace_back(lg->GetCurrentBinaryDirectory());
            } else if (mode == IncludeDirectoryFallBack::OBJECT) {
              entry.Values.emplace_back(
                dependency->GetObjectDirectory(config));
            }
          }

          cmExpandList(AddLangSpecificInterfaceIncludeDirectories(
                         target, dependency, lang, config, propertyName, mode,
                         &dagChecker),
                       entry.Values);
          entries.Entries.emplace_back(std::move(entry));
        }
      }
    }
  }
}

void processIncludeDirectories(cmGeneratorTarget const* tgt,
                               EvaluatedTargetPropertyEntries& entries,
                               std::vector<BT<std::string>>& includes,
                               std::unordered_set<std::string>& uniqueIncludes,
                               bool debugIncludes)
{
  for (EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();
    bool const fromImported = item.Target && item.Target->IsImported();

    std::string usedIncludes;
    for (std::string& entryInclude : entry.Values) {
      if (fromImported && !cmSystemTools::FileExists(entryInclude)) {
        tgt->GetLocalGenerator()->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat(
            "Imported target \"", targetName,
            "\" includes non-existent path\n  \"", entryInclude,
            "\"\nin its INTERFACE_INCLUDE_DIRECTORIES. Possible reasons "
            "include:\n"
            "* The path was deleted, renamed, or moved to another location.\n"
            "* An install or uninstall procedure did not complete "
            "successfully.\n"
            "* The installation package was faulty and references files it "
            "does not provide.\n"));
        return;
      }

      if (!cmSystemTools::FileIsFullPath(entryInclude)) {
        std::ostringstream e;
        MessageType messageType = MessageType::FATAL_ERROR;
        if (!targetName.empty()) {
          /* clang-format off */
          e << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_INCLUDE_DIRECTORIES:\n"
            "  \"" << entryInclude << "\"";
          /* clang-format on */
        } else {
          e << "Found relative path while evaluating include directories of "
               "\""
            << tgt->GetName() << "\":\n  \"" << entryInclude << "\"\n";
        }
        tgt->GetLocalGenerator()->IssueMessage(messageType, e.str());
        if (messageType == MessageType::FATAL_ERROR) {
          return;
        }
      }

      if (!cmIsOff(entryInclude)) {
        cmSystemTools::ConvertToUnixSlashes(entryInclude);
      }

      if (uniqueIncludes.insert(entryInclude).second) {
        includes.emplace_back(entryInclude, entry.Backtrace);
        if (debugIncludes) {
          usedIncludes += " * " + entryInclude + "\n";
        }
      }
    }
    if (!usedIncludes.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used includes for target ") + tgt->GetName() + ":\n" +
          usedIncludes,
        entry.Backtrace);
    }
  }
}
}

std::vector<BT<std::string>> cmGeneratorTarget::GetIncludeDirectories(
  std::string const& config, std::string const& lang) const
{
  ConfigAndLanguage cacheKey(config, lang);
  {
    auto it = this->IncludeDirectoriesCache.find(cacheKey);
    if (it != this->IncludeDirectoriesCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> includes;
  std::unordered_set<std::string> uniqueIncludes;

  cmGeneratorExpressionDAGChecker dagChecker{
    this,    "INCLUDE_DIRECTORIES", nullptr,
    nullptr, this->LocalGenerator,  config,
  };

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugIncludes = !this->DebugIncludesDone &&
    cm::contains(debugProperties, "INCLUDE_DIRECTORIES");

  this->DebugIncludesDone = true;

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, lang, &dagChecker, this->IncludeDirectoriesEntries);

  if (lang == "Swift") {
    AddLangSpecificImplicitIncludeDirectories(
      this, lang, config, "Swift_MODULE_DIRECTORY",
      IncludeDirectoryFallBack::BINARY, entries);
  }

  if (this->CanCompileSources() && (lang != "Swift" && lang != "Fortran")) {

    std::string const propertyName = "ISPC_HEADER_DIRECTORY";

    // If this target has ISPC sources make sure to add the header
    // directory to other compilation units
    if (cm::contains(this->GetAllConfigCompileLanguages(), "ISPC")) {
      if (cmValue val = this->GetProperty(propertyName)) {
        includes.emplace_back(*val);
      } else {
        includes.emplace_back(this->GetObjectDirectory(config));
      }
    }

    AddLangSpecificImplicitIncludeDirectories(
      this, "ISPC", config, propertyName, IncludeDirectoryFallBack::OBJECT,
      entries);
  }

  AddInterfaceEntries(this, config, "INTERFACE_INCLUDE_DIRECTORIES", lang,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  processIncludeDirectories(this, entries, includes, uniqueIncludes,
                            debugIncludes);

  if (this->IsApple()) {
    if (cmLinkImplementationLibraries const* impl =
          this->GetLinkImplementationLibraries(config, UseTo::Compile)) {
      for (cmLinkImplItem const& lib : impl->Libraries) {
        std::string libDir;
        if (!lib.Target) {
          libDir = cmSystemTools::CollapseFullPath(
            lib.AsStr(), this->Makefile->GetHomeOutputDirectory());
        } else if (lib.Target->Target->IsFrameworkOnApple() ||
                   this->IsImportedFrameworkFolderOnApple(config)) {
          libDir = lib.Target->GetLocation(config);
        } else {
          continue;
        }

        auto fwDescriptor =
          this->GetGlobalGenerator()->SplitFrameworkPath(libDir);
        if (!fwDescriptor) {
          continue;
        }

        auto fwInclude = fwDescriptor->GetFrameworkPath();
        if (uniqueIncludes.insert(fwInclude).second) {
          includes.emplace_back(fwInclude, cmListFileBacktrace());
        }
      }
    }
  }

  this->IncludeDirectoriesCache.emplace(cacheKey, includes);
  return includes;
}
