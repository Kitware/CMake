/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

class cmExecutionStatus;
class cmMakefile;

class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  bool IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile& mf,
                         cmExecutionStatus&) override;
  bool ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf) override;

  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;
  bool IsBlocking;
  bool HasRun = false;
  bool ElseSeen = false;
  unsigned int ScopeDepth = 0;
};

/// Starts an if block
bool cmIfCommand(std::vector<cmListFileArgument> const& args,
                 cmExecutionStatus& status);

#endif
