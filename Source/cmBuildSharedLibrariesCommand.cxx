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
#include "cmBuildSharedLibrariesCommand.h"

// cmBuildSharedLibrariesCommand
bool cmBuildSharedLibrariesCommand::Invoke(std::vector<std::string>& args)
{
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = cmCacheManager::GetInstance()->GetCacheValue("BUILD_SHARED_LIBS");
  if(!cacheValue)
    {
    cmCacheManager::GetInstance()->AddCacheEntry("BUILD_SHARED_LIBS","0",
                                                 cmCacheManager::BOOL);
    m_Makefile->AddDefinition("BUILD_SHARED_LIBS", "0");
    }
  else
    {
    m_Makefile->AddDefinition("BUILD_SHARED_LIBS", cacheValue);
    }
  return true;
}
