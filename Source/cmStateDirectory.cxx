/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmStateDirectory.h"

#include <algorithm>
#include <cassert>
#include <vector>

#include <cm/iterator>
#include <cmext/algorithm>

#include "cmAlgorithms.h"
#include "cmProperty.h"
#include "cmPropertyMap.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStatePrivate.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

static std::string const kBINARY_DIR = "BINARY_DIR";
static std::string const kBUILDSYSTEM_TARGETS = "BUILDSYSTEM_TARGETS";
static std::string const kSOURCE_DIR = "SOURCE_DIR";
static std::string const kSUBDIRECTORIES = "SUBDIRECTORIES";

void cmStateDirectory::ComputeRelativePathTopSource()
{
  // Relative path conversion inside the source tree is not used to
  // construct relative paths passed to build tools so it is safe to use
  // even when the source is a network path.

  cmStateSnapshot snapshot = this->Snapshot_;
  std::vector<cmStateSnapshot> snapshots;
  snapshots.push_back(snapshot);
  while (true) {
    snapshot = snapshot.GetBuildsystemDirectoryParent();
    if (snapshot.IsValid()) {
      snapshots.push_back(snapshot);
    } else {
      break;
    }
  }

  std::string result = snapshots.front().GetDirectory().GetCurrentSource();

  for (cmStateSnapshot const& snp : cmMakeRange(snapshots).advance(1)) {
    std::string currentSource = snp.GetDirectory().GetCurrentSource();
    if (cmSystemTools::IsSubDirectory(result, currentSource)) {
      result = currentSource;
    }
  }
  this->DirectoryState->RelativePathTopSource = result;
}

void cmStateDirectory::ComputeRelativePathTopBinary()
{
  cmStateSnapshot snapshot = this->Snapshot_;
  std::vector<cmStateSnapshot> snapshots;
  snapshots.push_back(snapshot);
  while (true) {
    snapshot = snapshot.GetBuildsystemDirectoryParent();
    if (snapshot.IsValid()) {
      snapshots.push_back(snapshot);
    } else {
      break;
    }
  }

  std::string result = snapshots.front().GetDirectory().GetCurrentBinary();

  for (cmStateSnapshot const& snp : cmMakeRange(snapshots).advance(1)) {
    std::string currentBinary = snp.GetDirectory().GetCurrentBinary();
    if (cmSystemTools::IsSubDirectory(result, currentBinary)) {
      result = currentBinary;
    }
  }

  // The current working directory on Windows cannot be a network
  // path.  Therefore relative paths cannot work when the binary tree
  // is a network path.
  if (result.size() < 2 || result.substr(0, 2) != "//") {
    this->DirectoryState->RelativePathTopBinary = result;
  } else {
    this->DirectoryState->RelativePathTopBinary.clear();
  }
}

std::string const& cmStateDirectory::GetCurrentSource() const
{
  return this->DirectoryState->Location;
}

void cmStateDirectory::SetCurrentSource(std::string const& dir)
{
  std::string& loc = this->DirectoryState->Location;
  loc = dir;
  cmSystemTools::ConvertToUnixSlashes(loc);
  loc = cmSystemTools::CollapseFullPath(loc);

  this->ComputeRelativePathTopSource();

  this->Snapshot_.SetDefinition("CMAKE_CURRENT_SOURCE_DIR", loc);
}

std::string const& cmStateDirectory::GetCurrentBinary() const
{
  return this->DirectoryState->OutputLocation;
}

void cmStateDirectory::SetCurrentBinary(std::string const& dir)
{
  std::string& loc = this->DirectoryState->OutputLocation;
  loc = dir;
  cmSystemTools::ConvertToUnixSlashes(loc);
  loc = cmSystemTools::CollapseFullPath(loc);

  this->ComputeRelativePathTopBinary();

  this->Snapshot_.SetDefinition("CMAKE_CURRENT_BINARY_DIR", loc);
}

std::string const& cmStateDirectory::GetRelativePathTopSource() const
{
  return this->DirectoryState->RelativePathTopSource;
}

std::string const& cmStateDirectory::GetRelativePathTopBinary() const
{
  return this->DirectoryState->RelativePathTopBinary;
}

void cmStateDirectory::SetRelativePathTopSource(const char* dir)
{
  this->DirectoryState->RelativePathTopSource = dir;
}

void cmStateDirectory::SetRelativePathTopBinary(const char* dir)
{
  this->DirectoryState->RelativePathTopBinary = dir;
}

