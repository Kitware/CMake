/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <set>

#include "cmListFileCache.h"

class cmGeneratorTarget;

/** One edge in the global target dependency graph.
    It may be marked as a 'link' or 'util' edge or both.  */
class cmTargetDepend
{
  cmGeneratorTarget const* Target;

  // The set order depends only on the Target, so we use
  // mutable members to achieve a map with set syntax.
  mutable bool Link;
  mutable bool Util;
  mutable bool Cross;
  mutable cmListFileBacktrace Backtrace;

public:
  cmTargetDepend(cmGeneratorTarget const* t)
    : Target(t)
    , Link(false)
    , Util(false)
    , Cross(false)
  {
  }
  operator cmGeneratorTarget const*() const { return this->Target; }
  cmGeneratorTarget const* operator->() const { return this->Target; }
  cmGeneratorTarget const& operator*() const { return *this->Target; }
  friend bool operator<(cmTargetDepend const& l, cmTargetDepend const& r)
  {
    return l.Target < r.Target;
  }
  void SetType(bool strong) const
  {
    if (strong) {
      this->Util = true;
    } else {
      this->Link = true;
    }
  }
  void SetCross(bool cross) const { this->Cross = cross; }
  void SetBacktrace(cmListFileBacktrace const& bt) const
  {
    this->Backtrace = bt;
  }
  bool IsLink() const { return this->Link; }
  bool IsUtil() const { return this->Util; }
  bool IsCross() const { return this->Cross; }
  cmListFileBacktrace const& GetBacktrace() const { return this->Backtrace; }
};

/** Unordered set of (direct) dependencies of a target. */
class cmTargetDependSet : public std::set<cmTargetDepend>
{
};
