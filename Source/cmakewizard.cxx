#include "cmCacheManager.h"
#include "cmake.h"

bool advanced = false;
  
void Ask(const char* key, cmCacheManager::CacheEntry & entry)
{
  if(!advanced)
    {
    if(cmCacheManager::GetInstance()->IsAdvanced(key))
      {
      return;
      }
    }
  
  std::cout << "Variable Name: " << key << "\n";
  std::cout << "Description:   " << entry.m_HelpString << "\n";
  std::cout << "Current Value: " << entry.m_Value.c_str() << "\n";
  std::cout << "New Value (Enter to keep current value): ";
  char buffer[4096];
  buffer[0] = 0;
  std::cin.getline(buffer, sizeof(buffer));
  if(buffer[0])
    {
    cmCacheManager::CacheEntry *entry = 
      cmCacheManager::GetInstance()->GetCacheEntry(key);
    if(entry)
      {
      entry->m_Value = buffer;
      }
    else
      {
      std::cerr << "strange error, should be in cache but is not... " << key << "\n";
      }
    }
  std::cout << "\n";
}


main(int ac, char** av)
{
  std::vector<std::string> args;
  for(int j=0; j < ac; ++j)
    {
    args.push_back(av[j]);
    }
  cmSystemTools::DisableRunCommandOutput();
  std::cout << "Would you like to see advanced options? [No]:";  
  char buffer[4096];
  buffer[0] = 0;
  std::cin.getline(buffer, sizeof(buffer));
  if(buffer[0])
    {
    if(buffer[0] == 'y' || buffer[0] == 'Y')
      {
      advanced = true;
      }
    }
  cmake make;
  cmCacheManager::CacheEntryMap askedCache;
  bool asked = false;
  // continue asking questions until no new questions are asked
  do
    {
    asked = false;
    // run cmake
    std::cout << "Please wait while cmake processes CMakeLists.txt files....\n";
    make.Generate(args);
    std::cout << "\n";
    // load the cache from disk
    cmCacheManager::GetInstance()->
      LoadCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());
    cmCacheManager::CacheEntryMap const& currentCache = 
      cmCacheManager::GetInstance()->GetCacheMap();
    // iterate over all entries in the cache
    for(cmCacheManager::CacheEntryMap::const_iterator i = currentCache.begin();
        i != currentCache.end(); ++i)
      { 
      std::string key = i->first;
      cmCacheManager::CacheEntry ce = i->second;
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
          Ask(key.c_str(), ce);
          asked = true;
          }
        }
      else
        {
        Ask(key.c_str(), ce);
        asked = true;
        }
      askedCache[key] = i->second;
      }
    cmCacheManager::GetInstance()->
      SaveCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());

    }
  while(asked);
  std::cout << "CMake complete, run make to build project.\n";
}
