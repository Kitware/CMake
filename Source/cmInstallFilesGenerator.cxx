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
                          const char* dest, bool programs):
  Files(files), Destination(dest), Programs(programs)
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
    this->AddInstallRule(os, this->Destination.c_str(),
                         (this->Programs
                          ? cmTarget::INSTALL_PROGRAMS
                          : cmTarget::INSTALL_FILES), fi->c_str());
    }
}
