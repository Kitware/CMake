/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSetDirectoryPropertiesCommand_h
#define cmSetDirectoryPropertiesCommand_h

#include "cmCommand.h"

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

  cmTypeMacro(cmSetDirectoryPropertiesCommand, cmCommand);
};

#endif
