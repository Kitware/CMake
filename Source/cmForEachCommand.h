/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmForEachCommand_h
#define cmForEachCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

class cmForEachFunctionBlocker : public cmFunctionBlocker
{
public:
  cmForEachFunctionBlocker() {this->Depth = 0;}
  virtual ~cmForEachFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf,
                                 cmExecutionStatus &);
  virtual bool ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf);
  
  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
private:
  int Depth;
};

/// Starts foreach() ... endforeach() block
class cmForEachCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmForEachCommand;
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
  virtual const char* GetName() const { return "foreach";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Evaluate a group of commands for each value in a list.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  foreach(loop_var arg1 arg2 ...)\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  endforeach(loop_var)\n"
      "All commands between foreach and the matching endforeach are recorded "
      "without being invoked.  Once the endforeach is evaluated, the "
      "recorded list of commands is invoked once for each argument listed "
      "in the original foreach command.  Before each iteration of the loop "
      "\"${loop_var}\" will be set as a variable with "
      "the current value in the list.\n"
      "  foreach(loop_var RANGE total)\n"
      "  foreach(loop_var RANGE start stop [step])\n"
      "Foreach can also iterate over a generated range of numbers. "
      "There are three types of this iteration:\n"
      "* When specifying single number, the range will have elements "
      "0 to \"total\".\n"
      "* When specifying two numbers, the range will have elements from "
      "the first number to the second number.\n"
      "* The third optional number is the increment used to iterate from "
      "the first number to the second number."
      "\n"
      "  foreach(loop_var IN [LISTS [list1 [...]]]\n"
      "                      [ITEMS [item1 [...]]])\n"
      "Iterates over a precise list of items.  "
      "The LISTS option names list-valued variables to be traversed, "
      "including empty elements (an empty string is a zero-length list).  "
      "The ITEMS option ends argument parsing and includes all arguments "
      "following it in the iteration."
      ;
    }
  
  cmTypeMacro(cmForEachCommand, cmCommand);
private:
  bool HandleInMode(std::vector<std::string> const& args);
};


#endif
