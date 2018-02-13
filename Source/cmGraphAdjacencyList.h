/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGraphAdjacencyList_h
#define cmGraphAdjacencyList_h

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmListFileCache.h"

#include <vector>

/**
 * Graph edge representation.  Most use cases just need the
 * destination vertex, so we support conversion to/from an int.  We
 * also store boolean to indicate whether an edge is "strong".
 */
class cmGraphEdge
{
public:
  cmGraphEdge(int n, bool s, cmListFileBacktrace const * pbt)
    : Dest(n)
    , Strong(s)
    , backtrace()
  {
    if (pbt != nullptr) {
      backtrace = *pbt;
    }
  }

  // Graph edges may be copied and assigned as values.
  cmGraphEdge(cmGraphEdge const& r)
     : Dest(r.Dest)
     , Strong(r.Strong)
     , backtrace(r.backtrace)
  {
     
  }
  cmGraphEdge& operator=(cmGraphEdge const& r)
  {
      this->Dest = r.Dest;
      this->Strong = r.Strong;
      this->backtrace = r.backtrace;
      return *this;
  }

  operator int() const { return this->Dest; }

  bool IsStrong() const { return this->Strong; }

  cmListFileBacktrace const& Backtrace() const { return this->backtrace; }

private:
  int Dest;
  bool Strong;
  cmListFileBacktrace backtrace;
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

#endif
