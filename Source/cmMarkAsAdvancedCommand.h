/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmMarkAsAdvancedCommand_h
#define cmMarkAsAdvancedCommand_h

#include <cmConfigure.h>
#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmMarkAsAdvancedCommand
 * \brief mark_as_advanced command
 *
 * cmMarkAsAdvancedCommand implements the mark_as_advanced CMake command
 */
class cmMarkAsAdvancedCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmMarkAsAdvancedCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "mark_as_advanced"; }

  /**
   * This determines if the command is invoked when in script mode.
   * mark_as_advanced() will have no effect in script mode, but this will
   * make many of the modules usable in cmake/ctest scripts, (among them
   * FindUnixMake.cmake used by the CTEST_BUILD command.
  */
  bool IsScriptable() const CM_OVERRIDE { return true; }
};

#endif
