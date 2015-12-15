/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Matthias Maennich <matthias@maennich.net>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseArgumentsCommand_h
#define cmParseArgumentsCommand_h

#include "cmCommand.h"

/** \class cmParseArgumentsCommand
 *
 */
class cmParseArgumentsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmParseArgumentsCommand;
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
  virtual std::string GetName() const { return "cmake_parse_arguments";}

  cmTypeMacro(cmParseArgumentsCommand, cmCommand);

};


#endif
