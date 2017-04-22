/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSetDirectoryPropertiesCommand_h
#define cmSetDirectoryPropertiesCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;
class cmMakefile;

class cmSetDirectoryPropertiesCommand : public cmCommand
{
public:
  cmCommand* Clone() CM_OVERRIDE
  {
    return new cmSetDirectoryPropertiesCommand;
  }

  /**
   * This is called when the command is first encountered in
   * the input file.
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
  std::string GetName() const CM_OVERRIDE
  {
    return "set_directory_properties";
  }

  /**
   * Static entry point for use by other commands
   */
  static bool RunCommand(cmMakefile* mf,
                         std::vector<std::string>::const_iterator ait,
                         std::vector<std::string>::const_iterator aitend,
                         std::string& errors);
};

#endif
