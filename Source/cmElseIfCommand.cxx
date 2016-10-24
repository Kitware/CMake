/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmElseIfCommand.h"

class cmExecutionStatus;

bool cmElseIfCommand::InitialPass(std::vector<std::string> const&,
                                  cmExecutionStatus&)
{
  this->SetError("An ELSEIF command was found outside of a proper "
                 "IF ENDIF structure.");
  return false;
}
