/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmTargetLinkLibrariesCommand_h
#define cmTargetLinkLibrariesCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

bool cmTargetLinkLibrariesCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status);

#endif
