/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmComputeTargetDepends.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <sstream>
#include <utility>

#include "cmComputeComponentGraph.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocationKind.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmValue.h"
#include "cmake.h"

/*

This class is meant to analyze inter-target dependencies globally
during the generation step.  The goal is to produce a set of direct
dependencies for each target such that no cycles are left and the
build order is safe.

For most target types cyclic dependencies are not allowed.  However
STATIC libraries may depend on each other in a cyclic fashion.  In
general the directed dependency graph forms a directed-acyclic-graph
of strongly connected components.  All strongly connected components
should consist of only STATIC_LIBRARY targets.

In order to safely break dependency cycles we must preserve all other
dependencies passing through the corresponding strongly connected component.
The approach taken by this class is as follows:

  - Collect all targets and form the original dependency graph
  - Run Tarjan's algorithm to extract the strongly connected components
    (error if any member of a non-trivial component is not STATIC)
  - The original dependencies imply a DAG on the components.
    Use the implied DAG to construct a final safe set of dependencies.

The final dependency set is constructed as follows:

  - For each connected component targets are placed in an arbitrary
    order.  Each target depends on the target following it in the order.
    The first target is designated the head and the last target the tail.
    (most components will be just 1 target anyway)

  - Original dependencies between targets in different components are
    converted to connect the depender's component tail to the
    dependee's component head.

In most cases this will reproduce the original dependencies.  However
when there are cycles of static libraries they will be broken in a
safe manner.

For example, consider targets A0, A1, A2, B0, B1, B2, and C with these
dependencies:

  A0 -> A1 -> A2 -> A0  ,  B0 -> B1 -> B2 -> B0 -> A0  ,  C -> B0

Components may be identified as

  Component 0: A0, A1, A2
  Component 1: B0, B1, B2
  Component 2: C

Intra-component dependencies are:

  0: A0 -> A1 -> A2   , head=A0, tail=A2
  1: B0 -> B1 -> B2   , head=B0, tail=B2
  2: head=C, tail=C

The inter-component dependencies are converted as:

  B0 -> A0  is component 1->0 and becomes  B2 -> A0
  C  -> B0  is component 2->1 and becomes  C  -> B0

This leads to the final target dependencies:

  C -> B0 -> B1 -> B2 -> A0 -> A1 -> A2

These produce a safe build order since C depends directly or
transitively on all the static libraries it links.

*/

cmComputeTargetDepends::cmComputeTargetDepends(cmGlobalGenerator* gg)
{
  this->GlobalGenerator = gg;
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  this->DebugMode =
    cm->GetState()->GetGlobalPropertyAsBool("GLOBAL_DEPENDS_DEBUG_MODE");
  this->NoCycles =
    cm->GetState()->GetGlobalPropertyAsBool("GLOBAL_DEPENDS_NO_CYCLES");
}

cmComputeTargetDepends::~cmComputeTargetDepends() = default;

bool cmComputeTargetDepends::Compute()
{
  // Build the original graph.
  this->CollectTargets();
  this->CollectDepends();
  if (this->DebugMode) {
    this->DisplayGraph(this->InitialGraph, "initial");
  }
  cmComputeComponentGraph ccg1(this->InitialGraph);
  ccg1.Compute();
  if (!this->CheckComponents(ccg1)) {
    return false;
  }

  // Compute the intermediate graph.
  this->CollectSideEffects();
  this->ComputeIntermediateGraph();
  if (this->DebugMode) {
    this->DisplaySideEffects();
    this->DisplayGraph(this->IntermediateGraph, "intermediate");
  }

  // Identify components.
  cmComputeComponentGraph ccg2(this->IntermediateGraph);
  ccg2.Compute();
  if (this->DebugMode) {
    this->DisplayComponents(ccg2, "intermediate");
  }
  if (!this->CheckComponents(ccg2)) {
    return false;
  }

  // Compute the final dependency graph.
  if (!this->ComputeFinalDepends(ccg2)) {
    return false;
  }
  if (this->DebugMode) {
    this->DisplayGraph(this->FinalGraph, "final");
  }

  return true;
}

