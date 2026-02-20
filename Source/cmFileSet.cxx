/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmFileSet.h"

#include <string>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"

namespace Metadata = cm::FileSetMetadata;

cmFileSet::cmFileSet(cmMakefile* makefile, std::string name, std::string type,
                     Metadata::Visibility visibility)
  : Makefile(makefile)
  , Name(std::move(name))
  , Type(std::move(type))
  , Visibility(visibility)
{
}

void cmFileSet::CopyEntries(cmFileSet const* fs)
{
  cm::append(this->DirectoryEntries, fs->DirectoryEntries);
  cm::append(this->FileEntries, fs->FileEntries);
}

void cmFileSet::ClearDirectoryEntries()
{
  this->DirectoryEntries.clear();
}

void cmFileSet::AddDirectoryEntry(BT<std::string> directories)
{
  this->DirectoryEntries.push_back(std::move(directories));
}

void cmFileSet::ClearFileEntries()
{
  this->FileEntries.clear();
}

void cmFileSet::AddFileEntry(BT<std::string> files)
{
  this->FileEntries.push_back(std::move(files));
}

std::string const cmFileSet::propCOMPILE_DEFINITIONS = "COMPILE_DEFINITIONS";
std::string const cmFileSet::propCOMPILE_OPTIONS = "COMPILE_OPTIONS";
std::string const cmFileSet::propINCLUDE_DIRECTORIES = "INCLUDE_DIRECTORIES";

void cmFileSet::SetProperty(std::string const& prop, cmValue value)
{
  if (prop == propINCLUDE_DIRECTORIES) {
    this->IncludeDirectories.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_OPTIONS) {
    this->CompileOptions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_DEFINITIONS) {
    this->CompileDefinitions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.SetProperty(prop, value);
  }
}

void cmFileSet::AppendProperty(std::string const& prop,
                               std::string const& value, bool asString)
{
  if (prop == propINCLUDE_DIRECTORIES) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_OPTIONS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_DEFINITIONS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.AppendProperty(prop, value, asString);
  }
}

cmValue cmFileSet::GetProperty(std::string const& prop) const
{
  // Check for the properties with backtraces.
  if (prop == propINCLUDE_DIRECTORIES) {
    if (this->IncludeDirectories.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->IncludeDirectories);
    return cmValue(output);
  }

  if (prop == propCOMPILE_OPTIONS) {
    if (this->CompileOptions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->CompileOptions);
    return cmValue(output);
  }

  if (prop == propCOMPILE_DEFINITIONS) {
    if (this->CompileDefinitions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->CompileDefinitions);
    return cmValue(output);
  }

  return this->Properties.GetPropertyValue(prop);
}
