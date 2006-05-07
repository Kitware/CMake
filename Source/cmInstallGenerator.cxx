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
void cmInstallGenerator::AddInstallRule(
  std::ostream& os,
                                        const char* dest,
                                        int type,
                                        const char* file,
                                        bool optional /* = false */,
                                        const char* properties /* = 0 */,
                                        const char* permissions /* = 0 */,
  std::vector<std::string> const& configurations /* = std::vector<std::string>() */,
                                        const char* component /* = 0 */,
  const char* rename /* = 0 */
  )
{
  // Use the FILE command to install the file.
  std::string stype;
  switch(type)
    {
    case cmTarget::INSTALL_DIRECTORY:stype = "DIRECTORY"; break;
    case cmTarget::INSTALL_PROGRAMS: stype = "PROGRAM"; break;
    case cmTarget::EXECUTABLE:       stype = "EXECUTABLE"; break;
    case cmTarget::STATIC_LIBRARY:   stype = "STATIC_LIBRARY"; break;
    case cmTarget::SHARED_LIBRARY:   stype = "SHARED_LIBRARY"; break;
    case cmTarget::MODULE_LIBRARY:   stype = "MODULE"; break;
    case cmTarget::INSTALL_FILES:
    default:                         stype = "FILE"; break;
    }
  os << "FILE(INSTALL DESTINATION \"" << dest << "\" TYPE " << stype.c_str();
  if(optional)
    {
    os << " OPTIONAL";
    }
  if(properties && *properties)
    {
    os << " PROPERTIES" << properties;
    }
  if(permissions && *permissions)
    {
    os << " PERMISSIONS" << permissions;
    }
  if(rename && *rename)
    {
    os << " RENAME \"" << rename << "\"";
    }
  if(!configurations.empty())
    {
    os << " CONFIGURATIONS";
    for(std::vector<std::string>::const_iterator c = configurations.begin();
        c != configurations.end(); ++c)
      {
      os << " \"" << *c << "\"";
      }
    }
  if(component && *component)
    {
    os << " COMPONENTS \"" << component << "\"";
    }
  os << " FILES \"" << file << "\")\n";
}
