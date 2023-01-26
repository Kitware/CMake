/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmGraphAdjacencyList.h"

class cmComputeComponentGraph;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmLinkItem;
class cmListFileBacktrace;
class cmSourceFile;
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

  std::vector<cmGeneratorTarget const*> const& GetTargets() const
  {
    return this->Targets;
  }
  void GetTargetDirectDepends(cmGeneratorTarget const* t,
                              cmTargetDependSet& deps);

private:
  struct TargetSideEffects
  {
    std::set<cmGeneratorTarget const*> CustomCommandSideEffects;
    std::map<std::string, std::set<cmGeneratorTarget const*>>
      LanguageSideEffects;
  };

  void CollectTargets();
  void CollectDepends();
  void CollectTargetDepends(size_t depender_index);
  void AddTargetDepend(size_t depender_index, cmLinkItem const& dependee_name,
                       bool linking, bool cross);
  void AddTargetDepend(size_t depender_index,
                       cmGeneratorTarget const* dependee,
                       cmListFileBacktrace const& dependee_backtrace,
                       bool linking, bool cross);
  void CollectSideEffects();
  void CollectSideEffectsForTarget(std::set<size_t>& visited,
                                   size_t depender_index);
  void ComputeIntermediateGraph();
  void OptimizeLinkDependencies(cmGeneratorTarget const* gt,
                                cmGraphEdgeList& outputEdges,
                                cmGraphEdgeList const& inputEdges);
  bool ComputeFinalDepends(cmComputeComponentGraph const& ccg);
  void AddInterfaceDepends(size_t depender_index,
                           cmLinkItem const& dependee_name,
                           const std::string& config,
                           std::set<cmLinkItem>& emitted);
  void AddInterfaceDepends(size_t depender_index,
                           cmGeneratorTarget const* dependee,
                           cmListFileBacktrace const& dependee_backtrace,
                           const std::string& config,
                           std::set<cmLinkItem>& emitted);
  void AddObjectDepends(size_t depender_index, cmSourceFile const* o,
                        std::set<cmLinkItem>& emitted);
  cmGlobalGenerator* GlobalGenerator;
  bool DebugMode;
  bool NoCycles;

  // Collect all targets.
  std::vector<cmGeneratorTarget const*> Targets;
  std::map<cmGeneratorTarget const*, size_t> TargetIndex;

  // Represent the target dependency graph.  The entry at each
  // top-level index corresponds to a depender whose dependencies are
  // listed.
  using NodeList = cmGraphNodeList;
  using EdgeList = cmGraphEdgeList;
  using Graph = cmGraphAdjacencyList;
  Graph InitialGraph;
  Graph IntermediateGraph;
  Graph FinalGraph;
  std::vector<TargetSideEffects> SideEffects;
  void DisplayGraph(Graph const& graph, const std::string& name);
  void DisplaySideEffects();

  // Deal with connected components.
  void DisplayComponents(cmComputeComponentGraph const& ccg,
                         const std::string& name);
  bool CheckComponents(cmComputeComponentGraph const& ccg);
  void ComplainAboutBadComponent(cmComputeComponentGraph const& ccg, size_t c,
                                 bool strong = false);

  std::vector<size_t> ComponentHead;
  std::vector<size_t> ComponentTail;
  bool IntraComponent(std::vector<size_t> const& cmap, size_t c, size_t i,
                      size_t* head, std::set<size_t>& emitted,
                      std::set<size_t>& visited);
};
