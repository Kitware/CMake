/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestRunScriptCommand_h
#define cmCTestRunScriptCommand_h

#include "cmCTestCommand.h"

/** \class cmCTestRunScript
 * \brief Run a ctest script
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmCTestRunScriptCommand : public cmCTestCommand
{
public:

  cmCTestRunScriptCommand() {}
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestRunScriptCommand* ni = new cmCTestRunScriptCommand;
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
  virtual const char* GetName() { return "ctest_run_script";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "runs a ctest -S script";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_run_script([NEW_PROCESS] script_file_name script_file_name1 \n"
      "              script_file_name2 ... [RETURN_VALUE var])\n"
      "Runs a script or scripts much like if it was run from ctest -S. "
      "If no argument is provided then the current script is run using "
      "the current settings of the variables. If NEW_PROCESS is specified "
      "then each script will be run in a seperate process."
      "If RETURN_VALUE is specified the return value of the last script "
      "run will be put into var.";
    }

  cmTypeMacro(cmCTestRunScriptCommand, cmCTestCommand);
};


#endif
