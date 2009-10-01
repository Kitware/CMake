/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ctest_sleep";}

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
      "  ctest_sleep( seconds )\n"
      "  ctest_sleep( time1 duration time2 )\n"
      "With one argument it will sleep for a given number of seconds. "
      "With three arguments it will wait for time2 - time1 - duration "
      "seconds.";
    }

  cmTypeMacro(cmCTestSleepCommand, cmCTestCommand);

};


#endif
