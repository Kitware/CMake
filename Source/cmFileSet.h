/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cmListFileCache.h"

class cmCompiledGeneratorExpression;
struct cmGeneratorExpressionDAGChecker;
class cmGeneratorTarget;
class cmLocalGenerator;

class cmFileSet
{
public:
  cmFileSet(std::string name, std::string type);

  const std::string& GetName() const { return this->Name; }
  const std::string& GetType() const { return this->Type; }

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
  std::string Name;
  std::string Type;
  std::vector<BT<std::string>> DirectoryEntries;
  std::vector<BT<std::string>> FileEntries;
};
