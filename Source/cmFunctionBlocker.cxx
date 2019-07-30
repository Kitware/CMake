/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFunctionBlocker.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmFunctionBlocker::IsFunctionBlocked(const cmListFileFunction& lff,
                                          cmExecutionStatus& status)
{
  if (lff.Name.Lower == this->StartCommandName()) {
    this->ScopeDepth++;
  } else if (lff.Name.Lower == this->EndCommandName()) {
    this->ScopeDepth--;
    if (this->ScopeDepth == 0U) {
      cmMakefile& mf = status.GetMakefile();
      auto self = mf.RemoveFunctionBlocker(this, lff);
      return this->Replay(this->Functions, status);
    }
  }

  this->Functions.push_back(lff);
  return true;
}