bool cmStateDirectory::ContainsBoth(std::string const& local_path,
                                    std::string const& remote_path) const
{
  auto PathEqOrSubDir = [](std::string const& a, std::string const& b) {
    return (cmSystemTools::ComparePath(a, b) ||
            cmSystemTools::IsSubDirectory(a, b));
  };

  bool bothInBinary =
    PathEqOrSubDir(local_path, this->GetRelativePathTopBinary()) &&
    PathEqOrSubDir(remote_path, this->GetRelativePathTopBinary());

  bool bothInSource =
    PathEqOrSubDir(local_path, this->GetRelativePathTopSource()) &&
    PathEqOrSubDir(remote_path, this->GetRelativePathTopSource());

  return bothInBinary || bothInSource;
}

std::string cmStateDirectory::ConvertToRelPathIfNotContained(
  std::string const& local_path, std::string const& remote_path) const
{
  if (!this->ContainsBoth(local_path, remote_path)) {
    return remote_path;
  }
  return cmSystemTools::ForceToRelativePath(local_path, remote_path);
}

cmStateDirectory::cmStateDirectory(
  cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator iter,
  const cmStateSnapshot& snapshot)
  : DirectoryState(iter)
  , Snapshot_(snapshot)
{
}

template <typename T, typename U>
cmStringRange GetPropertyContent(T const& content, U contentEndPosition)
{
  auto end = content.begin() + contentEndPosition;

  auto rbegin = cm::make_reverse_iterator(end);
  rbegin = std::find(rbegin, content.rend(), cmPropertySentinal);

  return cmMakeRange(rbegin.base(), end);
}

template <typename T, typename U, typename V>
cmBacktraceRange GetPropertyBacktraces(T const& content, U const& backtraces,
                                       V contentEndPosition)
{
  auto entryEnd = content.begin() + contentEndPosition;

  auto rbegin = cm::make_reverse_iterator(entryEnd);
  rbegin = std::find(rbegin, content.rend(), cmPropertySentinal);

  auto it = backtraces.begin() + std::distance(content.begin(), rbegin.base());

  auto end = backtraces.end();
  return cmMakeRange(it, end);
}

template <typename T, typename U, typename V>
void AppendEntry(T& content, U& backtraces, V& endContentPosition,
                 const std::string& value, const cmListFileBacktrace& lfbt)
{
  if (value.empty()) {
    return;
  }

  assert(endContentPosition == content.size());

  content.push_back(value);
  backtraces.push_back(lfbt);

  endContentPosition = content.size();
}

template <typename T, typename U, typename V>
void SetContent(T& content, U& backtraces, V& endContentPosition,
                const std::string& vec, const cmListFileBacktrace& lfbt)
{
  assert(endContentPosition == content.size());

  content.resize(content.size() + 2);
  backtraces.resize(backtraces.size() + 2);

  content.back() = vec;
  backtraces.back() = lfbt;

  endContentPosition = content.size();
}

template <typename T, typename U, typename V>
void ClearContent(T& content, U& backtraces, V& endContentPosition)
{
  assert(endContentPosition == content.size());

  content.resize(content.size() + 1);
  backtraces.resize(backtraces.size() + 1);

  endContentPosition = content.size();
}

cmStringRange cmStateDirectory::GetIncludeDirectoriesEntries() const
{
  return GetPropertyContent(
    this->DirectoryState->IncludeDirectories,
    this->Snapshot_.Position->IncludeDirectoryPosition);
}

cmBacktraceRange cmStateDirectory::GetIncludeDirectoriesEntryBacktraces() const
{
  return GetPropertyBacktraces(
    this->DirectoryState->IncludeDirectories,
    this->DirectoryState->IncludeDirectoryBacktraces,
    this->Snapshot_.Position->IncludeDirectoryPosition);
}

void cmStateDirectory::AppendIncludeDirectoriesEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->IncludeDirectories,
              this->DirectoryState->IncludeDirectoryBacktraces,
              this->Snapshot_.Position->IncludeDirectoryPosition, vec, lfbt);
}

