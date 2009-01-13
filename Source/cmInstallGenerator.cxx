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
::cmInstallGenerator(const char* destination,
                     std::vector<std::string> const& configurations,
                     const char* component):
  Destination(destination? destination:""),
  Configurations(configurations),
  Component(component? component:""),
  ConfigurationName(0),
  ConfigurationTypes(0)
{
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
void cmInstallGenerator
::AddInstallRule(
                 std::ostream& os,
                 int type,
                 std::vector<std::string> const& files,
                 bool optional /* = false */,
                 const char* properties /* = 0 */,
                 const char* permissions_file /* = 0 */,
                 const char* permissions_dir /* = 0 */,
                 const char* rename /* = 0 */,
                 const char* literal_args /* = 0 */,
                 cmInstallGeneratorIndent const& indent
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
  os << "FILE(INSTALL DESTINATION \"" << dest << "\" TYPE " << stype.c_str();
  if(optional)
    {
    os << " OPTIONAL";
    }
  if(properties && *properties)
    {
    os << " PROPERTIES" << properties;
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
static void cmInstallGeneratorEncodeConfig(const char* config,
                                           std::string& result)
{
  for(const char* c = config; *c; ++c)
    {
    if(*c >= 'a' && *c <= 'z')
      {
      result += "[";
      result += *c + ('A' - 'a');
      result += *c;
      result += "]";
      }
    else if(*c >= 'A' && *c <= 'Z')
      {
      result += "[";
      result += *c;
      result += *c + ('a' - 'A');
      result += "]";
      }
    else
      {
      result += *c;
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmInstallGenerator::CreateConfigTest(const char* config)
{
  std::string result = "\"${CMAKE_INSTALL_CONFIG_NAME}\" MATCHES \"^(";
  if(config && *config)
    {
    cmInstallGeneratorEncodeConfig(config, result);
    }
  result += ")$\"";
  return result;
}

//----------------------------------------------------------------------------
std::string
cmInstallGenerator::CreateConfigTest(std::vector<std::string> const& configs)
{
  std::string result = "\"${CMAKE_INSTALL_CONFIG_NAME}\" MATCHES \"^(";
  const char* sep = "";
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    result += sep;
    sep = "|";
    cmInstallGeneratorEncodeConfig(ci->c_str(), result);
    }
  result += ")$\"";
  return result;
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
void
cmInstallGenerator::GenerateScriptConfigs(std::ostream& os,
                                          Indent const& indent)
{
  if(this->Configurations.empty())
    {
    // This rule is for all configurations.
    this->GenerateScriptActions(os, indent);
    }
  else
    {
    // Generate a per-configuration block.
    std::string config_test = this->CreateConfigTest(this->Configurations);
    os << indent << "IF(" << config_test << ")\n";
    this->GenerateScriptActions(os, indent.Next());
    os << indent << "ENDIF(" << config_test << ")\n";
    }
}

//----------------------------------------------------------------------------
void cmInstallGenerator::GenerateScriptActions(std::ostream&, Indent const&)
{
  // No actions for this generator.
}

//----------------------------------------------------------------------------
bool cmInstallGenerator::InstallsForConfig(const char* config)
{
  // If this is not a configuration-specific rule then we install.
  if(this->Configurations.empty())
    {
    return true;
    }

  // This is a configuration-specific rule.  Check if the config
  // matches this rule.
  std::string config_upper = cmSystemTools::UpperCase(config?config:"");
  for(std::vector<std::string>::const_iterator i =
        this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    if(cmSystemTools::UpperCase(*i) == config_upper)
      {
      return true;
      }
    }
  return false;
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
