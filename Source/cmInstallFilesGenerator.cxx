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
  Files(files), Destination(dest), Programs(programs),
  FilePermissions(file_permissions), Configurations(configurations),
  Component(component), Rename(rename), Optional(optional)
{
}

//----------------------------------------------------------------------------
cmInstallFilesGenerator
::~cmInstallFilesGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallFilesGenerator::GenerateScript(std::ostream& os)
{
  // Write code to install the files.
  for(std::vector<std::string>::const_iterator fi = this->Files.begin();
      fi != this->Files.end(); ++fi)
    {
    const char* no_properties = 0;
    const char* no_dir_permissions = 0;
    this->AddInstallRule(os, this->Destination.c_str(),
                         (this->Programs
                          ? cmTarget::INSTALL_PROGRAMS
                          : cmTarget::INSTALL_FILES), fi->c_str(),
                         this->Optional, no_properties,
                         this->FilePermissions.c_str(), no_dir_permissions,
                         this->Configurations,
                         this->Component.c_str(),
                         this->Rename.c_str());
    }
}
