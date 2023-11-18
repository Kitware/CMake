/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmComputeLinkDepends.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmComputeComponentGraph.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

/*

This file computes an ordered list of link items to use when linking a
single target in one configuration.  Each link item is identified by
the string naming it.  A graph of dependencies is created in which
each node corresponds to one item and directed edges lead from nodes to
those which must *follow* them on the link line.  For example, the
graph

  A -> B -> C

will lead to the link line order

  A B C

The set of items placed in the graph is formed with a breadth-first
search of the link dependencies starting from the main target.

There are two types of items: those with known direct dependencies and
those without known dependencies.  We will call the two types "known
items" and "unknown items", respectively.  Known items are those whose
names correspond to targets (built or imported) and those for which an
old-style <item>_LIB_DEPENDS variable is defined.  All other items are
unknown and we must infer dependencies for them.  For items that look
like flags (beginning with '-') we trivially infer no dependencies,
and do not include them in the dependencies of other items.

Known items have dependency lists ordered based on how the user
specified them.  We can use this order to infer potential dependencies
of unknown items.  For example, if link items A and B are unknown and
items X and Y are known, then we might have the following dependency
lists:

  X: Y A B
  Y: A B

The explicitly known dependencies form graph edges

  X -> Y  ,  X -> A  ,  X -> B  ,  Y -> A  ,  Y -> B

We can also infer the edge

  A -> B

because *every* time A appears B is seen on its right.  We do not know
whether A really needs symbols from B to link, but it *might* so we
must preserve their order.  This is the case also for the following
explicit lists:

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

  X -> Y  ,  X -> A  ,  X -> B  ,  X -> C  ,  Y -> A  ,  Y -> B  ,  Y -> C

For the unknown items, we infer dependencies by looking at the
"follow" sets:

  A: intersect( {B,Y,C} , {C,B} ) = {B,C} ; infer edges  A -> B  ,  A -> C
  B: intersect( {Y,C}   , {}    ) = {}    ; infer no edges
  C: intersect( {}      , {B}   ) = {}    ; infer no edges

Note that targets are never inferred as dependees because outside
libraries should not depend on them.

------------------------------------------------------------------------------

The initial exploration of dependencies using a BFS associates an
integer index with each link item.  When the graph is built outgoing
edges are sorted by this index.

After the initial exploration of the link interface tree, any
transitive (dependent) shared libraries that were encountered and not
included in the interface are processed in their own BFS.  This BFS
follows only the dependent library lists and not the link interfaces.
They are added to the link items with a mark indicating that the are
transitive dependencies.  Then cmComputeLinkInformation deals with
them on a per-platform basis.

The complete graph formed from all known and inferred dependencies may
not be acyclic, so an acyclic version must be created.
The original graph is converted to a directed acyclic graph in which
each node corresponds to a strongly connected component of the
original graph.  For example, the dependency graph

  X -> A -> B -> C -> A -> Y

contains strongly connected components {X}, {A,B,C}, and {Y}.  The
implied directed acyclic graph (DAG) is

  {X} -> {A,B,C} -> {Y}

We then compute a topological order for the DAG nodes to serve as a
reference for satisfying dependencies efficiently.  We perform the DFS
in reverse order and assign topological order indices counting down so
that the result is as close to the original BFS order as possible
without violating dependencies.

------------------------------------------------------------------------------

The final link entry order is constructed as follows.  We first walk
through and emit the *original* link line as specified by the user.
As each item is emitted, a set of pending nodes in the component DAG
is maintained.  When a pending component has been completely seen, it
is removed from the pending set and its dependencies (following edges
of the DAG) are added.  A trivial component (those with one item) is
complete as soon as its item is seen.  A non-trivial component (one
with more than one item; assumed to be static libraries) is complete
when *all* its entries have been seen *twice* (all entries seen once,
then all entries seen again, not just each entry twice).  A pending
component tracks which items have been seen and a count of how many
times the component needs to be seen (once for trivial components,
twice for non-trivial).  If at any time another component finishes and
re-adds an already pending component, the pending component is reset
so that it needs to be seen in its entirety again.  This ensures that
all dependencies of a component are satisfied no matter where it
appears.

After the original link line has been completed, we append to it the
remaining pending components and their dependencies.  This is done by
repeatedly emitting the first item from the first pending component
and following the same update rules as when traversing the original
link line.  Since the pending components are kept in topological order
they are emitted with minimal repeats (we do not want to emit a
component just to have it added again when another component is
completed later).  This process continues until no pending components
remain.  We know it will terminate because the component graph is
guaranteed to be acyclic.

The final list of items produced by this procedure consists of the
original user link line followed by minimal additional items needed to
satisfy dependencies.  The final list is then filtered to de-duplicate
items that we know the linker will reuse automatically (shared libs).

*/

namespace {
// LINK_LIBRARY helpers
const auto LL_BEGIN = "<LINK_LIBRARY:"_s;
const auto LL_END = "</LINK_LIBRARY:"_s;

inline std::string ExtractFeature(std::string const& item)
{
  return item.substr(LL_BEGIN.length(),
                     item.find('>', LL_BEGIN.length()) - LL_BEGIN.length());
}

bool IsFeatureSupported(cmMakefile* makefile, std::string const& linkLanguage,
                        std::string const& feature)
{
  auto featureSupported = cmStrCat(
    "CMAKE_", linkLanguage, "_LINK_LIBRARY_USING_", feature, "_SUPPORTED");
  if (makefile->GetDefinition(featureSupported).IsOn()) {
    return true;
  }

  featureSupported =
    cmStrCat("CMAKE_LINK_LIBRARY_USING_", feature, "_SUPPORTED");
  return makefile->GetDefinition(featureSupported).IsOn();
}

// LINK_GROUP helpers
const auto LG_BEGIN = "<LINK_GROUP:"_s;
const auto LG_END = "</LINK_GROUP:"_s;

inline std::string ExtractGroupFeature(std::string const& item)
{
  return item.substr(LG_BEGIN.length(),
                     item.find(':', LG_BEGIN.length()) - LG_BEGIN.length());
}

bool IsGroupFeatureSupported(cmMakefile* makefile,
                             std::string const& linkLanguage,
                             std::string const& feature)
{
  auto featureSupported = cmStrCat(
    "CMAKE_", linkLanguage, "_LINK_GROUP_USING_", feature, "_SUPPORTED");
  if (makefile->GetDefinition(featureSupported).IsOn()) {
    return true;
  }

  featureSupported =
    cmStrCat("CMAKE_LINK_GROUP_USING_", feature, "_SUPPORTED");
  return makefile->GetDefinition(featureSupported).IsOn();
}
}

