/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

#include <cm/string_view>

#include "cmListFileCache.h"

class cmExecutionStatus;
class cmMakefile;

class cmFunctionBlocker
{
public:
  /**
   * should a function be blocked
   */
  bool IsFunctionBlocked(cmListFileFunction const& lff,
                         cmExecutionStatus& status);

  virtual ~cmFunctionBlocker() = default;

  /** Set/Get the context in which this blocker is created.  */
  void SetStartingContext(cmListFileContext const& lfc)
  {
    this->StartingContext = lfc;
  }
  cmListFileContext const& GetStartingContext() const
  {
    return this->StartingContext;
  }

private:
  virtual cm::string_view StartCommandName() const = 0;
  virtual cm::string_view EndCommandName() const = 0;

  virtual bool ArgumentsMatch(cmListFileFunction const& lff,
                              cmMakefile& mf) const = 0;

  virtual bool Replay(std::vector<cmListFileFunction> functions,
                      cmExecutionStatus& status) = 0;

  cmListFileContext StartingContext;
  std::vector<cmListFileFunction> Functions;
  unsigned int ScopeDepth = 1;
};
