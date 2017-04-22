/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmUnsetCommand_h
#define cmUnsetCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmUnsetCommand
 * \brief Unset a CMAKE variable
 *
 * cmUnsetCommand unsets or removes a variable.
 */
class cmUnsetCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmUnsetCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * This determines if the command is invoked when in script mode.
   */
  bool IsScriptable() const CM_OVERRIDE { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "unset"; }
};

#endif
