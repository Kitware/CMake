/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSeparateArgumentsCommand_h
#define cmSeparateArgumentsCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief separate_arguments command
 *
 * cmSeparateArgumentsCommand implements the separate_arguments CMake command
 */
bool cmSeparateArgumentsCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status);

#endif