void cmComputeTargetDepends::GetTargetDirectDepends(cmGeneratorTarget const* t,
                                                    cmTargetDependSet& deps)
{
  // Lookup the index for this target.  All targets should be known by
  // this point.
  auto tii = this->TargetIndex.find(t);
  assert(tii != this->TargetIndex.end());
  size_t i = tii->second;

  // Get its final dependencies.
  EdgeList const& nl = this->FinalGraph[i];
  for (cmGraphEdge const& ni : nl) {
    cmGeneratorTarget const* dep = this->Targets[ni];
    auto di = deps.insert(dep).first;
    di->SetType(ni.IsStrong());
    di->SetCross(ni.IsCross());
    di->SetBacktrace(ni.GetBacktrace());
  }
}

void cmComputeTargetDepends::CollectTargets()
{
  // Collect all targets from all generators.
  auto const& lgens = this->GlobalGenerator->GetLocalGenerators();
  for (const auto& lgen : lgens) {
    for (const auto& ti : lgen->GetGeneratorTargets()) {
      size_t index = this->Targets.size();
      this->TargetIndex[ti.get()] = index;
      this->Targets.push_back(ti.get());
    }
  }
}

void cmComputeTargetDepends::CollectDepends()
{
  // Allocate the dependency graph adjacency lists.
  this->InitialGraph.resize(this->Targets.size());

  // Compute each dependency list.
  for (size_t i = 0; i < this->Targets.size(); ++i) {
    this->CollectTargetDepends(i);
  }
}

void cmComputeTargetDepends::CollectTargetDepends(size_t depender_index)
{
  // Get the depender.
  cmGeneratorTarget const* depender = this->Targets[depender_index];
  if (!depender->IsInBuildSystem()) {
    return;
  }

  // Loop over all targets linked directly in all configs.
  // We need to make targets depend on the union of all config-specific
  // dependencies in all targets, because the generated build-systems can't
  // deal with config-specific dependencies.
  {
    std::set<cmLinkItem> emitted;

    std::vector<std::string> const& configs =
      depender->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
    for (std::string const& it : configs) {
      // A target should not depend on itself.
      emitted.insert(cmLinkItem(depender, false, cmListFileBacktrace()));
      emitted.insert(cmLinkItem(depender, true, cmListFileBacktrace()));

      if (cmLinkImplementation const* impl = depender->GetLinkImplementation(
            it, cmGeneratorTarget::LinkInterfaceFor::Link)) {
        for (cmLinkImplItem const& lib : impl->Libraries) {
          // Don't emit the same library twice for this target.
          if (emitted.insert(lib).second) {
            this->AddTargetDepend(depender_index, lib, true, false);
            this->AddInterfaceDepends(depender_index, lib, it, emitted);
          }
        }
        for (cmLinkItem const& obj : impl->Objects) {
          if (cmSourceFile const* o = depender->Makefile->GetSource(
                obj.AsStr(), cmSourceFileLocationKind::Known)) {
            this->AddObjectDepends(depender_index, o, emitted);
          }
        }
      }

      // Add dependencies on object libraries not otherwise handled above.
      std::vector<cmSourceFile const*> objectFiles;
      depender->GetExternalObjects(objectFiles, it);
      for (cmSourceFile const* o : objectFiles) {
        this->AddObjectDepends(depender_index, o, emitted);
      }
    }
  }

  // Loop over all utility dependencies.
  {
    std::set<cmLinkItem> const& tutils = depender->GetUtilityItems();
    std::set<cmLinkItem> emitted;
    // A target should not depend on itself.
    emitted.insert(cmLinkItem(depender, false, cmListFileBacktrace()));
    emitted.insert(cmLinkItem(depender, true, cmListFileBacktrace()));
    for (cmLinkItem const& litem : tutils) {
      // Don't emit the same utility twice for this target.
      if (emitted.insert(litem).second) {
        this->AddTargetDepend(depender_index, litem, false, litem.Cross);
      }
    }
  }
}

void cmComputeTargetDepends::AddInterfaceDepends(
  size_t depender_index, const cmGeneratorTarget* dependee,
  cmListFileBacktrace const& dependee_backtrace, const std::string& config,
  std::set<cmLinkItem>& emitted)
{
  cmGeneratorTarget const* depender = this->Targets[depender_index];
  if (cmLinkInterface const* iface =
        dependee->GetLinkInterface(config, depender)) {
    for (cmLinkItem const& lib : iface->Libraries) {
      // Don't emit the same library twice for this target.
      if (emitted.insert(lib).second) {
        // Inject the backtrace of the original link dependency whose
        // link interface we are adding.  This indicates the line of
        // code in the project that caused this dependency to be added.
        cmLinkItem libBT = lib;
        libBT.Backtrace = dependee_backtrace;
        this->AddTargetDepend(depender_index, libBT, true, false);
        this->AddInterfaceDepends(depender_index, libBT, config, emitted);
      }
    }
    for (cmLinkItem const& obj : iface->Objects) {
      if (cmSourceFile const* o = depender->Makefile->GetSource(
            obj.AsStr(), cmSourceFileLocationKind::Known)) {
        this->AddObjectDepends(depender_index, o, emitted);
      }
    }
  }
}

