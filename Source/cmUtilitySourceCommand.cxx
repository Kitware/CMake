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
#include "cmUtilitySourceCommand.h"

// cmUtilitySourceCommand
bool cmUtilitySourceCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  std::vector<std::string>::const_iterator arg = args.begin();
  
  // The first argument is the cache entry name.
  std::string cacheEntry = *arg++;
  const char* cacheValue =
    m_Makefile->GetDefinition(cacheEntry.c_str());
  // If it exists already, we are done.
  // unless this is Major
  if(cacheValue && 
     (m_Makefile->GetCacheMajorVersion() != 0 
      && m_Makefile->GetCacheMinorVersion() != 0 ))
    {
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
  
  // The source exists.
  std::string cmakeCFGout = m_Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  std::string utilityDirectory = m_Makefile->GetCurrentOutputDirectory();
  std::string exePath;
  if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    exePath = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    }
  if(exePath.size())
    {
    utilityDirectory = exePath;
    }
  else
    {
    utilityDirectory += "/"+relativeSource;
    }
  
  // Construct the cache entry for the executable's location.
  std::string utilityExecutable =
    utilityDirectory+"/"+cmakeCFGout+"/"
    +utilityName+cmSystemTools::GetExecutableExtension();
  
  // Enter the value into the cache.
  m_Makefile->AddCacheDefinition(cacheEntry.c_str(),
                                 utilityExecutable.c_str(),
                                 "Path to an internal program.",
                                 cmCacheManager::FILEPATH);
  // add a value into the cache that maps from the
  // full path to the name of the project
  cmSystemTools::ConvertToUnixSlashes(utilityExecutable);
  m_Makefile->AddCacheDefinition(utilityExecutable.c_str(),
                                 utilityName.c_str(),
                                 "Executable to project name.",
                                 cmCacheManager::INTERNAL);
  
  return true;
}

