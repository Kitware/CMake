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



cmListFile* cmListFileCache::GetFileCache(const char* path)
{
  ListFileMap::iterator sl = m_ListFileCache.find(path);
  if (sl == m_ListFileCache.end())
    {
    // if not already in the map, then parse and store the 
    // file
    if(!this->CacheFile(path))
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
    if(!this->CacheFile(path))
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


bool cmListFileCache::CacheFile(const char* path)
{
  std::ifstream fin(path);
  if(!fin)
    {
    cmSystemTools::Error("error can not open file ", path);
    return false;
    }
  std::string name;
  std::vector<std::string> arguments;
  cmListFile inFile;
  inFile.m_ModifiedTime = cmSystemTools::ModifiedTime(path);
  while ( fin )
    {
    cmListFileFunction inFunction;
    if(cmSystemTools::ParseFunction(fin, 
                                    inFunction.m_Name,
                                    inFunction.m_Arguments))
      {
      inFile.m_Functions.push_back(inFunction);
      }
    }
  m_ListFileCache[path] = inFile;
  return true;
}
