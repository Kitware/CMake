/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCMakePolicyCommand_h
#define cmCMakePolicyCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmCMakePolicyCommand
 * \brief Set how CMake should handle policies
 *
 * cmCMakePolicyCommand sets how CMake should deal with backwards
 * compatibility policies.
 */
class cmCMakePolicyCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmCMakePolicyCommand; }

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
  std::string GetName() const CM_OVERRIDE { return "cmake_policy"; }

private:
  bool HandleSetMode(std::vector<std::string> const& args);
  bool HandleGetMode(std::vector<std::string> const& args);
  bool HandleVersionMode(std::vector<std::string> const& args);
};

#endif
