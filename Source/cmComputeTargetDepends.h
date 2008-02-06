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

#include <stack>

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
  void ComputeFinalDepends();

  cmGlobalGenerator* GlobalGenerator;
  bool DebugMode;

  // Collect all targets.
  std::vector<cmTarget*> Targets;
  std::map<cmTarget*, int> TargetIndex;

  // Represent the target dependency graph.  The entry at each
  // top-level index corresponds to a depender whose dependencies are
  // listed.
  struct TargetDependList: public std::vector<int> {};
  std::vector<TargetDependList> TargetDependGraph;
  std::vector<TargetDependList> FinalDependGraph;
  void DisplayGraph(std::vector<TargetDependList> const& graph,
                    const char* name);

  // Tarjan's algorithm.
  struct TarjanEntry
  {
    int Root;
    int Component;
    int VisitIndex;
  };
  int TarjanWalkId;
  std::vector<int> TarjanVisited;
  std::vector<TarjanEntry> TarjanEntries;
  std::stack<int> TarjanStack;
  int TarjanIndex;
  void Tarjan();
  void TarjanVisit(int i);

  // Connected components.
  struct ComponentList: public std::vector<int> {};
  std::vector<ComponentList> Components;
  void DisplayComponents();
  bool CheckComponents();
  void ComplainAboutBadComponent(int c);
};

#endif
