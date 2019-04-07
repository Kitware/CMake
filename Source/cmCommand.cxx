/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommand.h"

#include <utility>

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

cmLegacyCommandWrapper::cmLegacyCommandWrapper(std::unique_ptr<cmCommand> cmd)
  : Command(std::move(cmd))
{
}

cmLegacyCommandWrapper::cmLegacyCommandWrapper(
  cmLegacyCommandWrapper const& other)
  : Command(other.Command->Clone())
{
}

cmLegacyCommandWrapper& cmLegacyCommandWrapper::operator=(
  cmLegacyCommandWrapper const& other)
{
  this->Command = other.Command->Clone();
  return *this;
}

bool cmLegacyCommandWrapper::operator()(
  std::vector<cmListFileArgument> const& args, cmExecutionStatus& status) const
{
  auto cmd = this->Command->Clone();
  cmd->SetExecutionStatus(&status);
  return cmd->InvokeInitialPass(args, status);
}
