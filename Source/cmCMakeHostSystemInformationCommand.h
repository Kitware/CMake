/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCMakeHostSystemInformationCommand_h
#define cmCMakeHostSystemInformationCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief Query host system specific information
 *
 * cmCMakeHostSystemInformationCommand queries system information of
 * the system on which CMake runs.
 */
bool cmCMakeHostSystemInformationCommand(std::vector<std::string> const& args,
                                         cmExecutionStatus& status);

#endif
