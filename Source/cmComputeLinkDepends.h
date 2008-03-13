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
#ifndef cmComputeLinkDepends_h
#define cmComputeLinkDepends_h

#include "cmStandardIncludes.h"
#include "cmTarget.h"

#include "cmGraphAdjacencyList.h"

#include <queue>

class cmComputeComponentGraph;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmTarget;
class cmake;

/** \class cmComputeLinkDepends
 * \brief Compute link dependencies for targets.
 */
class cmComputeLinkDepends
{
public:
  cmComputeLinkDepends(cmTarget* target, const char* config);
  ~cmComputeLinkDepends();

  // Basic information about each link item.
  struct LinkEntry
  {
    std::string Item;
    cmTarget* Target;
    bool IsSharedDep;
    LinkEntry(): Item(), Target(0), IsSharedDep(false) {}
    LinkEntry(LinkEntry const& r):
      Item(r.Item), Target(r.Target), IsSharedDep(r.IsSharedDep) {}
  };

  typedef std::vector<LinkEntry> EntryVector;
  EntryVector const& Compute();

private:

  // Context information.
  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator* GlobalGenerator;
  cmake* CMakeInstance;
  bool DebugMode;

  // Configuration information.
  const char* Config;

  // Output information.
  EntryVector FinalLinkEntries;

  typedef cmTarget::LinkLibraryVectorType LinkLibraryVectorType;

  std::map<cmStdString, int>::iterator
  AllocateLinkEntry(std::string const& item);
  int AddLinkEntry(std::string const& item);
  void AddVarLinkEntries(int depender_index, const char* value);
  void AddTargetLinkEntries(int depender_index,
                            LinkLibraryVectorType const& libs);
  void AddLinkEntries(int depender_index,
                      std::vector<std::string> const& libs);
  std::string CleanItemName(std::string const& item);

  // One entry for each unique item.
  std::vector<LinkEntry> EntryList;
  std::map<cmStdString, int> LinkEntryIndex;

  // BFS of initial dependencies.
  struct BFSEntry
  {
    int Index;
    const char* LibDepends;
  };
  std::queue<BFSEntry> BFSQueue;
  void FollowLinkEntry(BFSEntry const&);

  // Shared libraries that are included only because they are
  // dependencies of other shared libraries, not because they are part
  // of the interface.
  struct SharedDepEntry
  {
    std::string Item;
    int DependerIndex;
  };
  std::queue<SharedDepEntry> SharedDepQueue;
  void QueueSharedDependencies(int depender_index,
                               std::vector<std::string> const& deps);
  void HandleSharedDependency(SharedDepEntry const& dep);

  // Dependency inferral for each link item.
  struct DependSet: public std::set<int> {};
  struct DependSetList: public std::vector<DependSet> {};
  std::vector<DependSetList*> InferredDependSets;
  void InferDependencies();

  // Ordering constraint graph adjacency list.
  typedef cmGraphNodeList NodeList;
  typedef cmGraphAdjacencyList Graph;
  Graph EntryConstraintGraph;
  void CleanConstraintGraph();
  void DisplayConstraintGraph();

  // Ordering algorithm.
  void OrderLinkEntires();
  std::vector<char> ComponentVisited;
  void DisplayComponents(cmComputeComponentGraph const& ccg);
  void VisitComponent(cmComputeComponentGraph const& ccg, unsigned int i);
  void EmitComponent(NodeList const& nl);
  void DisplayFinalEntries();
};

#endif
