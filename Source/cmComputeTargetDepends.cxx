/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmComputeTargetDepends.h"

#include "cmComputeComponentGraph.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmake.h"

#include <algorithm>

#include <assert.h>

/*

This class is meant to analyze inter-target dependencies globally
during the generation step.  The goal is to produce a set of direct
dependencies for each target such that no cycles are left and the
build order is safe.

For most target types cyclic dependencies are not allowed.  However
STATIC libraries may depend on each other in a cyclic fasion.  In
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

//----------------------------------------------------------------------------
cmComputeTargetDepends::cmComputeTargetDepends(cmGlobalGenerator* gg)
{
  this->GlobalGenerator = gg;
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  this->DebugMode = cm->GetPropertyAsBool("GLOBAL_DEPENDS_DEBUG_MODE");
  this->NoCycles = cm->GetPropertyAsBool("GLOBAL_DEPENDS_NO_CYCLES");
}

//----------------------------------------------------------------------------
cmComputeTargetDepends::~cmComputeTargetDepends()
{
}

//----------------------------------------------------------------------------
bool cmComputeTargetDepends::Compute()
{
  // Build the original graph.
  this->CollectTargets();
  this->CollectDepends();
  if(this->DebugMode)
    {
    this->DisplayGraph(this->InitialGraph, "initial");
    }

  // Identify components.
  cmComputeComponentGraph ccg(this->InitialGraph);
  if(this->DebugMode)
    {
    this->DisplayComponents(ccg);
    }
  if(!this->CheckComponents(ccg))
    {
    return false;
    }

  // Compute the final dependency graph.
  this->ComputeFinalDepends(ccg);
  if(this->DebugMode)
    {
    this->DisplayGraph(this->FinalGraph, "final");
    }

  return true;
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends::GetTargetDirectDepends(cmTarget* t,
                                               std::set<cmTarget*>& deps)
{
  // Lookup the index for this target.  All targets should be known by
  // this point.
  std::map<cmTarget*, int>::const_iterator tii = this->TargetIndex.find(t);
  assert(tii != this->TargetIndex.end());
  int i = tii->second;

  // Get its final dependencies.
  NodeList const& nl = this->FinalGraph[i];
  for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
    {
    deps.insert(this->Targets[*ni]);
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::CollectTargets()
{
  // Collect all targets from all generators.
  std::vector<cmLocalGenerator*> const& lgens =
    this->GlobalGenerator->GetLocalGenerators();
  for(unsigned int i = 0; i < lgens.size(); ++i)
    {
    cmTargets& targets = lgens[i]->GetMakefile()->GetTargets();
    for(cmTargets::iterator ti = targets.begin(); ti != targets.end(); ++ti)
      {
      cmTarget* target = &ti->second;
      int index = static_cast<int>(this->Targets.size());
      this->TargetIndex[target] = index;
      this->Targets.push_back(target);
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::CollectDepends()
{
  // Allocate the dependency graph adjacency lists.
  this->InitialGraph.resize(this->Targets.size());

  // Compute each dependency list.
  for(unsigned int i=0; i < this->Targets.size(); ++i)
    {
    this->CollectTargetDepends(i);
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::CollectTargetDepends(int depender_index)
{
  // Get the depender.
  cmTarget* depender = this->Targets[depender_index];

  // Keep track of dependencies already listed.
  std::set<cmStdString> emitted;

  // A target should not depend on itself.
  emitted.insert(depender->GetName());

  // Loop over all targets linked directly.
  cmTarget::LinkLibraryVectorType const& tlibs =
    depender->GetOriginalLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator lib = tlibs.begin();
      lib != tlibs.end(); ++lib)
    {
    // Don't emit the same library twice for this target.
    if(emitted.insert(lib->first).second)
      {
      this->AddTargetDepend(depender_index, lib->first.c_str(), true);
      }
    }

  // Loop over all utility dependencies.
  std::set<cmStdString> const& tutils = depender->GetUtilities();
  for(std::set<cmStdString>::const_iterator util = tutils.begin();
      util != tutils.end(); ++util)
    {
    // Don't emit the same utility twice for this target.
    if(emitted.insert(*util).second)
      {
      this->AddTargetDepend(depender_index, util->c_str(), false);
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::AddTargetDepend(int depender_index,
                                             const char* dependee_name,
                                             bool linking)
{
  // Get the depender.
  cmTarget* depender = this->Targets[depender_index];

  // Check the target's makefile first.
  cmTarget* dependee =
    depender->GetMakefile()->FindTarget(dependee_name);

  // Then search globally.
  if(!dependee)
    {
    dependee = this->GlobalGenerator->FindTarget(0, dependee_name);
    }

  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if(linking && dependee &&
     dependee->GetType() == cmTarget::EXECUTABLE &&
     !dependee->IsExecutableWithExports())
    {
    dependee = 0;
    }

  // If not found then skip then the dependee.
  if(!dependee)
    {
    return;
    }

  // No imported targets should have been found.
  assert(!dependee->IsImported());

  // Lookup the index for this target.  All targets should be known by
  // this point.
  std::map<cmTarget*, int>::const_iterator tii =
    this->TargetIndex.find(dependee);
  assert(tii != this->TargetIndex.end());
  int dependee_index = tii->second;

  // Add this entry to the dependency graph.
  this->InitialGraph[depender_index].push_back(dependee_index);
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends::DisplayGraph(Graph const& graph, const char* name)
{
  fprintf(stderr, "The %s target dependency graph is:\n", name);
  int n = static_cast<int>(graph.size());
  for(int depender_index = 0; depender_index < n; ++depender_index)
    {
    NodeList const& nl = graph[depender_index];
    cmTarget* depender = this->Targets[depender_index];
    fprintf(stderr, "target %d is [%s]\n",
            depender_index, depender->GetName());
    for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int dependee_index = *ni;
      cmTarget* dependee = this->Targets[dependee_index];
      fprintf(stderr, "  depends on target %d [%s]\n", dependee_index,
              dependee->GetName());
      }
    }
  fprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends
::DisplayComponents(cmComputeComponentGraph const& ccg)
{
  fprintf(stderr, "The strongly connected components are:\n");
  std::vector<NodeList> const& components = ccg.GetComponents();
  int n = static_cast<int>(components.size());
  for(int c = 0; c < n; ++c)
    {
    NodeList const& nl = components[c];
    fprintf(stderr, "Component (%d):\n", c);
    for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int i = *ni;
      fprintf(stderr, "  contains target %d [%s]\n",
              i, this->Targets[i]->GetName());
      }
    }
  fprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
bool
cmComputeTargetDepends
::CheckComponents(cmComputeComponentGraph const& ccg)
{
  // All non-trivial components should consist only of static
  // libraries.
  std::vector<NodeList> const& components = ccg.GetComponents();
  int nc = static_cast<int>(components.size());
  for(int c=0; c < nc; ++c)
    {
    // Get the current component.
    NodeList const& nl = components[c];

    // Skip trivial components.
    if(nl.size() < 2)
      {
      continue;
      }

    // Immediately complain if no cycles are allowed at all.
    if(this->NoCycles)
      {
      this->ComplainAboutBadComponent(ccg, c);
      return false;
      }

    // Make sure the component is all STATIC_LIBRARY targets.
    for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      if(this->Targets[*ni]->GetType() != cmTarget::STATIC_LIBRARY)
        {
        this->ComplainAboutBadComponent(ccg, c);
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends
::ComplainAboutBadComponent(cmComputeComponentGraph const& ccg, int c)
{
  // Construct the error message.
  cmOStringStream e;
  e << "The inter-target dependency graph contains the following "
    << "strongly connected component (cycle):\n";
  std::vector<NodeList> const& components = ccg.GetComponents();
  std::vector<int> const& cmap = ccg.GetComponentMap();
  NodeList const& cl = components[c];
  for(NodeList::const_iterator ci = cl.begin(); ci != cl.end(); ++ci)
    {
    // Get the depender.
    int i = *ci;
    cmTarget* depender = this->Targets[i];

    // Describe the depender.
    e << "  \"" << depender->GetName() << "\" of type "
      << cmTarget::TargetTypeNames[depender->GetType()] << "\n";

    // List its dependencies that are inside the component.
    NodeList const& nl = this->InitialGraph[i];
    for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int j = *ni;
      if(cmap[j] == c)
        {
        cmTarget* dependee = this->Targets[j];
        e << "    depends on \"" << dependee->GetName() << "\"\n";
        }
      }
    }
  if(this->NoCycles)
    {
    e << "The GLOBAL_DEPENDS_NO_CYCLES global property is enabled, so "
      << "cyclic dependencies are not allowed even among static libraries.";
    }
  else
    {
    e << "At least one of these targets is not a STATIC_LIBRARY.  "
      << "Cyclic dependencies are allowed only among static libraries.";
    }
  cmSystemTools::Error(e.str().c_str());
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends
::ComputeFinalDepends(cmComputeComponentGraph const& ccg)
{
  // Get the component graph information.
  std::vector<NodeList> const& components = ccg.GetComponents();
  Graph const& cgraph = ccg.GetComponentGraph();

  // Allocate the final graph.
  this->FinalGraph.resize(0);
  this->FinalGraph.resize(this->InitialGraph.size());

  // Convert inter-component edges to connect component tails to heads.
  int n = static_cast<int>(cgraph.size());
  for(int depender_component=0; depender_component < n; ++depender_component)
    {
    int depender_component_tail = components[depender_component].back();
    NodeList const& nl = cgraph[depender_component];
    for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int dependee_component = *ni;
      int dependee_component_head = components[dependee_component].front();
      this->FinalGraph[depender_component_tail]
        .push_back(dependee_component_head);
      }
    }

  // Compute intra-component edges.
  int nc = static_cast<int>(components.size());
  for(int c=0; c < nc; ++c)
    {
    // Within the component each target depends on that following it.
    NodeList const& nl = components[c];
    NodeList::const_iterator ni = nl.begin();
    int last_i = *ni;
    for(++ni; ni != nl.end(); ++ni)
      {
      int i = *ni;
      this->FinalGraph[last_i].push_back(i);
      last_i = i;
      }
    }
}
