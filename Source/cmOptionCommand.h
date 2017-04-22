/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmOptionCommand_h
#define cmOptionCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmOptionCommand
 * \brief Provide an option to the user
 *
 * cmOptionCommand provides an option for the user to select
 */
class cmOptionCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmOptionCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "option"; }

  /**
   * This determines if the command is invoked when in script mode.
   */
  bool IsScriptable() const CM_OVERRIDE { return true; }
};

#endif
