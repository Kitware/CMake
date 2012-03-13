/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAddCustomTargetCommand_h
#define cmAddCustomTargetCommand_h

#include "cmCommand.h"

/** \class cmAddCustomTargetCommand
 * \brief Command that adds a target to the build system.
 *
 * cmAddCustomTargetCommand adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
class cmAddCustomTargetCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddCustomTargetCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const
    {return "add_custom_target";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Add a target with no output so it will always be built.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_custom_target(Name [ALL] [command1 [args1...]]\n"
      "                    [COMMAND command2 [args2...] ...]\n"
      "                    [DEPENDS depend depend depend ... ]\n"
      "                    [WORKING_DIRECTORY dir]\n"
      "                    [COMMENT comment] [VERBATIM]\n"
      "                    [SOURCES src1 [src2...]])\n"
      "Adds a target with the given name that executes the given commands. "
      "The target has no output file and is ALWAYS CONSIDERED OUT OF DATE "
      "even if the commands try to create a file with the name of the "
      "target. Use ADD_CUSTOM_COMMAND to generate a file with dependencies. "
      "By default nothing depends on the custom target. Use "
      "ADD_DEPENDENCIES to add dependencies to or from other targets. "
      "If the ALL option is specified "
      "it indicates that this target should be added to the default build "
      "target so that it will be run every time "
      "(the command cannot be called ALL). "
      "The command and arguments are optional and if not specified an "
      "empty target will be created. "
      "If WORKING_DIRECTORY is set, then the command will be run in that "
      "directory. "
      "If it is a relative path it will be interpreted relative to the "
      "build tree directory corresponding to the current source directory. "
      "If COMMENT is set, the value will be displayed as a "
      "message before the commands are executed at build time. "
      "Dependencies listed with the DEPENDS argument may reference files "
      "and outputs of custom commands created with add_custom_command() in "
      "the same directory (CMakeLists.txt file).\n"
      "If VERBATIM is given then all arguments to the commands will be "
      "escaped properly for the build tool so that the invoked command "
      "receives each argument unchanged.  "
      "Note that one level of escapes is still used by the CMake language "
      "processor before add_custom_target even sees the arguments. "
      "Use of VERBATIM is recommended as it enables correct behavior. "
      "When VERBATIM is not given the behavior is platform specific because "
      "there is no protection of tool-specific special characters."
      "\n"
      "The SOURCES option specifies additional source files to be included "
      "in the custom target.  "
      "Specified source files will be added to IDE project files for "
      "convenience in editing even if they have not build rules."
      ;
    }
  
  cmTypeMacro(cmAddCustomTargetCommand, cmCommand);
};

#endif
