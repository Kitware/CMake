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
#include "cmListFileCache.h"
#include "cmSystemTools.h"


cmListFileCache* cmListFileCache::Instance = 0;


cmListFileCache* cmListFileCache::GetInstance()
{
  if(!cmListFileCache::Instance)
    {
    cmListFileCache::Instance = new cmListFileCache;
    }
  return cmListFileCache::Instance;
}


void cmListFileCache::ClearCache()
{
  delete cmListFileCache::Instance;
  cmListFileCache::Instance = 0;
}



cmListFile* cmListFileCache::GetFileCache(const char* path,
                                          bool requireProjectCommand)
{
  ListFileMap::iterator sl = m_ListFileCache.find(path);
  if (sl == m_ListFileCache.end())
    {
    // if not already in the map, then parse and store the 
    // file
    if(!this->CacheFile(path, requireProjectCommand))
      {
      return 0;
      }
    sl = m_ListFileCache.find(path);
    if (sl == m_ListFileCache.end())
      {
      cmSystemTools::Error("Fatal error, in cmListFileCache CacheFile failed",
                           path);
      return 0;
      }
    }
  cmListFile& ret = sl->second;
  if(cmSystemTools::ModifiedTime(path) > ret.m_ModifiedTime )
    {
    if(!this->CacheFile(path, requireProjectCommand))
      {
      return 0;
      }
    else
      {
      sl = m_ListFileCache.find(path);
      return &sl->second;
      }
    } 
  return &ret;
}


bool cmListFileCache::CacheFile(const char* path, bool requireProjectCommand)
{
  if(!cmSystemTools::FileExists(path))
    {
    return false;
    }
    
  std::ifstream fin(path);
  if(!fin)
    {
    cmSystemTools::Error("cmListFileCache: error can not open file ", path);
    return false;
    }
  cmListFile inFile;
  inFile.m_ModifiedTime = cmSystemTools::ModifiedTime(path);
  bool parseError;
  while ( fin )
    {
    cmListFileFunction inFunction;
    if(cmSystemTools::ParseFunction(fin, 
                                    inFunction.m_Name,
                                    inFunction.m_Arguments,
                                    path, parseError))
      {
      inFile.m_Functions.push_back(inFunction);
      }
    if (parseError)
      {
      inFile.m_ModifiedTime = 0;
      }
    }
  if(requireProjectCommand)
    {
    bool hasProject = false;
    // search for a project command
    for(std::vector<cmListFileFunction>::iterator i 
          = inFile.m_Functions.begin();
        i != inFile.m_Functions.end(); ++i)
      {
      if(i->m_Name == "PROJECT")
        {
        hasProject = true;
        break;
        }
      }
    // if no project command is found, add one
    if(!hasProject)
      {
      cmListFileFunction project;
      project.m_Name = "PROJECT";
      project.m_Arguments.push_back("Project");
      inFile.m_Functions.push_back(project);
      }
    }
  m_ListFileCache[path] = inFile;
  return true;
}

void cmListFileCache::FlushCache(const char* path)
{
  ListFileMap::iterator it = m_ListFileCache.find(path);
  if ( it != m_ListFileCache.end() )
    {
    m_ListFileCache.erase(it);
    return;
    }
}
