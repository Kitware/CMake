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
#include "cmakewizard.h"
#include "cmake.h"
#include "cmCacheManager.h"


// On Mac OSX getline looks like it is broken, so we have to use
// fgets. This is why we are including stdio.h.
#include <stdio.h>

cmakewizard::cmakewizard()
{
  m_ShowAdvanced = false;
}

  
void cmakewizard::AskUser(const char* key, cmCacheManager::CacheIterator& iter)
{
  printf("Variable Name: %s\n", key);
  const char* helpstring = iter.GetProperty("HELPSTRING");
  printf("Description: %s\n", (helpstring?helpstring:"(none)"));
  printf("Current Value: %s\n", iter.GetValue());
  printf("New Value (Enter to keep current value): ");
  char buffer[4096];
  buffer[0] = 0;
  fgets(buffer, sizeof(buffer)-1, stdin);
          
  if(strlen(buffer) > 0)
    {
    std::string sbuffer = buffer;
    std::string::size_type pos = sbuffer.find_last_not_of(" \n\r\t");
    std::string value = "";
    if ( pos != std::string::npos )
      {
      value = sbuffer.substr(0, pos+1);
      }
    
    if ( value.size() > 0 )
      {
      if(iter.GetType() == cmCacheManager::PATH ||
         iter.GetType() == cmCacheManager::FILEPATH)
        {
        cmSystemTools::ConvertToUnixSlashes(value);
        }
      if(iter.GetType() == cmCacheManager::BOOL)
        {
        if(!cmSystemTools::IsOn(value.c_str()))
          {
          value = "OFF";
          }
        }
      iter.SetValue(value.c_str());
      }
    }
  printf("\n");
}

bool cmakewizard::AskAdvanced()
{
  printf("Would you like to see advanced options? [No]:");  
  char buffer[4096];
  buffer[0] = 0;
  fgets(buffer, sizeof(buffer)-1, stdin);
  if(buffer[0])
    {
    if(buffer[0] == 'y' || buffer[0] == 'Y')
      {
      return true;
      }
    }
  return false;
}


void cmakewizard::ShowMessage(const char* m)
{
  printf("%s\n", m);
}



void cmakewizard::RunWizard(std::vector<std::string> const& args)
{
  m_ShowAdvanced = this->AskAdvanced();
  cmSystemTools::DisableRunCommandOutput();
  cmake make;
  make.SetArgs(args);
  make.SetCMakeCommand(args[0].c_str());
  make.LoadCache();
  make.SetCacheArgs(args);
  std::map<std::string,std::string> askedCache;
  bool asked = false;
  // continue asking questions until no new questions are asked
  do
    {
    asked = false;
    // run cmake
    this->ShowMessage("Please wait while cmake processes CMakeLists.txt files....\n");

    make.Configure();
    this->ShowMessage("\n");
    // load the cache from disk
    cmCacheManager *cachem = make.GetCacheManager();
    cachem->LoadCache(make.GetHomeOutputDirectory());
    cmCacheManager::CacheIterator i = cachem->NewIterator();
    // iterate over all entries in the cache
    for(;!i.IsAtEnd(); i.Next())
      { 
      std::string key = i.GetName();
      if( i.GetType() == cmCacheManager::INTERNAL || 
          i.GetType() == cmCacheManager::STATIC ||
          i.GetType() == cmCacheManager::UNINITIALIZED )
        {
        continue;
        }
      if(askedCache.count(key))
        {
        std::string& e = askedCache.find(key)->second;
        if(e != i.GetValue())
          {
          if(m_ShowAdvanced || !i.GetPropertyAsBool("ADVANCED"))
            {
            this->AskUser(key.c_str(), i);
            asked = true;
            }
          }
        }
      else
        {    
        if(m_ShowAdvanced || !i.GetPropertyAsBool("ADVANCED"))
          {
          this->AskUser(key.c_str(), i);
          asked = true;
          }
        }
      askedCache[key] = i.GetValue();
      }
    cachem->SaveCache(make.GetHomeOutputDirectory());
    }
  while(asked);
  make.Generate();
  this->ShowMessage("CMake complete, run make to build project.\n");
}
