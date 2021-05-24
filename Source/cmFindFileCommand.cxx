/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindFileCommand.h"

#include "cmStateTypes.h"

class cmExecutionStatus;

cmFindFileCommand::cmFindFileCommand(cmExecutionStatus& status)
  : cmFindPathCommand("find_file", status)
{
  this->IncludeFileInPath = true;
  this->VariableType = cmStateEnums::FILEPATH;
}

bool cmFindFile(std::vector<std::string> const& args,
                cmExecutionStatus& status)
{
  return cmFindFileCommand(status).InitialPass(args);
}
