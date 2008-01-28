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

#include <queue>

class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmTarget;

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
    LinkEntry(): Item(), Target(0) {}
    LinkEntry(LinkEntry const& r): Item(r.Item), Target(r.Target) {}
  };

  typedef std::vector<LinkEntry> EntryVector;
  EntryVector const& Compute();

private:

  // Context information.
  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator* GlobalGenerator;
  bool DebugMode;

  // Configuration information.
  const char* Config;

  // Output information.
  EntryVector FinalLinkEntries;

  typedef cmTarget::LinkLibraryVectorType LinkLibraryVectorType;

  int AddLinkEntry(std::string const& item);
  void AddImportedLinkEntries(int depender_index, cmTarget* target);
  void AddVarLinkEntries(int depender_index, const char* value);
  void AddTargetLinkEntries(int depender_index,
                            LinkLibraryVectorType const& libs);
  void AddLinkEntries(int depender_index,
                      std::vector<std::string> const& libs);

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

  // Dependency inferral for each link item.
  struct DependSet: public std::set<int> {};
  struct DependSetList: public std::vector<DependSet> {};
  std::vector<DependSetList*> InferredDependSets;
  void InferDependencies();

  // Ordering constraint graph adjacency list.
  struct EntryConstraintSet: public std::set<int> {};
  std::vector<EntryConstraintSet> EntryConstraintGraph;
  void DisplayConstraintGraph();

  // Ordering algorithm.
  std::vector<int> EntryVisited;
  std::set<int> EntryEmitted;
  int WalkId;
  void OrderLinkEntires();
  void VisitLinkEntry(unsigned int i);
  void DisplayFinalEntries();
};

#endif
