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

const char* cmCacheManagerTypes[] = 
{ "BOOL",
  "PATH",
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



bool cmCacheManager::LoadCache(const char* path)
{
  std::ifstream fin(path);
  if(!fin)
    {
    cmSystemTools::Error("Unable to open cache file for load. ", path);
    return false;
    }
  const int bsize = 4096;
  char buffer[bsize];
  std::string inputLine;
  while(fin)
    {
    CacheEntry e;
    std::string key;
    fin.getline(buffer, bsize, '|');
    key = buffer;
    fin.getline(buffer, bsize, '|');
    e.m_Value = buffer;
    fin.getline(buffer, bsize); // last token is separated by a newline
    e.m_Type = cmCacheManager::StringToType(buffer);
    if(fin)
      {
      m_Cache[key] = e;
      }
    }
}

bool cmCacheManager::SaveCache(const char* path)
{
  std::ofstream fout(path);
  if(!fout)
    {
    cmSystemTools::Error("Unable to open cache file for save. ", path);
    return false;
    }
  for( std::map<std::string, CacheEntry>::iterator i = m_Cache.begin();
       i != m_Cache.end(); ++i)
    {
    fout << (*i).first.c_str() << " | " << (*i).second.m_Value << " | ";
    CacheEntryType t = (*i).second.m_Type;
    fout << cmCacheManagerTypes[t];
    }
  fout << "\n";
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
