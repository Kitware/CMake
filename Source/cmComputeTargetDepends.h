/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmComputeTargetDepends_h
#define cmComputeTargetDepends_h

#include "cmStandardIncludes.h"

#include "cmGraphAdjacencyList.h"

#include <stack>

class cmComputeComponentGraph;
class cmGlobalGenerator;
class cmTarget;
class cmTargetDependSet;

/** \class cmComputeTargetDepends
 * \brief Compute global interdependencies among targets.
 *
 * Static libraries may form cycles in the target dependency graph.
 * This class evaluates target dependencies globally and adjusts them
 * to remove cycles while preserving a safe build order.
 */
class cmComputeTargetDepends
{
public:
  cmComputeTargetDepends(cmGlobalGenerator* gg);
  ~cmComputeTargetDepends();

  bool Compute();

  std::vector<cmTarget*> const& GetTargets() const { return this->Targets; }
  void GetTargetDirectDepends(cmTarget* t, cmTargetDependSet& deps);
private:
  void CollectTargets();
  void CollectDepends();
  void CollectTargetDepends(int depender_index);
  void AddTargetDepend(int depender_index, const char* dependee_name,
                       bool linking);
  void AddTargetDepend(int depender_index, cmTarget* dependee, bool linking);
  bool ComputeFinalDepends(cmComputeComponentGraph const& ccg);
  void AddInterfaceDepends(int depender_index, const char* dependee_name,
                           bool linking, std::set<cmStdString> &emitted);
  void AddInterfaceDepends(int depender_index, cmTarget* dependee,
                           const char *config,
                           std::set<cmStdString> &emitted);
  cmGlobalGenerator* GlobalGenerator;
  bool DebugMode;
  bool NoCycles;

  // Collect all targets.
  std::vector<cmTarget*> Targets;
  std::map<cmTarget*, int> TargetIndex;

  // Represent the target dependency graph.  The entry at each
  // top-level index corresponds to a depender whose dependencies are
  // listed.
  typedef cmGraphNodeList NodeList;
  typedef cmGraphEdgeList EdgeList;
  typedef cmGraphAdjacencyList Graph;
  Graph InitialGraph;
  Graph FinalGraph;
  void DisplayGraph(Graph const& graph, const char* name);

  // Deal with connected components.
  void DisplayComponents(cmComputeComponentGraph const& ccg);
  bool CheckComponents(cmComputeComponentGraph const& ccg);
  void ComplainAboutBadComponent(cmComputeComponentGraph const& ccg, int c,
                                 bool strong = false);

  std::vector<int> ComponentHead;
  std::vector<int> ComponentTail;
  bool IntraComponent(std::vector<int> const& cmap, int c, int i, int* head,
                      std::set<int>& emitted, std::set<int>& visited);
};

#endif
