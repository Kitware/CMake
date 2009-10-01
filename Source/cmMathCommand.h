/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMathCommand_h
#define cmMathCommand_h

#include "cmCommand.h"

/** \class cmMathCommand
 * \brief Common string operations
 *
 */
class cmMathCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmMathCommand;
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
  virtual const char* GetName() { return "math";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Mathematical expressions.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  math(EXPR <output variable> <math expression>)\n"
      "EXPR evaluates mathematical expression and return result in the "
      "output variable. Example mathematical expression is "
      "'5 * ( 10 + 13 )'.  Supported operators are "
      "+ - * / % | & ^ ~ << >> * / %.  They have the same meaning "
      " as they do in c code.";
    }
  
  cmTypeMacro(cmMathCommand, cmCommand);
protected:
 
  bool HandleExprCommand(std::vector<std::string> const& args);
};


#endif

