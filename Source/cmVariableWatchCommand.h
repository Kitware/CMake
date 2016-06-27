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
  cmCommand* Clone() CM_OVERRIDE { return new cmVariableWatchCommand; }

  //! Default constructor
  cmVariableWatchCommand();

  //! Destructor.
  ~cmVariableWatchCommand() CM_OVERRIDE;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * This determines if the command is invoked when in script mode.
   */
  bool IsScriptable() const CM_OVERRIDE { return true; }

  /** This command does not really have a final pass but it needs to
      stay alive since it owns variable watch callback information. */
  bool HasFinalPass() const CM_OVERRIDE { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "variable_watch"; }

  cmTypeMacro(cmVariableWatchCommand, cmCommand);

protected:
  std::set<std::string> WatchedVariables;
};

#endif
