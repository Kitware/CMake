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
#include "cmComputeLinkDepends.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"

#include <cmsys/stl/algorithm>

/*

This file computes an ordered list of link items to use when linking a
single target in one configuration.  Each link item is identified by
the string naming it.  A graph of dependencies is created in which
each node corresponds to one item and directed eges lead from nodes to
those which must *precede* them on the link line.  For example, the
graph

  C -> B -> A

will lead to the link line order

  A B C

The set of items placed in the graph is formed with a breadth-first
search of the link dependencies starting from the main target.

There are two types of items: those with known direct dependencies and
those without known dependencies.  We will call the two types "known
items" and "unknown items", respecitvely.  Known items are those whose
names correspond to targets (built or imported) and those for which an
old-style <item>_LIB_DEPENDS variable is defined.  All other items are
unknown and we must infer dependencies for them.

Known items have dependency lists ordered based on how the user
specified them.  We can use this order to infer potential dependencies
of unknown items.  For example, if link items A and B are unknown and
items X and Y are known, then we might have the following dependency
lists:

  X: Y A B
  Y: A B

The explicitly known dependencies form graph edges

  X <- Y  ,  X <- A  ,  X <- B  ,  Y <- A  ,  Y <- B

We can also infer the edge

  A <- B

because *every* time A appears B is seen on its right.  We do not know
whether A really needs symbols from B to link, but it *might* so we
must preserve their order.  This is the case also for the following
explict lists:

  X: A B Y
  Y: A B

Here, A is followed by the set {B,Y} in one list, and {B} in the other
list.  The intersection of these sets is {B}, so we can infer that A
depends on at most B.  Meanwhile B is followed by the set {Y} in one
list and {} in the other.  The intersection is {} so we can infer that
B has no dependencies.

Let's make a more complex example by adding unknown item C and
considering these dependency lists:

  X: A B Y C
  Y: A C B

The explicit edges are

  X <- Y  ,  X <- A  ,  X <- B  ,  X <- C  ,  Y <- A  ,  Y <- B  ,  Y <- C

For the unknown items, we infer dependencies by looking at the
"follow" sets:

  A: intersect( {B,Y,C} , {C,B} ) = {B,C} ; infer edges  A <- B  ,  A <- C
  B: intersect( {Y,C}   , {}    ) = {}    ; infer no edges
  C: intersect( {}      , {B}   ) = {}    ; infer no edges

Once the complete graph is formed from all known and inferred
dependencies, we walk the graph with a series of depth-first-searches
in order to emit link items.  When visiting a node all edges are
followed first because the neighbors must precede the item.  Once
neighbors across all edges have been emitted it is safe to emit the
current node.

If a single DFS returns to a node it previously reached then a cycle
is present.  Cyclic link dependencies are resolved simply by repeating
one of the cycle entries at the beginning and end of the cycle
members.  For example, the graph

  A <- B  ,  B <- C  ,  C <- A

can be satisfied with the link item list

  A B C A

When a node is reached a second time during the same DFS we make sure
its item has been emitted and then skip following its outgoing edges
again.

The initial exploration of dependencies using a BFS associates an
integer index with each link item.  When the graph is built outgoing
edges are sorted by this index.  This preserves the original link
order as much as possible subject to the dependencies.

*/

//----------------------------------------------------------------------------
cmComputeLinkDepends
::cmComputeLinkDepends(cmTarget* target, const char* config)
{
  // Store context information.
  this->Target = target;
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();

  // The configuration being linked.
  this->Config = config;

  // Enable debug mode if requested.
  this->DebugMode = this->Makefile->IsOn("CMAKE_LINK_DEPENDS_DEBUG_MODE");
}

//----------------------------------------------------------------------------
cmComputeLinkDepends::~cmComputeLinkDepends()
{
  for(std::vector<DependSetList*>::iterator
        i = this->InferredDependSets.begin();
      i != this->InferredDependSets.end(); ++i)
    {
    delete *i;
    }
}

