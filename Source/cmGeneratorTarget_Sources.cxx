/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmEvaluatedTargetProperty.h"
#include "cmFileSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceGroup.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
using UseTo = cmGeneratorTarget::UseTo;

void AddObjectEntries(cmGeneratorTarget const* headTarget,
                      std::string const& config,
                      cmGeneratorExpressionDAGChecker* dagChecker,
                      EvaluatedTargetPropertyEntries& entries)
{
  if (cmLinkImplementationLibraries const* impl =
        headTarget->GetLinkImplementationLibraries(config, UseTo::Compile)) {
    entries.HadContextSensitiveCondition = impl->HadContextSensitiveCondition;
    for (cmLinkImplItem const& lib : impl->Libraries) {
      if (lib.Target &&
          lib.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        std::string uniqueName =
          headTarget->GetGlobalGenerator()->IndexGeneratorTargetUniquely(
            lib.Target);
        std::string genex = "$<TARGET_OBJECTS:" + std::move(uniqueName) + ">";
        cmGeneratorExpression ge(*headTarget->Makefile->GetCMakeInstance(),
                                 lib.Backtrace);
        std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(genex);
        cge->SetEvaluateForBuildsystem(true);

        EvaluatedTargetPropertyEntry ee(lib, lib.Backtrace);
        cmExpandList(cge->Evaluate(headTarget->GetLocalGenerator(), config,
                                   headTarget, dagChecker),
                     ee.Values);
        if (cge->GetHadContextSensitiveCondition()) {
          ee.ContextDependent = true;
        }
        entries.Entries.emplace_back(std::move(ee));
      }
    }
  }
}

void addFileSetEntry(cmGeneratorTarget const* headTarget,
                     std::string const& config,
                     cmGeneratorExpressionDAGChecker* dagChecker,
                     cmFileSet const* fileSet,
                     EvaluatedTargetPropertyEntries& entries)
{
  auto dirCges = fileSet->CompileDirectoryEntries();
  auto dirs = fileSet->EvaluateDirectoryEntries(
    dirCges, headTarget->GetLocalGenerator(), config, headTarget, dagChecker);
  bool contextSensitiveDirs = false;
  for (auto const& dirCge : dirCges) {
    if (dirCge->GetHadContextSensitiveCondition()) {
      contextSensitiveDirs = true;
      break;
    }
  }
  cmake* cm = headTarget->GetLocalGenerator()->GetCMakeInstance();
  for (auto& entryCge : fileSet->CompileFileEntries()) {
    auto tpe = cmGeneratorTarget::TargetPropertyEntry::CreateFileSet(
      dirs, contextSensitiveDirs, std::move(entryCge), fileSet);
    entries.Entries.emplace_back(
      EvaluateTargetPropertyEntry(headTarget, config, "", dagChecker, *tpe));
    EvaluatedTargetPropertyEntry const& entry = entries.Entries.back();
    for (auto const& file : entry.Values) {
      auto* sf = headTarget->Makefile->GetOrCreateSource(file);
      if (fileSet->GetType() == "HEADERS"_s) {
        sf->SetProperty("HEADER_FILE_ONLY", "TRUE");
      }

#ifndef CMAKE_BOOTSTRAP
      std::string e;
      std::string w;
      auto path = sf->ResolveFullPath(&e, &w);
      if (!w.empty()) {
        cm->IssueMessage(MessageType::AUTHOR_WARNING, w, entry.Backtrace);
      }
      if (path.empty()) {
        if (!e.empty()) {
          cm->IssueMessage(MessageType::FATAL_ERROR, e, entry.Backtrace);
        }
        return;
      }
      bool found = false;
      for (auto const& sg : headTarget->Makefile->GetSourceGroups()) {
        if (sg.MatchChildrenFiles(path)) {
          found = true;
          break;
        }
      }
      if (!found) {
        if (fileSet->GetType() == "HEADERS"_s) {
          headTarget->Makefile->GetOrCreateSourceGroup("Header Files")
            ->AddGroupFile(path);
        }
      }
#endif
    }
  }
}

