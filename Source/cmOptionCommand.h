/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Provide an option to the user
 *
 * cmOptionCommand provides an option for the user to select
 */
bool cmOptionCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status);
