/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallRuntimeDependencySet.h"

#include <set>
#include <string>
#include <utility>

#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallImportedRuntimeArtifactsGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmStateTypes.h"
#include "cmTargetDepend.h"

cmInstallRuntimeDependencySet::cmInstallRuntimeDependencySet(std::string name)
  : Name(std::move(name))
{
}

void cmInstallRuntimeDependencySet::AddExecutable(
  std::unique_ptr<Item> executable)
{
  this->Executables.push_back(std::move(executable));
}

void cmInstallRuntimeDependencySet::AddLibrary(std::unique_ptr<Item> library)
{
  this->Libraries.push_back(std::move(library));
}

void cmInstallRuntimeDependencySet::AddModule(std::unique_ptr<Item> module)
{
  this->Modules.push_back(std::move(module));
}

bool cmInstallRuntimeDependencySet::AddBundleExecutable(
  std::unique_ptr<Item> bundleExecutable)
{
  if (this->BundleExecutable) {
    return false;
  }
  this->BundleExecutable = bundleExecutable.get();
  this->AddExecutable(std::move(bundleExecutable));
  return true;
}

std::string cmInstallRuntimeDependencySet::TargetItem::GetItemPath(
  const std::string& config) const
{
  return this->Target->GetTarget()->GetFullPath(config);
}

namespace {
const std::set<const cmGeneratorTarget*>& GetTargetDependsClosure(
  std::map<const cmGeneratorTarget*, std::set<const cmGeneratorTarget*>>&
    targetDepends,
  const cmGeneratorTarget* tgt)
{
  auto it = targetDepends.insert({ tgt, {} });
  auto& retval = it.first->second;
  if (it.second) {
    auto const& deps = tgt->GetGlobalGenerator()->GetTargetDirectDepends(tgt);
    for (auto const& dep : deps) {
      if (!dep.IsCross() && dep.IsLink()) {
        auto type = dep->GetType();
        if (type == cmStateEnums::EXECUTABLE ||
            type == cmStateEnums::SHARED_LIBRARY ||
            type == cmStateEnums::MODULE_LIBRARY) {
          retval.insert(dep);
        }
        auto const& depDeps = GetTargetDependsClosure(targetDepends, dep);
        retval.insert(depDeps.begin(), depDeps.end());
      }
    }
  }
  return retval;
}
}

void cmInstallRuntimeDependencySet::TargetItem::AddPostExcludeFiles(
  const std::string& config, std::set<std::string>& files,
  cmInstallRuntimeDependencySet* set) const
{
  for (auto const* dep : GetTargetDependsClosure(set->TargetDepends,
                                                 this->Target->GetTarget())) {
    files.insert(dep->GetFullPath(config));
  }
}

std::string cmInstallRuntimeDependencySet::ImportedTargetItem::GetItemPath(
  const std::string& config) const
{
  return this->Target->GetTarget()->GetFullPath(config);
}
