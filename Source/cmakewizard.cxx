/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmakewizard.h"
#include "cmake.h"
#include "cmCacheManager.h"

cmakewizard::cmakewizard()
{
  this->ShowAdvanced = false;
}


void cmakewizard::AskUser(const char* key,
  cmCacheManager::CacheIterator& iter)
{
  printf("Variable Name: %s\n", key);
  const char* helpstring = iter.GetProperty("HELPSTRING");
  printf("Description: %s\n", (helpstring?helpstring:"(none)"));
  printf("Current Value: %s\n", iter.GetValue());
  printf("New Value (Enter to keep current value): ");
  char buffer[4096];
  if(!fgets(buffer, static_cast<int>(sizeof(buffer) - 1), stdin))
    {
    buffer[0] = 0;
    }

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
  if(!fgets(buffer, static_cast<int>(sizeof(buffer) - 1), stdin))
    {
    buffer[0] = 0;
    }
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



int cmakewizard::RunWizard(std::vector<std::string> const& args)
{
  this->ShowAdvanced = this->AskAdvanced();
  cmSystemTools::DisableRunCommandOutput();
  cmake make;
  make.SetArgs(args);
  make.SetCMakeCommand(args[0].c_str());
  make.LoadCache();
  make.SetCacheArgs(args);
  std::map<cmStdString, cmStdString> askedCache;
  bool asked = false;
  // continue asking questions until no new questions are asked
  do
    {
    asked = false;
    // run cmake
    this->ShowMessage(
      "Please wait while cmake processes CMakeLists.txt files....\n");

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
          if(this->ShowAdvanced || !i.GetPropertyAsBool("ADVANCED"))
            {
            this->AskUser(key.c_str(), i);
            asked = true;
            }
          }
        }
      else
        {
        if(this->ShowAdvanced || !i.GetPropertyAsBool("ADVANCED"))
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
  if(make.Generate() == 0)
    {
    this->ShowMessage("CMake complete, run make to build project.\n");
    return 0;
    }
  return 1;
}
