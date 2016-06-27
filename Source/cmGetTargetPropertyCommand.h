/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetTargetPropertyCommand_h
#define cmGetTargetPropertyCommand_h

#include "cmCommand.h"

class cmGetTargetPropertyCommand : public cmCommand
{
public:
  cmCommand* Clone() CM_OVERRIDE { return new cmGetTargetPropertyCommand; }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "get_target_property"; }

  cmTypeMacro(cmGetTargetPropertyCommand, cmCommand);
};

#endif
