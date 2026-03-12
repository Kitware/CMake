/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorFileSet.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmAlgorithms.h"
#include "cmEvaluatedTargetProperty.h"
#include "cmFileSet.h"
#include "cmGenExContext.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorTarget.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

class cmLinkItem;

namespace {
class FileSetPropertyEntry : public cm::TargetPropertyEntry
{
public:
  FileSetPropertyEntry(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> const& cge,
    cmGeneratorFileSet const* fileSet, cmLinkItem const& item = NoLinkItem)
    : cm::TargetPropertyEntry(item)
    , BaseDirs(std::move(dirs))
    , ContextSensitiveDirs(contextSensitiveDirs)
    , Cge(cge)
    , FileSet(fileSet)
  {
  }

  static std::unique_ptr<cm::TargetPropertyEntry> CreateFileSetEntry(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> const& cge,
    cmGeneratorFileSet const* fileSet, cmLinkItem const& item = NoLinkItem)
  {
    return cm::make_unique<FileSetPropertyEntry>(
      std::move(dirs), contextSensitiveDirs, cge, fileSet, item);
  }

  std::string const& Evaluate(
    cm::GenEx::Context const& context, cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    std::map<std::string, std::vector<std::string>> filesPerDir;
    this->FileSet->EvaluateFileEntry(this->BaseDirs, filesPerDir, this->Cge,
                                     context, headTarget, dagChecker);

    std::vector<std::string> files;
    for (auto const& it : filesPerDir) {
      files.insert(files.end(), it.second.begin(), it.second.end());
    }

    this->Value = cmList::to_string(files);
    return Value;
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->Cge->GetBacktrace();
  }

  std::string const& GetInput() const override
  {
    return this->Cge->GetInput();
  }

  bool GetHadContextSensitiveCondition() const override
  {
    return this->ContextSensitiveDirs ||
      this->Cge->GetHadContextSensitiveCondition();
  }

private:
  std::vector<std::string> const BaseDirs;
  bool const ContextSensitiveDirs;
  std::unique_ptr<cmCompiledGeneratorExpression> const& Cge;
  cmGeneratorFileSet const* FileSet;
  mutable std::string Value;
};

void CreatePropertyGeneratorExpressions(
  cmake& cmakeInstance, cmBTStringRange entries,
  std::vector<std::unique_ptr<cm::TargetPropertyEntry>>& items)
{
  for (auto const& entry : entries) {
    items.emplace_back(
      cm::TargetPropertyEntry::Create(cmakeInstance, entry, false));
  }
}

enum class OptionsParse
{
  None,
  Shell
};

std::vector<BT<std::string>> ProcessOptions(
  cm::EvaluatedTargetPropertyEntries const& entries,
  OptionsParse parse = OptionsParse::None)
{
  std::vector<BT<std::string>> options;
  std::unordered_set<std::string> uniqueOptions;

  for (cm::EvaluatedTargetPropertyEntry const& entry : entries.Entries) {
    for (std::string const& opt : entry.Values) {
      if (uniqueOptions.insert(opt).second) {
        if (parse == OptionsParse::Shell &&
            cmHasLiteralPrefix(opt, "SHELL:")) {
          std::vector<std::string> tmp;
          cmSystemTools::ParseUnixCommandLine(opt.c_str() + 6, tmp);
          for (std::string& o : tmp) {
            options.emplace_back(std::move(o), entry.Backtrace);
          }
        } else {
          options.emplace_back(opt, entry.Backtrace);
        }
      }
    }
  }
  return options;
}
std::vector<BT<std::string>> ProcessIncludes(
  cmGeneratorTarget const* target, std::string const& fileSetName,
  cm::string_view property, cm::EvaluatedTargetPropertyEntries& entries)
{
  std::vector<BT<std::string>> includes;
  std::unordered_set<std::string> uniqueIncludes;

  for (cm::EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    for (std::string& include : entry.Values) {
      if (!cmValue::IsOff(include)) {
        cmSystemTools::ConvertToUnixSlashes(include);
      }

      if (!cmSystemTools::FileIsFullPath(include)) {
        target->GetLocalGenerator()->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("File set \"", fileSetName, "\" from the target \"",
                   target->GetName(), "\" contains relative path in its ",
                   property, ":\n  \"", include, "\""));
        return includes;
      }

      if (uniqueIncludes.insert(include).second) {
        includes.emplace_back(include, entry.Backtrace);
      }
    }
  }
  return includes;
}
}

