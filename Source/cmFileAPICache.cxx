/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPICache.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmState.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

class Cache
{
  cmFileAPI& FileAPI;
  unsigned long Version;
  cmState* State;

  Json::Value DumpEntries();
  Json::Value DumpEntry(std::string const& name);
  Json::Value DumpEntryProperties(std::string const& name);
  Json::Value DumpEntryProperty(std::string const& name,
                                std::string const& prop);

public:
  Cache(cmFileAPI& fileAPI, unsigned long version);
  Json::Value Dump();
};

Cache::Cache(cmFileAPI& fileAPI, unsigned long version)
  : FileAPI(fileAPI)
  , Version(version)
  , State(this->FileAPI.GetCMakeInstance()->GetState())
{
  static_cast<void>(this->Version);
}

Json::Value Cache::Dump()
{
  Json::Value cache = Json::objectValue;
  cache["entries"] = this->DumpEntries();
  return cache;
}

Json::Value Cache::DumpEntries()
{
  Json::Value entries = Json::arrayValue;

  std::vector<std::string> names = this->State->GetCacheEntryKeys();
  std::sort(names.begin(), names.end());

  for (std::string const& name : names) {
    entries.append(this->DumpEntry(name));
  }

  return entries;
}

Json::Value Cache::DumpEntry(std::string const& name)
{
  Json::Value entry = Json::objectValue;
  entry["name"] = name;
  entry["type"] =
    cmState::CacheEntryTypeToString(this->State->GetCacheEntryType(name));
  entry["value"] = this->State->GetSafeCacheEntryValue(name);

  Json::Value properties = this->DumpEntryProperties(name);
  if (!properties.empty()) {
    entry["properties"] = std::move(properties);
  }

  return entry;
}

Json::Value Cache::DumpEntryProperties(std::string const& name)
{
  Json::Value properties = Json::arrayValue;
  std::vector<std::string> props =
    this->State->GetCacheEntryPropertyList(name);
  std::sort(props.begin(), props.end());
  for (std::string const& prop : props) {
    properties.append(this->DumpEntryProperty(name, prop));
  }
  return properties;
}

Json::Value Cache::DumpEntryProperty(std::string const& name,
                                     std::string const& prop)
{
  Json::Value property = Json::objectValue;
  property["name"] = prop;
  cmValue p = this->State->GetCacheEntryProperty(name, prop);
  property["value"] = p ? *p : "";
  return property;
}
}

Json::Value cmFileAPICacheDump(cmFileAPI& fileAPI, unsigned long version)
{
  Cache cache(fileAPI, version);
  return cache.Dump();
}