const std::string cmComputeLinkDepends::LinkEntry::DEFAULT = "DEFAULT";

cmComputeLinkDepends::cmComputeLinkDepends(const cmGeneratorTarget* target,
                                           const std::string& config,
                                           const std::string& linkLanguage)
{
  // Store context information.
  this->Target = target;
  this->Makefile = this->Target->Target->GetMakefile();
  this->GlobalGenerator =
    this->Target->GetLocalGenerator()->GetGlobalGenerator();
  this->CMakeInstance = this->GlobalGenerator->GetCMakeInstance();
  this->LinkLanguage = linkLanguage;

  // target oriented feature override property takes precedence over
  // global override property
  cm::string_view lloPrefix = "LINK_LIBRARY_OVERRIDE_"_s;
  auto const& keys = this->Target->GetPropertyKeys();
  std::for_each(
    keys.cbegin(), keys.cend(),
    [this, &lloPrefix, &config, &linkLanguage](std::string const& key) {
      if (cmHasPrefix(key, lloPrefix)) {
        if (cmValue feature = this->Target->GetProperty(key)) {
          if (!feature->empty() && key.length() > lloPrefix.length()) {
            auto item = key.substr(lloPrefix.length());
            cmGeneratorExpressionDAGChecker dag{ this->Target->GetBacktrace(),
                                                 this->Target,
                                                 "LINK_LIBRARY_OVERRIDE",
                                                 nullptr, nullptr };
            auto overrideFeature = cmGeneratorExpression::Evaluate(
              *feature, this->Target->GetLocalGenerator(), config,
              this->Target, &dag, this->Target, linkLanguage);
            this->LinkLibraryOverride.emplace(item, overrideFeature);
          }
        }
      }
    });
  // global override property
  if (cmValue linkLibraryOverride =
        this->Target->GetProperty("LINK_LIBRARY_OVERRIDE")) {
    cmGeneratorExpressionDAGChecker dag{ target->GetBacktrace(), target,
                                         "LINK_LIBRARY_OVERRIDE", nullptr,
                                         nullptr };
    auto overrideValue = cmGeneratorExpression::Evaluate(
      *linkLibraryOverride, target->GetLocalGenerator(), config, target, &dag,
      target, linkLanguage);

    auto overrideList = cmTokenize(overrideValue, ","_s);
    if (overrideList.size() >= 2) {
      auto const& feature = overrideList.front();
      for_each(overrideList.cbegin() + 1, overrideList.cend(),
               [this, &feature](std::string const& item) {
                 this->LinkLibraryOverride.emplace(item, feature);
               });
    }
  }

  // The configuration being linked.
  this->HasConfig = !config.empty();
  this->Config = (this->HasConfig) ? config : std::string();
  std::vector<std::string> debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();
  this->LinkType = CMP0003_ComputeLinkType(this->Config, debugConfigs);

  // Enable debug mode if requested.
  this->DebugMode = this->Makefile->IsOn("CMAKE_LINK_DEPENDS_DEBUG_MODE");

  // Assume no compatibility until set.
  this->OldLinkDirMode = false;

  // No computation has been done.
  this->CCG = nullptr;
}

cmComputeLinkDepends::~cmComputeLinkDepends() = default;

void cmComputeLinkDepends::SetOldLinkDirMode(bool b)
{
  this->OldLinkDirMode = b;
}

