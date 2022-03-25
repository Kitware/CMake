/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLinkItemGraphVisitor.h"

#include <map>
#include <utility>
#include <vector>

#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmMakefile.h"

void cmLinkItemGraphVisitor::VisitItem(cmLinkItem const& item)
{
  if (this->ItemVisited(item)) {
    return;
  }

  this->OnItem(item);

  this->VisitLinks(item, item);
}

void cmLinkItemGraphVisitor::VisitLinks(cmLinkItem const& item,
                                        cmLinkItem const& rootItem)
{
  if (item.Target == nullptr) {
    return;
  }

  for (auto const& config : item.Target->Makefile->GetGeneratorConfigs(
         cmMakefile::IncludeEmptyConfig)) {
    this->VisitLinks(item, rootItem, config);
  }
}

void cmLinkItemGraphVisitor::VisitLinks(cmLinkItem const& item,
                                        cmLinkItem const& rootItem,
                                        std::string const& config)
{
  auto const& target = *item.Target;

  DependencyMap dependencies;
  cmLinkItemGraphVisitor::GetDependencies(target, config, dependencies);

  for (auto const& d : dependencies) {
    auto const& dependency = d.second;
    auto const& dependencyType = dependency.first;
    auto const& dependee = dependency.second;
    this->VisitItem(dependee);

    if (this->LinkVisited(item, dependee)) {
      continue;
    }

    this->OnDirectLink(item, dependee, dependencyType);

    if (rootItem.AsStr() != item.AsStr()) {
      this->OnIndirectLink(rootItem, dependee);
    }

    // Visit all the direct and indirect links.
    this->VisitLinks(dependee, dependee);
    this->VisitLinks(dependee, item);
    this->VisitLinks(dependee, rootItem);
  }
}

bool cmLinkItemGraphVisitor::ItemVisited(cmLinkItem const& item)
{
  auto& collection = this->VisitedItems;

  bool const visited = collection.find(item.AsStr()) != collection.cend();

  if (!visited) {
    collection.insert(item.AsStr());
  }

  return visited;
}

bool cmLinkItemGraphVisitor::LinkVisited(cmLinkItem const& depender,
                                         cmLinkItem const& dependee)
{
  auto const link = std::make_pair<>(depender.AsStr(), dependee.AsStr());

  bool const linkVisited =
    this->VisitedLinks.find(link) != this->VisitedLinks.cend();

  if (!linkVisited) {
    this->VisitedLinks.insert(link);
  }

  return linkVisited;
}

void cmLinkItemGraphVisitor::GetDependencies(cmGeneratorTarget const& target,
                                             std::string const& config,
                                             DependencyMap& dependencies)
{
  const auto* implementationLibraries = target.GetLinkImplementationLibraries(
    config, cmGeneratorTarget::LinkInterfaceFor::Link);
  if (implementationLibraries != nullptr) {
    for (auto const& lib : implementationLibraries->Libraries) {
      auto const& name = lib.AsStr();
      dependencies[name] = Dependency(DependencyType::LinkPrivate, lib);
    }
  }

  const auto* interfaceLibraries = target.GetLinkInterfaceLibraries(
    config, &target, cmGeneratorTarget::LinkInterfaceFor::Usage);
  if (interfaceLibraries != nullptr) {
    for (auto const& lib : interfaceLibraries->Libraries) {
      auto const& name = lib.AsStr();
      if (dependencies.find(name) != dependencies.cend()) {
        dependencies[name] = Dependency(DependencyType::LinkPublic, lib);
      } else {
        dependencies[name] = Dependency(DependencyType::LinkInterface, lib);
      }
    }
  }

  std::vector<cmGeneratorTarget*> objectLibraries;
  target.GetObjectLibrariesCMP0026(objectLibraries);
  for (auto const& lib : objectLibraries) {
    auto const& name = lib->GetName();
    if (dependencies.find(name) == dependencies.cend()) {
      auto objectItem = cmLinkItem(lib, false, lib->GetBacktrace());
      dependencies[name] = Dependency(DependencyType::Object, objectItem);
    }
  }

  auto const& utilityItems = target.GetUtilityItems();
  for (auto const& item : utilityItems) {
    auto const& name = item.AsStr();
    if (dependencies.find(name) == dependencies.cend()) {
      dependencies[name] = Dependency(DependencyType::Utility, item);
    }
  }
}
