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
  virtual const char* GetName() { return "ctest_test";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Tests the repository.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  CTEST_TEST([BUILD build_dir]\n"
      "             [START start number] [END end number]\n"
      "             [STRIDE stride number] [EXCLUDE exclude regex ]\n"
      "             [INCLUDE include regex] [RETURN_VALUE res] )\n"
      "Tests the given build directory and stores results in Test.xml. The "
      "second argument is a variable that will hold value. Optionally, "
      "you can specify the starting test number START, the ending test number "
      "END, the number of tests to skip between each test STRIDE, a regular "
      "expression for tests to run INCLUDE, or a regular expression for tests "
      "to not run EXCLUDE.";
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
    ctt_LAST
  };
};


#endif
