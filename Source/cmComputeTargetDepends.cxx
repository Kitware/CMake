/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmComputeTargetDepends.h"

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
    this->DisplayGraph(this->TargetDependGraph, "initial");
    }

  // Identify components.
  this->Tarjan();
  if(this->DebugMode)
    {
    this->DisplayComponents();
    }
  if(!this->CheckComponents())
    {
    return false;
    }

  // Compute the final dependency graph.
  this->ComputeFinalDepends();
  if(this->DebugMode)
    {
    this->DisplayGraph(this->FinalDependGraph, "final");
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
  TargetDependList const& tdl = this->FinalDependGraph[i];
  for(TargetDependList::const_iterator di = tdl.begin();
      di != tdl.end(); ++di)
    {
    deps.insert(this->Targets[*di]);
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
  this->TargetDependGraph.resize(this->Targets.size());

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
      this->AddTargetDepend(depender_index, lib->first.c_str());
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
      this->AddTargetDepend(depender_index, util->c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::AddTargetDepend(int depender_index,
                                             const char* dependee_name)
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
  this->TargetDependGraph[depender_index].push_back(dependee_index);
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends
::DisplayGraph(std::vector<TargetDependList> const& graph,
               const char* name)
{
  fprintf(stderr, "The %s target dependency graph is:\n", name);
  int n = static_cast<int>(graph.size());
  for(int depender_index = 0; depender_index < n; ++depender_index)
    {
    TargetDependList const& tdl = graph[depender_index];
    cmTarget* depender = this->Targets[depender_index];
    fprintf(stderr, "target %d is [%s]\n",
            depender_index, depender->GetName());
    for(TargetDependList::const_iterator di = tdl.begin();
        di != tdl.end(); ++di)
      {
      int dependee_index = *di;
      cmTarget* dependee = this->Targets[dependee_index];
      fprintf(stderr, "  depends on target %d [%s]\n", dependee_index,
              dependee->GetName());
      }
    }
  fprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::Tarjan()
{
  int n = static_cast<int>(this->TargetDependGraph.size());
  TarjanEntry entry = {0,-1,0};
  this->TarjanEntries.resize(n, entry);
  this->TarjanWalkId = 0;
  this->TarjanVisited.resize(n, 0);
  for(int i = 0; i < n; ++i)
    {
    // Start a new DFS from this node if it has never been visited.
    if(!this->TarjanVisited[i])
      {
      assert(this->TarjanStack.empty());
      ++this->TarjanWalkId;
      this->TarjanIndex = 0;
      this->TarjanVisit(i);
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::TarjanVisit(int i)
{
  // We are now visiting this node.
  this->TarjanVisited[i] = this->TarjanWalkId;

  // Initialize the entry.
  this->TarjanEntries[i].Root = i;
  this->TarjanEntries[i].Component = -1;
  this->TarjanEntries[i].VisitIndex = ++this->TarjanIndex;
  this->TarjanStack.push(i);

  // Follow outgoing edges.
  TargetDependList const& tdl = this->TargetDependGraph[i];
  for(TargetDependList::const_iterator di = tdl.begin();
      di != tdl.end(); ++di)
    {
    int j = *di;

    // Ignore edges to nodes that have been reached by a previous DFS
    // walk.  Since we did not reach the current node from that walk
    // it must not belong to the same component and it has already
    // been assigned to a component.
    if(this->TarjanVisited[j] > 0 &&
       this->TarjanVisited[j] < this->TarjanWalkId)
      {
      continue;
      }

    // Visit the destination if it has not yet been visited.
    if(!this->TarjanVisited[j])
      {
      this->TarjanVisit(j);
      }

    // If the destination has not yet been assigned to a component,
    // check if it is a better potential root for the current object.
    if(this->TarjanEntries[j].Component < 0)
      {
      if(this->TarjanEntries[this->TarjanEntries[j].Root].VisitIndex <
         this->TarjanEntries[this->TarjanEntries[i].Root].VisitIndex)
        {
        this->TarjanEntries[i].Root = this->TarjanEntries[j].Root;
        }
      }
    }

  // Check if we have found a component.
  if(this->TarjanEntries[i].Root == i)
    {
    // Yes.  Create it.
    int c = static_cast<int>(this->Components.size());
    this->Components.push_back(ComponentList());
    ComponentList& component = this->Components[c];

    // Populate the component list.
    int j;
    do
      {
      // Get the next member of the component.
      j = this->TarjanStack.top();
      this->TarjanStack.pop();

      // Assign the member to the component.
      this->TarjanEntries[j].Component = c;
      this->TarjanEntries[j].Root = i;

      // Store the node in its component.
      component.push_back(j);
      } while(j != i);

    // Sort the component members for clarity.
    std::sort(component.begin(), component.end());
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::DisplayComponents()
{
  fprintf(stderr, "The strongly connected components are:\n");
  int n = static_cast<int>(this->Components.size());
  for(int c = 0; c < n; ++c)
    {
    ComponentList const& cl = this->Components[c];
    fprintf(stderr, "Component (%d):\n", c);
    for(ComponentList::const_iterator ci = cl.begin();
        ci != cl.end(); ++ci)
      {
      int i = *ci;
      fprintf(stderr, "  contains target %d [%s]\n",
              i, this->Targets[i]->GetName());
      }
    }
  fprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
bool cmComputeTargetDepends::CheckComponents()
{
  // All non-trivial components should consist only of static
  // libraries.
  int nc = static_cast<int>(this->Components.size());
  for(int c=0; c < nc; ++c)
    {
    // Get the current component.
    ComponentList const& cl = this->Components[c];

    // Skip trivial components.
    if(cl.size() < 2)
      {
      continue;
      }

    // Make sure the component is all STATIC_LIBRARY targets.
    for(ComponentList::const_iterator ci = cl.begin(); ci != cl.end(); ++ci)
      {
      if(this->Targets[*ci]->GetType() != cmTarget::STATIC_LIBRARY)
        {
        this->ComplainAboutBadComponent(c);
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends::ComplainAboutBadComponent(int c)
{
  // Get the bad component.
  ComponentList const& cl = this->Components[c];

  // Construct the error message.
  cmOStringStream e;
  e << "The inter-target dependency graph contains the following "
    << "strongly connected component (cycle):\n";
  for(ComponentList::const_iterator ci = cl.begin(); ci != cl.end(); ++ci)
    {
    // Get the depender.
    int i = *ci;
    cmTarget* depender = this->Targets[i];

    // Describe the depender.
    e << "  " << depender->GetName() << " of type "
      << cmTarget::TargetTypeNames[depender->GetType()] << "\n";

    // List its dependencies that are inside the component.
    TargetDependList const& tdl = this->TargetDependGraph[i];
    for(TargetDependList::const_iterator di = tdl.begin();
        di != tdl.end(); ++di)
      {
      int j = *di;
      if(this->TarjanEntries[j].Component == c)
        {
        cmTarget* dependee = this->Targets[j];
        e << "    depends on " << dependee->GetName() << "\n";
        }
      }
    }
  e << "At least one of these targets is not a STATIC_LIBRARY.  "
    << "Cyclic dependencies are allowed only among static libraries.";
  cmSystemTools::Error(e.str().c_str());
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::ComputeFinalDepends()
{
  int n = static_cast<int>(this->TargetDependGraph.size());
  this->FinalDependGraph.resize(n);

  // Convert inter-component edges to connect component tails to heads.
  for(int i=0; i < n; ++i)
    {
    int depender_component = this->TarjanEntries[i].Component;
    int depender_component_tail =
      this->Components[depender_component].back();

    TargetDependList const& tdl = this->TargetDependGraph[i];
    for(TargetDependList::const_iterator di = tdl.begin();
        di != tdl.end(); ++di)
      {
      int j = *di;
      int dependee_component = this->TarjanEntries[j].Component;
      int dependee_component_head =
        this->Components[dependee_component].front();
      if(depender_component != dependee_component)
        {
        this->FinalDependGraph[depender_component_tail]
          .push_back(dependee_component_head);
        }
      }
    }

  // Compute intra-component edges.
  int nc = static_cast<int>(this->Components.size());
  for(int c=0; c < nc; ++c)
    {
    // Within the component each target depends on that following it.
    ComponentList const& cl = this->Components[c];
    ComponentList::const_iterator ci = cl.begin();
    int last_i = *ci;
    for(++ci; ci != cl.end(); ++ci)
      {
      int i = *ci;
      this->FinalDependGraph[last_i].push_back(i);
      last_i = i;
      }
    }
}
