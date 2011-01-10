/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMacroCommand_h
#define cmMacroCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"

/** \class cmMacroFunctionBlocker
 * \brief subclass of function blocker
 *
 *
 */
class cmMacroFunctionBlocker : public cmFunctionBlocker
{
public:
  cmMacroFunctionBlocker() {this->Depth=0;}
  virtual ~cmMacroFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction&, 
                                 cmMakefile &mf,
                                 cmExecutionStatus &);
  virtual bool ShouldRemove(const cmListFileFunction&, cmMakefile &mf);
  
  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
  int Depth;
};

/** \class cmMacroCommand
 * \brief starts an if block
 *
 * cmMacroCommand starts an if block
 */
class cmMacroCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmMacroCommand;
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
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "macro";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Start recording a macro for later invocation as a command.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  macro(<name> [arg1 [arg2 [arg3 ...]]])\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  endmacro(<name>)\n"
      "Define a macro named <name> that takes arguments named "
      "arg1 arg2 arg3 (...).  Commands listed after macro, "
      "but before the matching endmacro, are not invoked until the macro "
      "is invoked.  When it is invoked, the commands recorded in the "
      "macro are first modified by replacing formal parameters (${arg1}) "
      "with the arguments passed, and then invoked as normal commands. In "
      "addition to referencing the formal parameters you can reference "
      "the values ${ARGC} which will be set to the number of arguments "
      "passed into the function as well as ${ARGV0} ${ARGV1} ${ARGV2} "
      "... which "
      "will have the actual values of the arguments passed in. This "
      "facilitates creating macros with optional arguments. Additionally "
      "${ARGV} holds the list of all arguments given to the macro and "
      "${ARGN} "
      "holds the list of argument past the last expected argument. "
      "Note that the parameters to a macro and values such as ARGN "
      "are not variables in the usual CMake sense. They are string "
      "replacements much like the c preprocessor would do with a "
      "macro. If you want true CMake variables you should look at "
      "the function command."
      "\n"
      "See the cmake_policy() command documentation for the behavior of "
      "policies inside macros."
      ;
    }
  cmTypeMacro(cmMacroCommand, cmCommand);
};


#endif
