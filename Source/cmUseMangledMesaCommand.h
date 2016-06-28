/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmUseMangledMesaCommand_h
#define cmUseMangledMesaCommand_h

#include "cmCommand.h"

class cmUseMangledMesaCommand : public cmCommand
{
public:
  cmTypeMacro(cmUseMangledMesaCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmUseMangledMesaCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "use_mangled_mesa"; }
  bool IsScriptable() const CM_OVERRIDE { return true; }
protected:
  void CopyAndFullPathMesaHeader(const char* source, const char* outdir);
};

#endif
