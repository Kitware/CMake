/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

class cmExecutionStatus;
struct cmListFileArgument;

/// Starts an if block
bool cmIfCommand(std::vector<cmListFileArgument> const& args,
                 cmExecutionStatus& status);

#endif
