/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmBuildCommand_h
#define cmBuildCommand_h

#include "cmCommand.h"

/** \class cmBuildCommand
 * \brief build_command command
 *
 * cmBuildCommand implements the build_command CMake command
 */
class cmBuildCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmBuildCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The primary command signature with optional, KEYWORD-based args.
   */
  virtual bool MainSignature(std::vector<std::string> const& args);

  /**
   * Legacy "exactly 2 args required" signature.
   */
  virtual bool TwoArgsSignature(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "build_command";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get the command line to build this project.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  build_command(<variable>\n"
      "                [CONFIGURATION <config>]\n"
      "                [PROJECT_NAME <projname>]\n"
      "                [TARGET <target>])\n"
      "Sets the given <variable> to a string containing the command line "
      "for building one configuration of a target in a project using the "
      "build tool appropriate for the current CMAKE_GENERATOR.\n"
      "If CONFIGURATION is omitted, CMake chooses a reasonable default "
      "value  for multi-configuration generators.  CONFIGURATION is "
      "ignored for single-configuration generators.\n"
      "If PROJECT_NAME is omitted, the resulting command line will build "
      "the top level PROJECT in the current build tree.\n"
      "If TARGET is omitted, the resulting command line will build "
      "everything, effectively using build target 'all' or 'ALL_BUILD'.\n"
      "  build_command(<cachevariable> <makecommand>)\n"
      "This second signature is deprecated, but still available for "
      "backwards compatibility. Use the first signature instead.\n"
      "Sets the given <cachevariable> to a string containing the command "
      "to build this project from the root of the build tree using "
      "the build tool given by <makecommand>.  <makecommand> should be "
      "the full path to msdev, devenv, nmake, make or one of the end "
      "user build tools."
      ;
    }

  cmTypeMacro(cmBuildCommand, cmCommand);
};

#endif