void cmStateDirectory::PrependIncludeDirectoriesEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  auto entryEnd = this->DirectoryState->IncludeDirectories.begin() +
    this->Snapshot_.Position->IncludeDirectoryPosition;

  auto rend = this->DirectoryState->IncludeDirectories.rend();
  auto rbegin = cm::make_reverse_iterator(entryEnd);
  rbegin = std::find(rbegin, rend, cmPropertySentinal);

  auto entryIt = rbegin.base();
  auto entryBegin = this->DirectoryState->IncludeDirectories.begin();

  auto btIt = this->DirectoryState->IncludeDirectoryBacktraces.begin() +
    std::distance(entryBegin, entryIt);

  this->DirectoryState->IncludeDirectories.insert(entryIt, vec);
  this->DirectoryState->IncludeDirectoryBacktraces.insert(btIt, lfbt);

  this->Snapshot_.Position->IncludeDirectoryPosition =
    this->DirectoryState->IncludeDirectories.size();
}

void cmStateDirectory::SetIncludeDirectories(const std::string& vec,
                                             const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->IncludeDirectories,
             this->DirectoryState->IncludeDirectoryBacktraces,
             this->Snapshot_.Position->IncludeDirectoryPosition, vec, lfbt);
}

void cmStateDirectory::ClearIncludeDirectories()
{
  ClearContent(this->DirectoryState->IncludeDirectories,
               this->DirectoryState->IncludeDirectoryBacktraces,
               this->Snapshot_.Position->IncludeDirectoryPosition);
}

cmStringRange cmStateDirectory::GetCompileDefinitionsEntries() const
{
  return GetPropertyContent(
    this->DirectoryState->CompileDefinitions,
    this->Snapshot_.Position->CompileDefinitionsPosition);
}

cmBacktraceRange cmStateDirectory::GetCompileDefinitionsEntryBacktraces() const
{
  return GetPropertyBacktraces(
    this->DirectoryState->CompileDefinitions,
    this->DirectoryState->CompileDefinitionsBacktraces,
    this->Snapshot_.Position->CompileDefinitionsPosition);
}

void cmStateDirectory::AppendCompileDefinitionsEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->CompileDefinitions,
              this->DirectoryState->CompileDefinitionsBacktraces,
              this->Snapshot_.Position->CompileDefinitionsPosition, vec, lfbt);
}

void cmStateDirectory::SetCompileDefinitions(const std::string& vec,
                                             const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->CompileDefinitions,
             this->DirectoryState->CompileDefinitionsBacktraces,
             this->Snapshot_.Position->CompileDefinitionsPosition, vec, lfbt);
}

void cmStateDirectory::ClearCompileDefinitions()
{
  ClearContent(this->DirectoryState->CompileDefinitions,
               this->DirectoryState->CompileDefinitionsBacktraces,
               this->Snapshot_.Position->CompileDefinitionsPosition);
}

cmStringRange cmStateDirectory::GetCompileOptionsEntries() const
{
  return GetPropertyContent(this->DirectoryState->CompileOptions,
                            this->Snapshot_.Position->CompileOptionsPosition);
}

cmBacktraceRange cmStateDirectory::GetCompileOptionsEntryBacktraces() const
{
  return GetPropertyBacktraces(
    this->DirectoryState->CompileOptions,
    this->DirectoryState->CompileOptionsBacktraces,
    this->Snapshot_.Position->CompileOptionsPosition);
}

void cmStateDirectory::AppendCompileOptionsEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->CompileOptions,
              this->DirectoryState->CompileOptionsBacktraces,
              this->Snapshot_.Position->CompileOptionsPosition, vec, lfbt);
}

void cmStateDirectory::SetCompileOptions(const std::string& vec,
                                         const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->CompileOptions,
             this->DirectoryState->CompileOptionsBacktraces,
             this->Snapshot_.Position->CompileOptionsPosition, vec, lfbt);
}

void cmStateDirectory::ClearCompileOptions()
{
  ClearContent(this->DirectoryState->CompileOptions,
               this->DirectoryState->CompileOptionsBacktraces,
               this->Snapshot_.Position->CompileOptionsPosition);
}

cmStringRange cmStateDirectory::GetLinkOptionsEntries() const
{
  return GetPropertyContent(this->DirectoryState->LinkOptions,
                            this->Snapshot_.Position->LinkOptionsPosition);
}

cmBacktraceRange cmStateDirectory::GetLinkOptionsEntryBacktraces() const
{
  return GetPropertyBacktraces(this->DirectoryState->LinkOptions,
                               this->DirectoryState->LinkOptionsBacktraces,
                               this->Snapshot_.Position->LinkOptionsPosition);
}

void cmStateDirectory::AppendLinkOptionsEntry(const std::string& vec,
                                              const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->LinkOptions,
              this->DirectoryState->LinkOptionsBacktraces,
              this->Snapshot_.Position->LinkOptionsPosition, vec, lfbt);
}

