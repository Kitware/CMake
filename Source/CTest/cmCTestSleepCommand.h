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
#ifndef cmCTestSleepCommand_h
#define cmCTestSleepCommand_h

#include "cmCTestCommand.h"

/** \class cmCTestSleep
 * \brief Run a ctest script
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmCTestSleepCommand : public cmCTestCommand
{
public:

  cmCTestSleepCommand() {}
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestSleepCommand* ni = new cmCTestSleepCommand;
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
  virtual const char* GetName() { return "CTEST_SLEEP";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "sleeps for some amount of time";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  CTEST_SLEEP( seconds )\n"
      "  CTEST_SLEEP( time1 duration time2 )\n"
      "With one argument it will sleep for a given number of seconds. "
      "With three arguments it will wait for time2 - time1 - duration "
      "seconds.";
    }

  cmTypeMacro(cmCTestSleepCommand, cmCTestCommand);

};


#endif
