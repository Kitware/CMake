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
#include "cmOptionCommand.h"

// cmOptionCommand
bool cmOptionCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = cmCacheManager::GetInstance()->GetCacheValue(args[0].c_str());
  if(!cacheValue)
    {
    cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),false);
    m_Makefile->AddDefinition(args[0].c_str(), "0");
    }
  else
    {
    m_Makefile->AddDefinition("WRAP_TCL", cacheValue);
    }

  return true;
}
