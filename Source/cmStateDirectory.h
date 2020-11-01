/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmAlgorithms.h"
#include "cmLinkedTree.h"
#include "cmListFileCache.h"
#include "cmProperty.h"
#include "cmStatePrivate.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"

class cmStateDirectory
{
  cmStateDirectory(
    cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator iter,
    cmStateSnapshot const& snapshot);

public:
  std::string const& GetCurrentSource() const;
  void SetCurrentSource(std::string const& dir);
  std::string const& GetCurrentBinary() const;
  void SetCurrentBinary(std::string const& dir);

  std::string const& GetRelativePathTopSource() const;
  std::string const& GetRelativePathTopBinary() const;
  void SetRelativePathTopSource(const char* dir);
  void SetRelativePathTopBinary(const char* dir);

  bool ContainsBoth(std::string const& local_path,
                    std::string const& remote_path) const;

  std::string ConvertToRelPathIfNotContained(
    std::string const& local_path, std::string const& remote_path) const;

  cmStringRange GetIncludeDirectoriesEntries() const;
  cmBacktraceRange GetIncludeDirectoriesEntryBacktraces() const;
  void AppendIncludeDirectoriesEntry(std::string const& vec,
                                     cmListFileBacktrace const& lfbt);
  void PrependIncludeDirectoriesEntry(std::string const& vec,
                                      cmListFileBacktrace const& lfbt);
  void SetIncludeDirectories(std::string const& vec,
                             cmListFileBacktrace const& lfbt);
  void ClearIncludeDirectories();

  cmStringRange GetCompileDefinitionsEntries() const;
  cmBacktraceRange GetCompileDefinitionsEntryBacktraces() const;
  void AppendCompileDefinitionsEntry(std::string const& vec,
                                     cmListFileBacktrace const& lfbt);
  void SetCompileDefinitions(std::string const& vec,
                             cmListFileBacktrace const& lfbt);
  void ClearCompileDefinitions();

  cmStringRange GetCompileOptionsEntries() const;
  cmBacktraceRange GetCompileOptionsEntryBacktraces() const;
  void AppendCompileOptionsEntry(std::string const& vec,
                                 cmListFileBacktrace const& lfbt);
  void SetCompileOptions(std::string const& vec,
                         cmListFileBacktrace const& lfbt);
  void ClearCompileOptions();

  cmStringRange GetLinkOptionsEntries() const;
  cmBacktraceRange GetLinkOptionsEntryBacktraces() const;
  void AppendLinkOptionsEntry(std::string const& vec,
                              cmListFileBacktrace const& lfbt);
  void PrependLinkDirectoriesEntry(std::string const& vec,
                                   cmListFileBacktrace const& lfbt);
  void SetLinkOptions(std::string const& vec, cmListFileBacktrace const& lfbt);
  void ClearLinkOptions();

  cmStringRange GetLinkDirectoriesEntries() const;
  cmBacktraceRange GetLinkDirectoriesEntryBacktraces() const;
  void AppendLinkDirectoriesEntry(std::string const& vec,
                                  cmListFileBacktrace const& lfbt);
  void SetLinkDirectories(std::string const& vec,
                          cmListFileBacktrace const& lfbt);
  void ClearLinkDirectories();

  void SetProperty(const std::string& prop, const char* value,
                   cmListFileBacktrace const& lfbt);
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString, cmListFileBacktrace const& lfbt);
  cmProp GetProperty(const std::string& prop) const;
  cmProp GetProperty(const std::string& prop, bool chain) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  std::vector<std::string> GetPropertyKeys() const;

  void AddNormalTargetName(std::string const& name);

private:
  void ComputeRelativePathTopSource();
  void ComputeRelativePathTopBinary();

private:
  cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator
    DirectoryState;
  cmStateSnapshot Snapshot_;
  friend class cmStateSnapshot;
};