void cmComputeTargetDepends::AddInterfaceDepends(
  size_t depender_index, cmLinkItem const& dependee_name,
  const std::string& config, std::set<cmLinkItem>& emitted)
{
  cmGeneratorTarget const* depender = this->Targets[depender_index];
  cmGeneratorTarget const* dependee = dependee_name.Target;
  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if (dependee && dependee->GetType() == cmStateEnums::EXECUTABLE &&
      !dependee->IsExecutableWithExports()) {
    dependee = nullptr;
  }

  if (dependee) {
    // A target should not depend on itself.
    emitted.insert(cmLinkItem(depender, false, cmListFileBacktrace()));
    emitted.insert(cmLinkItem(depender, true, cmListFileBacktrace()));
    this->AddInterfaceDepends(depender_index, dependee,
                              dependee_name.Backtrace, config, emitted);
  }
}

void cmComputeTargetDepends::AddObjectDepends(size_t depender_index,
                                              cmSourceFile const* o,
                                              std::set<cmLinkItem>& emitted)
{
  std::string const& objLib = o->GetObjectLibrary();
  if (objLib.empty()) {
    return;
  }
  cmGeneratorTarget const* depender = this->Targets[depender_index];
  cmLinkItem const& objItem =
    depender->ResolveLinkItem(BT<std::string>(objLib));
  if (emitted.insert(objItem).second) {
    if (depender->GetType() != cmStateEnums::EXECUTABLE &&
        depender->GetType() != cmStateEnums::STATIC_LIBRARY &&
        depender->GetType() != cmStateEnums::SHARED_LIBRARY &&
        depender->GetType() != cmStateEnums::MODULE_LIBRARY &&
        depender->GetType() != cmStateEnums::OBJECT_LIBRARY) {
      this->GlobalGenerator->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR,
        "Only executables and libraries may reference target objects.",
        depender->GetBacktrace());
      return;
    }
    const_cast<cmGeneratorTarget*>(depender)->Target->AddUtility(objLib,
                                                                 false);
  }
}

void cmComputeTargetDepends::AddTargetDepend(size_t depender_index,
                                             cmLinkItem const& dependee_name,
                                             bool linking, bool cross)
{
  // Get the depender.
  cmGeneratorTarget const* depender = this->Targets[depender_index];

  // Check the target's makefile first.
  cmGeneratorTarget const* dependee = dependee_name.Target;

  if (!dependee && !linking &&
      (depender->GetType() != cmStateEnums::GLOBAL_TARGET)) {
    MessageType messageType = MessageType::AUTHOR_WARNING;
    bool issueMessage = false;
    std::ostringstream e;
    switch (depender->GetPolicyStatusCMP0046()) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0046) << "\n";
        issueMessage = true;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        issueMessage = true;
        messageType = MessageType::FATAL_ERROR;
        break;
    }
    if (issueMessage) {
      cmake* cm = this->GlobalGenerator->GetCMakeInstance();

      e << "The dependency target \"" << dependee_name << "\" of target \""
        << depender->GetName() << "\" does not exist.";

      cm->IssueMessage(messageType, e.str(), dependee_name.Backtrace);
    }
  }

  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if (linking && dependee && dependee->GetType() == cmStateEnums::EXECUTABLE &&
      !dependee->IsExecutableWithExports()) {
    dependee = nullptr;
  }

  if (dependee) {
    this->AddTargetDepend(depender_index, dependee, dependee_name.Backtrace,
                          linking, cross);
  }
}

