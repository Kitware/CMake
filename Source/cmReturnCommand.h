/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmReturnCommand_h
#define cmReturnCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/// Return from a directory or function
bool cmReturnCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status);

#endif