void cmStateDirectory::SetLinkOptions(const std::string& vec,
                                      const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->LinkOptions,
             this->DirectoryState->LinkOptionsBacktraces,
             this->Snapshot_.Position->LinkOptionsPosition, vec, lfbt);
}

void cmStateDirectory::ClearLinkOptions()
{
  ClearContent(this->DirectoryState->LinkOptions,
               this->DirectoryState->LinkOptionsBacktraces,
               this->Snapshot_.Position->LinkOptionsPosition);
}

cmStringRange cmStateDirectory::GetLinkDirectoriesEntries() const
{
  return GetPropertyContent(this->DirectoryState->LinkDirectories,
                            this->Snapshot_.Position->LinkDirectoriesPosition);
}

cmBacktraceRange cmStateDirectory::GetLinkDirectoriesEntryBacktraces() const
{
  return GetPropertyBacktraces(
    this->DirectoryState->LinkDirectories,
    this->DirectoryState->LinkDirectoriesBacktraces,
    this->Snapshot_.Position->LinkDirectoriesPosition);
}

void cmStateDirectory::AppendLinkDirectoriesEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->LinkDirectories,
              this->DirectoryState->LinkDirectoriesBacktraces,
              this->Snapshot_.Position->LinkDirectoriesPosition, vec, lfbt);
}
void cmStateDirectory::PrependLinkDirectoriesEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  auto entryEnd = this->DirectoryState->LinkDirectories.begin() +
    this->Snapshot_.Position->LinkDirectoriesPosition;

  auto rend = this->DirectoryState->LinkDirectories.rend();
  auto rbegin = cm::make_reverse_iterator(entryEnd);
  rbegin = std::find(rbegin, rend, cmPropertySentinal);

  auto entryIt = rbegin.base();
  auto entryBegin = this->DirectoryState->LinkDirectories.begin();

  auto btIt = this->DirectoryState->LinkDirectoriesBacktraces.begin() +
    std::distance(entryBegin, entryIt);

  this->DirectoryState->LinkDirectories.insert(entryIt, vec);
  this->DirectoryState->LinkDirectoriesBacktraces.insert(btIt, lfbt);

  this->Snapshot_.Position->LinkDirectoriesPosition =
    this->DirectoryState->LinkDirectories.size();
}

void cmStateDirectory::SetLinkDirectories(const std::string& vec,
                                          const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->LinkDirectories,
             this->DirectoryState->LinkDirectoriesBacktraces,
             this->Snapshot_.Position->LinkDirectoriesPosition, vec, lfbt);
}

void cmStateDirectory::ClearLinkDirectories()
{
  ClearContent(this->DirectoryState->LinkDirectories,
               this->DirectoryState->LinkDirectoriesBacktraces,
               this->Snapshot_.Position->LinkDirectoriesPosition);
}

void cmStateDirectory::SetProperty(const std::string& prop, const char* value,
                                   cmListFileBacktrace const& lfbt)
{
  if (prop == "INCLUDE_DIRECTORIES") {
    if (!value) {
      this->ClearIncludeDirectories();
      return;
    }
    this->SetIncludeDirectories(value, lfbt);
    return;
  }
  if (prop == "COMPILE_OPTIONS") {
    if (!value) {
      this->ClearCompileOptions();
      return;
    }
    this->SetCompileOptions(value, lfbt);
    return;
  }
  if (prop == "COMPILE_DEFINITIONS") {
    if (!value) {
      this->ClearCompileDefinitions();
      return;
    }
    this->SetCompileDefinitions(value, lfbt);
    return;
  }
  if (prop == "LINK_OPTIONS") {
    if (!value) {
      this->ClearLinkOptions();
      return;
    }
    this->SetLinkOptions(value, lfbt);
    return;
  }
  if (prop == "LINK_DIRECTORIES") {
    if (!value) {
      this->ClearLinkDirectories();
      return;
    }
    this->SetLinkDirectories(value, lfbt);
    return;
  }

  this->DirectoryState->Properties.SetProperty(prop, value);
}

void cmStateDirectory::AppendProperty(const std::string& prop,
                                      const std::string& value, bool asString,
                                      cmListFileBacktrace const& lfbt)
{
  if (prop == "INCLUDE_DIRECTORIES") {
    this->AppendIncludeDirectoriesEntry(value, lfbt);
    return;
  }
  if (prop == "COMPILE_OPTIONS") {
    this->AppendCompileOptionsEntry(value, lfbt);
    return;
  }
  if (prop == "COMPILE_DEFINITIONS") {
    this->AppendCompileDefinitionsEntry(value, lfbt);
    return;
  }
  if (prop == "LINK_OPTIONS") {
    this->AppendLinkOptionsEntry(value, lfbt);
    return;
  }
  if (prop == "LINK_DIRECTORIES") {
    this->AppendLinkDirectoriesEntry(value, lfbt);
    return;
  }

  this->DirectoryState->Properties.AppendProperty(prop, value, asString);
}

