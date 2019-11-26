/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCMakePolicyCommand_h
#define cmCMakePolicyCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Set how CMake should handle policies
 *
 * cmCMakePolicyCommand sets how CMake should deal with backwards
 * compatibility policies.
 */
bool cmCMakePolicyCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status);

#endif
