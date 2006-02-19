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
#include "cmInstallGenerator.h"

#include "cmSystemTools.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmInstallGenerator
::cmInstallGenerator()
{
  this->ConfigurationName = 0;
  this->ConfigurationTypes = 0;
}

//----------------------------------------------------------------------------
cmInstallGenerator
::~cmInstallGenerator()
{
}

//----------------------------------------------------------------------------
void
cmInstallGenerator
::Generate(std::ostream& os, const char* config,
           std::vector<std::string> const& configurationTypes)
{
  this->ConfigurationName = config;
  this->ConfigurationTypes = &configurationTypes;
  this->GenerateScript(os);
  this->ConfigurationName = 0;
  this->ConfigurationTypes = 0;
}

//----------------------------------------------------------------------------
void cmInstallGenerator::AddInstallRule(std::ostream& os,
                                        const char* dest,
                                        int type,
                                        const char* file,
                                        bool optional /* = false */,
                                        const char* properties /* = 0 */)
{
  // If the file is optional test its existence before installing.
  const char* indent = "";
  if(optional)
    {
    os << "IF(EXISTS \"" << file << "\")\n";
    indent = "  ";
    }

  // Write a message indicating the file is being installed.
  std::string fname = cmSystemTools::GetFilenameName(file);
  os << indent << "MESSAGE(STATUS \"Installing " << dest
     << "/" << fname.c_str() << "\")\n";

  // Use the FILE command to install the file.
  std::string stype;
  switch(type)
    {
    case cmTarget::INSTALL_PROGRAMS: stype = "PROGRAM"; break;
    case cmTarget::EXECUTABLE:       stype = "EXECUTABLE"; break;
    case cmTarget::STATIC_LIBRARY:   stype = "STATIC_LIBRARY"; break;
    case cmTarget::SHARED_LIBRARY:   stype = "SHARED_LIBRARY"; break;
    case cmTarget::MODULE_LIBRARY:   stype = "MODULE"; break;
    case cmTarget::INSTALL_FILES:
    default:                         stype = "FILE"; break;
    }
  os << indent << "FILE(INSTALL DESTINATION \"" << dest
     << "\" TYPE " << stype.c_str() ;
  if(properties && *properties)
    {
    os << " PROPERTIES" << properties;
    }
  os << " FILES \"" << file << "\")\n";

  // If the file is optional close the IF block.
  if(optional)
    {
    os << "ENDIF(EXISTS \"" << file << "\")\n";
    }
}
