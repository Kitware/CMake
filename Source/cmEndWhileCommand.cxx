/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEndWhileCommand.h"

bool cmEndWhileCommand::InvokeInitialPass(
  std::vector<cmListFileArgument> const& args, cmExecutionStatus&)
{
  if (args.empty()) {
    this->SetError("An ENDWHILE command was found outside of a proper "
                   "WHILE ENDWHILE structure.");
  } else {
    this->SetError("An ENDWHILE command was found outside of a proper "
                   "WHILE ENDWHILE structure. Or its arguments did not "
                   "match the opening WHILE command.");
  }

  return false;
}
