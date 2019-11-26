/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmMakeDirectoryCommand_h
#define cmMakeDirectoryCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Specify auxiliary source code directories.
 *
 * cmMakeDirectoryCommand specifies source code directories
 * that must be built as part of this build process. This directories
 * are not recursively processed like the SUBDIR command (cmSubdirCommand).
 * A side effect of this command is to create a subdirectory in the build
 * directory structure.
 */
bool cmMakeDirectoryCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status);

#endif
