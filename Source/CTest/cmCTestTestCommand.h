/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestTestCommand_h
#define cmCTestTestCommand_h

#include "cmCTestHandlerCommand.h"

/** \class cmCTestTest
 * \brief Run a ctest script
 *
 * cmCTestTestCommand defineds the command to test the project.
 */
class cmCTestTestCommand : public cmCTestHandlerCommand
{
public:

  cmCTestTestCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestTestCommand* ni = new cmCTestTestCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "ctest_test";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Run tests in the project build tree.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  ctest_test([BUILD build_dir] [APPEND]\n"
      "             [START start number] [END end number]\n"
      "             [STRIDE stride number] [EXCLUDE exclude regex ]\n"
      "             [INCLUDE include regex] [RETURN_VALUE res] \n" 
      "             [EXCLUDE_LABEL exclude regex] \n"
      "             [INCLUDE_LABEL label regex] \n"
      "             [PARALLEL_LEVEL level] \n"
      "             [SCHEDULE_RANDOM on] \n"
      "             [STOP_TIME time of day]) \n"
      "Tests the given build directory and stores results in Test.xml. The "
      "second argument is a variable that will hold value. Optionally, "
      "you can specify the starting test number START, the ending test number "
      "END, the number of tests to skip between each test STRIDE, a regular "
      "expression for tests to run INCLUDE, or a regular expression for tests "
      "to not run EXCLUDE. EXCLUDE_LABEL and INCLUDE_LABEL are regular "
      "expression for test to be included or excluded by the test "
      "property LABEL. PARALLEL_LEVEL should be set to a positive number "
      "representing the number of tests to be run in parallel. "
      "SCHEDULE_RANDOM will launch tests in a random order, and is "
      "typically used to detect implicit test dependencies. STOP_TIME is the "
      "time of day at which the tests should all stop running."
      "\n"
      CTEST_COMMAND_APPEND_OPTION_DOCS;
    }

  cmTypeMacro(cmCTestTestCommand, cmCTestHandlerCommand);

protected:
  virtual cmCTestGenericHandler* InitializeActualHandler();
  cmCTestGenericHandler* InitializeHandler();

  enum {
    ctt_BUILD = ct_LAST,
    ctt_RETURN_VALUE,
    ctt_START,
    ctt_END,
    ctt_STRIDE,
    ctt_EXCLUDE,
    ctt_INCLUDE,
    ctt_EXCLUDE_LABEL,
    ctt_INCLUDE_LABEL,
    ctt_PARALLEL_LEVEL,
    ctt_SCHEDULE_RANDOM,
    ctt_STOP_TIME,
    ctt_LAST
  };
};


#endif
