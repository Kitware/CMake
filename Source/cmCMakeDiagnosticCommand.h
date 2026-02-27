/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Set how CMake should handle diagnostics
 *
 * cmCMakeDiagnosticCommand sets how CMake should deal with diagnostics.
 */
bool cmCMakeDiagnosticCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status);
