/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindFileCommand.h"

class cmExecutionStatus;

cmFindFileCommand::cmFindFileCommand(cmExecutionStatus& status)
  : cmFindPathCommand(status)
{
  this->IncludeFileInPath = true;
}

bool cmFindFile(std::vector<std::string> const& args,
                cmExecutionStatus& status)
{
  return cmFindFileCommand(status).InitialPass(args);
}
