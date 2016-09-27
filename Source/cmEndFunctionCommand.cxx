/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEndFunctionCommand.h"

bool cmEndFunctionCommand::InvokeInitialPass(
  std::vector<cmListFileArgument> const&, cmExecutionStatus&)
{
  this->SetError("An ENDFUNCTION command was found outside of a proper "
                 "FUNCTION ENDFUNCTION structure. Or its arguments did not "
                 "match the opening FUNCTION command.");
  return false;
}
