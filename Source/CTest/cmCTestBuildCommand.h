/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
      "  ctest_build([BUILD build_dir] [RETURN_VALUE res] [APPEND]\n"
      "              [NUMBER_ERRORS val] [NUMBER_WARNINGS val])\n"
      "Builds the given build directory and stores results in Build.xml. "
      "If no BUILD is given, the CTEST_BINARY_DIRECTORY variable is used. "
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
    ctb_LAST
  };

  cmCTestGenericHandler* InitializeHandler();
};


#endif
