/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmWriteFileCommand_h
#define cmWriteFileCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Writes a message to a file
 *
 */
bool cmWriteFileCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status);

#endif
