/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportLibraryDependenciesCommand_h
#define cmExportLibraryDependenciesCommand_h

#include "cmCommand.h"

class cmExportLibraryDependenciesCommand : public cmCommand
{
public:
  cmTypeMacro(cmExportLibraryDependenciesCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE
  {
    return new cmExportLibraryDependenciesCommand;
  }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE
  {
    return "export_library_dependencies";
  }

  void FinalPass() CM_OVERRIDE;
  bool HasFinalPass() const CM_OVERRIDE { return true; }

private:
  std::string Filename;
  bool Append;
  void ConstFinalPass() const;
};

#endif
