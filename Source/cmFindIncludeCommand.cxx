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
#include "cmFindIncludeCommand.h"
#include "cmCacheManager.h"

// cmFindIncludeCommand
bool cmFindIncludeCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = cmCacheManager::GetInstance()->GetCacheValue(args[0].c_str());
  if(cacheValue )
    {
    if(strcmp(cacheValue, "NOTFOUND") != 0)
      {
      m_Makefile->AddDefinition(args[0].c_str(), cacheValue);
      }
    return true;
    }
  std::vector<std::string> path;
  // add any user specified paths
  for (unsigned int j = 2; j < args.size(); j++)
    {
    // expand variables
    std::string exp = args[j];
    m_Makefile->ExpandVariablesInString(exp);
    path.push_back(exp);
    }

  // add the standard path
  cmSystemTools::GetPath(path);
  unsigned int k;
  for(k=0; k < path.size(); k++)
    {
    std::string tryPath = path[k];
    tryPath += "/";
    tryPath += args[1];
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      m_Makefile->AddDefinition(args[0].c_str(), path[k].c_str());
      // Save the value in the cache
      cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                                   path[k].c_str(),
                                                   cmCacheManager::PATH);
      return true;
      }
    }
  cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                               "NOTFOUND",
                                               cmCacheManager::PATH);
  std::string message = "Include not found: ";
  message += args[1];
  message += "\n";
  message += "looked in ";
  for(k=0; k < path.size(); k++)
    {
    message += path[k];
    message += "\n";
    }
  this->SetError(message.c_str());
  return false;
}

