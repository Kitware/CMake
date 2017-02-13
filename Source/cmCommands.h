/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCommands_h
#define cmCommands_h

#include <cmConfigure.h> // IWYU pragma: keep

#include <vector>

class cmCommand;
/**
 * Global function to return all compiled in commands.
 * To add a new command edit cmCommands.cxx and add your command.
 * It is up to the caller to delete the commands created by this call.
 */
std::vector<cmCommand*> GetPredefinedCommands();

#endif
