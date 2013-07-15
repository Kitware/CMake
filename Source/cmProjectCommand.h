/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmProjectCommand_h
#define cmProjectCommand_h

#include "cmCommand.h"

/** \class cmProjectCommand
 * \brief Specify the name for this build project.
 *
 * cmProjectCommand is used to specify a name for this build project.
 * It is defined once per set of CMakeList.txt files (including
 * all subdirectories). Currently it just sets the name of the workspace
 * file for Microsoft Visual C++
 */
class cmProjectCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmProjectCommand;
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
  virtual const char* GetName() const {return "project";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Set a name for the entire project.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  project(<projectname> [languageName1 languageName2 ... ] )\n"
      "Sets the name of the project.  "
      "Additionally this sets the variables <projectName>_BINARY_DIR and "
      "<projectName>_SOURCE_DIR to the respective values.\n"
      "Optionally you can specify which languages your project supports.  "
      "Example languages are CXX (i.e. C++), C, Fortran, etc. "
      "By default C and CXX are enabled.  E.g. if you do not have a "
      "C++ compiler, you can disable the check for it by explicitly listing "
      "the languages you want to support, e.g. C.  By using the special "
      "language \"NONE\" all checks for any language can be disabled. "
      "If a variable exists called CMAKE_PROJECT_<projectName>_INCLUDE, "
      "the file pointed to by that variable will be included as the last step "
      "of the project command."
      "\n"
      "The top-level CMakeLists.txt file for a project must contain a "
      "literal, direct call to the project() command; loading one through "
      "the include() command is not sufficient.  "
      "If no such call exists CMake will implicitly add one to the top that "
      "enables the default languages (C and CXX).";
    }

  cmTypeMacro(cmProjectCommand, cmCommand);
};



#endif
