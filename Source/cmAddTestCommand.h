/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAddTestCommand_h
#define cmAddTestCommand_h

#include "cmCommand.h"
#include "cmDocumentGeneratorExpressions.h"

/** \class cmAddTestCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmAddTestCommand adds a test to the list of tests to run .
 */
class cmAddTestCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddTestCommand;
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
  virtual const char* GetName() const { return "add_test";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Add a test to the project with the specified arguments.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_test(testname Exename arg1 arg2 ... )\n"
      "If the ENABLE_TESTING command has been run, this command adds a "
      "test target to the current directory. If ENABLE_TESTING has not "
      "been run, this command does nothing.  "
      "The tests are run by the testing subsystem by executing Exename "
      "with the specified arguments.  Exename can be either an executable "
      "built by this project or an arbitrary executable on the "
      "system (like tclsh).  The test will be run with the current working "
      "directory set to the CMakeList.txt files corresponding directory "
      "in the binary tree.\n"
      "\n"
      "  add_test(NAME <name> [CONFIGURATIONS [Debug|Release|...]]\n"
      "           [WORKING_DIRECTORY dir]\n"
      "           COMMAND <command> [arg1 [arg2 ...]])\n"
      "If COMMAND specifies an executable target (created by "
      "add_executable) it will automatically be replaced by the location "
      "of the executable created at build time.  "
      "If a CONFIGURATIONS option is given then the test will be executed "
      "only when testing under one of the named configurations.  "
      "If a WORKING_DIRECTORY option is given then the test will be executed "
      "in the given directory."
      "\n"
      "Arguments after COMMAND may use \"generator expressions\" with the "
      "syntax \"$<...>\".  "
      CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS
      "Example usage:\n"
      "  add_test(NAME mytest\n"
      "           COMMAND testDriver --config $<CONFIGURATION>\n"
      "                              --exe $<TARGET_FILE:myexe>)\n"
      "This creates a test \"mytest\" whose command runs a testDriver "
      "tool passing the configuration name and the full path to the "
      "executable file produced by target \"myexe\"."
      ;
    }
  
  cmTypeMacro(cmAddTestCommand, cmCommand);
private:
  bool HandleNameMode(std::vector<std::string> const& args);
};


#endif
