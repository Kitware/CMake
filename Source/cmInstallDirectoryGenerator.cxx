/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallDirectoryGenerator.h"

#include "cmGeneratorExpression.h"
#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmInstallDirectoryGenerator
::cmInstallDirectoryGenerator(cmMakefile* mf,
                              std::vector<std::string> const& dirs,
                              const char* dest,
                              const char* file_permissions,
                              const char* dir_permissions,
                              std::vector<std::string> const& configurations,
                              const char* component,
                              MessageLevel message,
                              const char* literal_args,
                              bool optional):
  cmInstallGenerator(dest, configurations, component, message),
  Makefile(mf),
  Directories(dirs),
  FilePermissions(file_permissions), DirPermissions(dir_permissions),
  LiteralArguments(literal_args), Optional(optional)
{
  // We need per-config actions if any files have generator expressions.
  for(std::vector<std::string>::const_iterator i = dirs.begin();
      !this->ActionsPerConfig && i != dirs.end(); ++i)
    {
    if(cmGeneratorExpression::Find(*i) != std::string::npos)
      {
      this->ActionsPerConfig = true;
      }
    }
}

//----------------------------------------------------------------------------
cmInstallDirectoryGenerator
::~cmInstallDirectoryGenerator()
{
}

//----------------------------------------------------------------------------
void
cmInstallDirectoryGenerator::AddDirectoriesInstallRule(std::ostream& os,
                                                       Indent const& indent,
                                                       std::vector<std::string> const& directories)
{
  // Write code to install the directories.
  const char* no_rename = 0;
  this->AddInstallRule(os, cmInstallType_DIRECTORY,
                       directories,
                       this->Optional,
                       this->FilePermissions.c_str(),
                       this->DirPermissions.c_str(),
                       no_rename, this->LiteralArguments.c_str(),
                       indent);
}

//----------------------------------------------------------------------------
void cmInstallDirectoryGenerator::GenerateScriptActions(std::ostream& os,
                                                    Indent const& indent)
{
  if(this->ActionsPerConfig)
    {
    this->cmInstallGenerator::GenerateScriptActions(os, indent);
    }
  else
    {
    this->AddDirectoriesInstallRule(os, indent, this->Directories);
    }
}

//----------------------------------------------------------------------------
void cmInstallDirectoryGenerator::GenerateScriptForConfig(std::ostream& os,
                                                    const std::string& config,
                                                    Indent const& indent)
{
  std::vector<std::string> directories;
  cmGeneratorExpression ge;
  for(std::vector<std::string>::const_iterator i = this->Directories.begin();
      i != this->Directories.end(); ++i)
    {
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*i);
    cmSystemTools::ExpandListArgument(cge->Evaluate(this->Makefile, config),
                                      directories);
    }
  this->AddDirectoriesInstallRule(os, indent, directories);
}
