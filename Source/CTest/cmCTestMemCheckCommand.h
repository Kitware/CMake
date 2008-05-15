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
    return "Tests the repository.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_memcheck([BUILD build_dir] [RETURN_VALUE res])\n"
      "Performs a memory checking of tests in the given build directory and "
      "stores results in MemCheck.xml. The second argument is a variable "
      "that will hold value.";
    }

  cmTypeMacro(cmCTestMemCheckCommand, cmCTestTestCommand);

protected:
  cmCTestGenericHandler* InitializeActualHandler();
};


#endif