void cmComputeTargetDepends::AddTargetDepend(
  size_t depender_index, cmGeneratorTarget const* dependee,
  cmListFileBacktrace const& dependee_backtrace, bool linking, bool cross)
{
  if (!dependee->IsInBuildSystem()) {
    // Skip targets that are not in the buildsystem but follow their
    // utility dependencies.
    std::set<cmLinkItem> const& utils = dependee->GetUtilityItems();
    for (cmLinkItem const& i : utils) {
      if (cmGeneratorTarget const* transitive_dependee = i.Target) {
        this->AddTargetDepend(depender_index, transitive_dependee, i.Backtrace,
                              false, i.Cross);
      }
    }
  } else {
    // Lookup the index for this target.  All targets should be known by
    // this point.
    auto tii = this->TargetIndex.find(dependee);
    assert(tii != this->TargetIndex.end());
    size_t dependee_index = tii->second;

    // Add this entry to the dependency graph.
    this->InitialGraph[depender_index].emplace_back(dependee_index, !linking,
                                                    cross, dependee_backtrace);
  }
}

void cmComputeTargetDepends::CollectSideEffects()
{
  this->SideEffects.resize(0);
  this->SideEffects.resize(this->InitialGraph.size());

  size_t n = this->InitialGraph.size();
  std::set<size_t> visited;
  for (size_t i = 0; i < n; ++i) {
    this->CollectSideEffectsForTarget(visited, i);
  }
}

void cmComputeTargetDepends::CollectSideEffectsForTarget(
  std::set<size_t>& visited, size_t depender_index)
{
  if (!visited.count(depender_index)) {
    auto& se = this->SideEffects[depender_index];
    visited.insert(depender_index);
    this->Targets[depender_index]->AppendCustomCommandSideEffects(
      se.CustomCommandSideEffects);
    this->Targets[depender_index]->AppendLanguageSideEffects(
      se.LanguageSideEffects);

    for (auto const& edge : this->InitialGraph[depender_index]) {
      this->CollectSideEffectsForTarget(visited, edge);
      auto const& dse = this->SideEffects[edge];
      se.CustomCommandSideEffects.insert(dse.CustomCommandSideEffects.cbegin(),
                                         dse.CustomCommandSideEffects.cend());
      for (auto const& it : dse.LanguageSideEffects) {
        se.LanguageSideEffects[it.first].insert(it.second.cbegin(),
                                                it.second.cend());
      }
    }
  }
}

void cmComputeTargetDepends::ComputeIntermediateGraph()
{
  this->IntermediateGraph.resize(0);
  this->IntermediateGraph.resize(this->InitialGraph.size());

  size_t n = this->InitialGraph.size();
  for (size_t i = 0; i < n; ++i) {
    auto const& initialEdges = this->InitialGraph[i];
    auto& intermediateEdges = this->IntermediateGraph[i];
    cmGeneratorTarget const* gt = this->Targets[i];
    if (gt->GetType() != cmStateEnums::STATIC_LIBRARY &&
        gt->GetType() != cmStateEnums::OBJECT_LIBRARY) {
      intermediateEdges = initialEdges;
    } else {
      if (cmValue optimizeDependencies =
            gt->GetProperty("OPTIMIZE_DEPENDENCIES")) {
        if (cmIsOn(optimizeDependencies)) {
          this->OptimizeLinkDependencies(gt, intermediateEdges, initialEdges);
        } else {
          intermediateEdges = initialEdges;
        }
      } else {
        intermediateEdges = initialEdges;
      }
    }
  }
}

void cmComputeTargetDepends::OptimizeLinkDependencies(
  cmGeneratorTarget const* gt, cmGraphEdgeList& outputEdges,
  cmGraphEdgeList const& inputEdges)
{
  std::set<size_t> emitted;
  for (auto const& edge : inputEdges) {
    if (edge.IsStrong()) {
      // Preserve strong edges
      outputEdges.push_back(edge);
    } else {
      auto const& dse = this->SideEffects[edge];

      // Add edges that have custom command side effects
      for (cmGeneratorTarget const* dep : dse.CustomCommandSideEffects) {
        auto index = this->TargetIndex[dep];
        if (!emitted.count(index)) {
          emitted.insert(index);
          outputEdges.push_back(
            cmGraphEdge(index, false, edge.IsCross(), edge.GetBacktrace()));
        }
      }

      // Add edges that have language side effects for languages we
      // care about
      for (auto const& lang : gt->GetAllConfigCompileLanguages()) {
        auto it = dse.LanguageSideEffects.find(lang);
        if (it != dse.LanguageSideEffects.end()) {
          for (cmGeneratorTarget const* dep : it->second) {
            auto index = this->TargetIndex[dep];
            if (!emitted.count(index)) {
              emitted.insert(index);
              outputEdges.push_back(cmGraphEdge(index, false, edge.IsCross(),
                                                edge.GetBacktrace()));
            }
          }
        }
      }
    }
  }
}