void AddFileSetEntries(cmGeneratorTarget const* headTarget,
                       std::string const& config,
                       cmGeneratorExpressionDAGChecker* dagChecker,
                       EvaluatedTargetPropertyEntries& entries)
{
  for (auto const& entry : headTarget->Target->GetHeaderSetsEntries()) {
    for (auto const& name : cmList{ entry.Value }) {
      auto const* headerSet = headTarget->Target->GetFileSet(name);
      addFileSetEntry(headTarget, config, dagChecker, headerSet, entries);
    }
  }
  for (auto const& entry : headTarget->Target->GetCxxModuleSetsEntries()) {
    for (auto const& name : cmList{ entry.Value }) {
      auto const* cxxModuleSet = headTarget->Target->GetFileSet(name);
      addFileSetEntry(headTarget, config, dagChecker, cxxModuleSet, entries);
    }
  }
}

bool processSources(cmGeneratorTarget const* tgt,
                    EvaluatedTargetPropertyEntries& entries,
                    std::vector<BT<std::string>>& srcs,
                    std::unordered_set<std::string>& uniqueSrcs,
                    bool debugSources)
{
  cmMakefile* mf = tgt->Target->GetMakefile();

  bool contextDependent = entries.HadContextSensitiveCondition;

  for (EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    if (entry.ContextDependent) {
      contextDependent = true;
    }

    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();

    for (std::string& src : entry.Values) {
      cmSourceFile* sf = mf->GetOrCreateSource(src);
      std::string e;
      std::string w;
      std::string fullPath = sf->ResolveFullPath(&e, &w);
      cmake* cm = tgt->GetLocalGenerator()->GetCMakeInstance();
      if (!w.empty()) {
        cm->IssueMessage(MessageType::AUTHOR_WARNING, w, entry.Backtrace);
      }
      if (fullPath.empty()) {
        if (!e.empty()) {
          cm->IssueMessage(MessageType::FATAL_ERROR, e, entry.Backtrace);
        }
        return contextDependent;
      }

      if (!targetName.empty() && !cmSystemTools::FileIsFullPath(src)) {
        std::ostringstream err;
        if (!targetName.empty()) {
          err << "Target \"" << targetName
              << "\" contains relative path in its INTERFACE_SOURCES:\n  \""
              << src << "\"";
        } else {
          err << "Found relative path while evaluating sources of \""
              << tgt->GetName() << "\":\n  \"" << src << "\"\n";
        }
        tgt->GetLocalGenerator()->IssueMessage(MessageType::FATAL_ERROR,
                                               err.str());
        return contextDependent;
      }
      src = fullPath;
    }
    std::string usedSources;
    for (std::string const& src : entry.Values) {
      if (uniqueSrcs.insert(src).second) {
        srcs.emplace_back(src, entry.Backtrace);
        if (debugSources) {
          usedSources += " * " + src + "\n";
        }
      }
    }
    if (!usedSources.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used sources for target ") + tgt->GetName() + ":\n" +
          usedSources,
        entry.Backtrace);
    }
  }
  return contextDependent;
}
}