std::vector<cmComputeLinkDepends::LinkEntry> const&
cmComputeLinkDepends::Compute()
{
  // Follow the link dependencies of the target to be linked.
  this->AddDirectLinkEntries();

  // Complete the breadth-first search of dependencies.
  while (!this->BFSQueue.empty()) {
    // Get the next entry.
    BFSEntry qe = this->BFSQueue.front();
    this->BFSQueue.pop();

    // Follow the entry's dependencies.
    this->FollowLinkEntry(qe);
  }

  // Complete the search of shared library dependencies.
  while (!this->SharedDepQueue.empty()) {
    // Handle the next entry.
    this->HandleSharedDependency(this->SharedDepQueue.front());
    this->SharedDepQueue.pop();
  }

  // Infer dependencies of targets for which they were not known.
  this->InferDependencies();

  // finalize groups dependencies
  // All dependencies which are raw items must be replaced by the group
  // it belongs to, if any.
  this->UpdateGroupDependencies();

  // Cleanup the constraint graph.
  this->CleanConstraintGraph();

  // Display the constraint graph.
  if (this->DebugMode) {
    fprintf(stderr,
            "---------------------------------------"
            "---------------------------------------\n");
    fprintf(stderr, "Link dependency analysis for target %s, config %s\n",
            this->Target->GetName().c_str(),
            this->HasConfig ? this->Config.c_str() : "noconfig");
    this->DisplayConstraintGraph();
  }

  // Compute the DAG of strongly connected components.  The algorithm
  // used by cmComputeComponentGraph should identify the components in
  // the same order in which the items were originally discovered in
  // the BFS.  This should preserve the original order when no
  // constraints disallow it.
  this->CCG =
    cm::make_unique<cmComputeComponentGraph>(this->EntryConstraintGraph);
  this->CCG->Compute();

  if (!this->CheckCircularDependencies()) {
    return this->FinalLinkEntries;
  }

  // Compute the final ordering.
  this->OrderLinkEntries();

  // Compute the final set of link entries.
  // Iterate in reverse order so we can keep only the last occurrence
  // of a shared library.
  std::set<size_t> emitted;
  for (size_t i : cmReverseRange(this->FinalLinkOrder)) {
    LinkEntry const& e = this->EntryList[i];
    cmGeneratorTarget const* t = e.Target;
    // Entries that we know the linker will reuse do not need to be repeated.
    bool uniquify = t && t->GetType() == cmStateEnums::SHARED_LIBRARY;
    if (!uniquify || emitted.insert(i).second) {
      this->FinalLinkEntries.push_back(e);
    }
  }
  // Place explicitly linked object files in the front.  The linker will
  // always use them anyway, and they may depend on symbols from libraries.
  // Append in reverse order since we reverse the final order below.
  for (size_t i : cmReverseRange(this->ObjectEntries)) {
    this->FinalLinkEntries.emplace_back(this->EntryList[i]);
  }
  // Reverse the resulting order since we iterated in reverse.
  std::reverse(this->FinalLinkEntries.begin(), this->FinalLinkEntries.end());

  // Expand group items
  if (!this->GroupItems.empty()) {
    for (const auto& group : this->GroupItems) {
      const LinkEntry& groupEntry = this->EntryList[group.first];
      auto it = this->FinalLinkEntries.begin();
      while (true) {
        it = std::find_if(it, this->FinalLinkEntries.end(),
                          [&groupEntry](const LinkEntry& entry) -> bool {
                            return groupEntry.Item == entry.Item;
                          });
        if (it == this->FinalLinkEntries.end()) {
          break;
        }
        it->Item.Value = "</LINK_GROUP>";
        for (auto i = group.second.rbegin(); i != group.second.rend(); ++i) {
          it = this->FinalLinkEntries.insert(it, this->EntryList[*i]);
        }
        it = this->FinalLinkEntries.insert(it, groupEntry);
        it->Item.Value = "<LINK_GROUP>";
      }
    }
  }

  // Display the final set.
  if (this->DebugMode) {
    this->DisplayFinalEntries();
  }

  return this->FinalLinkEntries;
}

std::string const& cmComputeLinkDepends::GetCurrentFeature(
  std::string const& item, std::string const& defaultFeature) const
{
  auto it = this->LinkLibraryOverride.find(item);
  return it == this->LinkLibraryOverride.end() ? defaultFeature : it->second;
}

std::pair<std::map<cmLinkItem, size_t>::iterator, bool>
cmComputeLinkDepends::AllocateLinkEntry(cmLinkItem const& item)
{
  std::map<cmLinkItem, size_t>::value_type index_entry(
    item, static_cast<size_t>(this->EntryList.size()));
  auto lei = this->LinkEntryIndex.insert(index_entry);
  if (lei.second) {
    this->EntryList.emplace_back();
    this->InferredDependSets.emplace_back();
    this->EntryConstraintGraph.emplace_back();
  }
  return lei;
}

std::pair<size_t, bool> cmComputeLinkDepends::AddLinkEntry(
  cmLinkItem const& item, size_t groupIndex)
{
  // Allocate a spot for the item entry.
  auto lei = this->AllocateLinkEntry(item);

  // Check if the item entry has already been added.
  if (!lei.second) {
    // Yes.  We do not need to follow the item's dependencies again.
    return { lei.first->second, false };
  }

  // Initialize the item entry.
  size_t index = lei.first->second;
  LinkEntry& entry = this->EntryList[index];
  entry.Item = BT<std::string>(item.AsStr(), item.Backtrace);
  entry.Target = item.Target;
  if (!entry.Target && entry.Item.Value[0] == '-' &&
      entry.Item.Value[1] != 'l' &&
      entry.Item.Value.substr(0, 10) != "-framework") {
    entry.Kind = LinkEntry::Flag;
  } else if (cmHasPrefix(entry.Item.Value, LG_BEGIN) &&
             cmHasSuffix(entry.Item.Value, '>')) {
    entry.Kind = LinkEntry::Group;
  }

  if (entry.Kind != LinkEntry::Group) {
    // If the item has dependencies queue it to follow them.
    if (entry.Target) {
      // Target dependencies are always known.  Follow them.
      BFSEntry qe = { index, groupIndex, nullptr };
      this->BFSQueue.push(qe);
    } else {
      // Look for an old-style <item>_LIB_DEPENDS variable.
      std::string var = cmStrCat(entry.Item.Value, "_LIB_DEPENDS");
      if (cmValue val = this->Makefile->GetDefinition(var)) {
        // The item dependencies are known.  Follow them.
        BFSEntry qe = { index, groupIndex, val->c_str() };
        this->BFSQueue.push(qe);
      } else if (entry.Kind != LinkEntry::Flag) {
        // The item dependencies are not known.  We need to infer them.
        this->InferredDependSets[index].Initialized = true;
      }
    }
  }

  return { index, true };
}

void cmComputeLinkDepends::AddLinkObject(cmLinkItem const& item)
{
  assert(!item.Target); // The item is an object file, not its target.

  // Allocate a spot for the item entry.
  auto lei = this->AllocateLinkEntry(item);

  // Check if the item entry has already been added.
  if (!lei.second) {
    return;
  }

  // Initialize the item entry.
  size_t index = lei.first->second;
  LinkEntry& entry = this->EntryList[index];
  entry.Item = BT<std::string>(item.AsStr(), item.Backtrace);
  entry.Kind = LinkEntry::Object;
  entry.ObjectSource = item.ObjectSource;

  // Record explicitly linked object files separately.
  this->ObjectEntries.emplace_back(index);
}

