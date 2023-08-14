/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <vector>

#include "cmAlgorithms.h"
#include "cmLinkedTree.h"
#include "cmStatePrivate.h"
#include "cmStateSnapshot.h"
#include "cmValue.h"

class cmListFileBacktrace;
template <typename T>
class BT;

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

  cmBTStringRange GetIncludeDirectoriesEntries() const;
  void AppendIncludeDirectoriesEntry(BT<std::string> const& vec);
  void PrependIncludeDirectoriesEntry(BT<std::string> const& vec);
  void SetIncludeDirectories(BT<std::string> const& vec);
  void ClearIncludeDirectories();

  cmBTStringRange GetCompileDefinitionsEntries() const;
  void AppendCompileDefinitionsEntry(BT<std::string> const& vec);
  void SetCompileDefinitions(BT<std::string> const& vec);
  void ClearCompileDefinitions();

  cmBTStringRange GetCompileOptionsEntries() const;
  void AppendCompileOptionsEntry(BT<std::string> const& vec);
  void SetCompileOptions(BT<std::string> const& vec);
  void ClearCompileOptions();

  cmBTStringRange GetLinkOptionsEntries() const;
  void AppendLinkOptionsEntry(BT<std::string> const& vec);
  void PrependLinkDirectoriesEntry(BT<std::string> const& vec);
  void SetLinkOptions(BT<std::string> const& vec);
  void ClearLinkOptions();

  cmBTStringRange GetLinkDirectoriesEntries() const;
  void AppendLinkDirectoriesEntry(BT<std::string> const& vec);
  void SetLinkDirectories(BT<std::string> const& vecs);
  void ClearLinkDirectories();

  void SetProperty(const std::string& prop, cmValue value,
                   cmListFileBacktrace const& lfbt);
  void SetProperty(const std::string& prop, std::nullptr_t,
                   cmListFileBacktrace const& lfbt)
  {
    this->SetProperty(prop, cmValue{ nullptr }, lfbt);
  }
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString, cmListFileBacktrace const& lfbt);
  cmValue GetProperty(const std::string& prop) const;
  cmValue GetProperty(const std::string& prop, bool chain) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  std::vector<std::string> GetPropertyKeys() const;

  void AddNormalTargetName(std::string const& name);
  void AddImportedTargetName(std::string const& name);

private:
  template <typename ValueType>
  void StoreProperty(const std::string& prop, ValueType value,
                     cmListFileBacktrace const& lfbt);

  cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator
    DirectoryState;
  cmStateSnapshot Snapshot_;
  friend class cmStateSnapshot;
};
