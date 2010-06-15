/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestBuildCommand_h
#define cmCTestBuildCommand_h

#include "cmCTestHandlerCommand.h"

class cmGlobalGenerator;
class cmCTestBuildHandler;

/** \class cmCTestBuild
 * \brief Run a ctest script
 *
 * cmCTestBuildCommand defineds the command to build the project.
 */
class cmCTestBuildCommand : public cmCTestHandlerCommand
{
public:

  cmCTestBuildCommand();
  ~cmCTestBuildCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestBuildCommand* ni = new cmCTestBuildCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ctest_build";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Build the project.";
    }
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_build([BUILD build_dir] [TARGET target] [RETURN_VALUE res]\n"
      "              [APPEND][NUMBER_ERRORS val] [NUMBER_WARNINGS val])\n"
      "Builds the given build directory and stores results in Build.xml. "
      "If no BUILD is given, the CTEST_BINARY_DIRECTORY variable is used.\n"
      "The TARGET variable can be used to specify a build target.  If none "
      "is specified, the \"all\" target will be built.\n"
      "The RETURN_VALUE option specifies a variable in which to store the "
      "return value of the native build tool. "
      "The NUMBER_ERRORS and NUMBER_WARNINGS options specify variables in "
      "which to store the number of build errors and warnings detected."
      "\n"
      CTEST_COMMAND_APPEND_OPTION_DOCS;
    }

  cmTypeMacro(cmCTestBuildCommand, cmCTestHandlerCommand);

  cmGlobalGenerator* GlobalGenerator;

protected:
  cmCTestBuildHandler* Handler;
  enum {
    ctb_BUILD = ct_LAST,
    ctb_NUMBER_ERRORS,
    ctb_NUMBER_WARNINGS,
    ctb_TARGET,
    ctb_CONFIGURATION,
    ctb_FLAGS,
    ctb_PROJECT_NAME,
    ctb_LAST
  };

  cmCTestGenericHandler* InitializeHandler();
};


#endif
