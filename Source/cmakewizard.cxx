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

  
void cmakewizard::AskUser(const char* key, cmCacheManager::CacheEntry & entry,
                          cmCacheManager *cacheManager)
{
  std::cout << "Variable Name: " << key << "\n";
  std::cout << "Description:   " << entry.m_HelpString << "\n";
  std::cout << "Current Value: " << entry.m_Value.c_str() << "\n";
  std::cout << "New Value (Enter to keep current value): ";
  char buffer[4096];
  buffer[0] = 0;
  std::cin.getline(buffer, sizeof(buffer));
  if(buffer[0])
    {
    cmCacheManager::CacheEntry *entry = cacheManager->GetCacheEntry(key);
    if(entry)
      {
      entry->m_Value = buffer;
      if(entry->m_Type == cmCacheManager::PATH ||
         entry->m_Type == cmCacheManager::FILEPATH)
        {
        cmSystemTools::ConvertToUnixSlashes(entry->m_Value);
        }
      if(entry->m_Type == cmCacheManager::BOOL)
        {
        if(!cmSystemTools::IsOn(buffer))
          {
          entry->m_Value = "OFF";
          }
        }
      }
    else
      {
      std::cerr << "strange error, should be in cache but is not... " << key << "\n";
      }
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
  cmCacheManager::CacheEntryMap askedCache;
  bool asked = false;
  // continue asking questions until no new questions are asked
  do
    {
    asked = false;
    // run cmake
    this->ShowMessage("Please wait while cmake processes CMakeLists.txt files....\n");
    make.Generate(args);
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
      cmCacheManager::CacheEntry ce = i.GetEntry();
      if(ce.m_Type == cmCacheManager::INTERNAL
         || ce.m_Type == cmCacheManager::STATIC)
        {
        continue;
        }
      if(askedCache.count(key))
        {
        cmCacheManager::CacheEntry& e = askedCache.find(key)->second;
        if(e.m_Value != ce.m_Value)
          {
          if(m_ShowAdvanced || !cachem->IsAdvanced(key.c_str()))
            {
            this->AskUser(key.c_str(), ce, cachem);
            asked = true;
            }
          }
        }
      else
        {    
        if(m_ShowAdvanced || !cachem->IsAdvanced(key.c_str()))
          {
          this->AskUser(key.c_str(), ce, cachem);
          asked = true;
          }
        }
      askedCache[key] = i.GetEntry();
      }
    cachem->SaveCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());
    }
  while(asked);
  this->ShowMessage("CMake complete, run make to build project.\n");
}
