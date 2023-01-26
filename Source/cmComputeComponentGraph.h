/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <stack>
#include <vector>

#include "cmGraphAdjacencyList.h"

/** \class cmComputeComponentGraph
 * \brief Analyze a graph to determine strongly connected components.
 *
 * Convert a directed graph into a directed acyclic graph whose nodes
 * correspond to strongly connected components of the original graph.
 *
 * We use Tarjan's algorithm to enumerate the components efficiently.
 * An advantage of this approach is that the components are identified
 * in a topologically sorted order.
 */
class cmComputeComponentGraph
{
public:
  // Represent the graph with an adjacency list.
  using NodeList = cmGraphNodeList;
  using EdgeList = cmGraphEdgeList;
  using Graph = cmGraphAdjacencyList;

  cmComputeComponentGraph(Graph const& input);
  ~cmComputeComponentGraph();

  /** Run the computation.  */
  void Compute();

  /** Get the adjacency list of the component graph.  */
  Graph const& GetComponentGraph() const { return this->ComponentGraph; }
  EdgeList const& GetComponentGraphEdges(size_t c) const
  {
    return this->ComponentGraph[c];
  }

  /** Get map from component index to original node indices.  */
  std::vector<NodeList> const& GetComponents() const
  {
    return this->Components;
  }
  NodeList const& GetComponent(size_t c) const { return this->Components[c]; }

  /** Get map from original node index to component index.  */
  std::vector<size_t> const& GetComponentMap() const
  {
    return this->TarjanComponents;
  }

  static const size_t INVALID_COMPONENT;

private:
  void TransferEdges();

  Graph const& InputGraph;
  Graph ComponentGraph;

  // Tarjan's algorithm.
  struct TarjanEntry
  {
    size_t Root;
    size_t VisitIndex;
  };
  std::vector<size_t> TarjanVisited;
  std::vector<size_t> TarjanComponents;
  std::vector<TarjanEntry> TarjanEntries;
  std::vector<NodeList> Components;
  std::stack<size_t> TarjanStack;
  size_t TarjanWalkId;
  size_t TarjanIndex;
  void Tarjan();
  void TarjanVisit(size_t i);

  // Connected components.
};
