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
#include "cmFindPackageCommand.h"

//----------------------------------------------------------------------------
bool cmFindPackageCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  this->Name = args[0];
  this->UpperName = cmSystemTools::UpperCase(this->Name);
  
  // See if there is a Find<name>.cmake module.
  bool foundModule = false;
  if(!this->FindModule(foundModule))
    {
    return false;
    }
  if(foundModule)
    {
    return true;
    }
  // No find module.  Assume the project has a CMake config file.  Use
  // a <NAME>_DIR cache variable to locate it.
  this->Variable = this->UpperName;
  this->Variable += "_DIR";
  this->Config = this->Name;
  this->Config += "Config.cmake";
  const char* def = m_Makefile->GetDefinition(this->Variable.c_str());
  if(cmSystemTools::IsOff(def))
    {
    if(!this->FindConfig())
      {
      return false;
      }
    }
  
  // If the config file was found, load it.
  bool result = true;
  bool found = false;
  def = m_Makefile->GetDefinition(this->Variable.c_str());
  if(!cmSystemTools::IsOff(def))
    {
    std::string f = def;
    f += "/";
    f += this->Config;
    if(cmSystemTools::FileExists(f.c_str()))
      {
      if(this->ReadListFile(f.c_str()))
        {
        found = true;
        }
      else
        {
        result = false;
        }
      }
    else
      {
      cmOStringStream e;
      e << this->Variable << " is set to \"" << def << "\", which is "
        << "not a directory containing " << this->Config;
      cmSystemTools::Error(e.str().c_str());
      result = true;
      }
    }
  else
    {
    cmOStringStream e;
    e << this->Variable << " is not set.  It must be set to the directory "
      << "containing " << this->Config << " so in order to use "
      << this->Name << ".";
    cmSystemTools::Error(e.str().c_str());
    result = true;
    }
  
  std::string foundVar = this->UpperName;
  foundVar += "_FOUND";
  m_Makefile->AddDefinition(foundVar.c_str(), found? "1":"0");
  return result;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindModule(bool& found)
{
  // Search the CMAKE_MODULE_PATH for a Find<name>.cmake module.
  found = false;
  std::string module;
  std::vector<std::string> modulePath;
  const char* def = m_Makefile->GetDefinition("CMAKE_MODULE_PATH");
  if(def)
    {
    cmSystemTools::ExpandListArgument(def, modulePath);
    }
  
  // Also search in the standard modules location.
  def = m_Makefile->GetDefinition("CMAKE_ROOT");
  if(def)
    {
    std::string rootModules = def;
    rootModules += "/Modules";
    modulePath.push_back(rootModules);
    }
  
  // Look through the possible module directories.
  for(std::vector<std::string>::iterator i = modulePath.begin();
      i != modulePath.end(); ++i)
    {
    module = *i;
    cmSystemTools::ConvertToUnixSlashes(module);
    module += "/Find";
    module += this->Name;
    module += ".cmake";
    if(cmSystemTools::FileExists(module.c_str()))
      {
      found = true;
      return this->ReadListFile(module.c_str());
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindConfig()
{
  std::string help = "The directory containing ";
  help += this->Config;
  help += ".";
  
  // Construct the list of relative paths to each prefix to be
  // searched.
  std::string rel = "/lib/";
  rel += cmSystemTools::LowerCase(this->Name);
  this->Relatives.push_back(rel);
  rel = "/lib/";
  rel += this->Name;
  this->Relatives.push_back(rel);
  
  // It is likely that CMake will have recently built the project.
  for(int i=1; i <= 10; ++i)
    {
    cmOStringStream r;
    r << "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild"
      << i << "]";
    std::string entry = r.str();
    cmSystemTools::ExpandRegistryValues(entry);
    cmSystemTools::ConvertToUnixSlashes(entry);
    if(cmSystemTools::FileIsDirectory(entry.c_str()))
      {
      this->Builds.push_back(entry);
      }
    }
  
  // The project may be installed.  Use the system search path to
  // construct a list of possible install prefixes.
  std::vector<std::string> systemPath;
  cmSystemTools::GetPath(systemPath);
  for(std::vector<std::string>::iterator i = systemPath.begin();
      i != systemPath.end(); ++i)
    {
    *i += "/..";
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {      
      this->Prefixes.push_back(cmSystemTools::CollapseFullPath(i->c_str()));
      }
    }
#if !defined(WIN32) || defined(__CYGWIN__)
  this->Prefixes.push_back("/usr/local");
  this->Prefixes.push_back("/usr");
#endif
  
  // Look for the project's configuration file.
  std::string init = this->SearchForConfig();
  
  // Store the entry in the cache so it can be set by the user.
  m_Makefile->AddCacheDefinition(this->Variable.c_str(),
                                 init.c_str(),
                                 help.c_str(),
                                 cmCacheManager::PATH);
  return true;
}

//----------------------------------------------------------------------------
std::string cmFindPackageCommand::SearchForConfig() const
{
  // Search the build directories.
  for(std::vector<cmStdString>::const_iterator b = this->Builds.begin();
      b != this->Builds.end(); ++b)
    {
    std::string f = *b;
    f += "/";
    f += this->Config;
    if(cmSystemTools::FileExists(f.c_str()))
      {
      return *b;
      }
    }
  
  // Search paths relative to each installation prefix.
  for(std::vector<cmStdString>::const_iterator p = this->Prefixes.begin();
      p != this->Prefixes.end(); ++p)
    {
    std::string prefix = *p;
    for(std::vector<cmStdString>::const_iterator r = this->Relatives.begin();
        r != this->Relatives.end(); ++r)
      {
      std::string dir = prefix;
      dir += *r;
      std::string f = dir;
      f += "/";
      f += this->Config;
      if(cmSystemTools::FileExists(f.c_str()))
        {
        return dir;
        }
      }
    }
  
  return "NOTFOUND";
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::ReadListFile(const char* f)
{
  if(m_Makefile->ReadListFile(m_Makefile->GetCurrentListFile(), f))
    {
    return true;
    }
  std::string e = "Error reading CMake code from \"";
  e += f;
  e += "\".";
  this->SetError(e.c_str());
  return false;
}