void cmComputeLinkDepends::FollowLinkEntry(BFSEntry qe)
{
  // Get this entry representation.
  size_t depender_index =
    qe.GroupIndex == cmComputeComponentGraph::INVALID_COMPONENT
    ? qe.Index
    : qe.GroupIndex;
  LinkEntry const& entry = this->EntryList[qe.Index];

  // Follow the item's dependencies.
  if (entry.Target) {
    // Follow the target dependencies.
    if (cmLinkInterface const* iface =
          entry.Target->GetLinkInterface(this->Config, this->Target)) {
      const bool isIface =
        entry.Target->GetType() == cmStateEnums::INTERFACE_LIBRARY;
      // This target provides its own link interface information.
      this->AddLinkEntries(depender_index, iface->Libraries);
      this->AddLinkObjects(iface->Objects);
      for (auto const& language : iface->Languages) {
        auto runtimeEntries = iface->LanguageRuntimeLibraries.find(language);
        if (runtimeEntries != iface->LanguageRuntimeLibraries.end()) {
          this->AddLinkEntries(depender_index, runtimeEntries->second);
        }
      }

      if (isIface) {
        return;
      }

      // Handle dependent shared libraries.
      this->FollowSharedDeps(depender_index, iface);

      // Support for CMP0003.
      for (cmLinkItem const& oi : iface->WrongConfigLibraries) {
        this->CheckWrongConfigItem(oi);
      }
    }
  } else {
    // Follow the old-style dependency list.
    this->AddVarLinkEntries(depender_index, qe.LibDepends);
  }
}

void cmComputeLinkDepends::FollowSharedDeps(size_t depender_index,
                                            cmLinkInterface const* iface,
                                            bool follow_interface)
{
  // Follow dependencies if we have not followed them already.
  if (this->SharedDepFollowed.insert(depender_index).second) {
    if (follow_interface) {
      this->QueueSharedDependencies(depender_index, iface->Libraries);
    }
    this->QueueSharedDependencies(depender_index, iface->SharedDeps);
  }
}

void cmComputeLinkDepends::QueueSharedDependencies(
  size_t depender_index, std::vector<cmLinkItem> const& deps)
{
  for (cmLinkItem const& li : deps) {
    SharedDepEntry qe;
    qe.Item = li;
    qe.DependerIndex = depender_index;
    this->SharedDepQueue.push(qe);
  }
}

void cmComputeLinkDepends::HandleSharedDependency(SharedDepEntry const& dep)
{
  // Allocate a spot for the item entry.
  auto lei = this->AllocateLinkEntry(dep.Item);
  size_t index = lei.first->second;

  // Check if the target does not already has an entry.
  if (lei.second) {
    // Initialize the item entry.
    LinkEntry& entry = this->EntryList[index];
    entry.Item = BT<std::string>(dep.Item.AsStr(), dep.Item.Backtrace);
    entry.Target = dep.Item.Target;

    // This item was added specifically because it is a dependent
    // shared library.  It may get special treatment
    // in cmComputeLinkInformation.
    entry.Kind = LinkEntry::SharedDep;
  }

  // Get the link entry for this target.
  LinkEntry& entry = this->EntryList[index];

  // This shared library dependency must follow the item that listed
  // it.
  this->EntryConstraintGraph[dep.DependerIndex].emplace_back(
    index, true, false, cmListFileBacktrace());

  // Target items may have their own dependencies.
  if (entry.Target) {
    if (cmLinkInterface const* iface =
          entry.Target->GetLinkInterface(this->Config, this->Target)) {
      // Follow public and private dependencies transitively.
      this->FollowSharedDeps(index, iface, true);
    }
  }
}

void cmComputeLinkDepends::AddVarLinkEntries(size_t depender_index,
                                             const char* value)
{
  // This is called to add the dependencies named by
  // <item>_LIB_DEPENDS.  The variable contains a semicolon-separated
  // list.  The list contains link-type;item pairs and just items.
  cmList deplist{ value };

  // Look for entries meant for this configuration.
  std::vector<cmLinkItem> actual_libs;
  cmTargetLinkLibraryType llt = GENERAL_LibraryType;
  bool haveLLT = false;
  for (std::string const& d : deplist) {
    if (d == "debug") {
      llt = DEBUG_LibraryType;
      haveLLT = true;
    } else if (d == "optimized") {
      llt = OPTIMIZED_LibraryType;
      haveLLT = true;
    } else if (d == "general") {
      llt = GENERAL_LibraryType;
      haveLLT = true;
    } else if (!d.empty()) {
      // If no explicit link type was given prior to this entry then
      // check if the entry has its own link type variable.  This is
      // needed for compatibility with dependency files generated by
      // the export_library_dependencies command from CMake 2.4 and
      // lower.
      if (!haveLLT) {
        std::string var = cmStrCat(d, "_LINK_TYPE");
        if (cmValue val = this->Makefile->GetDefinition(var)) {
          if (*val == "debug") {
            llt = DEBUG_LibraryType;
          } else if (*val == "optimized") {
            llt = OPTIMIZED_LibraryType;
          }
        }
      }

      // If the library is meant for this link type then use it.
      if (llt == GENERAL_LibraryType || llt == this->LinkType) {
        actual_libs.emplace_back(this->ResolveLinkItem(depender_index, d));
      } else if (this->OldLinkDirMode) {
        cmLinkItem item = this->ResolveLinkItem(depender_index, d);
        this->CheckWrongConfigItem(item);
      }

      // Reset the link type until another explicit type is given.
      llt = GENERAL_LibraryType;
      haveLLT = false;
    }
  }

  // Add the entries from this list.
  this->AddLinkEntries(depender_index, actual_libs);
}

void cmComputeLinkDepends::AddDirectLinkEntries()
{
  // Add direct link dependencies in this configuration.
  cmLinkImplementation const* impl = this->Target->GetLinkImplementation(
    this->Config, cmGeneratorTarget::LinkInterfaceFor::Link);
  this->AddLinkEntries(cmComputeComponentGraph::INVALID_COMPONENT,
                       impl->Libraries);
  this->AddLinkObjects(impl->Objects);

  for (auto const& language : impl->Languages) {
    auto runtimeEntries = impl->LanguageRuntimeLibraries.find(language);
    if (runtimeEntries != impl->LanguageRuntimeLibraries.end()) {
      this->AddLinkEntries(cmComputeComponentGraph::INVALID_COMPONENT,
                           runtimeEntries->second);
    }
  }
  for (cmLinkItem const& wi : impl->WrongConfigLibraries) {
    this->CheckWrongConfigItem(wi);
  }
}

