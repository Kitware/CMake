/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorFileSets.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmGenExContext.h"
#include "cmGeneratorFileSet.h"
#include "cmGeneratorTarget.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

cmGeneratorFileSets::cmGeneratorFileSets(cmGeneratorTarget* target,
                                         cmLocalGenerator* lg)
  : Target(target)
  , LocalGenerator(lg)
{
  for (auto const& name : target->Target->GetAllFileSetNames()) {
    auto entry = this->FileSets.emplace(
      name,
      cm::make_unique<cmGeneratorFileSet>(target->Target->GetFileSet(name)));

    auto const* fileSet = entry.first->second.get();
    this->AllFileSets.push_back(fileSet);
    if (fileSet->IsForSelf()) {
      this->SelfFileSets[fileSet->GetType()].push_back(fileSet);
    }
    if (fileSet->IsForInterface()) {
      this->InterfaceFileSets[fileSet->GetType()].push_back(fileSet);
    }
  }
}
cmGeneratorFileSets::~cmGeneratorFileSets() = default;

std::vector<cmGeneratorFileSet const*> const&
cmGeneratorFileSets::GetAllFileSets() const
{
  return this->AllFileSets;
}

namespace {
std::vector<cmGeneratorFileSet const*> NoFileSets;
}

std::vector<cmGeneratorFileSet const*> const& cmGeneratorFileSets::GetFileSets(
  cm::string_view type) const
{
  auto it = this->SelfFileSets.find(type);
  if (it != this->SelfFileSets.end()) {
    return it->second;
  }
  return NoFileSets;
}
std::vector<cmGeneratorFileSet const*> const&
cmGeneratorFileSets::GetInterfaceFileSets(cm::string_view type) const
{
  auto it = this->InterfaceFileSets.find(type);
  if (it != this->InterfaceFileSets.end()) {
    return it->second;
  }
  return NoFileSets;
}

cmGeneratorFileSet const* cmGeneratorFileSets::GetFileSet(
  std::string const& name) const
{
  auto const it = this->FileSets.find(name);
  if (it != this->FileSets.end()) {
    return it->second.get();
  }
  return nullptr;
}

cmGeneratorFileSet const* cmGeneratorFileSets::GetFileSetForSource(
  std::string const& config, std::string const& path) const
{
  this->BuildInfoCache(config);

  auto const& info = this->Configs[config];

  auto const it = info.FileSetCache.find(path);
  if (it == info.FileSetCache.end()) {
    return nullptr;
  }
  return it->second;
}
cmGeneratorFileSet const* cmGeneratorFileSets::GetFileSetForSource(
  std::string const& config, cmSourceFile const* sf) const
{
  return this->GetFileSetForSource(config, sf->GetFullPath());
}

std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetSources(
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  std::vector<std::unique_ptr<TargetPropertyEntry>> entries;

  for (auto const& entry : this->FileSets) {
    auto const* fileSet = entry.second.get();
    if (fileSet->IsForSelf()) {
      auto sources = fileSet->GetSources(context, target, dagChecker);
      std::move(sources.begin(), sources.end(), std::back_inserter(entries));
    }
  }

  return entries;
}

std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetSources(
  std::string type, cm::GenEx::Context const& context,
  cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  std::vector<std::unique_ptr<TargetPropertyEntry>> entries;

  for (auto const& entry : this->FileSets) {
    auto const* fileSet = entry.second.get();
    if (!fileSet->IsForSelf() && fileSet->GetType() == type) {
      auto sources = fileSet->GetSources(context, target, dagChecker);
      std::move(sources.begin(), sources.end(), std::back_inserter(entries));
    }
  }

  return entries;
}

void cmGeneratorFileSets::BuildInfoCache(std::string const& config) const
{
  auto& info = this->Configs[config];

  if (info.BuiltCache) {
    return;
  }

  for (auto const& item : this->FileSets) {
    cm::GenEx::Context context(this->LocalGenerator, config);
    auto const* file_set = item.second.get();

    auto files = file_set->GetFiles(context, this->Target);

    for (auto const& it : files.first) {
      for (auto const& filename : it.second) {
        auto collapsedFile = cmSystemTools::CollapseFullPath(filename);
        info.FileSetCache[collapsedFile] = file_set;
      }
    }
  }

  info.BuiltCache = true;
}
