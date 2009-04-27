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
#include "cmInstallFilesGenerator.h"

#include "cmTarget.h"

//----------------------------------------------------------------------------
cmInstallFilesGenerator
::cmInstallFilesGenerator(std::vector<std::string> const& files,
                          const char* dest, bool programs,
                          const char* file_permissions,
                          std::vector<std::string> const& configurations,
                          const char* component,
                          const char* rename,
                          bool optional):
  cmInstallGenerator(dest, configurations, component),
  Files(files), Programs(programs),
  FilePermissions(file_permissions),
  Rename(rename), Optional(optional)
{
}

//----------------------------------------------------------------------------
cmInstallFilesGenerator
::~cmInstallFilesGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallFilesGenerator::GenerateScriptActions(std::ostream& os,
                                                    Indent const& indent)
{
  // Write code to install the files.
  const char* no_dir_permissions = 0;
  this->AddInstallRule(os,
                       (this->Programs
                        ? cmTarget::INSTALL_PROGRAMS
                        : cmTarget::INSTALL_FILES),
                       this->Files,
                       this->Optional,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       this->Rename.c_str(), 0, indent);
}