std::vector<BT<std::string>> cmGeneratorTarget::GetSourceFilePaths(
  std::string const& config) const
{
  std::vector<BT<std::string>> files;

  if (!this->LocalGenerator->GetGlobalGenerator()->GetConfigureDoneCMP0026()) {
    // At configure-time, this method can be called as part of getting the
    // LOCATION property or to export() a file to be include()d.  However
    // there is no cmGeneratorTarget at configure-time, so search the SOURCES
    // for TARGET_OBJECTS instead for backwards compatibility with OLD
    // behavior of CMP0024 and CMP0026 only.

    cmBTStringRange sourceEntries = this->Target->GetSourceEntries();
    for (auto const& entry : sourceEntries) {
      cmList items{ entry.Value };
      for (auto const& item : items) {
        if (cmHasLiteralPrefix(item, "$<TARGET_OBJECTS:") &&
            item.back() == '>') {
          continue;
        }
        files.emplace_back(item);
      }
    }
    return files;
  }

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugSources =
    !this->DebugSourcesDone && cm::contains(debugProperties, "SOURCES");

  if (this->LocalGenerator->GetGlobalGenerator()->GetConfigureDoneCMP0026()) {
    this->DebugSourcesDone = true;
  }

  cmGeneratorExpressionDAGChecker dagChecker(this, "SOURCES", nullptr, nullptr,
                                             this->LocalGenerator, config);

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, std::string(), &dagChecker, this->SourceEntries);

  std::unordered_set<std::string> uniqueSrcs;
  bool contextDependentDirectSources =
    processSources(this, entries, files, uniqueSrcs, debugSources);

  // Collect INTERFACE_SOURCES of all direct link-dependencies.
  EvaluatedTargetPropertyEntries linkInterfaceSourcesEntries;
  AddInterfaceEntries(this, config, "INTERFACE_SOURCES", std::string(),
                      &dagChecker, linkInterfaceSourcesEntries,
                      IncludeRuntimeInterface::No, UseTo::Compile);
  bool contextDependentInterfaceSources = processSources(
    this, linkInterfaceSourcesEntries, files, uniqueSrcs, debugSources);

  // Collect TARGET_OBJECTS of direct object link-dependencies.
  bool contextDependentObjects = false;
  if (this->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    EvaluatedTargetPropertyEntries linkObjectsEntries;
    AddObjectEntries(this, config, &dagChecker, linkObjectsEntries);
    contextDependentObjects = processSources(this, linkObjectsEntries, files,
                                             uniqueSrcs, debugSources);
    // Note that for imported targets or multi-config generators supporting
    // cross-config builds the paths to the object files must be per-config,
    // so contextDependentObjects will be true here even if object libraries
    // are specified without per-config generator expressions.
  }

  // Collect this target's file sets.
  EvaluatedTargetPropertyEntries fileSetEntries;
  AddFileSetEntries(this, config, &dagChecker, fileSetEntries);
  bool contextDependentFileSets =
    processSources(this, fileSetEntries, files, uniqueSrcs, debugSources);

  // Determine if sources are context-dependent or not.
  if (!contextDependentDirectSources && !contextDependentInterfaceSources &&
      !contextDependentObjects && !contextDependentFileSets) {
    this->SourcesAreContextDependent = Tribool::False;
  } else {
    this->SourcesAreContextDependent = Tribool::True;
  }

  return files;
}

void cmGeneratorTarget::GetSourceFiles(std::vector<cmSourceFile*>& files,
                                       const std::string& config) const
{
  std::vector<BT<cmSourceFile*>> tmp = this->GetSourceFiles(config);
  files.reserve(tmp.size());
  for (BT<cmSourceFile*>& v : tmp) {
    files.push_back(v.Value);
  }
}

std::vector<BT<cmSourceFile*>> cmGeneratorTarget::GetSourceFiles(
  std::string const& config) const
{
  std::vector<BT<cmSourceFile*>> files;
  if (!this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    // Since we are still configuring not all sources may exist yet,
    // so we need to avoid full source classification because that
    // requires the absolute paths to all sources to be determined.
    // Since this is only for compatibility with old policies that
    // projects should not depend on anymore, just compute the files
    // without memoizing them.
    std::vector<BT<std::string>> srcs = this->GetSourceFilePaths(config);
    std::set<cmSourceFile*> emitted;
    for (BT<std::string> const& s : srcs) {
      cmSourceFile* sf = this->Makefile->GetOrCreateSource(s.Value);
      if (emitted.insert(sf).second) {
        files.emplace_back(sf, s.Backtrace);
      }
    }
    return files;
  }

  KindedSources const& kinded = this->GetKindedSources(config);
  files.reserve(kinded.Sources.size());
  for (SourceAndKind const& si : kinded.Sources) {
    files.push_back(si.Source);
  }
  return files;
}

