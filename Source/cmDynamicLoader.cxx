/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDynamicLoader.h"

#include <map>
#include <string>
#include <utility>

namespace {
class cmDynamicLoaderCache
{
public:
  ~cmDynamicLoaderCache();
  void CacheFile(char const* path, cmsys::DynamicLoader::LibraryHandle /*p*/);
  bool GetCacheFile(char const* path,
                    cmsys::DynamicLoader::LibraryHandle& /*p*/);
  bool FlushCache(char const* path);
  void FlushCache();
  static cmDynamicLoaderCache& GetInstance();

private:
  std::map<std::string, cmsys::DynamicLoader::LibraryHandle> CacheMap;
  static cmDynamicLoaderCache Instance;
};

cmDynamicLoaderCache cmDynamicLoaderCache::Instance;
}

cmDynamicLoaderCache::~cmDynamicLoaderCache() = default;

void cmDynamicLoaderCache::CacheFile(char const* path,
                                     cmsys::DynamicLoader::LibraryHandle p)
{
  cmsys::DynamicLoader::LibraryHandle h;
  if (this->GetCacheFile(path, h)) {
    this->FlushCache(path);
  }
  this->CacheMap[path] = p;
}

bool cmDynamicLoaderCache::GetCacheFile(char const* path,
                                        cmsys::DynamicLoader::LibraryHandle& p)
{
  auto it = this->CacheMap.find(path);
  if (it != this->CacheMap.end()) {
    p = it->second;
    return true;
  }
  return false;
}

bool cmDynamicLoaderCache::FlushCache(char const* path)
{
  auto it = this->CacheMap.find(path);
  bool ret = false;
  if (it != this->CacheMap.end()) {
    cmsys::DynamicLoader::CloseLibrary(it->second);
    this->CacheMap.erase(it);
    ret = true;
  }
  return ret;
}

void cmDynamicLoaderCache::FlushCache()
{
  for (auto const& it : this->CacheMap) {
    cmsys::DynamicLoader::CloseLibrary(it.second);
  }
  this->CacheMap.clear();
}

cmDynamicLoaderCache& cmDynamicLoaderCache::GetInstance()
{
  return cmDynamicLoaderCache::Instance;
}

cmsys::DynamicLoader::LibraryHandle cmDynamicLoader::OpenLibrary(
  char const* libname)
{
  cmsys::DynamicLoader::LibraryHandle lh;
  if (cmDynamicLoaderCache::GetInstance().GetCacheFile(libname, lh)) {
    return lh;
  }
  lh = cmsys::DynamicLoader::OpenLibrary(libname);
  cmDynamicLoaderCache::GetInstance().CacheFile(libname, lh);
  return lh;
}

void cmDynamicLoader::FlushCache()
{
  cmDynamicLoaderCache::GetInstance().FlushCache();
}
