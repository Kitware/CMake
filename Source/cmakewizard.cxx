/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmakewizard.h"
#include "cmake.h"
#include "cmCacheManager.h"

cmakewizard::cmakewizard()
{
  m_ShowAdvanced = false;
}

  
void cmakewizard::AskUser(const char* key, cmCacheManager::CacheIterator& iter)
{
  std::cout << "Variable Name: " << key << "\n";
  const char* helpstring = iter.GetProperty("HELPSTRING");
  std::cout << "Description:   " << (helpstring?helpstring:"(none)") << "\n";
  std::cout << "Current Value: " << iter.GetValue() << "\n";
  std::cout << "New Value (Enter to keep current value): ";
  char buffer[4096];
  buffer[0] = 0;
  std::cin.getline(buffer, sizeof(buffer));
  if(buffer[0])
    {
    std::string value = buffer;
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
  std::cout << "\n";
}

bool cmakewizard::AskAdvanced()
{
  std::cout << "Would you like to see advanced options? [No]:";  
  char buffer[4096];
  buffer[0] = 0;
  std::cin.getline(buffer, sizeof(buffer));
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
  std::cout << m << "\n";
}



void cmakewizard::RunWizard(std::vector<std::string> const& args)
{
  m_ShowAdvanced = this->AskAdvanced();
  cmSystemTools::DisableRunCommandOutput();
  cmake make;
  make.SetArgs(args);
  std::map<std::string,std::string> askedCache;
  bool asked = false;
  // continue asking questions until no new questions are asked
  do
    {
    asked = false;
    // run cmake
    this->ShowMessage("Please wait while cmake processes CMakeLists.txt files....\n");

    make.Configure(args[0].c_str(),&args);
    this->ShowMessage("\n");
    // load the cache from disk
    cmCacheManager *cachem = make.GetCacheManager();
    cachem->
      LoadCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());
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
    cachem->SaveCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());
    }
  while(asked);
  make.Generate();
  this->ShowMessage("CMake complete, run make to build project.\n");
}
