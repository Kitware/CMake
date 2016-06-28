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
  cmCommand* Clone() CM_OVERRIDE
  {
    return new cmIncludeRegularExpressionCommand;
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE
  {
    return "include_regular_expression";
  }

  cmTypeMacro(cmIncludeRegularExpressionCommand, cmCommand);
};

#endif
