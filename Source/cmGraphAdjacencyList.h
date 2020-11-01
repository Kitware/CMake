/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <utility>
#include <vector>

#include "cmListFileCache.h"

/**
 * Graph edge representation.  Most use cases just need the
 * destination vertex, so we support conversion to/from an int.  We
 * also store boolean to indicate whether an edge is "strong".
 */
class cmGraphEdge
{
public:
  cmGraphEdge(int n, bool s, bool c, cmListFileBacktrace bt)
    : Dest(n)
    , Strong(s)
    , Cross(c)
    , Backtrace(std::move(bt))
  {
  }
  operator int() const { return this->Dest; }

  bool IsStrong() const { return this->Strong; }

  bool IsCross() const { return this->Cross; }

  cmListFileBacktrace const& GetBacktrace() const { return this->Backtrace; }

private:
  int Dest;
  bool Strong;
  bool Cross;
  cmListFileBacktrace Backtrace;
};
struct cmGraphEdgeList : public std::vector<cmGraphEdge>
{
};
struct cmGraphNodeList : public std::vector<int>
{
};
struct cmGraphAdjacencyList : public std::vector<cmGraphEdgeList>
{
};