//
// Class cmGeneratorFileSet
//
cmGeneratorFileSet::cmGeneratorFileSet(cmGeneratorTarget const* target,
                                       cmFileSet const* fileSet)
  : Target(target)
  , FileSet(fileSet)
{
  auto& cmake = *target->GetLocalGenerator()->GetCMakeInstance();

  CreatePropertyGeneratorExpressions(cmake, fileSet->GetIncludeDirectories(),
                                     this->IncludeDirectories);
  CreatePropertyGeneratorExpressions(cmake,
                                     fileSet->GetInterfaceIncludeDirectories(),
                                     this->InterfaceIncludeDirectories);

  CreatePropertyGeneratorExpressions(cmake, fileSet->GetCompileOptions(),
                                     this->CompileOptions);
  CreatePropertyGeneratorExpressions(cmake,
                                     fileSet->GetInterfaceCompileOptions(),
                                     this->InterfaceCompileOptions);

  CreatePropertyGeneratorExpressions(cmake, fileSet->GetCompileDefinitions(),
                                     this->CompileDefinitions);
  CreatePropertyGeneratorExpressions(cmake,
                                     fileSet->GetInterfaceCompileDefinitions(),
                                     this->InterfaceCompileDefinitions);
}

std::string const& cmGeneratorFileSet::GetName() const
{
  return this->FileSet->GetName();
}
std::string const& cmGeneratorFileSet::GetType() const
{
  return this->FileSet->GetType();
}
cm::FileSetMetadata::Visibility cmGeneratorFileSet::GetVisibility() const
{
  return this->FileSet->GetVisibility();
}

bool cmGeneratorFileSet::IsForSelf() const
{
  return this->FileSet->IsForSelf();
}
bool cmGeneratorFileSet::IsForInterface() const
{
  return this->FileSet->IsForInterface();
}
bool cmGeneratorFileSet::CanBeIncluded() const
{
  return this->FileSet->CanBeIncluded();
}

cmValue cmGeneratorFileSet::GetProperty(std::string const& prop) const
{
  return this->FileSet->GetProperty(prop);
}

std::vector<BT<std::string>> cmGeneratorFileSet::GetIncludeDirectories(
  std::string const& config, std::string const& lang) const
{
  ConfigAndLanguage cacheKey(config, lang);
  {
    auto it = this->IncludeDirectoriesCache.find(cacheKey);
    if (it != this->IncludeDirectoriesCache.end()) {
      return it->second;
    }
  }

  cm::GenEx::Context context(this->Target->GetLocalGenerator(), config, lang);
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target, "INCLUDE_DIRECTORIES", nullptr, nullptr, context,
  };

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this->Target, context, &dagChecker,
                                      this->IncludeDirectories);
  auto includes = ProcessIncludes(this->Target, this->GetName(),
                                  "INCLUDE_DIRECTORIES"_s, entries);
  this->IncludeDirectoriesCache.emplace(cacheKey, includes);

  return includes;
}
std::vector<BT<std::string>>
cmGeneratorFileSet::GetInterfaceIncludeDirectories(
  std::string const& config, std::string const& lang) const
{
  ConfigAndLanguage cacheKey(config, lang);
  {
    auto it = this->InterfaceIncludeDirectoriesCache.find(cacheKey);
    if (it != this->InterfaceIncludeDirectoriesCache.end()) {
      return it->second;
    }
  }

  cm::GenEx::Context context(this->Target->GetLocalGenerator(), config, lang);
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target, "INCLUDE_DIRECTORIES", nullptr, nullptr, context,
  };

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this->Target, context, &dagChecker,
                                      this->InterfaceIncludeDirectories);
  auto includes = ProcessIncludes(this->Target, this->GetName(),
                                  "INTERFACE_INCLUDE_DIRECTORIES"_s, entries);
  this->InterfaceIncludeDirectoriesCache.emplace(cacheKey, includes);

  return includes;
}

std::vector<BT<std::string>> cmGeneratorFileSet::GetCompileOptions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->CompileOptionsCache.find(cacheKey);
    if (it != this->CompileOptionsCache.end()) {
      return it->second;
    }
  }

  cm::GenEx::Context context(this->Target->GetLocalGenerator(), config,
                             language);
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target, "COMPILE_OPTIONS", nullptr, nullptr, context,
  };

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this->Target, context, &dagChecker,
                                      this->CompileOptions);
  auto options = ProcessOptions(entries, OptionsParse::Shell);
  this->CompileOptionsCache.emplace(cacheKey, options);

  return options;
}
std::vector<BT<std::string>> cmGeneratorFileSet::GetInterfaceCompileOptions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->InterfaceCompileOptionsCache.find(cacheKey);
    if (it != this->InterfaceCompileOptionsCache.end()) {
      return it->second;
    }
  }

  cm::GenEx::Context context(this->Target->GetLocalGenerator(), config,
                             language);
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target, "COMPILE_OPTIONS", nullptr, nullptr, context,
  };

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this->Target, context, &dagChecker,
                                      this->InterfaceCompileOptions);
  auto options = ProcessOptions(entries, OptionsParse::Shell);
  this->InterfaceCompileOptionsCache.emplace(cacheKey, options);

  return options;
}

