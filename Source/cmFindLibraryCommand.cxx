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
#include "cmFindLibraryCommand.h"
#include "cmCacheManager.h"

// cmFindLibraryCommand
bool cmFindLibraryCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    } 
  std::string helpString;
  size_t size = argsIn.size();
  std::vector<std::string> args;
  for(unsigned int j = 0; j < size; ++j)
    {
    if(argsIn[j] != "DOC")
      {
      args.push_back(argsIn[j]);
      }
    else
      {
      if(j+1 < size)
        {
        helpString = argsIn[j+1];
        }
      break;
      }
    }

  std::vector<std::string> path;
  std::vector<std::string> names;
  bool foundName = false;
  bool foundPath = false;
  bool doingNames = true;
  for (unsigned int j = 1; j < args.size(); ++j)
    {
    if(args[j] == "NAMES")
      {
      doingNames = true;
      foundName = true;
      }
    else if (args[j] == "PATHS")
      {
      doingNames = false;
      foundPath = true;
      }
    else
      { 
      if(doingNames)
        {
        names.push_back(args[j]);
        }
      else
        {
        cmSystemTools::ExpandRegistryValues(args[j]);
        // Glob the entry in case of wildcards.
        cmSystemTools::GlobDirs(args[j].c_str(), path);
        }
      }
    }
  // old style name path1 path2 path3
  if(!foundPath && !foundName)
    {
    names.clear();
    path.clear();
    names.push_back(args[1]);
    // add any user specified paths
    for (unsigned int j = 2; j < args.size(); j++)
      {
      // expand variables
      std::string exp = args[j];
      cmSystemTools::ExpandRegistryValues(exp);
      
      // Glob the entry in case of wildcards.
      cmSystemTools::GlobDirs(exp.c_str(), path);
      }
    }
  if(helpString.size() == 0)
    {
    helpString = "Where can ";
    if (names.size() == 0)
      {
      helpString += "the (unknown) library be found";
      }
    else if (names.size() == 1)
      {
      helpString += "the " + names[0] + " library be found";
      }
    else
      {
      helpString += "one of the " + names[0];
      for (unsigned int j = 1; j < names.size() - 1; ++j)
        {
        helpString += ", " + names[j];
        }
      helpString += " or " + names[names.size() - 1] + " libraries be found";
      }
    }
  
  const char* cacheValue
    = m_Makefile->GetDefinition(args[0].c_str());
  if(cacheValue && strcmp(cacheValue, "NOTFOUND"))
    { 
    return true;
    }

  std::string library;
  for(std::vector<std::string>::iterator i = names.begin();
      i != names.end() ; ++i)
    {
    library = m_Makefile->FindLibrary(i->c_str(), path);
    if(library != "")
      {  
      m_Makefile->AddCacheDefinition(args[0].c_str(),
                                     library.c_str(),
                                     helpString.c_str(),
                                     cmCacheManager::FILEPATH);
      return true;
      } 
    }
  m_Makefile->AddCacheDefinition(args[0].c_str(),
                                 "NOTFOUND",
                                 helpString.c_str(),
                                 cmCacheManager::FILEPATH);
  return true;
}

