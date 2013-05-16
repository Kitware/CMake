/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetCompileOptionsCommand.h"

bool cmTargetCompileOptionsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->HandleArguments(args, "COMPILE_OPTIONS", PROCESS_BEFORE);
}

void cmTargetCompileOptionsCommand
::HandleImportedTarget(const std::string &tgt)
{
  cmOStringStream e;
  e << "Cannot specify compile options for imported target \""
    << tgt << "\".";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

void cmTargetCompileOptionsCommand
::HandleMissingTarget(const std::string &name)
{
  cmOStringStream e;
  e << "Cannot specify compile options for target \"" << name << "\" "
       "which is not built by this project.";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
std::string cmTargetCompileOptionsCommand
::Join(const std::vector<std::string> &content)
{
  std::string defs;
  std::string sep;
  for(std::vector<std::string>::const_iterator it = content.begin();
    it != content.end(); ++it)
    {
    defs += sep + *it;
    sep = ";";
    }
  return defs;
}

//----------------------------------------------------------------------------
void cmTargetCompileOptionsCommand
::HandleDirectContent(cmTarget *tgt, const std::vector<std::string> &content,
                                   bool)
{
  cmListFileBacktrace lfbt;
  this->Makefile->GetBacktrace(lfbt);
  cmValueWithOrigin entry(this->Join(content), lfbt);
  tgt->InsertCompileOption(entry);
}