void cmComputeTargetDepends::DisplayGraph(Graph const& graph,
                                          const std::string& name)
{
  fprintf(stderr, "The %s target dependency graph is:\n", name.c_str());
  size_t n = graph.size();
  for (size_t depender_index = 0; depender_index < n; ++depender_index) {
    EdgeList const& nl = graph[depender_index];
    cmGeneratorTarget const* depender = this->Targets[depender_index];
    fprintf(stderr, "target %zu is [%s]\n", depender_index,
            depender->GetName().c_str());
    for (cmGraphEdge const& ni : nl) {
      size_t dependee_index = ni;
      cmGeneratorTarget const* dependee = this->Targets[dependee_index];
      fprintf(stderr, "  depends on target %zu [%s] (%s)\n", dependee_index,
              dependee->GetName().c_str(), ni.IsStrong() ? "strong" : "weak");
    }
  }
  fprintf(stderr, "\n");
}

void cmComputeTargetDepends::DisplaySideEffects()
{
  fprintf(stderr, "The side effects are:\n");
  size_t n = this->SideEffects.size();
  for (size_t depender_index = 0; depender_index < n; ++depender_index) {
    cmGeneratorTarget const* depender = this->Targets[depender_index];
    fprintf(stderr, "target %zu is [%s]\n", depender_index,
            depender->GetName().c_str());
    if (!this->SideEffects[depender_index].CustomCommandSideEffects.empty()) {
      fprintf(stderr, "  custom commands\n");
      for (auto const* gt :
           this->SideEffects[depender_index].CustomCommandSideEffects) {
        fprintf(stderr, "    from target %zu [%s]\n", this->TargetIndex[gt],
                gt->GetName().c_str());
      }
    }
    for (auto const& it :
         this->SideEffects[depender_index].LanguageSideEffects) {
      fprintf(stderr, "  language %s\n", it.first.c_str());
      for (auto const* gt : it.second) {
        fprintf(stderr, "    from target %zu [%s]\n", this->TargetIndex[gt],
                gt->GetName().c_str());
      }
    }
  }
  fprintf(stderr, "\n");
}

void cmComputeTargetDepends::DisplayComponents(
  cmComputeComponentGraph const& ccg, const std::string& name)
{
  fprintf(stderr, "The strongly connected components for the %s graph are:\n",
          name.c_str());
  std::vector<NodeList> const& components = ccg.GetComponents();
  size_t n = components.size();
  for (size_t c = 0; c < n; ++c) {
    NodeList const& nl = components[c];
    fprintf(stderr, "Component (%zu):\n", c);
    for (size_t i : nl) {
      fprintf(stderr, "  contains target %zu [%s]\n", i,
              this->Targets[i]->GetName().c_str());
    }
  }
  fprintf(stderr, "\n");
}

bool cmComputeTargetDepends::CheckComponents(
  cmComputeComponentGraph const& ccg)
{
  // All non-trivial components should consist only of static
  // libraries.
  std::vector<NodeList> const& components = ccg.GetComponents();
  size_t nc = components.size();
  for (size_t c = 0; c < nc; ++c) {
    // Get the current component.
    NodeList const& nl = components[c];

    // Skip trivial components.
    if (nl.size() < 2) {
      continue;
    }

    // Immediately complain if no cycles are allowed at all.
    if (this->NoCycles) {
      this->ComplainAboutBadComponent(ccg, c);
      return false;
    }

    // Make sure the component is all STATIC_LIBRARY targets.
    for (size_t ni : nl) {
      if (this->Targets[ni]->GetType() != cmStateEnums::STATIC_LIBRARY) {
        this->ComplainAboutBadComponent(ccg, c);
        return false;
      }
    }
  }
  return true;
}

