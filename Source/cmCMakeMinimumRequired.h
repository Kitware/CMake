/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCMakeMinimumRequired_h
#define cmCMakeMinimumRequired_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief cmake_minimum_required command
 *
 * cmCMakeMinimumRequired implements the cmake_minimum_required CMake command
 */
bool cmCMakeMinimumRequired(std::vector<std::string> const& args,
                            cmExecutionStatus& status);

#endif