template <typename T>
void cmComputeLinkDepends::AddLinkEntries(size_t depender_index,
                                          std::vector<T> const& libs)
{
  // Track inferred dependency sets implied by this list.
  std::map<size_t, DependSet> dependSets;
  std::string feature = LinkEntry::DEFAULT;

  bool inGroup = false;
  std::pair<size_t, bool> groupIndex{
    cmComputeComponentGraph::INVALID_COMPONENT, false
  };
  std::vector<size_t> groupItems;

  // Loop over the libraries linked directly by the depender.
  for (T const& l : libs) {
    // Skip entries that will resolve to the target getting linked or
    // are empty.
    cmLinkItem const& item = l;
    if (item.AsStr() == this->Target->GetName() || item.AsStr().empty()) {
      continue;
    }

    if (cmHasPrefix(item.AsStr(), LL_BEGIN) &&
        cmHasSuffix(item.AsStr(), '>')) {
      feature = ExtractFeature(item.AsStr());
      // emit a warning if an undefined feature is used as part of
      // an imported target
      if (depender_index != cmComputeComponentGraph::INVALID_COMPONENT) {
        const auto& depender = this->EntryList[depender_index];
        if (depender.Target != nullptr && depender.Target->IsImported() &&
            !IsFeatureSupported(this->Makefile, this->LinkLanguage, feature)) {
          this->CMakeInstance->IssueMessage(
            MessageType::AUTHOR_ERROR,
            cmStrCat("The 'IMPORTED' target '", depender.Target->GetName(),
                     "' uses the generator-expression '$<LINK_LIBRARY>' with "
                     "the feature '",
                     feature,
                     "', which is undefined or unsupported.\nDid you miss to "
                     "define it by setting variables \"CMAKE_",
                     this->LinkLanguage, "_LINK_LIBRARY_USING_", feature,
                     "\" and \"CMAKE_", this->LinkLanguage,
                     "_LINK_LIBRARY_USING_", feature, "_SUPPORTED\"?"),
            this->Target->GetBacktrace());
        }
      }
      continue;
    }
    if (cmHasPrefix(item.AsStr(), LL_END) && cmHasSuffix(item.AsStr(), '>')) {
      feature = LinkEntry::DEFAULT;
      continue;
    }

    if (cmHasPrefix(item.AsStr(), LG_BEGIN) &&
        cmHasSuffix(item.AsStr(), '>')) {
      groupIndex = this->AddLinkEntry(item);
      if (groupIndex.second) {
        LinkEntry& entry = this->EntryList[groupIndex.first];
        entry.Feature = ExtractGroupFeature(item.AsStr());
      }
      inGroup = true;
      if (depender_index != cmComputeComponentGraph::INVALID_COMPONENT) {
        this->EntryConstraintGraph[depender_index].emplace_back(
          groupIndex.first, false, false, cmListFileBacktrace());
      } else {
        // This is a direct dependency of the target being linked.
        this->OriginalEntries.push_back(groupIndex.first);
      }
      continue;
    }

    size_t dependee_index;

    if (cmHasPrefix(item.AsStr(), LG_END) && cmHasSuffix(item.AsStr(), '>')) {
      dependee_index = groupIndex.first;
      if (groupIndex.second) {
        this->GroupItems.emplace(groupIndex.first, groupItems);
      }
      inGroup = false;
      groupIndex = std::make_pair(-1, false);
      groupItems.clear();
      continue;
    }

    if (depender_index != cmComputeComponentGraph::INVALID_COMPONENT &&
        inGroup) {
      const auto& depender = this->EntryList[depender_index];
      const auto& groupFeature = this->EntryList[groupIndex.first].Feature;
      if (depender.Target != nullptr && depender.Target->IsImported() &&
          !IsGroupFeatureSupported(this->Makefile, this->LinkLanguage,
                                   groupFeature)) {
        this->CMakeInstance->IssueMessage(
          MessageType::AUTHOR_ERROR,
          cmStrCat("The 'IMPORTED' target '", depender.Target->GetName(),
                   "' uses the generator-expression '$<LINK_GROUP>' with "
                   "the feature '",
                   groupFeature,
                   "', which is undefined or unsupported.\nDid you miss to "
                   "define it by setting variables \"CMAKE_",
                   this->LinkLanguage, "_LINK_GROUP_USING_", groupFeature,
                   "\" and \"CMAKE_", this->LinkLanguage, "_LINK_GROUP_USING_",
                   groupFeature, "_SUPPORTED\"?"),
          this->Target->GetBacktrace());
      }
    }

    // Add a link entry for this item.
    auto ale = this->AddLinkEntry(item, groupIndex.first);
    dependee_index = ale.first;
    LinkEntry& entry = this->EntryList[dependee_index];
    auto const& itemFeature =
      this->GetCurrentFeature(entry.Item.Value, feature);
    if (inGroup && ale.second && entry.Target != nullptr &&
        (entry.Target->GetType() == cmStateEnums::TargetType::OBJECT_LIBRARY ||
         entry.Target->GetType() ==
           cmStateEnums::TargetType::INTERFACE_LIBRARY)) {
      const auto& groupFeature = this->EntryList[groupIndex.first].Feature;
      this->CMakeInstance->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(
          "The feature '", groupFeature,
          "', specified as part of a generator-expression "
          "'$",
          LG_BEGIN, groupFeature, ">', will not be applied to the ",
          (entry.Target->GetType() == cmStateEnums::TargetType::OBJECT_LIBRARY
             ? "OBJECT"
             : "INTERFACE"),
          " library '", entry.Item.Value, "'."),
        this->Target->GetBacktrace());
    }
    if (itemFeature != LinkEntry::DEFAULT) {
      if (ale.second) {
        // current item not yet defined
        if (entry.Target != nullptr &&
            (entry.Target->GetType() ==
               cmStateEnums::TargetType::OBJECT_LIBRARY ||
             entry.Target->GetType() ==
               cmStateEnums::TargetType::INTERFACE_LIBRARY)) {
          this->CMakeInstance->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmStrCat("The feature '", feature,
                     "', specified as part of a generator-expression "
                     "'$",
                     LL_BEGIN, feature, ">', will not be applied to the ",
                     (entry.Target->GetType() ==
                          cmStateEnums::TargetType::OBJECT_LIBRARY
                        ? "OBJECT"
                        : "INTERFACE"),
                     " library '", entry.Item.Value, "'."),
            this->Target->GetBacktrace());
        } else {
          entry.Feature = itemFeature;
        }
      }
    }

    bool supportedItem = entry.Target == nullptr ||
      (entry.Target->GetType() != cmStateEnums::TargetType::OBJECT_LIBRARY &&
       entry.Target->GetType() != cmStateEnums::TargetType::INTERFACE_LIBRARY);

    if (supportedItem) {
      if (inGroup) {
        const auto& currentFeature = this->EntryList[groupIndex.first].Feature;
        for (const auto& g : this->GroupItems) {
          const auto& groupFeature = this->EntryList[g.first].Feature;
          if (groupFeature == currentFeature) {
            continue;
          }
          if (std::find(g.second.cbegin(), g.second.cend(), dependee_index) !=
              g.second.cend()) {
            this->CMakeInstance->IssueMessage(
              MessageType::FATAL_ERROR,
              cmStrCat("Impossible to link target '", this->Target->GetName(),
                       "' because the link item '", entry.Item.Value,
                       "', specified with the group feature '", currentFeature,
                       '\'', ", has already occurred with the feature '",
                       groupFeature, '\'', ", which is not allowed."),
              this->Target->GetBacktrace());
            continue;
          }
        }
      }
      if (entry.Feature != itemFeature) {
        // incompatibles features occurred
        this->CMakeInstance->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Impossible to link target '", this->Target->GetName(),
                   "' because the link item '", entry.Item.Value,
                   "', specified ",
                   (itemFeature == LinkEntry::DEFAULT
                      ? "without any feature or 'DEFAULT' feature"
                      : cmStrCat("with the feature '", itemFeature, '\'')),
                   ", has already occurred ",
                   (entry.Feature == LinkEntry::DEFAULT
                      ? "without any feature or 'DEFAULT' feature"
                      : cmStrCat("with the feature '", entry.Feature, '\'')),
                   ", which is not allowed."),
          this->Target->GetBacktrace());
      }
    }

    if (inGroup) {
      // store item index for dependencies handling
      groupItems.push_back(dependee_index);
    } else {
      std::vector<size_t> indexes;
      bool entryHandled = false;
      // search any occurrence of the library in already defined groups
      for (const auto& group : this->GroupItems) {
        for (auto index : group.second) {
          if (entry.Item.Value == this->EntryList[index].Item.Value) {
            indexes.push_back(group.first);
            entryHandled = true;
            break;
          }
        }
      }
      if (!entryHandled) {
        indexes.push_back(dependee_index);
      }

      for (auto index : indexes) {
        // The dependee must come after the depender.
        if (depender_index != cmComputeComponentGraph::INVALID_COMPONENT) {
          this->EntryConstraintGraph[depender_index].emplace_back(
            index, false, false, cmListFileBacktrace());
        } else {
          // This is a direct dependency of the target being linked.
          this->OriginalEntries.push_back(index);
        }

        // Update the inferred dependencies for earlier items.
        for (auto& dependSet : dependSets) {
          // Add this item to the inferred dependencies of other items.
          // Target items are never inferred dependees because unknown
          // items are outside libraries that should not be depending on
          // targets.
          if (!this->EntryList[index].Target &&
              this->EntryList[index].Kind != LinkEntry::Flag &&
              this->EntryList[index].Kind != LinkEntry::Group &&
              dependee_index != dependSet.first) {
            dependSet.second.insert(index);
          }
        }

        // If this item needs to have dependencies inferred, do so.
        if (this->InferredDependSets[index].Initialized) {
          // Make sure an entry exists to hold the set for the item.
          dependSets[index];
        }
      }
    }
  }

  // Store the inferred dependency sets discovered for this list.
  for (auto const& dependSet : dependSets) {
    this->InferredDependSets[dependSet.first].push_back(dependSet.second);
  }
}

