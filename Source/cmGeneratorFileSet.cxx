/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorFileSet.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>

#include "cmFileSet.h"
#include "cmGenExContext.h"
#include "cmGeneratorExpression.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

class cmLinkItem;

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

//
// Class cmGeneratorFileSet
//
cmGeneratorFileSet::cmGeneratorFileSet(cmFileSet const* fileSet)
  : FileSet(fileSet)
{
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
  if (this->CompiledFileEntries.empty() &&
      !this->FileSet->GetFileEntries().empty()) {
    for (auto const& entry : this->FileSet->GetFileEntries()) {
      for (auto const& ex : cmList{ entry.Value }) {
        cmGeneratorExpression ge(
          *this->FileSet->GetMakefile()->GetCMakeInstance(), entry.Backtrace);
        auto cge = ge.Parse(ex);
        this->CompiledFileEntries.push_back(std::move(cge));
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
