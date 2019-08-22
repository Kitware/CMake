/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmExecProgramCommand_h
#define cmExecProgramCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Command that adds a target to the build system.
 *
 * cmExecProgramCommand adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
bool cmExecProgramCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status);

#endif
