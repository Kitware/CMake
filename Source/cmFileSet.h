/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmListFileCache.h"

class cmCompiledGeneratorExpression;
struct cmGeneratorExpressionDAGChecker;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmake;

enum class cmFileSetVisibility
{
  Private,
  Public,
  Interface,
};
cm::static_string_view cmFileSetVisibilityToName(cmFileSetVisibility vis);
cmFileSetVisibility cmFileSetVisibilityFromName(cm::string_view name,
                                                cmMakefile* mf);
bool cmFileSetVisibilityIsForSelf(cmFileSetVisibility vis);
bool cmFileSetVisibilityIsForInterface(cmFileSetVisibility vis);

bool cmFileSetTypeCanBeIncluded(std::string const& type);

class cmFileSet
{
public:
  cmFileSet(cmake& cmakeInstance, std::string name, std::string type,
            cmFileSetVisibility visibility);

  std::string const& GetName() const { return this->Name; }
  std::string const& GetType() const { return this->Type; }
  cmFileSetVisibility GetVisibility() const { return this->Visibility; }

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
    cmLocalGenerator* lg, std::string const& config,
    cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  void EvaluateFileEntry(
    std::vector<std::string> const& dirs,
    std::map<std::string, std::vector<std::string>>& filesPerDir,
    std::unique_ptr<cmCompiledGeneratorExpression> const& cge,
    cmLocalGenerator* lg, std::string const& config,
    cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  static bool IsValidName(std::string const& name);

private:
  cmake& CMakeInstance;
  std::string Name;
  std::string Type;
  cmFileSetVisibility Visibility;
  std::vector<BT<std::string>> DirectoryEntries;
  std::vector<BT<std::string>> FileEntries;
};