cmProp cmStateDirectory::GetProperty(const std::string& prop) const
{
  const bool chain =
    this->Snapshot_.State->IsPropertyChained(prop, cmProperty::DIRECTORY);
  return this->GetProperty(prop, chain);
}

cmProp cmStateDirectory::GetProperty(const std::string& prop, bool chain) const
{
  static std::string output;
  output.clear();
  if (prop == "PARENT_DIRECTORY") {
    cmStateSnapshot parent = this->Snapshot_.GetBuildsystemDirectoryParent();
    if (parent.IsValid()) {
      return &parent.GetDirectory().GetCurrentSource();
    }
    return &output;
  }
  if (prop == kBINARY_DIR) {
    output = this->GetCurrentBinary();
    return &output;
  }
  if (prop == kSOURCE_DIR) {
    output = this->GetCurrentSource();
    return &output;
  }
  if (prop == kSUBDIRECTORIES) {
    std::vector<std::string> child_dirs;
    std::vector<cmStateSnapshot> const& children =
      this->DirectoryState->Children;
    child_dirs.reserve(children.size());
    for (cmStateSnapshot const& ci : children) {
      child_dirs.push_back(ci.GetDirectory().GetCurrentSource());
    }
    output = cmJoin(child_dirs, ";");
    return &output;
  }
  if (prop == kBUILDSYSTEM_TARGETS) {
    output = cmJoin(this->DirectoryState->NormalTargetNames, ";");
    return &output;
  }

  if (prop == "LISTFILE_STACK") {
    std::vector<std::string> listFiles;
    cmStateSnapshot snp = this->Snapshot_;
    while (snp.IsValid()) {
      listFiles.push_back(snp.GetExecutionListFile());
      snp = snp.GetCallStackParent();
    }
    std::reverse(listFiles.begin(), listFiles.end());
    output = cmJoin(listFiles, ";");
    return &output;
  }
  if (prop == "CACHE_VARIABLES") {
    output = cmJoin(this->Snapshot_.State->GetCacheEntryKeys(), ";");
    return &output;
  }
  if (prop == "VARIABLES") {
    std::vector<std::string> res = this->Snapshot_.ClosureKeys();
    cm::append(res, this->Snapshot_.State->GetCacheEntryKeys());
    std::sort(res.begin(), res.end());
    output = cmJoin(res, ";");
    return &output;
  }
  if (prop == "INCLUDE_DIRECTORIES") {
    output = cmJoin(this->GetIncludeDirectoriesEntries(), ";");
    return &output;
  }
  if (prop == "COMPILE_OPTIONS") {
    output = cmJoin(this->GetCompileOptionsEntries(), ";");
    return &output;
  }
  if (prop == "COMPILE_DEFINITIONS") {
    output = cmJoin(this->GetCompileDefinitionsEntries(), ";");
    return &output;
  }
  if (prop == "LINK_OPTIONS") {
    output = cmJoin(this->GetLinkOptionsEntries(), ";");
    return &output;
  }
  if (prop == "LINK_DIRECTORIES") {
    output = cmJoin(this->GetLinkDirectoriesEntries(), ";");
    return &output;
  }

  cmProp retVal = this->DirectoryState->Properties.GetPropertyValue(prop);
  if (!retVal && chain) {
    cmStateSnapshot parentSnapshot =
      this->Snapshot_.GetBuildsystemDirectoryParent();
    if (parentSnapshot.IsValid()) {
      return parentSnapshot.GetDirectory().GetProperty(prop, chain);
    }
    return this->Snapshot_.State->GetGlobalProperty(prop);
  }

  return retVal;
}

bool cmStateDirectory::GetPropertyAsBool(const std::string& prop) const
{
  return cmIsOn(this->GetProperty(prop));
}

std::vector<std::string> cmStateDirectory::GetPropertyKeys() const
{
  return this->DirectoryState->Properties.GetKeys();
}

void cmStateDirectory::AddNormalTargetName(std::string const& name)
{
  this->DirectoryState->NormalTargetNames.push_back(name);
}
