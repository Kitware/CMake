/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFunctionCommand_h
#define cmFunctionCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"

class cmFunctionFunctionBlocker : public cmFunctionBlocker
{
public:
  cmFunctionFunctionBlocker() {this->Depth=0;}
  virtual ~cmFunctionFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction&,
                                 cmMakefile &mf,
                                 cmExecutionStatus &);
  virtual bool ShouldRemove(const cmListFileFunction&, cmMakefile &mf);

  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
  int Depth;
};

/// Starts function() ... endfunction() block
class cmFunctionCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmFunctionCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "function";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Start recording a function for later invocation as a command.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  function(<name> [arg1 [arg2 [arg3 ...]]])\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  endfunction(<name>)\n"
      "Define a function named <name> that takes arguments named "
      "arg1 arg2 arg3 (...).  Commands listed after function, but before "
      "the matching endfunction, are not invoked until the function "
      "is invoked.  When it is invoked, the commands recorded in the "
      "function are first modified by replacing formal parameters (${arg1}) "
      "with the arguments passed, and then invoked as normal commands. In "
      "addition to referencing the formal parameters you can reference "
      "the variable ARGC which will be set to the number of arguments "
      "passed into the function as well as ARGV0 ARGV1 ARGV2 ... which "
      "will have the actual values of the arguments passed in. This "
      "facilitates creating functions with optional arguments. Additionally "
      "ARGV holds the list of all arguments given to the function and ARGN "
      "holds the list of arguments past the last expected argument."
      "\n"
      "A function opens a new scope: see set(var PARENT_SCOPE) for details."
      "\n"
      "See the cmake_policy() command documentation for the behavior of "
      "policies inside functions."
      ;
    }

  cmTypeMacro(cmFunctionCommand, cmCommand);
};


#endif
