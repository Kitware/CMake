/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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

  const std::string& GetName() const { return this->Name; }
  const std::string& GetType() const { return this->Type; }
  cmFileSetVisibility GetVisibility() const { return this->Visibility; }

  void CopyEntries(cmFileSet const* fs);

  void ClearDirectoryEntries();
  void AddDirectoryEntry(BT<std::string> directories);
  const std::vector<BT<std::string>>& GetDirectoryEntries() const
  {
    return this->DirectoryEntries;
  }

  void ClearFileEntries();
  void AddFileEntry(BT<std::string> files);
  const std::vector<BT<std::string>>& GetFileEntries() const
  {
    return this->FileEntries;
  }

  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>
  CompileFileEntries() const;

  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>
  CompileDirectoryEntries() const;

  std::vector<std::string> EvaluateDirectoryEntries(
    const std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>& cges,
    cmLocalGenerator* lg, const std::string& config,
    const cmGeneratorTarget* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  void EvaluateFileEntry(
    const std::vector<std::string>& dirs,
    std::map<std::string, std::vector<std::string>>& filesPerDir,
    const std::unique_ptr<cmCompiledGeneratorExpression>& cge,
    cmLocalGenerator* lg, const std::string& config,
    const cmGeneratorTarget* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  static bool IsValidName(const std::string& name);

private:
  cmake& CMakeInstance;
  std::string Name;
  std::string Type;
  cmFileSetVisibility Visibility;
  std::vector<BT<std::string>> DirectoryEntries;
  std::vector<BT<std::string>> FileEntries;
};
