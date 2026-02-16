/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cmFileSetMetadata.h"
#include "cmGeneratorExpression.h"
#include "cmTargetPropertyEntry.h"
#include "cmValue.h"

namespace cm {
namespace GenEx {
struct Context;
}
}

template <typename T>
class BT;

struct cmGeneratorExpressionDAGChecker;

class cmFileSet;
class cmGeneratorTarget;

class cmGeneratorFileSet
{
public:
  using TargetPropertyEntry = cm::TargetPropertyEntry;

  cmGeneratorFileSet(cmFileSet const*);
  ~cmGeneratorFileSet() = default;

  cmGeneratorFileSet(cmGeneratorFileSet&&) = default;
  cmGeneratorFileSet(cmGeneratorFileSet const&) = delete;
  cmGeneratorFileSet& operator=(cmGeneratorFileSet const&) = delete;

  std::string const& GetName() const;
  std::string const& GetType() const;
  cm::FileSetMetadata::Visibility GetVisibility() const;

  bool IsForSelf() const;
  bool IsForInterface() const;
  bool CanBeIncluded() const;

  cmFileSet const* GetFileSet() const { return this->FileSet; }

  cmValue GetProperty(std::string const& prop) const;

  std::vector<BT<std::string>> const& GetDirectoryEntries() const;
  std::vector<BT<std::string>> const& GetFileEntries() const;

  std::vector<std::unique_ptr<TargetPropertyEntry>> GetSources(
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  // returned value:
  // first: list of directories
  // second: is context sensitive
  std::pair<std::vector<std::string>, bool> GetDirectories(
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;
  // returned value:
  // first: list of files per directory
  // second: is context sensitive
  std::pair<std::map<std::string, std::vector<std::string>>, bool> GetFiles(
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>> const&
  CompileFileEntries() const;

  std::vector<std::unique_ptr<cmCompiledGeneratorExpression>> const&
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

private:
  cmFileSet const* FileSet;
  mutable std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>
    CompiledDirectoryEntries;
  mutable std::vector<std::unique_ptr<cmCompiledGeneratorExpression>>
    CompiledFileEntries;
};
