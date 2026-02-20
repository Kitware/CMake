/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmFileSetMetadata.h"
#include "cmListFileCache.h"
#include "cmPropertyMap.h"
#include "cmValue.h"

class cmMakefile;

class cmFileSet
{
public:
  cmFileSet(cmMakefile* makefile, std::string name, std::string type,
            cm::FileSetMetadata::Visibility visibility);

  std::string const& GetName() const { return this->Name; }
  std::string const& GetType() const { return this->Type; }
  cm::FileSetMetadata::Visibility GetVisibility() const
  {
    return this->Visibility;
  }

  cmMakefile* GetMakefile() const { return this->Makefile; }

  bool IsForSelf() const
  {
    return cm::FileSetMetadata::VisibilityIsForSelf(this->GetVisibility());
  }
  bool IsForInterface() const
  {
    return cm::FileSetMetadata::VisibilityIsForInterface(
      this->GetVisibility());
  }
  bool CanBeIncluded() const
  {
    return this->Type == cm::FileSetMetadata::HEADERS;
  }

  void CopyEntries(cmFileSet const* fs);

  void ClearDirectoryEntries();
  void AddDirectoryEntry(BT<std::string> directories);
  std::vector<BT<std::string>> const& GetDirectoryEntries() const
  {
    return this->DirectoryEntries;
  }

  void ClearFileEntries();
  void AddFileEntry(BT<std::string> files);
  std::vector<BT<std::string>> const& GetFileEntries() const
  {
    return this->FileEntries;
  }

  //! Set/Get a property of this file set
  void SetProperty(std::string const& prop, cmValue value);
  void SetProperty(std::string const& prop, std::nullptr_t)
  {
    this->SetProperty(prop, cmValue{ nullptr });
  }
  void RemoveProperty(std::string const& prop)
  {
    this->SetProperty(prop, cmValue{ nullptr });
  }
  void SetProperty(std::string const& prop, std::string const& value)
  {
    this->SetProperty(prop, cmValue{ value });
  }
  void AppendProperty(std::string const& prop, std::string const& value,
                      bool asString = false);
  cmValue GetProperty(std::string const& prop) const;

private:
  cmMakefile* Makefile;
  std::string Name;
  std::string Type;
  cm::FileSetMetadata::Visibility Visibility;
  std::vector<BT<std::string>> DirectoryEntries;
  std::vector<BT<std::string>> FileEntries;
  cmPropertyMap Properties;
  std::vector<BT<std::string>> CompileOptions;
  std::vector<BT<std::string>> CompileDefinitions;
  std::vector<BT<std::string>> IncludeDirectories;

  static std::string const propCOMPILE_DEFINITIONS;
  static std::string const propCOMPILE_OPTIONS;
  static std::string const propINCLUDE_DIRECTORIES;
};
