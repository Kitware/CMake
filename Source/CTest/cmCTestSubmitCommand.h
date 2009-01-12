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
#ifndef cmCTestSubmitCommand_h
#define cmCTestSubmitCommand_h

#include "cmCTestHandlerCommand.h"
#include "cmCTest.h"

/** \class cmCTestSubmit
 * \brief Run a ctest script
 *
 * cmCTestSubmitCommand defineds the command to submit the test results for
 * the project.
 */
class cmCTestSubmitCommand : public cmCTestHandlerCommand
{
public:

  cmCTestSubmitCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestSubmitCommand* ni = new cmCTestSubmitCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ctest_submit";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Submits the repository.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_submit([RETURN_VALUE res] [PARTS ...])\n"
      "Submits the test results for the project.  "
      "By default all available parts are submitted.  "
      "The PARTS option lists a subset of parts to be submitted.";
    }

  cmTypeMacro(cmCTestSubmitCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();

  virtual bool CheckArgumentKeyword(std::string const& arg);
  virtual bool CheckArgumentValue(std::string const& arg);
  enum
  {
    ArgumentDoingParts = Superclass::ArgumentDoingLast1,
    ArgumentDoingLast2
  };
  std::set<cmCTest::Part> Parts;
};


#endif