void cmComputeTargetDepends::ComplainAboutBadComponent(
  cmComputeComponentGraph const& ccg, size_t c, bool strong)
{
  // Construct the error message.
  std::ostringstream e;
  e << "The inter-target dependency graph contains the following "
    << "strongly connected component (cycle):\n";
  std::vector<NodeList> const& components = ccg.GetComponents();
  std::vector<size_t> const& cmap = ccg.GetComponentMap();
  NodeList const& cl = components[c];
  for (size_t i : cl) {
    // Get the depender.
    cmGeneratorTarget const* depender = this->Targets[i];

    // Describe the depender.
    e << "  \"" << depender->GetName() << "\" of type "
      << cmState::GetTargetTypeName(depender->GetType()) << "\n";

    // List its dependencies that are inside the component.
    EdgeList const& nl = this->InitialGraph[i];
    for (cmGraphEdge const& ni : nl) {
      size_t j = ni;
      if (cmap[j] == c) {
        cmGeneratorTarget const* dependee = this->Targets[j];
        e << "    depends on \"" << dependee->GetName() << "\""
          << " (" << (ni.IsStrong() ? "strong" : "weak") << ")\n";
      }
    }
  }
  if (strong) {
    // Custom command executable dependencies cannot occur within a
    // component of static libraries.  The cycle must appear in calls
    // to add_dependencies.
    e << "The component contains at least one cycle consisting of strong "
      << "dependencies (created by add_dependencies) that cannot be broken.";
  } else if (this->NoCycles) {
    e << "The GLOBAL_DEPENDS_NO_CYCLES global property is enabled, so "
      << "cyclic dependencies are not allowed even among static libraries.";
  } else {
    e << "At least one of these targets is not a STATIC_LIBRARY.  "
      << "Cyclic dependencies are allowed only among static libraries.";
  }
  cmSystemTools::Error(e.str());
}

bool cmComputeTargetDepends::IntraComponent(std::vector<size_t> const& cmap,
                                            size_t c, size_t i, size_t* head,
                                            std::set<size_t>& emitted,
                                            std::set<size_t>& visited)
{
  if (!visited.insert(i).second) {
    // Cycle in utility depends!
    return false;
  }
  if (emitted.insert(i).second) {
    // Honor strong intra-component edges in the final order.
    EdgeList const& el = this->InitialGraph[i];
    for (cmGraphEdge const& edge : el) {
      size_t j = edge;
      if (cmap[j] == c && edge.IsStrong()) {
        this->FinalGraph[i].emplace_back(j, true, edge.IsCross(),
                                         edge.GetBacktrace());
        if (!this->IntraComponent(cmap, c, j, head, emitted, visited)) {
          return false;
        }
      }
    }

    // Prepend to a linear linked-list of intra-component edges.
    if (*head != cmComputeComponentGraph::INVALID_COMPONENT) {
      this->FinalGraph[i].emplace_back(*head, false, false,
                                       cmListFileBacktrace());
    } else {
      this->ComponentTail[c] = i;
    }
    *head = i;
  }
  return true;
}

bool cmComputeTargetDepends::ComputeFinalDepends(
  cmComputeComponentGraph const& ccg)
{
  // Get the component graph information.
  std::vector<NodeList> const& components = ccg.GetComponents();
  Graph const& cgraph = ccg.GetComponentGraph();

  // Allocate the final graph.
  this->FinalGraph.resize(0);
  this->FinalGraph.resize(this->InitialGraph.size());

  // Choose intra-component edges to linearize dependencies.
  std::vector<size_t> const& cmap = ccg.GetComponentMap();
  this->ComponentHead.resize(components.size());
  this->ComponentTail.resize(components.size());
  size_t nc = components.size();
  for (size_t c = 0; c < nc; ++c) {
    size_t head = cmComputeComponentGraph::INVALID_COMPONENT;
    std::set<size_t> emitted;
    NodeList const& nl = components[c];
    for (size_t ni : cmReverseRange(nl)) {
      std::set<size_t> visited;
      if (!this->IntraComponent(cmap, c, ni, &head, emitted, visited)) {
        // Cycle in add_dependencies within component!
        this->ComplainAboutBadComponent(ccg, c, true);
        return false;
      }
    }
    this->ComponentHead[c] = head;
  }

  // Convert inter-component edges to connect component tails to heads.
  size_t n = cgraph.size();
  for (size_t depender_component = 0; depender_component < n;
       ++depender_component) {
    size_t depender_component_tail = this->ComponentTail[depender_component];
    EdgeList const& nl = cgraph[depender_component];
    for (cmGraphEdge const& ni : nl) {
      size_t dependee_component = ni;
      size_t dependee_component_head = this->ComponentHead[dependee_component];
      this->FinalGraph[depender_component_tail].emplace_back(
        dependee_component_head, ni.IsStrong(), ni.IsCross(),
        ni.GetBacktrace());
    }
  }
  return true;
}
