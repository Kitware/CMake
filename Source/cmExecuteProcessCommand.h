/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExecuteProcessCommand_h
#define cmExecuteProcessCommand_h

#include "cmCommand.h"

/** \class cmExecuteProcessCommand
 * \brief Command that adds a target to the build system.
 *
 * cmExecuteProcessCommand is a CMake language interface to the KWSys
 * Process Execution implementation.
 */
class cmExecuteProcessCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmExecuteProcessCommand;
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
  virtual const char* GetName() const
    {return "execute_process";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Execute one or more child processes.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  execute_process(COMMAND <cmd1> [args1...]]\n"
      "                  [COMMAND <cmd2> [args2...] [...]]\n"
      "                  [WORKING_DIRECTORY <directory>]\n"
      "                  [TIMEOUT <seconds>]\n"
      "                  [RESULT_VARIABLE <variable>]\n"
      "                  [OUTPUT_VARIABLE <variable>]\n"
      "                  [ERROR_VARIABLE <variable>]\n"
      "                  [INPUT_FILE <file>]\n"
      "                  [OUTPUT_FILE <file>]\n"
      "                  [ERROR_FILE <file>]\n"
      "                  [OUTPUT_QUIET]\n"
      "                  [ERROR_QUIET]\n"
      "                  [OUTPUT_STRIP_TRAILING_WHITESPACE]\n"
      "                  [ERROR_STRIP_TRAILING_WHITESPACE])\n"
      "Runs the given sequence of one or more commands with the standard "
      "output of each process piped to the standard input of the next.  "
      "A single standard error pipe is used for all processes.  "
      "If WORKING_DIRECTORY is given the named directory will be set as "
      "the current working directory of the child processes.  "
      "If TIMEOUT is given the child processes will be terminated if they "
      "do not finish in the specified number of seconds "
      "(fractions are allowed).  "
      "If RESULT_VARIABLE is given the variable will be set to contain "
      "the result of running the processes.  This will be an integer return "
      "code from the last child or a string describing an error condition.  "
      "If OUTPUT_VARIABLE or ERROR_VARIABLE are given the variable named "
      "will be set with the contents of the standard output and standard "
      "error pipes respectively.  If the same variable is named for both "
      "pipes their output will be merged in the order produced.  "
      "If INPUT_FILE, OUTPUT_FILE, or ERROR_FILE is given the file named "
      "will be attached to the standard input of the first process, "
      "standard output of the last process, or standard error of all "
      "processes respectively.  "
      "If OUTPUT_QUIET or ERROR_QUIET is given then the standard output "
      "or standard error results will be quietly ignored.  "
      "If more than one OUTPUT_* or ERROR_* option is given for the same "
      "pipe the precedence is not specified.  "
      "If no OUTPUT_* or ERROR_* options are given the output will be shared "
      "with the corresponding pipes of the CMake process itself.\n"
      "The execute_process command is a newer more powerful version of "
      "exec_program, but the old command has been kept for compatibility."
      ;
    }

  cmTypeMacro(cmExecuteProcessCommand, cmCommand);
};

#endif