//----------------------------------------------------------------------------
std::vector<cmComputeLinkDepends::LinkEntry> const&
cmComputeLinkDepends::Compute()
{
  // Follow the link dependencies of the target to be linked.
  this->AddTargetLinkEntries(-1, this->Target->GetOriginalLinkLibraries());

  // Complete the breadth-first search of dependencies.
  while(!this->BFSQueue.empty())
    {
    // Get the next entry.
    BFSEntry qe = this->BFSQueue.front();
    this->BFSQueue.pop();

    // Follow the entry's dependencies.
    this->FollowLinkEntry(qe);
    }

  // Complete the search of shared library dependencies.
  while(!this->SharedDepQueue.empty())
    {
    // Handle the next entry.
    this->HandleSharedDependency(this->SharedDepQueue.front());
    this->SharedDepQueue.pop();
    }

  // Infer dependencies of targets for which they were not known.
  this->InferDependencies();

  // Display the constraint graph.
  if(this->DebugMode)
    {
    fprintf(stderr,
            "---------------------------------------"
            "---------------------------------------\n");
    fprintf(stderr, "Link dependency analysis for target %s, config %s\n",
            this->Target->GetName(), this->Config?this->Config:"noconfig");
    this->DisplayConstraintGraph();
    }

  // Compute the final set of link entries.
  this->OrderLinkEntires();

  // Display the final set.
  if(this->DebugMode)
    {
    this->DisplayFinalEntries();
    }

  return this->FinalLinkEntries;
}

//----------------------------------------------------------------------------
std::map<cmStdString, int>::iterator
cmComputeLinkDepends::AllocateLinkEntry(std::string const& item)
{
  std::map<cmStdString, int>::value_type
    index_entry(item, static_cast<int>(this->EntryList.size()));
  std::map<cmStdString, int>::iterator
    lei = this->LinkEntryIndex.insert(index_entry).first;
  this->EntryList.push_back(LinkEntry());
  this->InferredDependSets.push_back(0);
  this->EntryConstraintGraph.push_back(EntryConstraintSet());
  return lei;
}

