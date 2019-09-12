/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFindFileCommand_h
#define cmFindFileCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmFindPathCommand.h"

class cmExecutionStatus;

/** \class cmFindFileCommand
 * \brief Define a command to search for an executable program.
 *
 * cmFindFileCommand is used to define a CMake variable
 * that specifies an executable program. The command searches
 * in the current path (e.g., PATH environment variable) for
 * an executable that matches one of the supplied names.
 */
class cmFindFileCommand : public cmFindPathCommand
{
public:
  cmFindFileCommand(cmExecutionStatus& status);
};

bool cmFindFile(std::vector<std::string> const& args,
                cmExecutionStatus& status);

#endif
