/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEndMacroCommand.h"

bool cmEndMacroCommand::InvokeInitialPass(
  std::vector<cmListFileArgument> const&, cmExecutionStatus&)
{
  this->SetError("An ENDMACRO command was found outside of a proper "
                 "MACRO ENDMACRO structure. Or its arguments did not "
                 "match the opening MACRO command.");
  return false;
}