//----------------------------------------------------------------------------
int cmComputeLinkDepends::AddLinkEntry(std::string const& item)
{
  // Check if the item entry has already been added.
  std::map<cmStdString, int>::iterator lei = this->LinkEntryIndex.find(item);
  if(lei != this->LinkEntryIndex.end())
    {
    // Yes.  We do not need to follow the item's dependencies again.
    return lei->second;
    }

  // Allocate a spot for the item entry.
  lei = this->AllocateLinkEntry(item);

  // Initialize the item entry.
  int index = lei->second;
  LinkEntry& entry = this->EntryList[index];
  entry.Item = item;
  entry.Target = this->Makefile->FindTargetToUse(entry.Item.c_str());

  // If the item has dependencies queue it to follow them.
  if(entry.Target)
    {
    // Target dependencies are always known.  Follow them.
    BFSEntry qe = {index, 0};
    this->BFSQueue.push(qe);
    }
  else
    {
    // Look for an old-style <item>_LIB_DEPENDS variable.
    std::string var = entry.Item;
    var += "_LIB_DEPENDS";
    if(const char* val = this->Makefile->GetDefinition(var.c_str()))
      {
      // The item dependencies are known.  Follow them.
      BFSEntry qe = {index, val};
      this->BFSQueue.push(qe);
      }
    else
      {
      // The item dependencies are not known.  We need to infer them.
      this->InferredDependSets[index] = new DependSetList;
      }
    }

  return index;
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::FollowLinkEntry(BFSEntry const& qe)
{
  // Get this entry representation.
  int depender_index = qe.Index;
  LinkEntry const& entry = this->EntryList[depender_index];

  // Follow the item's dependencies.
  if(entry.Target)
    {
    // Follow the target dependencies.
    if(cmTargetLinkInterface const* interface =
       entry.Target->GetLinkInterface(this->Config))
      {
      // This target provides its own link interface information.
      this->AddLinkEntries(depender_index, interface->Libraries);

      // Handle dependent shared libraries.
      this->QueueSharedDependencies(depender_index, interface->SharedDeps);
      }
    else if(!entry.Target->IsImported() &&
            entry.Target->GetType() != cmTarget::EXECUTABLE)
      {
      // Use the target's link implementation as the interface.
      this->AddTargetLinkEntries(depender_index,
                                 entry.Target->GetOriginalLinkLibraries());
      }
    }
  else
    {
    // Follow the old-style dependency list.
    this->AddVarLinkEntries(depender_index, qe.LibDepends);
    }
}

//----------------------------------------------------------------------------
void
cmComputeLinkDepends
::QueueSharedDependencies(int depender_index,
                          std::vector<std::string> const& deps)
{
  for(std::vector<std::string>::const_iterator li = deps.begin();
      li != deps.end(); ++li)
    {
    SharedDepEntry qe;
    qe.Item = *li;
    qe.DependerIndex = depender_index;
    this->SharedDepQueue.push(qe);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::HandleSharedDependency(SharedDepEntry const& dep)
{
  // Check if the target already has an entry.
  std::map<cmStdString, int>::iterator lei =
    this->LinkEntryIndex.find(dep.Item);
  if(lei == this->LinkEntryIndex.end())
    {
    // Allocate a spot for the item entry.
    lei = this->AllocateLinkEntry(dep.Item);

    // Initialize the item entry.
    LinkEntry& entry = this->EntryList[lei->second];
    entry.Item = dep.Item;
    entry.Target = this->Makefile->FindTargetToUse(dep.Item.c_str());

    // This item was added specifically because it is a dependent
    // shared library.  It may get special treatment
    // in cmComputeLinkInformation.
    entry.IsSharedDep = true;
    }

  // Get the link entry for this target.
  int index = lei->second;
  LinkEntry& entry = this->EntryList[index];

  // This shared library dependency must be preceded by the item that
  // listed it.
  this->EntryConstraintGraph[index].insert(dep.DependerIndex);

  // Target items may have their own dependencies.
  if(entry.Target)
    {
    if(cmTargetLinkInterface const* interface =
       entry.Target->GetLinkInterface(this->Config))
      {
      // We use just the shared dependencies, not the interface.
      this->QueueSharedDependencies(index, interface->SharedDeps);
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::AddVarLinkEntries(int depender_index,
                                             const char* value)
{
  // This is called to add the dependencies named by
  // <item>_LIB_DEPENDS.  The variable contains a semicolon-separated
  // list.  The list contains link-type;item pairs and just items.
  std::vector<std::string> deplist;
  cmSystemTools::ExpandListArgument(value, deplist);

  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(this->Config && cmSystemTools::UpperCase(this->Config) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }

  // Look for entries meant for this configuration.
  std::vector<std::string> actual_libs;
  cmTarget::LinkLibraryType llt = cmTarget::GENERAL;
  for(std::vector<std::string>::const_iterator di = deplist.begin();
      di != deplist.end(); ++di)
    {
    if(*di == "debug")
      {
      llt = cmTarget::DEBUG;
      }
    else if(*di == "optimized")
      {
      llt = cmTarget::OPTIMIZED;
      }
    else if(*di == "general")
      {
      llt = cmTarget::GENERAL;
      }
    else if(!di->empty())
      {
      if(llt == cmTarget::GENERAL || llt == linkType)
        {
        actual_libs.push_back(*di);
        }
      linkType = cmTarget::GENERAL;
      }
    }

  // Add the entries from this list.
  this->AddLinkEntries(depender_index, actual_libs);
}

//----------------------------------------------------------------------------
void
cmComputeLinkDepends::AddTargetLinkEntries(int depender_index,
                                           LinkLibraryVectorType const& libs)
{
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(this->Config && cmSystemTools::UpperCase(this->Config) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }

  // Look for entries meant for this configuration.
  std::vector<std::string> actual_libs;
  for(cmTarget::LinkLibraryVectorType::const_iterator li = libs.begin();
      li != libs.end(); ++li)
    {
    if(li->second == cmTarget::GENERAL || li->second == linkType)
      {
      actual_libs.push_back(li->first);
      }
    }

  // Add these entries.
  this->AddLinkEntries(depender_index, actual_libs);
}

//----------------------------------------------------------------------------
void
cmComputeLinkDepends::AddLinkEntries(int depender_index,
                                     std::vector<std::string> const& libs)
{
  // Track inferred dependency sets implied by this list.
  std::map<int, DependSet> dependSets;

  // Loop over the libraries linked directly by the depender.
  for(std::vector<std::string>::const_iterator li = libs.begin();
      li != libs.end(); ++li)
    {
    // Skip entries that will resolve to the target getting linked or
    // are empty.
    if(*li == this->Target->GetName() || li->empty())
      {
      continue;
      }

    // Add a link entry for this item.
    int dependee_index = this->AddLinkEntry(*li);

    // The depender must come before the dependee.
    if(depender_index >= 0)
      {
      this->EntryConstraintGraph[dependee_index].insert(depender_index);
      }

    // Update the inferred dependencies for earlier items.
    for(std::map<int, DependSet>::iterator dsi = dependSets.begin();
        dsi != dependSets.end(); ++dsi)
      {
      if(dependee_index != dsi->first)
        {
        dsi->second.insert(dependee_index);
        }
      }

    // If this item needs to have dependencies inferred, do so.
    if(this->InferredDependSets[dependee_index])
      {
      // Make sure an entry exists to hold the set for the item.
      dependSets[dependee_index];
      }
    }

  // Store the inferred dependency sets discovered for this list.
  for(std::map<int, DependSet>::iterator dsi = dependSets.begin();
      dsi != dependSets.end(); ++dsi)
    {
    this->InferredDependSets[dsi->first]->push_back(dsi->second);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::InferDependencies()
{
  // The inferred dependency sets for each item list the possible
  // dependencies.  The intersection of the sets for one item form its
  // inferred dependencies.
  for(unsigned int depender_index=0;
      depender_index < this->InferredDependSets.size(); ++depender_index)
    {
    // Skip items for which dependencies do not need to be inferred or
    // for which the inferred dependency sets are empty.
    DependSetList* sets = this->InferredDependSets[depender_index];
    if(!sets || sets->empty())
      {
      continue;
      }

    // Intersect the sets for this item.
    DependSetList::const_iterator i = sets->begin();
    DependSet common = *i;
    for(++i; i != sets->end(); ++i)
      {
      DependSet intersection;
      cmsys_stl::set_intersection
        (common.begin(), common.end(), i->begin(), i->end(),
         std::inserter(intersection, intersection.begin()));
      common = intersection;
      }

    // Add the inferred dependencies to the graph.
    for(DependSet::const_iterator j = common.begin(); j != common.end(); ++j)
      {
      int dependee_index = *j;
      this->EntryConstraintGraph[dependee_index].insert(depender_index);
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::DisplayConstraintGraph()
{
  // Display the conflict graph.
  cmOStringStream e;
  for(unsigned int i=0; i < this->EntryConstraintGraph.size(); ++i)
    {
    EntryConstraintSet const& cset = this->EntryConstraintGraph[i];
    e << "item " << i << " is [" << this->EntryList[i].Item << "]\n";
    for(EntryConstraintSet::const_iterator j = cset.begin();
        j != cset.end(); ++j)
      {
      e << "  item " << *j << " must precede it\n";
      }
    }
  fprintf(stderr, "%s\n", e.str().c_str());
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::OrderLinkEntires()
{
  // Setup visit tracking.
  this->EntryVisited.resize(this->EntryList.size(), 0);
  this->WalkId = 0;

  // Start a DFS from every entry.
  for(unsigned int i=0; i < this->EntryList.size(); ++i)
    {
    ++this->WalkId;
    this->VisitLinkEntry(i);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::VisitLinkEntry(unsigned int i)
{
  // Check if the node has already been visited.
  if(this->EntryVisited[i])
    {
    if(this->EntryVisited[i] == this->WalkId)
      {
      // We have reached a node previously visited on this DFS.  There
      // is a cycle.  In order to allow linking with cyclic
      // dependencies we make sure the item is emitted but do not
      // follow its outgoing edges again.
      if(this->EntryEmitted.insert(i).second)
        {
        // The item has not been previously emitted so we emit it now.
        // It will be emitted again when the stack unwinds back up to
        // the beginning of the cycle.
        this->FinalLinkEntries.push_back(this->EntryList[i]);
        }
      }
    return;
    }

  // We are now visiting this node so mark it.
  this->EntryVisited[i] = this->WalkId;

  // Visit the neighbors of the node first.
  EntryConstraintSet const& cset = this->EntryConstraintGraph[i];
  for(EntryConstraintSet::const_iterator j = cset.begin();
      j != cset.end(); ++j)
    {
    this->VisitLinkEntry(*j);
    }

  // Now that all items required to come before this one have been
  // emmitted, emit this item.
  this->EntryEmitted.insert(i);
  this->FinalLinkEntries.push_back(this->EntryList[i]);
}

//----------------------------------------------------------------------------
void cmComputeLinkDepends::DisplayFinalEntries()
{
  fprintf(stderr, "target [%s] links to:\n", this->Target->GetName());
  for(std::vector<LinkEntry>::const_iterator lei =
        this->FinalLinkEntries.begin();
      lei != this->FinalLinkEntries.end(); ++lei)
    {
    if(lei->Target)
      {
      fprintf(stderr, "  target [%s]\n", lei->Target->GetName());
      }
    else
      {
      fprintf(stderr, "  item [%s]\n", lei->Item.c_str());
      }
    }
  fprintf(stderr, "\n");
}