void cmGeneratorTarget::GetSourceFilesWithoutObjectLibraries(
  std::vector<cmSourceFile*>& files, const std::string& config) const
{
  std::vector<BT<cmSourceFile*>> tmp =
    this->GetSourceFilesWithoutObjectLibraries(config);
  files.reserve(tmp.size());
  for (BT<cmSourceFile*>& v : tmp) {
    files.push_back(v.Value);
  }
}

std::vector<BT<cmSourceFile*>>
cmGeneratorTarget::GetSourceFilesWithoutObjectLibraries(
  std::string const& config) const
{
  std::vector<BT<cmSourceFile*>> files;
  KindedSources const& kinded = this->GetKindedSources(config);
  files.reserve(kinded.Sources.size());
  for (SourceAndKind const& si : kinded.Sources) {
    if (si.Source.Value->GetObjectLibrary().empty()) {
      files.push_back(si.Source);
    }
  }
  return files;
}

cmGeneratorTarget::KindedSources const& cmGeneratorTarget::GetKindedSources(
  std::string const& config) const
{
  // If we already processed one configuration and found no dependency
  // on configuration then always use the one result.
  if (this->SourcesAreContextDependent == Tribool::False) {
    return this->KindedSourcesMap.begin()->second;
  }

  // Lookup any existing link implementation for this configuration.
  std::string const key = cmSystemTools::UpperCase(config);
  auto it = this->KindedSourcesMap.find(key);
  if (it != this->KindedSourcesMap.end()) {
    if (!it->second.Initialized) {
      std::ostringstream e;
      e << "The SOURCES of \"" << this->GetName()
        << "\" use a generator expression that depends on the "
           "SOURCES themselves.";
      this->GlobalGenerator->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR, e.str(), this->GetBacktrace());
      static KindedSources empty;
      return empty;
    }
    return it->second;
  }

  // Add an entry to the map for this configuration.
  KindedSources& files = this->KindedSourcesMap[key];
  this->ComputeKindedSources(files, config);
  files.Initialized = true;
  return files;
}

