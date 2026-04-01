/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <cm/string_view>

#include "cmTargetPropertyEntry.h"

namespace cm {
namespace GenEx {
struct Context;
struct Evaluation;
}
}

struct cmGeneratorExpressionDAGChecker;

class cmSourceFile;
class cmGeneratorTarget;
class cmGeneratorFileSet;
class cmLocalGenerator;

class cmGeneratorFileSets
{
public:
  using TargetPropertyEntry = cm::TargetPropertyEntry;

  cmGeneratorFileSets(cmGeneratorTarget* target, cmLocalGenerator* lg);
  ~cmGeneratorFileSets();

  cmGeneratorFileSets(cmGeneratorFileSets const&) = delete;
  cmGeneratorFileSets& operator=(cmGeneratorFileSets const&) = delete;

  bool Empty() const { return this->FileSets.empty(); }

  std::vector<cm::string_view> GetFileSetTypes() const;
  std::vector<cm::string_view> GetInterfaceFileSetTypes() const;

  std::vector<cmGeneratorFileSet const*> const& GetAllFileSets() const;
  std::vector<cmGeneratorFileSet const*> const& GetFileSets(
    cm::string_view type) const;
  std::vector<cmGeneratorFileSet const*> const& GetInterfaceFileSets(
    cm::string_view type) const;

  cmGeneratorFileSet const* GetFileSet(std::string const& name) const;

  cmGeneratorFileSet const* GetFileSetForSource(std::string const& config,
                                                std::string const& file) const;
  cmGeneratorFileSet const* GetFileSetForSource(std::string const& config,
                                                cmSourceFile const* sf) const;

  std::vector<std::unique_ptr<TargetPropertyEntry>> GetSources(
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;
  std::vector<std::unique_ptr<TargetPropertyEntry>> GetSources(
    std::string type, cm::GenEx::Context const& context,
    cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  std::vector<std::unique_ptr<TargetPropertyEntry>> GetInterfaceSources(
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;
  std::vector<std::unique_ptr<TargetPropertyEntry>> GetInterfaceSources(
    std::string type, cm::GenEx::Context const& context,
    cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr) const;

  std::string EvaluateInterfaceProperty(
    cm::string_view type, std::string const& prop, cm::GenEx::Evaluation* eval,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const;

private:
  std::vector<std::unique_ptr<cm::TargetPropertyEntry>> GetSources(
    std::function<bool(cmGeneratorFileSet const*)> include,
    cm::GenEx::Context const& context, cmGeneratorTarget const* target,
    cmGeneratorExpressionDAGChecker* dagChecker) const;

  // file sets indexed by name
  std::map<std::string, std::unique_ptr<cmGeneratorFileSet>> FileSets;
  std::vector<cmGeneratorFileSet const*> AllFileSets;
  // list of private file sets indexed by type
  std::unordered_map<cm::string_view, std::vector<cmGeneratorFileSet const*>>
    SelfFileSets;
  // list of interface file sets indexed by type
  std::unordered_map<cm::string_view, std::vector<cmGeneratorFileSet const*>>
    InterfaceFileSets;

  mutable std::unordered_map<std::string, bool> MaybeInterfacePropertyExists;
  bool MaybeHaveInterfaceProperty(cm::string_view type,
                                  std::string const& prop,
                                  cm::GenEx::Evaluation* eval) const;

  struct InfoByConfig
  {
    bool BuiltCache = false;
    std::map<std::string, cmGeneratorFileSet const*> FileSetCache;
    std::map<std::string, cmGeneratorFileSet const*> InterfaceFileSetCache;
  };
  mutable std::map<std::string, InfoByConfig> Configs;

  void BuildInfoCache(std::string const& config) const;

  cmGeneratorTarget* Target;
  cmLocalGenerator* LocalGenerator;
};
