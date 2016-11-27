/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEndWhileCommand.h"

class cmExecutionStatus;

bool cmEndWhileCommand::InitialPass(std::vector<std::string> const&,
                                    cmExecutionStatus&)
{
  this->SetError("An ENDWHILE command was found outside of a proper "
                 "WHILE ENDWHILE structure. Or its arguments did not "
                 "match the opening WHILE command.");
  return false;
}
