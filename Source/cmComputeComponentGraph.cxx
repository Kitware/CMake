/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmComputeComponentGraph.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>

const size_t cmComputeComponentGraph::INVALID_COMPONENT =
  std::numeric_limits<size_t>::max();

cmComputeComponentGraph::cmComputeComponentGraph(Graph const& input)
  : InputGraph(input)
{
}

cmComputeComponentGraph::~cmComputeComponentGraph() = default;

void cmComputeComponentGraph::Compute()
{
  // Identify components.
  this->Tarjan();

  // Compute the component graph.
  this->ComponentGraph.resize(0);
  this->ComponentGraph.resize(this->Components.size());
  this->TransferEdges();
}

void cmComputeComponentGraph::Tarjan()
{
  size_t n = this->InputGraph.size();
  TarjanEntry entry = { 0, 0 };
  this->TarjanEntries.resize(0);
  this->TarjanEntries.resize(n, entry);
  this->TarjanComponents.resize(0);
  this->TarjanComponents.resize(n, INVALID_COMPONENT);
  this->TarjanWalkId = 0;
  this->TarjanVisited.resize(0);
  this->TarjanVisited.resize(n, 0);
  for (size_t i = 0; i < n; ++i) {
    // Start a new DFS from this node if it has never been visited.
    if (!this->TarjanVisited[i]) {
      assert(this->TarjanStack.empty());
      ++this->TarjanWalkId;
      this->TarjanIndex = 0;
      this->TarjanVisit(i);
    }
  }
}

void cmComputeComponentGraph::TarjanVisit(size_t i)
{
  // We are now visiting this node.
  this->TarjanVisited[i] = this->TarjanWalkId;

  // Initialize the entry.
  this->TarjanEntries[i].Root = i;
  this->TarjanComponents[i] = INVALID_COMPONENT;
  this->TarjanEntries[i].VisitIndex = ++this->TarjanIndex;
  this->TarjanStack.push(i);

  // Follow outgoing edges.
  EdgeList const& nl = this->InputGraph[i];
  for (cmGraphEdge const& ni : nl) {
    size_t j = ni;

    // Ignore edges to nodes that have been reached by a previous DFS
    // walk.  Since we did not reach the current node from that walk
    // it must not belong to the same component and it has already
    // been assigned to a component.
    if (this->TarjanVisited[j] > 0 &&
        this->TarjanVisited[j] < this->TarjanWalkId) {
      continue;
    }

    // Visit the destination if it has not yet been visited.
    if (!this->TarjanVisited[j]) {
      this->TarjanVisit(j);
    }

    // If the destination has not yet been assigned to a component,
    // check if it has a better root for the current object.
    if (this->TarjanComponents[j] == INVALID_COMPONENT) {
      if (this->TarjanEntries[this->TarjanEntries[j].Root].VisitIndex <
          this->TarjanEntries[this->TarjanEntries[i].Root].VisitIndex) {
        this->TarjanEntries[i].Root = this->TarjanEntries[j].Root;
      }
    }
  }

  // Check if we have found a component.
  if (this->TarjanEntries[i].Root == i) {
    // Yes.  Create it.
    size_t c = this->Components.size();
    this->Components.emplace_back();
    NodeList& component = this->Components[c];

    // Populate the component list.
    size_t j;
    do {
      // Get the next member of the component.
      j = this->TarjanStack.top();
      this->TarjanStack.pop();

      // Assign the member to the component.
      this->TarjanComponents[j] = c;
      this->TarjanEntries[j].Root = i;

      // Store the node in its component.
      component.push_back(j);
    } while (j != i);

    // Sort the component members for clarity.
    std::sort(component.begin(), component.end());
  }
}

void cmComputeComponentGraph::TransferEdges()
{
  // Map inter-component edges in the original graph to edges in the
  // component graph.
  size_t n = this->InputGraph.size();
  for (size_t i = 0; i < n; ++i) {
    size_t i_component = this->TarjanComponents[i];
    EdgeList const& nl = this->InputGraph[i];
    for (cmGraphEdge const& ni : nl) {
      size_t j = ni;
      size_t j_component = this->TarjanComponents[j];
      if (i_component != j_component) {
        // We do not attempt to combine duplicate edges, but instead
        // store the inter-component edges with suitable multiplicity.
        this->ComponentGraph[i_component].emplace_back(
          j_component, ni.IsStrong(), ni.IsCross(), ni.GetBacktrace());
      }
    }
  }
}