void cmComputeLinkDepends::AddLinkObjects(std::vector<cmLinkItem> const& objs)
{
  for (cmLinkItem const& obj : objs) {
    this->AddLinkObject(obj);
  }
}

cmLinkItem cmComputeLinkDepends::ResolveLinkItem(size_t depender_index,
                                                 const std::string& name)
{
  // Look for a target in the scope of the depender.
  cmGeneratorTarget const* from = this->Target;
  if (depender_index != cmComputeComponentGraph::INVALID_COMPONENT) {
    if (cmGeneratorTarget const* depender =
          this->EntryList[depender_index].Target) {
      from = depender;
    }
  }
  return from->ResolveLinkItem(BT<std::string>(name));
}

void cmComputeLinkDepends::InferDependencies()
{
  // The inferred dependency sets for each item list the possible
  // dependencies.  The intersection of the sets for one item form its
  // inferred dependencies.
  for (size_t depender_index = 0;
       depender_index < this->InferredDependSets.size(); ++depender_index) {
    // Skip items for which dependencies do not need to be inferred or
    // for which the inferred dependency sets are empty.
    DependSetList& sets = this->InferredDependSets[depender_index];
    if (!sets.Initialized || sets.empty()) {
      continue;
    }

    // Intersect the sets for this item.
    DependSet common = sets.front();
    for (DependSet const& i : cmMakeRange(sets).advance(1)) {
      DependSet intersection;
      std::set_intersection(common.begin(), common.end(), i.begin(), i.end(),
                            std::inserter(intersection, intersection.begin()));
      common = intersection;
    }

    // Add the inferred dependencies to the graph.
    cmGraphEdgeList& edges = this->EntryConstraintGraph[depender_index];
    edges.reserve(edges.size() + common.size());
    for (auto const& c : common) {
      edges.emplace_back(c, true, false, cmListFileBacktrace());
    }
  }
}

