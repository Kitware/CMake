/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCMakeCommand_h
#define cmCMakeCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

class cmExecutionStatus;
struct cmListFileArgument;

/**
 * \brief Calls a scripted or build-in command
 *
 */
bool cmCMakeCommand(std::vector<cmListFileArgument> const& args,
                    cmExecutionStatus& status);

#endif
