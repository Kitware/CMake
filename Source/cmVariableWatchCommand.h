/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmVariableWatchCommand_h
#define cmVariableWatchCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Watch when the variable changes and invoke command
 *
 */
bool cmVariableWatchCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status);

#endif
