/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmComputeTargetDepends_h
#define cmComputeTargetDepends_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmGraphAdjacencyList.h"
#include "cmListFileCache.h"

class cmComputeComponentGraph;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmLinkItem;
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
  void CollectTargets();
  void CollectDepends();
  void CollectTargetDepends(int depender_index);
  void AddTargetDepend(int depender_index, cmLinkItem const& dependee_name,
                       bool linking, bool cross);
  void AddTargetDepend(int depender_index, cmGeneratorTarget const* dependee,
                       cmListFileBacktrace const& dependee_backtrace,
                       bool linking, bool cross);
  bool ComputeFinalDepends(cmComputeComponentGraph const& ccg);
  void AddInterfaceDepends(int depender_index, cmLinkItem const& dependee_name,
                           const std::string& config,
                           std::set<cmLinkItem>& emitted);
  void AddInterfaceDepends(int depender_index,
                           cmGeneratorTarget const* dependee,
                           cmListFileBacktrace const& dependee_backtrace,
                           const std::string& config,
                           std::set<cmLinkItem>& emitted);
  cmGlobalGenerator* GlobalGenerator;
  bool DebugMode;
  bool NoCycles;

  // Collect all targets.
  std::vector<cmGeneratorTarget const*> Targets;
  std::map<cmGeneratorTarget const*, int> TargetIndex;

  // Represent the target dependency graph.  The entry at each
  // top-level index corresponds to a depender whose dependencies are
  // listed.
  using NodeList = cmGraphNodeList;
  using EdgeList = cmGraphEdgeList;
  using Graph = cmGraphAdjacencyList;
  Graph InitialGraph;
  Graph FinalGraph;
  void DisplayGraph(Graph const& graph, const std::string& name);

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
