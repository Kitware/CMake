/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmWhileCommand_h
#define cmWhileCommand_h

#include "cmCommand.h"

#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker(cmMakefile* mf);
  ~cmWhileFunctionBlocker() CM_OVERRIDE;
  bool IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile& mf,
                         cmExecutionStatus&) CM_OVERRIDE;
  bool ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf) CM_OVERRIDE;

  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;

private:
  cmMakefile* Makefile;
  int Depth;
};

/// \brief Starts a while loop
class cmWhileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmWhileCommand; }

  /**
   * This overrides the default InvokeInitialPass implementation.
   * It records the arguments before expansion.
   */
  bool InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                         cmExecutionStatus&) CM_OVERRIDE;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const&,
                   cmExecutionStatus&) CM_OVERRIDE
  {
    return false;
  }

  /**
   * This determines if the command is invoked when in script mode.
   */
  bool IsScriptable() const CM_OVERRIDE { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "while"; }

  cmTypeMacro(cmWhileCommand, cmCommand);
};

#endif
