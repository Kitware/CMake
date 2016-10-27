/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEndIfCommand.h"

#include <stdlib.h> // required for atof

#include "cmMakefile.h"

class cmExecutionStatus;

bool cmEndIfCommand::InitialPass(std::vector<std::string> const&,
                                 cmExecutionStatus&)
{
  const char* versionValue =
    this->Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (!versionValue || (atof(versionValue) <= 1.4)) {
    return true;
  }

  this->SetError("An ENDIF command was found outside of a proper "
                 "IF ENDIF structure. Or its arguments did not match "
                 "the opening IF command.");
  return false;
}