std::vector<BT<std::string>> cmGeneratorFileSet::GetCompileDefinitions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->CompileDefinitionsCache.find(cacheKey);
    if (it != this->CompileDefinitionsCache.end()) {
      return it->second;
    }
  }

  cm::GenEx::Context context(this->Target->GetLocalGenerator(), config,
                             language);
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target, "COMPILE_DEFINITIONS", nullptr, nullptr, context,
  };

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this->Target, context, &dagChecker,
                                      this->CompileDefinitions);
  auto defines = ProcessOptions(entries);
  this->CompileDefinitionsCache.emplace(cacheKey, defines);

  return defines;
}
std::vector<BT<std::string>>
cmGeneratorFileSet::GetInterfaceCompileDefinitions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->InterfaceCompileDefinitionsCache.find(cacheKey);
    if (it != this->InterfaceCompileDefinitionsCache.end()) {
      return it->second;
    }
  }

  cm::GenEx::Context context(this->Target->GetLocalGenerator(), config,
                             language);
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target, "COMPILE_DEFINITIONS", nullptr, nullptr, context,
  };

  cm::EvaluatedTargetPropertyEntries entries =
    cm::EvaluateTargetPropertyEntries(this->Target, context, &dagChecker,
                                      this->InterfaceCompileDefinitions);
  auto defines = ProcessOptions(entries);
  this->InterfaceCompileDefinitionsCache.emplace(cacheKey, defines);

  return defines;
}

std::vector<BT<std::string>> const& cmGeneratorFileSet::GetDirectoryEntries()
  const
{
  return this->FileSet->GetDirectoryEntries();
}

std::vector<BT<std::string>> const& cmGeneratorFileSet::GetFileEntries() const
{
  return this->FileSet->GetFileEntries();
}

std::vector<std::unique_ptr<cmCompiledGeneratorExpression>> const&
cmGeneratorFileSet::CompileFileEntries() const
{
  std::unordered_set<std::string> uniqueSrcs;

  if (this->CompiledFileEntries.empty() &&
      !this->FileSet->GetFileEntries().empty()) {
    for (auto const& entry : this->FileSet->GetFileEntries()) {
      for (auto const& ex : cmList{ entry.Value }) {
        if (uniqueSrcs.insert(ex).second) {
          cmGeneratorExpression ge(
            *this->FileSet->GetMakefile()->GetCMakeInstance(),
            entry.Backtrace);
          auto cge = ge.Parse(ex);
          this->CompiledFileEntries.push_back(std::move(cge));
        }
      }
    }
  }

  return this->CompiledFileEntries;
}

std::vector<std::unique_ptr<cmCompiledGeneratorExpression>> const&
cmGeneratorFileSet::CompileDirectoryEntries() const
{
  if (this->CompiledDirectoryEntries.empty() &&
      !this->FileSet->GetDirectoryEntries().empty()) {
    for (auto const& entry : this->FileSet->GetDirectoryEntries()) {
      for (auto const& ex : cmList{ entry.Value }) {
        cmGeneratorExpression ge(
          *this->FileSet->GetMakefile()->GetCMakeInstance(), entry.Backtrace);
        auto cge = ge.Parse(ex);
        this->CompiledDirectoryEntries.push_back(std::move(cge));
      }
    }
  }

  return this->CompiledDirectoryEntries;
}

