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
#include "cmUtilitySourceCommand.h"

// cmUtilitySourceCommand
bool cmUtilitySourceCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::const_iterator arg = args.begin();
  
  // The first argument is the cache entry name.
  std::string cacheEntry = *arg++;
  const char* cacheValue =
    cmCacheManager::GetInstance()->GetCacheValue(cacheEntry.c_str());
  // If it exists already, we are done.
  if(cacheValue)
    {
    // Set the makefile's definition with the cache value.
    m_Makefile->AddDefinition(cacheEntry.c_str(), cacheValue);
    return true;
    }
  
  // The second argument is the utility's executable name, which will be
  // needed later.
  std::string utilityName = *arg++;
  
  // The third argument specifies the relative directory of the source
  // of the utility.
  std::string relativeSource = *arg++;
  std::string utilitySource = m_Makefile->GetCurrentDirectory();
  utilitySource = utilitySource+"/"+relativeSource;
  
  // If the directory doesn't exist, the source has not been included.
  if(!cmSystemTools::FileExists(utilitySource.c_str()))
    { return true; }
  
  // Make sure all the files exist in the source directory.
  while(arg != args.end())
    {
    std::string file = utilitySource+"/"+*arg++;
    if(!cmSystemTools::FileExists(file.c_str()))
      { return true; }
    }
  
  // The source exists.  Construct the cache entry for the executable's
  // location.
  std::string cmakeCFG = m_Makefile->GetDefinition("CMAKE_CFG");
  std::string utilityExecutable = m_Makefile->GetCurrentOutputDirectory();
  utilityExecutable =
    (utilityExecutable+"/"+relativeSource+"/"+cmakeCFG+"/"
     +utilityName+cmSystemTools::GetExecutableExtension());
  
  // Enter the value into the cache.
  cmCacheManager::GetInstance()->AddCacheEntry(cacheEntry.c_str(),
                                               utilityExecutable.c_str(),
                                               cmCacheManager::FILEPATH);
  
  // Set the definition in the makefile.
  m_Makefile->AddDefinition(cacheEntry.c_str(), utilityExecutable.c_str());
  
  return true;
}

