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

#include "cmCTestCommand.h"

/** \class cmCTestSubmit
 * \brief Run a ctest script
 *
 * cmCTestSubmitCommand defineds the command to submit the test results for the project.
 */
class cmCTestSubmitCommand : public cmCTestCommand
{
public:

  cmCTestSubmitCommand() {}
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestSubmitCommand* ni = new cmCTestSubmitCommand;
    ni->m_CTest = this->m_CTest;
    ni->m_CTestScriptHandler = this->m_CTestScriptHandler;
    return ni;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CTEST_SUBMIT";}

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
      "  CTEST_SUBMIT([RETURN_VALUE res])\n"
      "Submits the test results for the project.";
    }

  cmTypeMacro(cmCTestSubmitCommand, cmCTestCommand);

};


#endif