std::vector<std::string> cmGeneratorFileSet::EvaluateDirectoryEntries(
  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>> const& cges,
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  struct DirCacheEntry
  {
    std::string collapsedDir;
    cm::optional<cmSystemTools::FileId> fileId;
  };

  std::unordered_map<std::string, DirCacheEntry> dirCache;
  std::vector<std::string> result;
  for (auto const& cge : cges) {
    auto entry = cge->Evaluate(context, dagChecker, target);
    cmList dirs{ entry };
    for (std::string dir : dirs) {
      if (!cmSystemTools::FileIsFullPath(dir)) {
        dir = cmStrCat(context.LG->GetCurrentSourceDirectory(), '/', dir);
      }

      auto dirCacheResult = dirCache.emplace(dir, DirCacheEntry());
      auto& dirCacheEntry = dirCacheResult.first->second;
      auto const isNewCacheEntry = dirCacheResult.second;

      if (isNewCacheEntry) {
        cmSystemTools::FileId fileId;
        auto isFileIdValid = cmSystemTools::GetFileId(dir, fileId);
        dirCacheEntry.collapsedDir = cmSystemTools::CollapseFullPath(dir);
        dirCacheEntry.fileId =
          isFileIdValid ? cm::optional<decltype(fileId)>(fileId) : cm::nullopt;
      }

      for (auto const& priorDir : result) {
        auto priorDirCacheEntry = dirCache.at(priorDir);
        bool sameFile = dirCacheEntry.fileId.has_value() &&
          priorDirCacheEntry.fileId.has_value() &&
          (*dirCacheEntry.fileId == *priorDirCacheEntry.fileId);
        if (!sameFile &&
            (cmSystemTools::IsSubDirectory(dirCacheEntry.collapsedDir,
                                           priorDirCacheEntry.collapsedDir) ||
             cmSystemTools::IsSubDirectory(priorDirCacheEntry.collapsedDir,
                                           dirCacheEntry.collapsedDir))) {
          context.LG->GetCMakeInstance()->IssueMessage(
            MessageType::FATAL_ERROR,
            cmStrCat(
              "Base directories in file set cannot be subdirectories of each "
              "other:\n  ",
              priorDir, "\n  ", dir),
            cge->GetBacktrace());
          return {};
        }
      }
      result.push_back(dir);
    }
  }
  return result;
}

void cmGeneratorFileSet::EvaluateFileEntry(
  std::vector<std::string> const& dirs,
  std::map<std::string, std::vector<std::string>>& filesPerDir,
  std::unique_ptr<cmCompiledGeneratorExpression> const& cge,
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  auto files = cge->Evaluate(context, dagChecker, target);
  for (std::string file : cmList{ files }) {
    if (!cmSystemTools::FileIsFullPath(file)) {
      file = cmStrCat(context.LG->GetCurrentSourceDirectory(), '/', file);
    }
    auto collapsedFile = cmSystemTools::CollapseFullPath(file);
    bool found = false;
    std::string relDir;
    for (auto const& dir : dirs) {
      auto collapsedDir = cmSystemTools::CollapseFullPath(dir);
      if (cmSystemTools::IsSubDirectory(collapsedFile, collapsedDir)) {
        found = true;
        relDir = cmSystemTools::GetParentDirectory(
          cmSystemTools::RelativePath(collapsedDir, collapsedFile));
        break;
      }
    }
    if (!found) {
      std::ostringstream e;
      e << "File:\n  " << file
        << "\nmust be in one of the file set's base directories:";
      for (auto const& dir : dirs) {
        e << "\n  " << dir;
      }
      context.LG->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR, e.str(), cge->GetBacktrace());
      return;
    }

    filesPerDir[relDir].push_back(file);
  }
}

namespace {
bool EntryIsContextSensitive(
  std::unique_ptr<cmCompiledGeneratorExpression> const& cge)
{
  return cge->GetHadContextSensitiveCondition();
}
}

std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSet::GetSources(
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  std::vector<std::unique_ptr<TargetPropertyEntry>> entries;

  auto directories = this->GetDirectories(context, target, dagChecker);
  bool contextSensitive = directories.second;

  for (auto const& entry : this->CompileFileEntries()) {
    auto propEntry = FileSetPropertyEntry::CreateFileSetEntry(
      directories.first, contextSensitive, entry, this);
    entries.push_back(std::move(propEntry));
  }

  return entries;
}

std::pair<std::vector<std::string>, bool> cmGeneratorFileSet::GetDirectories(
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  auto const& directoryEntries = this->CompileDirectoryEntries();
  auto directories = this->EvaluateDirectoryEntries(directoryEntries, context,
                                                    target, dagChecker);
  bool contextSensitive = std::any_of(
    directoryEntries.begin(), directoryEntries.end(), EntryIsContextSensitive);

  return std::make_pair(std::move(directories), contextSensitive);
}

std::pair<std::map<std::string, std::vector<std::string>>, bool>
cmGeneratorFileSet::GetFiles(cm::GenEx::Context const& context,
                             cmGeneratorTarget const* target,
                             cmGeneratorExpressionDAGChecker* dagChecker) const
{
  auto directories = this->GetDirectories(context, target, dagChecker);

  auto const& fileEntries = this->CompileFileEntries();
  std::map<std::string, std::vector<std::string>> files;
  for (auto const& entry : fileEntries) {
    this->EvaluateFileEntry(directories.first, files, entry, context, target,
                            dagChecker);
  }
  bool contextSensitive = directories.second ||
    std::any_of(fileEntries.begin(), fileEntries.end(),
                EntryIsContextSensitive);

  return std::make_pair(std::move(files), contextSensitive);
}
