/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorFileSets.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmFileSetMetadata.h"
#include "cmGenExContext.h"
#include "cmGenExEvaluation.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionNode.h"
#include "cmGeneratorFileSet.h"
#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

cmGeneratorFileSets::cmGeneratorFileSets(cmGeneratorTarget* target,
                                         cmLocalGenerator* lg)
  : Target(target)
  , LocalGenerator(lg)
{
  for (auto const& name : target->Target->GetAllPrivateFileSets()) {
    auto entry =
      this->FileSets.emplace(name,
                             cm::make_unique<cmGeneratorFileSet>(
                               target, target->Target->GetFileSet(name)));

    auto const* fileSet = entry.first->second.get();
    this->AllFileSets.push_back(fileSet);
    this->SelfFileSets[fileSet->GetType()].push_back(fileSet);
  }
  for (auto const& name : target->Target->GetAllInterfaceFileSets()) {
    auto it = this->FileSets.find(name);
    cmGeneratorFileSet const* fileSet = nullptr;
    if (it == this->FileSets.end()) {
      auto entry =
        this->FileSets.emplace(name,
                               cm::make_unique<cmGeneratorFileSet>(
                                 target, target->Target->GetFileSet(name)));
      fileSet = entry.first->second.get();
      this->AllFileSets.push_back(fileSet);
    } else {
      fileSet = it->second.get();
    }
    this->InterfaceFileSets[fileSet->GetType()].push_back(fileSet);
  }
}
cmGeneratorFileSets::~cmGeneratorFileSets() = default;

std::vector<cm::string_view> cmGeneratorFileSets::GetFileSetTypes() const
{
  return cm::keys(this->SelfFileSets);
}

std::vector<cm::string_view> cmGeneratorFileSets::GetInterfaceFileSetTypes()
  const
{
  return cm::keys(this->InterfaceFileSets);
}

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
  using Lookup = cm::FileSetMetadata::FileSetLookup;

  this->BuildInfoCache(config);

  auto const& info = this->Configs[config];

  auto const it = info.FileSetCache.find(path);
  if (it != info.FileSetCache.end()) {
    return it->second;
  }

  // search in all the dependents
  auto const it2 = info.InterfaceFileSetCache.find(path);
  if (it2 != info.InterfaceFileSetCache.end() &&
      cm::FileSetMetadata::GetFileSetDescriptor(it2->second->GetType())
          .value_or(
            cm::FileSetMetadata::FileSetDescriptor{ ""_s, Lookup::Target })
          .Lookup == Lookup::Dependencies) {
    return it2->second;
  }

  return nullptr;
}
cmGeneratorFileSet const* cmGeneratorFileSets::GetFileSetForSource(
  std::string const& config, cmSourceFile const* sf) const
{
  return this->GetFileSetForSource(config, sf->GetFullPath());
}

std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetSources(
  std::function<bool(cmGeneratorFileSet const*)> include,
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  std::vector<std::unique_ptr<TargetPropertyEntry>> entries;

  for (auto const& entry : this->FileSets) {
    auto const* fileSet = entry.second.get();
    if (include(fileSet)) {
      auto sources = fileSet->GetSources(context, target, dagChecker);
      std::move(sources.begin(), sources.end(), std::back_inserter(entries));
    }
  }

  return entries;
}

std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetSources(
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  return this->GetSources(
    [](cmGeneratorFileSet const* fileSet) -> bool {
      return fileSet->IsForSelf();
    },
    context, target, dagChecker);
}
std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetSources(
  std::string type, cm::GenEx::Context const& context,
  cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  return this->GetSources(
    [&type](cmGeneratorFileSet const* fileSet) -> bool {
      return fileSet->IsForSelf() && fileSet->GetType() == type;
    },
    context, target, dagChecker);
}

