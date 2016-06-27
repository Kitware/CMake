/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmVariableRequiresCommand_h
#define cmVariableRequiresCommand_h

#include "cmCommand.h"

class cmVariableRequiresCommand : public cmCommand
{
public:
  cmTypeMacro(cmVariableRequiresCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmVariableRequiresCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "variable_requires"; }
};

#endif
