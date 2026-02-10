/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmList.h"
#include "cmListFileCache.h"
#include "cmPropertyMap.h"
#include "cmValue.h"

namespace cm {
namespace GenEx {
struct Context;
}
}

class cmCompiledGeneratorExpression;
struct cmGeneratorExpressionDAGChecker;
class cmGeneratorTarget;
class cmMakefile;

class cmFileSet
{
public:
  enum class Visibility
  {
    Private,
    Public,
    Interface,
  };
  static cm::static_string_view VisibilityToName(Visibility vis);
  static Visibility VisibilityFromName(cm::string_view name, cmMakefile* mf);

  static bool VisibilityIsForSelf(Visibility vis);
  static bool VisibilityIsForInterface(Visibility vis);

  static bool IsKnownType(std::string const& type);
  static bool TypeCanBeIncluded(std::string const& type);

  // Pre-defined FileSet types
  static cm::static_string_view const HEADERS;
  static cm::static_string_view const CXX_MODULES;

  static cmList const& GetKnownTypes();

  cmFileSet(cmMakefile* makefile, std::string name, std::string type,
            Visibility visibility);

  std::string const& GetName() const { return this->Name; }
  std::string const& GetType() const { return this->Type; }
  Visibility GetVisibility() const { return this->FSVisibility; }

  cmMakefile* GetMakefile() const { return this->Makefile; }

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

  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>
  CompileFileEntries() const;

  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>
  CompileDirectoryEntries() const;

  std::vector<std::string> EvaluateDirectoryEntries(
    std::vector<std::unique_ptr<cmCompiledGeneratorExpression>> const& cges,
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  void EvaluateFileEntry(
    std::vector<std::string> const& dirs,
    std::map<std::string, std::vector<std::string>>& filesPerDir,
    std::unique_ptr<cmCompiledGeneratorExpression> const& cge,
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  static bool IsValidName(std::string const& name);

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
  static cmList KnownTypes;

  cmMakefile* Makefile;
  std::string Name;
  std::string Type;
  Visibility FSVisibility;
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
