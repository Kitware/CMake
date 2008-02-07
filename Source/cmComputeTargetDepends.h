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
#ifndef cmComputeTargetDepends_h
#define cmComputeTargetDepends_h

#include "cmStandardIncludes.h"

#include "cmGraphAdjacencyList.h"

#include <stack>

class cmComputeComponentGraph;
class cmGlobalGenerator;
class cmTarget;

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
  void GetTargetDirectDepends(cmTarget* t, std::set<cmTarget*>& deps);
private:
  void CollectTargets();
  void CollectDepends();
  void CollectTargetDepends(int depender_index);
  void AddTargetDepend(int depender_index, const char* dependee_name);
  void ComputeFinalDepends(cmComputeComponentGraph const& ccg);

  cmGlobalGenerator* GlobalGenerator;
  bool DebugMode;

  // Collect all targets.
  std::vector<cmTarget*> Targets;
  std::map<cmTarget*, int> TargetIndex;

  // Represent the target dependency graph.  The entry at each
  // top-level index corresponds to a depender whose dependencies are
  // listed.
  typedef cmGraphNodeList NodeList;
  typedef cmGraphAdjacencyList Graph;
  Graph InitialGraph;
  Graph FinalGraph;
  void DisplayGraph(Graph const& graph, const char* name);

  // Deal with connected components.
  void DisplayComponents(cmComputeComponentGraph const& ccg);
  bool CheckComponents(cmComputeComponentGraph const& ccg);
  void ComplainAboutBadComponent(cmComputeComponentGraph const& ccg, int c);
};

#endif
