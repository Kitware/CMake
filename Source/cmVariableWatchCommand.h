/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmVariableWatchCommand_h
#define cmVariableWatchCommand_h

#include "cmCommand.h"

/** \class cmVariableWatchCommand
 * \brief Watch when the variable changes and invoke command
 *
 */
class cmVariableWatchCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmVariableWatchCommand;
    }

  //! Default constructor
  cmVariableWatchCommand();

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

  /** This command does not really have a final pass but it needs to
      stay alive since it owns variable watch callback information. */
  virtual bool HasFinalPass() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "variable_watch";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Watch the CMake variable for change.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  variable_watch(<variable name> [<command to execute>])\n"
      "If the specified variable changes, the message will be printed about "
      "the variable being changed. If the command is specified, the command "
      "will be executed. The command will receive the following arguments:"
      " COMMAND(<variable> <access> <value> <current list file> <stack>)";
    }

  cmTypeMacro(cmVariableWatchCommand, cmCommand);
};


#endif