void cmGeneratorTarget::ComputeKindedSources(KindedSources& files,
                                             std::string const& config) const
{
  // Get the source file paths by string.
  std::vector<BT<std::string>> srcs = this->GetSourceFilePaths(config);

  cmsys::RegularExpression header_regex(CM_HEADER_REGEX);
  std::vector<cmSourceFile*> badObjLib;

  std::set<cmSourceFile*> emitted;
  for (BT<std::string> const& s : srcs) {
    // Create each source at most once.
    cmSourceFile* sf = this->Makefile->GetOrCreateSource(s.Value);
    if (!emitted.insert(sf).second) {
      continue;
    }

    // Compute the kind (classification) of this source file.
    SourceKind kind;
    std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
    cmFileSet const* fs = this->GetFileSetForSource(config, sf);
    if (sf->GetCustomCommand()) {
      kind = SourceKindCustomCommand;
    } else if (!this->Target->IsNormal() && !this->Target->IsImported() &&
               fs && (fs->GetType() == "CXX_MODULES"_s)) {
      kind = SourceKindCxxModuleSource;
    } else if (this->Target->GetType() == cmStateEnums::UTILITY ||
               this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY
               // XXX(clang-tidy): https://bugs.llvm.org/show_bug.cgi?id=44165
               // NOLINTNEXTLINE(bugprone-branch-clone)
    ) {
      kind = SourceKindExtra;
    } else if (this->IsSourceFilePartOfUnityBatch(sf->ResolveFullPath())) {
      kind = SourceKindUnityBatched;
      // XXX(clang-tidy): https://bugs.llvm.org/show_bug.cgi?id=44165
      // NOLINTNEXTLINE(bugprone-branch-clone)
    } else if (sf->GetPropertyAsBool("HEADER_FILE_ONLY")) {
      kind = SourceKindHeader;
    } else if (sf->GetPropertyAsBool("EXTERNAL_OBJECT")) {
      kind = SourceKindExternalObject;
    } else if (!sf->GetOrDetermineLanguage().empty()) {
      kind = SourceKindObjectSource;
    } else if (ext == "def") {
      kind = SourceKindModuleDefinition;
      if (this->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        badObjLib.push_back(sf);
      }
    } else if (ext == "idl") {
      kind = SourceKindIDL;
      if (this->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        badObjLib.push_back(sf);
      }
    } else if (ext == "resx") {
      kind = SourceKindResx;
    } else if (ext == "appxmanifest") {
      kind = SourceKindAppManifest;
    } else if (ext == "manifest") {
      if (sf->GetPropertyAsBool("VS_DEPLOYMENT_CONTENT")) {
        kind = SourceKindExtra;
      } else {
        kind = SourceKindManifest;
      }
    } else if (ext == "pfx") {
      kind = SourceKindCertificate;
    } else if (ext == "xaml") {
      kind = SourceKindXaml;
    } else if (header_regex.find(sf->ResolveFullPath())) {
      kind = SourceKindHeader;
    } else {
      kind = SourceKindExtra;
    }

    // Save this classified source file in the result vector.
    files.Sources.push_back({ BT<cmSourceFile*>(sf, s.Backtrace), kind });
  }

  if (!badObjLib.empty()) {
    std::ostringstream e;
    e << "OBJECT library \"" << this->GetName() << "\" contains:\n";
    for (cmSourceFile* i : badObjLib) {
      e << "  " << i->GetLocation().GetName() << "\n";
    }
    e << "but may contain only sources that compile, header files, and "
         "other files that would not affect linking of a normal library.";
    this->GlobalGenerator->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR, e.str(), this->GetBacktrace());
  }
}

std::vector<cmGeneratorTarget::AllConfigSource> const&
cmGeneratorTarget::GetAllConfigSources() const
{
  if (this->AllConfigSources.empty()) {
    this->ComputeAllConfigSources();
  }
  return this->AllConfigSources;
}

void cmGeneratorTarget::ComputeAllConfigSources() const
{
  std::vector<std::string> configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  std::map<cmSourceFile const*, size_t> index;

  for (size_t ci = 0; ci < configs.size(); ++ci) {
    KindedSources const& sources = this->GetKindedSources(configs[ci]);
    for (SourceAndKind const& src : sources.Sources) {
      auto mi = index.find(src.Source.Value);
      if (mi == index.end()) {
        AllConfigSource acs;
        acs.Source = src.Source.Value;
        acs.Kind = src.Kind;
        this->AllConfigSources.push_back(std::move(acs));
        std::map<cmSourceFile const*, size_t>::value_type entry(
          src.Source.Value, this->AllConfigSources.size() - 1);
        mi = index.insert(entry).first;
      }
      this->AllConfigSources[mi->second].Configs.push_back(ci);
    }
  }
}

std::vector<cmGeneratorTarget::AllConfigSource>
cmGeneratorTarget::GetAllConfigSources(SourceKind kind) const
{
  std::vector<AllConfigSource> result;
  for (AllConfigSource const& source : this->GetAllConfigSources()) {
    if (source.Kind == kind) {
      result.push_back(source);
    }
  }
  return result;
}

std::set<std::string> cmGeneratorTarget::GetAllConfigCompileLanguages() const
{
  std::set<std::string> languages;
  std::vector<AllConfigSource> const& sources = this->GetAllConfigSources();
  for (AllConfigSource const& si : sources) {
    std::string const& lang = si.Source->GetOrDetermineLanguage();
    if (!lang.empty()) {
      languages.emplace(lang);
    }
  }
  return languages;
}
