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
  if(!this->ComputeFinalDepends(ccg))
    {
    return false;
    }
  if(this->DebugMode)
    {
    this->DisplayGraph(this->FinalGraph, "final");
    }

  return true;
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends::GetTargetDirectDepends(cmTarget* t,
                                               cmTargetDependSet& deps)
{
  // Lookup the index for this target.  All targets should be known by
  // this point.
  std::map<cmTarget*, int>::const_iterator tii = this->TargetIndex.find(t);
  assert(tii != this->TargetIndex.end());
  int i = tii->second;

  // Get its final dependencies.
  EdgeList const& nl = this->FinalGraph[i];
  for(EdgeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
    {
    cmTarget* dep = this->Targets[*ni];
    cmTargetDependSet::iterator di = deps.insert(dep).first;
    di->SetType(ni->IsStrong());
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

  // Loop over all targets linked directly in all configs.
  // We need to make targets depend on the union of all config-specific
  // dependencies in all targets, because the generated build-systems can't
  // deal with config-specific dependencies.
  {
  std::set<cmStdString> emitted;
  {
  std::vector<std::string> tlibs;
  if (depender->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    // For INTERFACE_LIBRARY depend on the interface instead.
    depender->GetInterfaceLinkLibraries(0, tlibs, depender);
    }
  else
    {
    depender->GetDirectLinkLibraries(0, tlibs, depender);
    }
  // A target should not depend on itself.
  emitted.insert(depender->GetName());
  for(std::vector<std::string>::const_iterator lib = tlibs.begin();
      lib != tlibs.end(); ++lib)
    {
    // Don't emit the same library twice for this target.
    if(emitted.insert(*lib).second)
      {
      this->AddTargetDepend(depender_index, lib->c_str(), true);
      this->AddInterfaceDepends(depender_index, lib->c_str(),
                                true, emitted);
      }
    }
  }
  std::vector<std::string> configs;
  depender->GetMakefile()->GetConfigurations(configs);
  for (std::vector<std::string>::const_iterator it = configs.begin();
    it != configs.end(); ++it)
    {
    std::vector<std::string> tlibs;
    depender->GetDirectLinkLibraries(it->c_str(), tlibs, depender);
    // A target should not depend on itself.
    emitted.insert(depender->GetName());
    for(std::vector<std::string>::const_iterator lib = tlibs.begin();
        lib != tlibs.end(); ++lib)
      {
      // Don't emit the same library twice for this target.
      if(emitted.insert(*lib).second)
        {
        this->AddTargetDepend(depender_index, lib->c_str(), true);
        this->AddInterfaceDepends(depender_index, lib->c_str(),
                                  true, emitted);
        }
      }
    }
  }

  // Loop over all utility dependencies.
  {
  std::set<cmStdString> const& tutils = depender->GetUtilities();
  std::set<cmStdString> emitted;
  // A target should not depend on itself.
  emitted.insert(depender->GetName());
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
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::AddInterfaceDepends(int depender_index,
                                                 cmTarget* dependee,
                                                 const char *config,
                                               std::set<cmStdString> &emitted)
{
  cmTarget* depender = this->Targets[depender_index];
  if(cmTarget::LinkInterface const* iface =
                                dependee->GetLinkInterface(config, depender))
    {
    for(std::vector<std::string>::const_iterator
        lib = iface->Libraries.begin();
        lib != iface->Libraries.end(); ++lib)
      {
      // Don't emit the same library twice for this target.
      if(emitted.insert(*lib).second)
        {
        this->AddTargetDepend(depender_index, lib->c_str(), true);
        this->AddInterfaceDepends(depender_index, lib->c_str(),
                                  true, emitted);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::AddInterfaceDepends(int depender_index,
                                             const char* dependee_name,
                                             bool linking,
                                             std::set<cmStdString> &emitted)
{
  cmTarget* depender = this->Targets[depender_index];
  cmTarget* dependee =
    depender->GetMakefile()->FindTargetToUse(dependee_name);
  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if(linking && dependee &&
     dependee->GetType() == cmTarget::EXECUTABLE &&
     !dependee->IsExecutableWithExports())
    {
    dependee = 0;
    }

  if(dependee)
    {
    this->AddInterfaceDepends(depender_index, dependee, 0, emitted);
    std::vector<std::string> configs;
    depender->GetMakefile()->GetConfigurations(configs);
    for (std::vector<std::string>::const_iterator it = configs.begin();
      it != configs.end(); ++it)
      {
      // A target should not depend on itself.
      emitted.insert(depender->GetName());
      this->AddInterfaceDepends(depender_index, dependee,
                                it->c_str(), emitted);
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
    depender->GetMakefile()->FindTargetToUse(dependee_name);

  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if(linking && dependee &&
     dependee->GetType() == cmTarget::EXECUTABLE &&
     !dependee->IsExecutableWithExports())
    {
    dependee = 0;
    }

  if(dependee)
    {
    this->AddTargetDepend(depender_index, dependee, linking);
    }
}

//----------------------------------------------------------------------------
void cmComputeTargetDepends::AddTargetDepend(int depender_index,
                                             cmTarget* dependee,
                                             bool linking)
{
  if(dependee->IsImported())
    {
    // Skip imported targets but follow their utility dependencies.
    std::set<cmStdString> const& utils = dependee->GetUtilities();
    for(std::set<cmStdString>::const_iterator i = utils.begin();
        i != utils.end(); ++i)
      {
      if(cmTarget* transitive_dependee =
         dependee->GetMakefile()->FindTargetToUse(i->c_str()))
        {
        this->AddTargetDepend(depender_index, transitive_dependee, false);
        }
      }
    }
  else
    {
    // Lookup the index for this target.  All targets should be known by
    // this point.
    std::map<cmTarget*, int>::const_iterator tii =
      this->TargetIndex.find(dependee);
    assert(tii != this->TargetIndex.end());
    int dependee_index = tii->second;

    // Add this entry to the dependency graph.
    this->InitialGraph[depender_index].push_back(
      cmGraphEdge(dependee_index, !linking));
    }
}

//----------------------------------------------------------------------------
void
cmComputeTargetDepends::DisplayGraph(Graph const& graph, const char* name)
{
  fprintf(stderr, "The %s target dependency graph is:\n", name);
  int n = static_cast<int>(graph.size());
  for(int depender_index = 0; depender_index < n; ++depender_index)
    {
    EdgeList const& nl = graph[depender_index];
    cmTarget* depender = this->Targets[depender_index];
    fprintf(stderr, "target %d is [%s]\n",
            depender_index, depender->GetName());
    for(EdgeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int dependee_index = *ni;
      cmTarget* dependee = this->Targets[dependee_index];
      fprintf(stderr, "  depends on target %d [%s] (%s)\n", dependee_index,
              dependee->GetName(), ni->IsStrong()? "strong" : "weak");
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
::ComplainAboutBadComponent(cmComputeComponentGraph const& ccg, int c,
                            bool strong)
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
      << cmTarget::GetTargetTypeName(depender->GetType()) << "\n";

    // List its dependencies that are inside the component.
    EdgeList const& nl = this->InitialGraph[i];
    for(EdgeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int j = *ni;
      if(cmap[j] == c)
        {
        cmTarget* dependee = this->Targets[j];
        e << "    depends on \"" << dependee->GetName() << "\""
          << " (" << (ni->IsStrong()? "strong" : "weak") << ")\n";
        }
      }
    }
  if(strong)
    {
    // Custom command executable dependencies cannot occur within a
    // component of static libraries.  The cycle must appear in calls
    // to add_dependencies.
    e << "The component contains at least one cycle consisting of strong "
      << "dependencies (created by add_dependencies) that cannot be broken.";
    }
  else if(this->NoCycles)
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
bool
cmComputeTargetDepends
::IntraComponent(std::vector<int> const& cmap, int c, int i, int* head,
                 std::set<int>& emitted, std::set<int>& visited)
{
  if(!visited.insert(i).second)
    {
    // Cycle in utility depends!
    return false;
    }
  if(emitted.insert(i).second)
    {
    // Honor strong intra-component edges in the final order.
    EdgeList const& el = this->InitialGraph[i];
    for(EdgeList::const_iterator ei = el.begin(); ei != el.end(); ++ei)
      {
      int j = *ei;
      if(cmap[j] == c && ei->IsStrong())
        {
        this->FinalGraph[i].push_back(cmGraphEdge(j, true));
        if(!this->IntraComponent(cmap, c, j, head, emitted, visited))
          {
          return false;
          }
        }
      }

    // Prepend to a linear linked-list of intra-component edges.
    if(*head >= 0)
      {
      this->FinalGraph[i].push_back(cmGraphEdge(*head, false));
      }
    else
      {
      this->ComponentTail[c] = i;
      }
    *head = i;
    }
  return true;
}

//----------------------------------------------------------------------------
bool
cmComputeTargetDepends
::ComputeFinalDepends(cmComputeComponentGraph const& ccg)
{
  // Get the component graph information.
  std::vector<NodeList> const& components = ccg.GetComponents();
  Graph const& cgraph = ccg.GetComponentGraph();

  // Allocate the final graph.
  this->FinalGraph.resize(0);
  this->FinalGraph.resize(this->InitialGraph.size());

  // Choose intra-component edges to linearize dependencies.
  std::vector<int> const& cmap = ccg.GetComponentMap();
  this->ComponentHead.resize(components.size());
  this->ComponentTail.resize(components.size());
  int nc = static_cast<int>(components.size());
  for(int c=0; c < nc; ++c)
    {
    int head = -1;
    std::set<int> emitted;
    NodeList const& nl = components[c];
    for(NodeList::const_reverse_iterator ni = nl.rbegin();
        ni != nl.rend(); ++ni)
      {
      std::set<int> visited;
      if(!this->IntraComponent(cmap, c, *ni, &head, emitted, visited))
        {
        // Cycle in add_dependencies within component!
        this->ComplainAboutBadComponent(ccg, c, true);
        return false;
        }
      }
    this->ComponentHead[c] = head;
    }

  // Convert inter-component edges to connect component tails to heads.
  int n = static_cast<int>(cgraph.size());
  for(int depender_component=0; depender_component < n; ++depender_component)
    {
    int depender_component_tail = this->ComponentTail[depender_component];
    EdgeList const& nl = cgraph[depender_component];
    for(EdgeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int dependee_component = *ni;
      int dependee_component_head = this->ComponentHead[dependee_component];
      this->FinalGraph[depender_component_tail]
        .push_back(cmGraphEdge(dependee_component_head, ni->IsStrong()));
      }
    }
  return true;
}
