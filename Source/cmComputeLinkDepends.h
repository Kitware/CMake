/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "cmGraphAdjacencyList.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmTargetLinkLibraryType.h"

class cmComputeComponentGraph;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmake;

/** \class cmComputeLinkDepends
 * \brief Compute link dependencies for targets.
 */
class cmComputeLinkDepends
{
public:
  cmComputeLinkDepends(cmGeneratorTarget const* target,
                       const std::string& config);
  ~cmComputeLinkDepends();

  cmComputeLinkDepends(const cmComputeLinkDepends&) = delete;
  cmComputeLinkDepends& operator=(const cmComputeLinkDepends&) = delete;

  // Basic information about each link item.
  struct LinkEntry
  {
    BT<std::string> Item;
    cmGeneratorTarget const* Target = nullptr;
    bool IsSharedDep = false;
    bool IsFlag = false;
  };

  using EntryVector = std::vector<LinkEntry>;
  EntryVector const& Compute();

  void SetOldLinkDirMode(bool b);
  std::set<cmGeneratorTarget const*> const& GetOldWrongConfigItems() const
  {
    return this->OldWrongConfigItems;
  }

private:
  // Context information.
  cmGeneratorTarget const* Target;
  cmMakefile* Makefile;
  cmGlobalGenerator const* GlobalGenerator;
  cmake* CMakeInstance;
  std::string Config;
  EntryVector FinalLinkEntries;

  std::map<cmLinkItem, int>::iterator AllocateLinkEntry(
    cmLinkItem const& item);
  int AddLinkEntry(cmLinkItem const& item);
  void AddVarLinkEntries(int depender_index, const char* value);
  void AddDirectLinkEntries();
  template <typename T>
  void AddLinkEntries(int depender_index, std::vector<T> const& libs);
  cmLinkItem ResolveLinkItem(int depender_index, const std::string& name);

  // One entry for each unique item.
  std::vector<LinkEntry> EntryList;
  std::map<cmLinkItem, int> LinkEntryIndex;

  // BFS of initial dependencies.
  struct BFSEntry
  {
    int Index;
    const char* LibDepends;
  };
  std::queue<BFSEntry> BFSQueue;
  void FollowLinkEntry(BFSEntry qe);

  // Shared libraries that are included only because they are
  // dependencies of other shared libraries, not because they are part
  // of the interface.
  struct SharedDepEntry
  {
    cmLinkItem Item;
    int DependerIndex;
  };
  std::queue<SharedDepEntry> SharedDepQueue;
  std::set<int> SharedDepFollowed;
  void FollowSharedDeps(int depender_index, cmLinkInterface const* iface,
                        bool follow_interface = false);
  void QueueSharedDependencies(int depender_index,
                               std::vector<cmLinkItem> const& deps);
  void HandleSharedDependency(SharedDepEntry const& dep);

  // Dependency inferral for each link item.
  struct DependSet : public std::set<int>
  {
  };
  struct DependSetList : public std::vector<DependSet>
  {
    bool Initialized = false;
  };
  std::vector<DependSetList> InferredDependSets;
  void InferDependencies();

  // Ordering constraint graph adjacency list.
  using NodeList = cmGraphNodeList;
  using EdgeList = cmGraphEdgeList;
  using Graph = cmGraphAdjacencyList;
  Graph EntryConstraintGraph;
  void CleanConstraintGraph();
  void DisplayConstraintGraph();

  // Ordering algorithm.
  void OrderLinkEntires();
  std::vector<char> ComponentVisited;
  std::vector<int> ComponentOrder;

  struct PendingComponent
  {
    // The real component id.  Needed because the map is indexed by
    // component topological index.
    int Id;

    // The number of times the component needs to be seen.  This is
    // always 1 for trivial components and is initially 2 for
    // non-trivial components.
    int Count;

    // The entries yet to be seen to complete the component.
    std::set<int> Entries;
  };
  std::map<int, PendingComponent> PendingComponents;
  std::unique_ptr<cmComputeComponentGraph> CCG;
  std::vector<int> FinalLinkOrder;
  void DisplayComponents();
  void VisitComponent(unsigned int c);
  void VisitEntry(int index);
  PendingComponent& MakePendingComponent(unsigned int component);
  int ComputeComponentCount(NodeList const& nl);
  void DisplayFinalEntries();

  // Record of the original link line.
  std::vector<int> OriginalEntries;
  std::set<cmGeneratorTarget const*> OldWrongConfigItems;
  void CheckWrongConfigItem(cmLinkItem const& item);

  int ComponentOrderId;
  cmTargetLinkLibraryType LinkType;
  bool HasConfig;
  bool DebugMode;
  bool OldLinkDirMode;
};
