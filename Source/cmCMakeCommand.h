/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCMakeCommand_h
#define cmCMakeCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Calls a scripted or build-in command
 *
 */
bool cmCMakeCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status);

#endif
