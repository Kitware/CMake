/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Command that adds a target to the build system.
 *
 * cmExecuteProcessCommand is a CMake language interface to the KWSys
 * Process Execution implementation.
 */
bool cmExecuteProcessCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status);
