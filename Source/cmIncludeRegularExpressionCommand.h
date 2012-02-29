/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmIncludeRegularExpressionCommand_h
#define cmIncludeRegularExpressionCommand_h

#include "cmCommand.h"

/** \class cmIncludeRegularExpressionCommand
 * \brief Set the regular expression for following #includes.
 *
 * cmIncludeRegularExpressionCommand is used to specify the regular expression
 * that determines whether to follow a #include file in dependency checking.
 */
class cmIncludeRegularExpressionCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIncludeRegularExpressionCommand;
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
  virtual const char* GetName() const {return "include_regular_expression";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Set the regular expression used for dependency checking.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  include_regular_expression(regex_match [regex_complain])\n"
      "Set the regular expressions used in dependency checking.  Only files "
      "matching regex_match will be traced as dependencies.  Only files "
      "matching regex_complain will generate warnings if they cannot be "
      "found "
      "(standard header paths are not searched).  The defaults are:\n"
      "  regex_match    = \"^.*$\" (match everything)\n"
      "  regex_complain = \"^$\" (match empty string only)";
    }
  
  cmTypeMacro(cmIncludeRegularExpressionCommand, cmCommand);
};



#endif
