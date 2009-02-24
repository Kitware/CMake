/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmInstallDirectoryGenerator.h"

#include "cmTarget.h"

//----------------------------------------------------------------------------
cmInstallDirectoryGenerator
::cmInstallDirectoryGenerator(std::vector<std::string> const& dirs,
                              const char* dest,
                              const char* file_permissions,
                              const char* dir_permissions,
                              std::vector<std::string> const& configurations,
                              const char* component,
                              const char* literal_args,
                              bool optional):
  cmInstallGenerator(dest, configurations, component), Directories(dirs),
  FilePermissions(file_permissions), DirPermissions(dir_permissions),
  LiteralArguments(literal_args), Optional(optional)
{
}

//----------------------------------------------------------------------------
cmInstallDirectoryGenerator
::~cmInstallDirectoryGenerator()
{
}

//----------------------------------------------------------------------------
void
cmInstallDirectoryGenerator::GenerateScriptActions(std::ostream& os,
                                                   Indent const& indent)
{
  // Write code to install the directories.
  const char* no_properties = 0;
  const char* no_rename = 0;
  this->AddInstallRule(os, cmTarget::INSTALL_DIRECTORY,
                       this->Directories,
                       this->Optional, no_properties,
                       this->FilePermissions.c_str(),
                       this->DirPermissions.c_str(),
                       no_rename, this->LiteralArguments.c_str(),
                       indent);
}