std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetInterfaceSources(
  cm::GenEx::Context const& context, cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  return this->GetSources(
    [](cmGeneratorFileSet const* fileSet) -> bool {
      return fileSet->IsForInterface();
    },
    context, target, dagChecker);
}
std::vector<std::unique_ptr<cm::TargetPropertyEntry>>
cmGeneratorFileSets::GetInterfaceSources(
  std::string type, cm::GenEx::Context const& context,
  cmGeneratorTarget const* target,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  return this->GetSources(
    [&type](cmGeneratorFileSet const* fileSet) -> bool {
      return fileSet->IsForInterface() && fileSet->GetType() == type;
    },
    context, target, dagChecker);
}

bool cmGeneratorFileSets::MaybeHaveInterfaceProperty(
  cm::string_view type, std::string const& prop,
  cm::GenEx::Evaluation* eval) const
{
  std::string const key =
    cmStrCat(type, "::", prop, '@', eval->Context.Config);
  auto i = this->MaybeInterfacePropertyExists.find(key);
  if (i == this->MaybeInterfacePropertyExists.end()) {
    // Insert an entry now in case there is a cycle.
    i = this->MaybeInterfacePropertyExists.emplace(key, false).first;
    bool& maybeInterfaceProp = i->second;

    for (auto const* fileSet : this->GetInterfaceFileSets(type)) {
      // If this file set itself has a non-empty property value, we are done.
      maybeInterfaceProp = !fileSet->GetProperty(prop).IsEmpty();
      if (maybeInterfaceProp) {
        break;
      }
    }

    // Otherwise, recurse to interface dependencies.
    if (!maybeInterfaceProp) {
      cmGeneratorTarget const* headTarget =
        eval->HeadTarget ? eval->HeadTarget : this->Target;
      if (cmLinkInterfaceLibraries const* iface =
            this->Target->GetLinkInterfaceLibraries(
              eval->Context.Config, headTarget,
              cmGeneratorTarget::UseTo::Compile)) {
        if (iface->HadHeadSensitiveCondition) {
          // With a different head target we may get to a library with
          // this interface property.
          maybeInterfaceProp = true;
        } else {
          // The transitive interface libraries do not depend on the
          // head target, so we can follow them.
          for (cmLinkItem const& lib : iface->Libraries) {
            if (lib.Target &&
                lib.Target->GetGeneratorFileSets()->MaybeHaveInterfaceProperty(
                  type, prop, eval)) {
              maybeInterfaceProp = true;
              break;
            }
          }
        }
      }
    }
  }
  return i->second;
}

std::string cmGeneratorFileSets::EvaluateInterfaceProperty(
  cm::string_view type, std::string const& prop, cm::GenEx::Evaluation* eval,
  cmGeneratorExpressionDAGChecker* dagCheckerParent) const
{
  // If the property does not appear transitively at all, we are done.
  if (!this->MaybeHaveInterfaceProperty(type, prop, eval)) {
    return std::string{};
  }

  cmList result;
  cmGeneratorExpressionDAGChecker dagChecker{
    this->Target,     prop,          nullptr,
    dagCheckerParent, eval->Context, eval->Backtrace,
  };
  switch (dagChecker.Check()) {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      dagChecker.ReportError(
        eval,
        cmStrCat("$<FILE_SET_PROPERTY:*,TARGET:", this->Target->GetName(), ',',
                 prop, '>'));
      return std::string{};
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      // No error. We have already seen this transitive property.
      return std::string{};
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
  }

  cmGeneratorTarget const* headTarget =
    eval->HeadTarget ? eval->HeadTarget : this->Target;

  for (auto const* fileSet : this->GetInterfaceFileSets(type)) {
    if (cmValue p = fileSet->GetProperty(prop)) {
      result.append(cmGeneratorExpressionNode::EvaluateDependentExpression(
        *p, eval, headTarget, &dagChecker, this->Target));
    }
  }

  if (cmLinkInterfaceLibraries const* iface =
        this->Target->GetLinkInterfaceLibraries(
          eval->Context.Config, headTarget,
          cmGeneratorTarget::UseTo::Compile)) {
    eval->HadContextSensitiveCondition = eval->HadContextSensitiveCondition ||
      iface->HadContextSensitiveCondition;
    for (cmLinkItem const& lib : iface->Libraries) {
      // Broken code can have a target in its own link interface.
      // Don't follow such link interface entries so as not to create a
      // self-referencing loop.
      if (lib.Target && lib.Target != this->Target) {
        // Pretend $<FILE_SET__PROPERTY:fileSet,TARGET:lib.Target,prop>
        // appeared in the above property and hand-evaluate it as if it were
        // compiled.
        // Create a context as cmCompiledGeneratorExpression::Evaluate does.
        cm::GenEx::Evaluation libEval(
          eval->Context, eval->Quiet, headTarget, this->Target,
          eval->EvaluateForBuildsystem, eval->Backtrace);
        std::string libResult = cmGeneratorExpression::StripEmptyListElements(
          lib.Target->GetGeneratorFileSets()->EvaluateInterfaceProperty(
            type, prop, &libEval, &dagChecker));
        if (!libResult.empty()) {
          result.append(libResult);
        }
        eval->HadContextSensitiveCondition =
          eval->HadContextSensitiveCondition ||
          libEval.HadContextSensitiveCondition;
        eval->HadHeadSensitiveCondition =
          eval->HadHeadSensitiveCondition || libEval.HadHeadSensitiveCondition;
      }
    }
  }

  return result.to_string();
}

