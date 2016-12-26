/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

struct cmListFileArgument;

void cmCommand::SetExecutionStatus(cmExecutionStatus* status)
{
  this->Status = status;
  this->Makefile = &status->GetMakefile();
}

bool cmCommand::InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                                  cmExecutionStatus& status)
{
  std::vector<std::string> expandedArguments;
  if (!this->Makefile->ExpandArguments(args, expandedArguments)) {
    // There was an error expanding arguments.  It was already
    // reported, so we can skip this command without error.
    return true;
  }
  return this->InitialPass(expandedArguments, status);
}

void cmCommand::SetError(const std::string& e)
{
  this->Status->SetError(e);
}
