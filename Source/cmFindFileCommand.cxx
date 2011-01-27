/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindFileCommand.h"
#include "cmSystemTools.h"

cmFindFileCommand::cmFindFileCommand()
{
  this->IncludeFileInPath = true;
}

void cmFindFileCommand::GenerateDocumentation()
{
  this->cmFindPathCommand::GenerateDocumentation();
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "find_path", "find_file");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "directory containing the named file",
                               "full path to named file");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "file in a directory", "full path to a file");
}
