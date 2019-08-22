/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmIncludeCommand_h
#define cmIncludeCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;

/**
 * \brief cmIncludeCommand defines a list of distant
 *  files that can be "included" in the current list file.
 *  In almost every sense, this is identical to a C/C++
 *  #include command.  Arguments are first expended as usual.
 */
bool cmIncludeCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status);

#endif
