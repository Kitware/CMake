/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallGenerator.h"

#include "cmSystemTools.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmInstallGenerator
::cmInstallGenerator(const char* destination,
                     std::vector<std::string> const& configurations,
                     const char* component):
  cmScriptGenerator("CMAKE_INSTALL_CONFIG_NAME", configurations),
  Destination(destination? destination:""),
  Component(component? component:"")
{
}

//----------------------------------------------------------------------------
cmInstallGenerator
::~cmInstallGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallGenerator
::AddInstallRule(
                 std::ostream& os,
                 int type,
                 std::vector<std::string> const& files,
                 bool optional /* = false */,
                 const char* permissions_file /* = 0 */,
                 const char* permissions_dir /* = 0 */,
                 const char* rename /* = 0 */,
                 const char* literal_args /* = 0 */,
                 Indent const& indent
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
  os << indent;
  std::string dest = this->GetInstallDestination();
  if (cmSystemTools::FileIsFullPath(dest.c_str()))
     {
     os << "list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES\n";
     os << indent << " \"";
     for(std::vector<std::string>::const_iterator fi = files.begin();
               fi != files.end(); ++fi)
             {
               if (fi!=files.begin())
                 {
                 os << ";";
                 }
               os << dest << "/";
               if (rename && *rename)
                 {
                 os << rename;
                 }
               else
                 {
                 os << cmSystemTools::GetFilenameName(*fi);
                 }
             }
     os << "\")\n";
     }
  os << "FILE(INSTALL DESTINATION \"" << dest << "\" TYPE " << stype.c_str();
  if(optional)
    {
    os << " OPTIONAL";
    }
  if(permissions_file && *permissions_file)
    {
    os << " PERMISSIONS" << permissions_file;
    }
  if(permissions_dir && *permissions_dir)
    {
    os << " DIR_PERMISSIONS" << permissions_dir;
    }
  if(rename && *rename)
    {
    os << " RENAME \"" << rename << "\"";
    }
  os << " FILES";
  if(files.size() == 1)
    {
    os << " \"" << files[0] << "\"";
    }
  else
    {
    for(std::vector<std::string>::const_iterator fi = files.begin();
        fi != files.end(); ++fi)
      {
      os << "\n" << indent << "  \"" << *fi << "\"";
      }
    os << "\n" << indent << " ";
    if(!(literal_args && *literal_args))
      {
      os << " ";
      }
    }
  if(literal_args && *literal_args)
    {
    os << literal_args;
    }
  os << ")\n";
}

//----------------------------------------------------------------------------
std::string
cmInstallGenerator::CreateComponentTest(const char* component)
{
  std::string result = "NOT CMAKE_INSTALL_COMPONENT OR "
    "\"${CMAKE_INSTALL_COMPONENT}\" STREQUAL \"";
  result += component;
  result += "\"";
  return result;
}

//----------------------------------------------------------------------------
void cmInstallGenerator::GenerateScript(std::ostream& os)
{
  // Track indentation.
  Indent indent;

  // Begin this block of installation.
  std::string component_test =
    this->CreateComponentTest(this->Component.c_str());
  os << indent << "IF(" << component_test << ")\n";

  // Generate the script possibly with per-configuration code.
  this->GenerateScriptConfigs(os, indent.Next());

  // End this block of installation.
  os << indent << "ENDIF(" << component_test << ")\n\n";
}

//----------------------------------------------------------------------------
bool cmInstallGenerator::InstallsForConfig(const char* config)
{
  return this->GeneratesForConfig(config);
}

//----------------------------------------------------------------------------
std::string cmInstallGenerator::GetInstallDestination() const
{
  std::string result;
  if(!this->Destination.empty() &&
     !cmSystemTools::FileIsFullPath(this->Destination.c_str()))
    {
    result = "${CMAKE_INSTALL_PREFIX}/";
    }
  result += this->Destination;
  return result;
}
