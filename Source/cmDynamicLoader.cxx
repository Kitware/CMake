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
#include "cmDynamicLoader.h"

class cmDynamicLoaderCache
{
public:
  ~cmDynamicLoaderCache();
  void CacheFile(const char* path,
    const cmsys::DynamicLoader::LibraryHandle&);
  bool GetCacheFile(const char* path, cmsys::DynamicLoader::LibraryHandle&);
  bool FlushCache(const char* path);
  void FlushCache();
  static cmDynamicLoaderCache* GetInstance();

private:
  std::map<cmStdString, cmsys::DynamicLoader::LibraryHandle> CacheMap;
  static cmDynamicLoaderCache* Instance;
};

cmDynamicLoaderCache* cmDynamicLoaderCache::Instance = 0;

cmDynamicLoaderCache::~cmDynamicLoaderCache()
{
}

void cmDynamicLoaderCache::CacheFile(const char* path,
  const cmsys::DynamicLoader::LibraryHandle& p)
{
  cmsys::DynamicLoader::LibraryHandle h;
  if ( this->GetCacheFile(path, h) )
    {
    this->FlushCache(path);
    }
  this->CacheMap[path] = p;
}

bool cmDynamicLoaderCache::GetCacheFile(const char* path,
  cmsys::DynamicLoader::LibraryHandle& p)
{
  std::map<cmStdString, cmsys::DynamicLoader::LibraryHandle>::iterator it
    = this->CacheMap.find(path);
  if ( it != this->CacheMap.end() )
    {
    p = it->second;
    return true;
    }
  return false;
}

bool cmDynamicLoaderCache::FlushCache(const char* path)
{
  std::map<cmStdString, cmsys::DynamicLoader::LibraryHandle>::iterator it
    = this->CacheMap.find(path);
  bool ret = false;
  if ( it != this->CacheMap.end() )
    {
    cmsys::DynamicLoader::CloseLibrary(it->second);
    this->CacheMap.erase(it);
    ret = true;
    }
  return ret;
}

void cmDynamicLoaderCache::FlushCache()
{
  for ( std::map<cmStdString,
    cmsys::DynamicLoader::LibraryHandle>::iterator it
    = this->CacheMap.begin();
        it != this->CacheMap.end(); it++ )
    {
    cmsys::DynamicLoader::CloseLibrary(it->second);
    }
  delete cmDynamicLoaderCache::Instance;
  cmDynamicLoaderCache::Instance = 0;
}

cmDynamicLoaderCache* cmDynamicLoaderCache::GetInstance()
{
  if ( !cmDynamicLoaderCache::Instance )
    {
    cmDynamicLoaderCache::Instance = new cmDynamicLoaderCache;
    }
  return cmDynamicLoaderCache::Instance;
}

cmsys::DynamicLoader::LibraryHandle cmDynamicLoader::OpenLibrary(
  const char* libname )
{
  cmsys::DynamicLoader::LibraryHandle lh;
  if ( cmDynamicLoaderCache::GetInstance()->GetCacheFile(libname, lh) )
    {
    return lh;
    }
  lh = cmsys::DynamicLoader::OpenLibrary(libname);
  cmDynamicLoaderCache::GetInstance()->CacheFile(libname, lh);
  return lh;
}

void cmDynamicLoader::FlushCache()
{
  cmDynamicLoaderCache::GetInstance()->FlushCache();
}

// Stay consistent with the Modules/Platform directory as
// to what the correct prefix and lib extension
const char* cmDynamicLoader::LibPrefix()
{
  return CMAKE_SHARED_MODULE_PREFIX;
}

const char* cmDynamicLoader::LibExtension()
{
  return CMAKE_SHARED_MODULE_SUFFIX;
}

