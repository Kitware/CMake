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
#include "cmRegularExpression.h"

const char* cmCacheManagerTypes[] = 
{ "BOOL",
  "PATH",
  "FILEPATH",
  "STRING",
  "INTERNAL",
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
  // input line is:         key:type=value
  cmRegularExpression reg("(.*):(.*)=(.*)");
  while(fin)
    {
    // Format is key:type=value
    CacheEntry e;
    fin.getline(buffer, bsize);
    // skip blank lines and comment lines
    if(buffer[0] == '#' || buffer[0] == 0)
      {
      continue;
      }
    if(reg.find(buffer))
      {
      e.m_Type = cmCacheManager::StringToType(reg.match(2).c_str());
      e.m_Value = reg.match(3);
      m_Cache[reg.match(1)] = e;
      }
    else
      {
      cmSystemTools::Error("Parse error in cache file ", cacheFile.c_str());
      }
    }
  return true;
}

bool cmCacheManager::SaveCache(cmMakefile* mf)
{
  std::string cacheFile = mf->GetHomeOutputDirectory();
  cacheFile += "/CMakeCache.txt";
  std::string tempFile = cacheFile;
  tempFile += ".tmp";
  std::ofstream fout(tempFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Unable to open cache file for save. ", 
                         cacheFile.c_str());
    return false;
    }
  fout << "# This is the CMakeCache file.\n"
       << "# You can edit this file to change values found and used by cmake.\n"
       << "# If you do not want to change any of the values, simply exit the editor.\n"
       << "# If you do want to change a value, simply edit, save, and exit the editor.\n"
       << "# The syntax for the file is as follows:\n"
       << "# KEY:TYPE=VALUE\n"
       << "# KEY is the name of a varible in the cache.\n"
       << "# TYPE is a hint to GUI's for the type of VALUE, DO NOT EDIT TYPE!.\n"
       << "# VALUE is the current value for the KEY.\n\n";

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
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempFile.c_str(),
                                     cacheFile.c_str());
  cmSystemTools::RemoveFile(tempFile.c_str());
  return true;
}

void cmCacheManager::RemoveCacheEntry(const char* key)
{
  m_Cache.erase(key);
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


bool cmCacheManager::IsOn(const char* key)  
{ 
  if(!m_Cache.count(key))
    {
    return false;
    }
  std::string &v = m_Cache[key].m_Value;
  return (v == "ON" || v == "on" || v == "1" || v == "true" || v == "yev"
          || v == "TRUE" || v == "True" || v == "y" || v == "Y");
}



void cmCacheManager::PrintCache(std::ostream& out)
{
  out << "=================================================" << std::endl;
  out << "CMakeCache Contents:" << std::endl;
  for(std::map<std::string, CacheEntry>::iterator i = m_Cache.begin();
      i != m_Cache.end(); ++i)
    {
    out << (*i).first.c_str() << " = " << (*i).second.m_Value.c_str() << std::endl;
    }
  out << "\n\n";
  out << "To change values in the CMakeCache, \nedit CMakeCache.txt in your output directory.\n";
  out << "=================================================" << std::endl;
}


void cmCacheManager::AddCacheEntry(const char* key, bool v)
{
  if(v)
    {
    this->AddCacheEntry(key, "ON", cmCacheManager::BOOL);
    }
  else
    {
    this->AddCacheEntry(key, "OFF", cmCacheManager::BOOL);
    }
}