void cmComputeLinkDepends::UpdateGroupDependencies()
{
  if (this->GroupItems.empty()) {
    return;
  }

  // Walks through all entries of the constraint graph to replace dependencies
  // over raw items by the group it belongs to, if any.
  for (auto& edgeList : this->EntryConstraintGraph) {
    for (auto& edge : edgeList) {
      size_t index = edge;
      if (this->EntryList[index].Kind == LinkEntry::Group ||
          this->EntryList[index].Kind == LinkEntry::Flag ||
          this->EntryList[index].Kind == LinkEntry::Object) {
        continue;
      }
      // search the item in the defined groups
      for (const auto& groupItems : this->GroupItems) {
        auto pos = std::find(groupItems.second.cbegin(),
                             groupItems.second.cend(), index);
        if (pos != groupItems.second.cend()) {
          // replace lib dependency by the group it belongs to
          edge = cmGraphEdge{ groupItems.first, false, false,
                              cmListFileBacktrace() };
        }
      }
    }
  }
}

void cmComputeLinkDepends::CleanConstraintGraph()
{
  for (cmGraphEdgeList& edgeList : this->EntryConstraintGraph) {
    // Sort the outgoing edges for each graph node so that the
    // original order will be preserved as much as possible.
    std::sort(edgeList.begin(), edgeList.end());

    // Make the edge list unique.
    edgeList.erase(std::unique(edgeList.begin(), edgeList.end()),
                   edgeList.end());
  }
}

bool cmComputeLinkDepends::CheckCircularDependencies() const
{
  std::vector<NodeList> const& components = this->CCG->GetComponents();
  size_t nc = components.size();
  for (size_t c = 0; c < nc; ++c) {
    // Get the current component.
    NodeList const& nl = components[c];

    // Skip trivial components.
    if (nl.size() < 2) {
      continue;
    }

    // no group must be evolved
    bool cycleDetected = false;
    for (size_t ni : nl) {
      if (this->EntryList[ni].Kind == LinkEntry::Group) {
        cycleDetected = true;
        break;
      }
    }
    if (!cycleDetected) {
      continue;
    }

    // Construct the error message.
    auto formatItem = [](LinkEntry const& entry) -> std::string {
      if (entry.Kind == LinkEntry::Group) {
        auto items =
          entry.Item.Value.substr(entry.Item.Value.find(':', 12) + 1);
        items.pop_back();
        std::replace(items.begin(), items.end(), '|', ',');
        return cmStrCat("group \"", ExtractGroupFeature(entry.Item.Value),
                        ":{", items, "}\"");
      }
      return cmStrCat('"', entry.Item.Value, '"');
    };

    std::ostringstream e;
    e << "The inter-target dependency graph, for the target \""
      << this->Target->GetName()
      << "\", contains the following strongly connected component "
         "(cycle):\n";
    std::vector<size_t> const& cmap = this->CCG->GetComponentMap();
    for (size_t i : nl) {
      // Get the depender.
      LinkEntry const& depender = this->EntryList[i];

      // Describe the depender.
      e << "  " << formatItem(depender) << "\n";

      // List its dependencies that are inside the component.
      EdgeList const& el = this->EntryConstraintGraph[i];
      for (cmGraphEdge const& ni : el) {
        size_t j = ni;
        if (cmap[j] == c) {
          LinkEntry const& dependee = this->EntryList[j];
          e << "    depends on " << formatItem(dependee) << "\n";
        }
      }
    }
    this->CMakeInstance->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                                      this->Target->GetBacktrace());

    return false;
  }

  return true;
}

void cmComputeLinkDepends::DisplayConstraintGraph()
{
  // Display the graph nodes and their edges.
  std::ostringstream e;
  for (size_t i = 0; i < this->EntryConstraintGraph.size(); ++i) {
    EdgeList const& nl = this->EntryConstraintGraph[i];
    e << "item " << i << " is [" << this->EntryList[i].Item << "]\n";
    e << cmWrap("  item ", nl, " must follow it", "\n") << "\n";
  }
  fprintf(stderr, "%s\n", e.str().c_str());
}

void cmComputeLinkDepends::OrderLinkEntries()
{
  // The component graph is guaranteed to be acyclic.  Start a DFS
  // from every entry to compute a topological order for the
  // components.
  Graph const& cgraph = this->CCG->GetComponentGraph();
  size_t n = cgraph.size();
  this->ComponentVisited.resize(cgraph.size(), 0);
  this->ComponentOrder.resize(cgraph.size(), n);
  this->ComponentOrderId = n;
  // Run in reverse order so the topological order will preserve the
  // original order where there are no constraints.
  for (size_t c = n - 1; c != cmComputeComponentGraph::INVALID_COMPONENT;
       --c) {
    this->VisitComponent(c);
  }

  // Display the component graph.
  if (this->DebugMode) {
    this->DisplayComponents();
  }

  // Start with the original link line.
  for (size_t originalEntry : this->OriginalEntries) {
    this->VisitEntry(originalEntry);
  }

  // Now explore anything left pending.  Since the component graph is
  // guaranteed to be acyclic we know this will terminate.
  while (!this->PendingComponents.empty()) {
    // Visit one entry from the first pending component.  The visit
    // logic will update the pending components accordingly.  Since
    // the pending components are kept in topological order this will
    // not repeat one.
    size_t e = *this->PendingComponents.begin()->second.Entries.begin();
    this->VisitEntry(e);
  }
}

