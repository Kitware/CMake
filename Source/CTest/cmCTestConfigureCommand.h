/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestConfigureCommand_h
#define cmCTestConfigureCommand_h

#include "cmCTestHandlerCommand.h"

/** \class cmCTestConfigure
 * \brief Run a ctest script
 *
 * cmCTestConfigureCommand defineds the command to configures the project.
 */
class cmCTestConfigureCommand : public cmCTestHandlerCommand
{
public:
  cmCTestConfigureCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestConfigureCommand* ni = new cmCTestConfigureCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "ctest_configure";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Configure the project build tree.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  ctest_configure([BUILD build_dir] [SOURCE source_dir] [APPEND]\n"
      "                  [OPTIONS options] [RETURN_VALUE res])\n"
      "Configures the given build directory and stores results in "
      "Configure.xml. "
      "If no BUILD is given, the CTEST_BINARY_DIRECTORY variable is used. "
      "If no SOURCE is given, the CTEST_SOURCE_DIRECTORY variable is used. "
      "The OPTIONS argument specifies command line arguments to pass to "
      "the configuration tool. "
      "The RETURN_VALUE option specifies a variable in which to store the "
      "return value of the native build tool."
      "\n"
      CTEST_COMMAND_APPEND_OPTION_DOCS;
    }

  cmTypeMacro(cmCTestConfigureCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();

  enum {
    ctc_FIRST = ct_LAST,
    ctc_OPTIONS,
    ctc_LAST
  };
};


#endif