namespace {
void GetInterfaceFiles(cmGeneratorTarget const* target,
                       cm::GenEx::Context const& context,
                       std::unordered_set<cmGeneratorTarget const*>& targets,
                       std::map<std::string, cmGeneratorFileSet const*>& cache)
{
  namespace Metadata = cm::FileSetMetadata;

  for (auto const& type : Metadata::GetKnownTypes()) {
    auto fileSetDescriptor = Metadata::GetFileSetDescriptor(type);
    if (fileSetDescriptor &&
        fileSetDescriptor->Lookup == Metadata::FileSetLookup::Dependencies) {
      for (auto const* fileSet : target->GetInterfaceFileSets(type)) {
        auto files = fileSet->GetFiles(context, target);

        for (auto const& it : files.first) {
          for (auto const& filename : it.second) {
            auto collapsedFile = cmSystemTools::CollapseFullPath(filename);
            cache[collapsedFile] = fileSet;
          }
        }
      }
    }
  }

  if (cmLinkInterfaceLibraries const* iface =
        target->GetLinkInterfaceLibraries(context.Config, target,
                                          cmGeneratorTarget::UseTo::Compile)) {
    for (cmLinkItem const& lib : iface->Libraries) {
      if (lib.Target && lib.Target != target &&
          targets.insert(lib.Target).second) {
        GetInterfaceFiles(lib.Target, context, targets, cache);
      }
    }
  }
}
}

void cmGeneratorFileSets::BuildInfoCache(std::string const& config) const
{
  auto& info = this->Configs[config];

  if (info.BuiltCache) {
    return;
  }

  cm::GenEx::Context context(this->LocalGenerator, config);

  for (auto const& item : this->FileSets) {
    auto const* fileSet = item.second.get();

    auto files = fileSet->GetFiles(context, this->Target);

    for (auto const& it : files.first) {
      for (auto const& filename : it.second) {
        auto collapsedFile = cmSystemTools::CollapseFullPath(filename);
        info.FileSetCache[collapsedFile] = fileSet;
      }
    }
  }

  // retrieve all files inherited from dependent targets
  std::unordered_set<cmGeneratorTarget const*> targets;

  if (cmLinkImplementationLibraries const* impl =
        this->Target->GetLinkImplementationLibraries(
          config, cmGeneratorTarget::UseTo::Compile)) {
    for (cmLinkItem const& lib : impl->Libraries) {
      if (lib.Target) {
        GetInterfaceFiles(lib.Target, context, targets,
                          info.InterfaceFileSetCache);
      }
    }
  }

  info.BuiltCache = true;
}
