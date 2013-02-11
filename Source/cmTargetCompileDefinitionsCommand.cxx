/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetCompileDefinitionsCommand.h"

#include "cmMakefileIncludeDirectoriesEntry.h"

bool cmTargetCompileDefinitionsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->HandleArguments(args, "COMPILE_DEFINITIONS");
}

void cmTargetCompileDefinitionsCommand
::HandleImportedTarget(const std::string &tgt)
{
  cmOStringStream e;
  e << "Cannot specify compile definitions for imported target \""
    << tgt << "\".";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

void cmTargetCompileDefinitionsCommand
::HandleMissingTarget(const std::string &name)
{
  cmOStringStream e;
  e << "Cannot specify compile definitions for target \"" << name << "\" "
       "which is not built by this project.";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

bool cmTargetCompileDefinitionsCommand
::HandleNonTargetArg(std::string &content,
                   const std::string &sep,
                   const std::string &entry,
                   const std::string &)
{
  content += sep + entry;
  return true;
}

void cmTargetCompileDefinitionsCommand
::HandleDirectContent(cmTarget *tgt, const std::string &content,
                                   bool)
{
  tgt->AppendProperty("COMPILE_DEFINITIONS", content.c_str());
}
