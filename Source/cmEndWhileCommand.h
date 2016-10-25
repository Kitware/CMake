/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmEndWhileCommand_h
#define cmEndWhileCommand_h

#include <cmConfigure.h>
#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;
struct cmListFileArgument;

/** \class cmEndWhileCommand
 * \brief ends a while loop
 *
 * cmEndWhileCommand ends a while loop
 */
class cmEndWhileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmEndWhileCommand; }

  /**
   * Override cmCommand::InvokeInitialPass to get arguments before
   * expansion.
   */
  bool InvokeInitialPass(std::vector<cmListFileArgument> const& args,
                         cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const&,
                   cmExecutionStatus&) CM_OVERRIDE
  {
    return false;
  }

  /**
   * This determines if the command is invoked when in script mode.
   */
  bool IsScriptable() const CM_OVERRIDE { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "endwhile"; }
};

#endif
