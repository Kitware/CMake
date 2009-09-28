/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestMemCheckCommand_h
#define cmCTestMemCheckCommand_h

#include "cmCTestTestCommand.h"

class cmCTestGenericHandler;

/** \class cmCTestMemCheck
 * \brief Run a ctest script
 *
 * cmCTestMemCheckCommand defineds the command to test the project.
 */
class cmCTestMemCheckCommand : public cmCTestTestCommand
{
public:

  cmCTestMemCheckCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestMemCheckCommand* ni = new cmCTestMemCheckCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ctest_memcheck";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Run tests with a dynamic analysis tool.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_memcheck([BUILD build_dir] [RETURN_VALUE res] [APPEND]\n"
      "             [START start number] [END end number]\n"
      "             [STRIDE stride number] [EXCLUDE exclude regex ]\n"
      "             [INCLUDE include regex] \n" 
      "             [EXCLUDE_LABEL exclude regex] \n"
      "             [INCLUDE_LABEL label regex] \n"
      "             [PARALLEL_LEVEL level] )\n"
      "Tests the given build directory and stores results in MemCheck.xml. "
      "The second argument is a variable that will hold value. Optionally, "
      "you can specify the starting test number START, the ending test number "
      "END, the number of tests to skip between each test STRIDE, a regular "
      "expression for tests to run INCLUDE, or a regular expression for tests "
      "not to run EXCLUDE. EXCLUDE_LABEL and INCLUDE_LABEL are regular "
      "expressions for tests to be included or excluded by the test "
      "property LABEL. PARALLEL_LEVEL should be set to a positive number "
      "representing the number of tests to be run in parallel."
      "\n"
      CTEST_COMMAND_APPEND_OPTION_DOCS;
    }

  cmTypeMacro(cmCTestMemCheckCommand, cmCTestTestCommand);

protected:
  cmCTestGenericHandler* InitializeActualHandler();
};


#endif

