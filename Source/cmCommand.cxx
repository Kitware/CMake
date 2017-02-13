/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommand.h"

#include "cmMakefile.h"
#include "cmake.h"

class cmExecutionStatus;
struct cmListFileArgument;

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

const char* cmCommand::GetError()
{
  if (this->Error.empty()) {
    this->Error = this->GetName();
    this->Error += " unknown error.";
  }
  return this->Error.c_str();
}

void cmCommand::SetError(const std::string& e)
{
  this->Error = this->GetName();
  this->Error += " ";
  this->Error += e;
}

bool cmCommand::Disallowed(cmPolicies::PolicyID pol, const char* e)
{
  switch (this->Makefile->GetPolicyStatus(pol)) {
    case cmPolicies::WARN:
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING,
                                   cmPolicies::GetPolicyWarning(pol));
    case cmPolicies::OLD:
      return false;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e);
      break;
  }
  return true;
}
