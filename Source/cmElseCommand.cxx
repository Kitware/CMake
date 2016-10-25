/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmElseCommand.h"

class cmExecutionStatus;

bool cmElseCommand::InitialPass(std::vector<std::string> const&,
                                cmExecutionStatus&)
{
  this->SetError("An ELSE command was found outside of a proper "
                 "IF ENDIF structure. Or its arguments did not match "
                 "the opening IF command.");
  return false;
}
