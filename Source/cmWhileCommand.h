/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmWhileCommand_h
#define cmWhileCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

class cmExecutionStatus;
class cmMakefile;

class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker(cmMakefile* mf);
  ~cmWhileFunctionBlocker() override;
  bool IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile& mf,
                         cmExecutionStatus&) override;
  bool ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf) override;

  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;

private:
  cmMakefile* Makefile;
  int Depth;
};

/// \brief Starts a while loop
bool cmWhileCommand(std::vector<cmListFileArgument> const& args,
                    cmExecutionStatus& status);

#endif
