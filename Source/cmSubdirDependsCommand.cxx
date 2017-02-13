/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSubdirDependsCommand.h"

#include "cmPolicies.h"

class cmExecutionStatus;

bool cmSubdirDependsCommand::InitialPass(std::vector<std::string> const&,
                                         cmExecutionStatus&)
{
  this->Disallowed(
    cmPolicies::CMP0029,
    "The subdir_depends command should not be called; see CMP0029.");
  return true;
}
