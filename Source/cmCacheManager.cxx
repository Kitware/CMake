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

#include "cmCacheManager.h"
#include "cmSystemTools.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"

const char* cmCacheManagerTypes[] = 
{ "BOOL",
  "PATH",
  "FILEPATH",
  "STRING",
  0
};

cmCacheManager::CacheEntryType cmCacheManager::StringToType(const char* s)
{
  int i = 0;
  while(cmCacheManagerTypes[i])
    {
    if(strcmp(s, cmCacheManagerTypes[i]) == 0)
      {
      return static_cast<CacheEntryType>(i);
      }
    ++i;
    }
  return STRING;
}

    
cmCacheManager* cmCacheManager::s_Instance = 0;

cmCacheManager* cmCacheManager::GetInstance()
{
  if(!cmCacheManager::s_Instance)
    {
    cmCacheManager::s_Instance = new cmCacheManager;
    }
  return cmCacheManager::s_Instance;
}



bool cmCacheManager::LoadCache(cmMakefile* mf)
{
  std::string cacheFile = mf->GetHomeOutputDirectory();
  cacheFile += "/CMakeCache.txt";
  // clear the old cache
  m_Cache.clear();
  std::ifstream fin(cacheFile.c_str());
  if(!fin)
    {
    return false;
    }
  const int bsize = 4096;
  char buffer[bsize];
  std::string inputLine;
  while(fin)
    {
    // Format is key:type=value
    CacheEntry e;
    std::string key;
    fin.getline(buffer, bsize, ':');
    key = buffer;
    fin.getline(buffer, bsize, '=');
    e.m_Type = cmCacheManager::StringToType(buffer);
    fin.getline(buffer, bsize); // last token is separated by a newline
    e.m_Value = buffer;
    if(fin)
      {
      m_Cache[key] = e;
      }
    }
  return true;
}

bool cmCacheManager::SaveCache(cmMakefile* mf)
{
  std::string cacheFile = mf->GetHomeOutputDirectory();
  cacheFile += "/CMakeCache.txt";
  std::ofstream fout(cacheFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Unable to open cache file for save. ", 
                         cacheFile.c_str());
    return false;
    }
  for( std::map<std::string, CacheEntry>::iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    CacheEntryType t = (*i).second.m_Type;
    // Format is key:type=value
    fout << (*i).first.c_str() << ":"
         << cmCacheManagerTypes[t] << "="
         << (*i).second.m_Value << "\n";
    }
  fout << "\n";
  return true;
}

void cmCacheManager::AddCacheEntry(const char* key, 
				   const char* value, 
				   CacheEntryType type)
{
  CacheEntry e;
  e.m_Value = value;
  e.m_Type = type;
  m_Cache[key] = e;
}

const char* cmCacheManager::GetCacheValue(const char* key)
{
  if(m_Cache.count(key))
    {
    return m_Cache[key].m_Value.c_str();
    }
  return 0;
}
