/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmBuildNameCommand_h
#define cmBuildNameCommand_h

#include "cmCommand.h"

class cmBuildNameCommand : public cmCommand
{
public:
  cmTypeMacro(cmBuildNameCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmBuildNameCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "build_name"; }
  bool IsScriptable() const CM_OVERRIDE { return true; }
};

#endif