void cmComputeLinkDepends::DisplayComponents()
{
  fprintf(stderr, "The strongly connected components are:\n");
  std::vector<NodeList> const& components = this->CCG->GetComponents();
  for (size_t c = 0; c < components.size(); ++c) {
    fprintf(stderr, "Component (%zu):\n", c);
    NodeList const& nl = components[c];
    for (size_t i : nl) {
      fprintf(stderr, "  item %zu [%s]\n", i,
              this->EntryList[i].Item.Value.c_str());
    }
    EdgeList const& ol = this->CCG->GetComponentGraphEdges(c);
    for (cmGraphEdge const& oi : ol) {
      size_t i = oi;
      fprintf(stderr, "  followed by Component (%zu)\n", i);
    }
    fprintf(stderr, "  topo order index %zu\n", this->ComponentOrder[c]);
  }
  fprintf(stderr, "\n");
}

void cmComputeLinkDepends::VisitComponent(size_t c)
{
  // Check if the node has already been visited.
  if (this->ComponentVisited[c]) {
    return;
  }

  // We are now visiting this component so mark it.
  this->ComponentVisited[c] = 1;

  // Visit the neighbors of the component first.
  // Run in reverse order so the topological order will preserve the
  // original order where there are no constraints.
  EdgeList const& nl = this->CCG->GetComponentGraphEdges(c);
  for (cmGraphEdge const& edge : cmReverseRange(nl)) {
    this->VisitComponent(edge);
  }

  // Assign an ordering id to this component.
  this->ComponentOrder[c] = --this->ComponentOrderId;
}

void cmComputeLinkDepends::VisitEntry(size_t index)
{
  // Include this entry on the link line.
  this->FinalLinkOrder.push_back(index);

  // This entry has now been seen.  Update its component.
  bool completed = false;
  size_t component = this->CCG->GetComponentMap()[index];
  auto mi = this->PendingComponents.find(this->ComponentOrder[component]);
  if (mi != this->PendingComponents.end()) {
    // The entry is in an already pending component.
    PendingComponent& pc = mi->second;

    // Remove the entry from those pending in its component.
    pc.Entries.erase(index);
    if (pc.Entries.empty()) {
      // The complete component has been seen since it was last needed.
      --pc.Count;

      if (pc.Count == 0) {
        // The component has been completed.
        this->PendingComponents.erase(mi);
        completed = true;
      } else {
        // The whole component needs to be seen again.
        NodeList const& nl = this->CCG->GetComponent(component);
        assert(nl.size() > 1);
        pc.Entries.insert(nl.begin(), nl.end());
      }
    }
  } else {
    // The entry is not in an already pending component.
    NodeList const& nl = this->CCG->GetComponent(component);
    if (nl.size() > 1) {
      // This is a non-trivial component.  It is now pending.
      PendingComponent& pc = this->MakePendingComponent(component);

      // The starting entry has already been seen.
      pc.Entries.erase(index);
    } else {
      // This is a trivial component, so it is already complete.
      completed = true;
    }
  }

  // If the entry completed a component, the component's dependencies
  // are now pending.
  if (completed) {
    EdgeList const& ol = this->CCG->GetComponentGraphEdges(component);
    for (cmGraphEdge const& oi : ol) {
      // This entire component is now pending no matter whether it has
      // been partially seen already.
      this->MakePendingComponent(oi);
    }
  }
}

cmComputeLinkDepends::PendingComponent&
cmComputeLinkDepends::MakePendingComponent(size_t component)
{
  // Create an entry (in topological order) for the component.
  PendingComponent& pc =
    this->PendingComponents[this->ComponentOrder[component]];
  pc.Id = component;
  NodeList const& nl = this->CCG->GetComponent(component);

  if (nl.size() == 1) {
    // Trivial components need be seen only once.
    pc.Count = 1;
  } else {
    // This is a non-trivial strongly connected component of the
    // original graph.  It consists of two or more libraries
    // (archives) that mutually require objects from one another.  In
    // the worst case we may have to repeat the list of libraries as
    // many times as there are object files in the biggest archive.
    // For now we just list them twice.
    //
    // The list of items in the component has been sorted by the order
    // of discovery in the original BFS of dependencies.  This has the
    // advantage that the item directly linked by a target requiring
    // this component will come first which minimizes the number of
    // repeats needed.
    pc.Count = this->ComputeComponentCount(nl);
  }

  // Store the entries to be seen.
  pc.Entries.insert(nl.begin(), nl.end());

  return pc;
}

size_t cmComputeLinkDepends::ComputeComponentCount(NodeList const& nl)
{
  size_t count = 2;
  for (size_t ni : nl) {
    if (cmGeneratorTarget const* target = this->EntryList[ni].Target) {
      if (cmLinkInterface const* iface =
            target->GetLinkInterface(this->Config, this->Target)) {
        if (iface->Multiplicity > count) {
          count = iface->Multiplicity;
        }
      }
    }
  }
  return count;
}

void cmComputeLinkDepends::DisplayFinalEntries()
{
  fprintf(stderr, "target [%s] links to:\n", this->Target->GetName().c_str());
  char space[] = "  ";
  int count = 2;
  for (LinkEntry const& lei : this->FinalLinkEntries) {
    if (lei.Kind == LinkEntry::Group) {
      fprintf(stderr, "  %s group",
              lei.Item.Value == "<LINK_GROUP>" ? "start" : "end");
      count = lei.Item.Value == "<LINK_GROUP>" ? 4 : 2;
    } else if (lei.Target) {
      fprintf(stderr, "%*starget [%s]", count, space,
              lei.Target->GetName().c_str());
    } else {
      fprintf(stderr, "%*sitem [%s]", count, space, lei.Item.Value.c_str());
    }
    if (lei.Feature != LinkEntry::DEFAULT) {
      fprintf(stderr, ", feature [%s]", lei.Feature.c_str());
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

void cmComputeLinkDepends::CheckWrongConfigItem(cmLinkItem const& item)
{
  if (!this->OldLinkDirMode) {
    return;
  }

  // For CMake 2.4 bug-compatibility we need to consider the output
  // directories of targets linked in another configuration as link
  // directories.
  if (item.Target && !item.Target->IsImported()) {
    this->OldWrongConfigItems.insert(item.Target);
  }
}
