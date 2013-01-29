/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetIncludeDirectoriesCommand.h"

#include "cmMakefileIncludeDirectoriesEntry.h"

//----------------------------------------------------------------------------
bool cmTargetIncludeDirectoriesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->HandleArguments(args, "INCLUDE_DIRECTORIES", PROCESS_BEFORE);
}

//----------------------------------------------------------------------------
void cmTargetIncludeDirectoriesCommand
::HandleImportedTarget(const std::string &tgt)
{
  cmOStringStream e;
  e << "Cannot specify include directories for imported target \""
    << tgt << "\".";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTargetIncludeDirectoriesCommand
::HandleMissingTarget(const std::string &name)
{
  cmOStringStream e;
  e << "Cannot specify include directories for target \"" << name << "\" "
       "which is not built by this project.";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTargetIncludeDirectoriesCommand
::HandleDirectContent(cmTarget *tgt, const std::string &content,
                      bool prepend)
{
  cmListFileBacktrace lfbt;
  this->Makefile->GetBacktrace(lfbt);
  cmMakefileIncludeDirectoriesEntry entry(content, lfbt);
  tgt->InsertInclude(entry, prepend);
}
