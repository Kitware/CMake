/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmFindProgramCommand.h"
#include "cmCacheManager.h"
#include <stdlib.h>
#include <stdio.h>
  

// cmFindProgramCommand
bool cmFindProgramCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>::iterator i = args.begin();
  // Use the first argument as the name of something to be defined
  const char* define = (*i).c_str();
  i++; // move iterator to next arg
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = cmCacheManager::GetInstance()->GetCacheValue(define);
  if(cacheValue)
    {
    m_Makefile->AddDefinition(define, cacheValue);
    return true;
    }
  // if it is not in the cache, then search the system path
  std::vector<std::string> path;
  cmSystemTools::GetPath(path);
  for(; i != args.end(); ++i)
    {
    for(int k=0; k < path.size(); k++)
      {
      std::string tryPath = path[k];
      tryPath += "/";
      tryPath += *i;
#ifdef _WIN32
      tryPath += ".exe";
#endif
      if(cmSystemTools::FileExists(tryPath.c_str()))
        {
        // Save the value in the cache
        cmCacheManager::GetInstance()->AddCacheEntry(define,
                                                     tryPath.c_str(),
                                                     cmCacheManager::FILEPATH);
        m_Makefile->AddDefinition(define, tryPath.c_str());
        return true;
        }
      }
    }
  return false;
}

