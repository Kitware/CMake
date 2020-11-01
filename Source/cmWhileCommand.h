/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <vector>

class cmExecutionStatus;
struct cmListFileArgument;

/// \brief Starts a while loop
bool cmWhileCommand(std::vector<cmListFileArgument> const& args,
                    cmExecutionStatus& status);
