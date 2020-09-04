/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief site_name command
 *
 * cmSiteNameCommand implements the site_name CMake command
 */
bool cmSiteNameCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status);
