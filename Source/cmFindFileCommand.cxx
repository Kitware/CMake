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
#include "cmFindFileCommand.h"
#include "cmCacheManager.h"
#include <stdlib.h>
#include <stdio.h>
  

// cmFindFileCommand
bool cmFindFileCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string helpString = "Where can the ";
  helpString += argsIn[1] + " file be found";
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

  std::vector<std::string>::const_iterator i = args.begin();
  // Use the first argument as the name of something to be defined
  const char* define = (*i).c_str();
  i++; // move iterator to next arg
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = m_Makefile->GetDefinition(define);
  if(cacheValue && strcmp(cacheValue, "NOTFOUND"))
    {
    return true;
    }
  // if it is not in the cache, then search the system path
  std::vector<std::string> path;

  // add any user specified paths
  for (unsigned int j = 2; j < args.size(); j++)
    {
    // Glob the entry in case of wildcards.
    cmSystemTools::GlobDirs(args[j].c_str(), path);
    }

  // add the standard path
  cmSystemTools::GetPath(path);
  for(unsigned int k=0; k < path.size(); k++)
    {
    std::string tryPath = path[k];
    tryPath += "/";
    tryPath += *i;
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      // Save the value in the cache
      m_Makefile->AddCacheDefinition(define,
                                     tryPath.c_str(),
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

