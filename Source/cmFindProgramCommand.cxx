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
#include "cmFindProgramCommand.h"
#include "cmCacheManager.h"
#include <stdlib.h>
  

// cmFindProgramCommand
bool cmFindProgramCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string doc = "Path to a program.";
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
        doc = argsIn[j+1];
        }
      break;
      }
    }

  std::vector<std::string>::iterator i = args.begin();
  // Use the first argument as the name of something to be defined
  const char* define = (*i).c_str();
  i++; // move iterator to next arg
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = m_Makefile->GetDefinition(define);
  if(cacheValue && !cmSystemTools::IsNOTFOUND(cacheValue))
    {
    return true;
    }
  if(cacheValue)
    {
    cmCacheManager::CacheIterator it = 
      m_Makefile->GetCacheManager()->GetCacheIterator(args[0].c_str());
    if(!it.IsAtEnd())
      {
      const char* hs = it.GetProperty("HELPSTRING");
      doc = hs?hs:"(none)";
      }
    }
  std::vector<std::string> path;
  std::vector<std::string> names;
  bool foundName = false;
  bool foundPath = false;
  bool doingNames = true;
  bool no_system_path = false;
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
    else if (args[j] == "NO_SYSTEM_PATH")
      {
      no_system_path = true;
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
  // if it is not in the cache, then search the system path
  // add any user specified paths 
  if(!foundPath && !foundName)
    {
    path.clear();
    names.clear();
    names.push_back(args[1]);
    for (unsigned int j = 2; j < args.size(); j++)
      {
      // expand variables
      std::string exp = args[j];
      cmSystemTools::ExpandRegistryValues(exp);
      
      // Glob the entry in case of wildcards.
      cmSystemTools::GlobDirs(exp.c_str(), path);
      }
    }
  for(std::vector<std::string>::iterator it = names.begin();
      it != names.end() ; ++it)
    {
    // Try to find the program.
    std::string result = cmSystemTools::FindProgram(it->c_str(), 
                                                    path, 
                                                    no_system_path);
    if(result != "")
      {
      // Save the value in the cache
      m_Makefile->AddCacheDefinition(define,
                                     result.c_str(),
                                     doc.c_str(),
                                     cmCacheManager::FILEPATH);
      
      return true;
      }
    }
  m_Makefile->AddCacheDefinition(args[0].c_str(),
                                 (args[0] + "-NOTFOUND").c_str(),
                                 doc.c_str(),
                                 cmCacheManager::FILEPATH);
  return true;
}

