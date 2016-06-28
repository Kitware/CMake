/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmOutputRequiredFilesCommand_h
#define cmOutputRequiredFilesCommand_h

#include "cmCommand.h"

class cmDependInformation;

class cmOutputRequiredFilesCommand : public cmCommand
{
public:
  cmTypeMacro(cmOutputRequiredFilesCommand, cmCommand);
  cmCommand* Clone() CM_OVERRIDE { return new cmOutputRequiredFilesCommand; }
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;
  std::string GetName() const CM_OVERRIDE { return "output_required_files"; }

  void ListDependencies(cmDependInformation const* info, FILE* fout,
                        std::set<cmDependInformation const*>* visited);

private:
  std::string File;
  std::string OutputFile;
};

#endif
