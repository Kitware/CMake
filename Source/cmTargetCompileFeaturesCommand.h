/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTargetCompileFeaturesCommand_h
#define cmTargetCompileFeaturesCommand_h

#include "cmCommand.h"

class cmTargetCompileFeaturesCommand : public cmCommand
{
  virtual cmCommand* Clone()
    {
    return new cmTargetCompileFeaturesCommand;
    }

  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  virtual const char* GetName() const { return "target_compile_features";}

  cmTypeMacro(cmTargetCompileFeaturesCommand, cmCommand);
};

#endif
