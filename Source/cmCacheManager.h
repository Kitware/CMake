/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmPropertyMap.h"
#include "cmStateTypes.h"
#include "cmValue.h"

class cmMessenger;

/** \class cmCacheManager
 * \brief Control class for cmake's cache
 *
 * Load and Save CMake cache files.
 *
 */
class cmCacheManager
{
  class CacheEntry
  {
    friend class cmCacheManager;

  public:
    std::string const& GetValue() const { return this->Value; }
    void SetValue(cmValue);

    cmStateEnums::CacheEntryType GetType() const { return this->Type; }
    void SetType(cmStateEnums::CacheEntryType ty) { this->Type = ty; }

    std::vector<std::string> GetPropertyList() const;
    cmValue GetProperty(std::string const& property) const;
    bool GetPropertyAsBool(std::string const& property) const;
    void SetProperty(std::string const& property, std::string const& value);
    void SetProperty(std::string const& property, bool value);
    void RemoveProperty(std::string const& property);
    void AppendProperty(std::string const& property, std::string const& value,
                        bool asString = false);

  private:
    std::string Value;
    cmStateEnums::CacheEntryType Type = cmStateEnums::UNINITIALIZED;
    cmPropertyMap Properties;
    bool Initialized = false;
  };

public:
  //! Load a cache for given makefile.  Loads from path/CMakeCache.txt.
  bool LoadCache(std::string const& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);

  //! Save cache for given makefile.  Saves to output path/CMakeCache.txt
  bool SaveCache(std::string const& path, cmMessenger* messenger);

  //! Delete the cache given
  bool DeleteCache(std::string const& path);

  //! Print the cache to a stream
  void PrintCache(std::ostream&) const;

  //! Get whether or not cache is loaded
  bool IsCacheLoaded() const { return this->CacheLoaded; }

  //! Get a value from the cache given a key
  cmValue GetInitializedCacheValue(std::string const& key) const;

  cmValue GetCacheEntryValue(std::string const& key) const
  {
    if (auto const* entry = this->GetCacheEntry(key)) {
      return cmValue(entry->GetValue());
    }
    return nullptr;
  }

  void SetCacheEntryValue(std::string const& key, std::string const& value)
  {
    if (auto* entry = this->GetCacheEntry(key)) {
      entry->SetValue(cmValue(value));
    }
  }

  cmStateEnums::CacheEntryType GetCacheEntryType(std::string const& key) const
  {
    if (auto const* entry = this->GetCacheEntry(key)) {
      return entry->GetType();
    }
    return cmStateEnums::UNINITIALIZED;
  }

  std::vector<std::string> GetCacheEntryPropertyList(
    std::string const& key) const
  {
    if (auto const* entry = this->GetCacheEntry(key)) {
      return entry->GetPropertyList();
    }
    return {};
  }

  cmValue GetCacheEntryProperty(std::string const& key,
                                std::string const& propName) const
  {
    if (auto const* entry = this->GetCacheEntry(key)) {
      return entry->GetProperty(propName);
    }
    return nullptr;
  }

  bool GetCacheEntryPropertyAsBool(std::string const& key,
                                   std::string const& propName) const
  {
    if (auto const* entry = this->GetCacheEntry(key)) {
      return entry->GetPropertyAsBool(propName);
    }
    return false;
  }

  void SetCacheEntryProperty(std::string const& key,
                             std::string const& propName,
                             std::string const& value)
  {
    if (auto* entry = this->GetCacheEntry(key)) {
      entry->SetProperty(propName, value);
    }
  }

  void SetCacheEntryBoolProperty(std::string const& key,
                                 std::string const& propName, bool value)
  {
    if (auto* entry = this->GetCacheEntry(key)) {
      entry->SetProperty(propName, value);
    }
  }

  void RemoveCacheEntryProperty(std::string const& key,
                                std::string const& propName)
  {
    if (auto* entry = this->GetCacheEntry(key)) {
      entry->RemoveProperty(propName);
    }
  }

  void AppendCacheEntryProperty(std::string const& key,
                                std::string const& propName,
                                std::string const& value,
                                bool asString = false)
  {
    if (auto* entry = this->GetCacheEntry(key)) {
      entry->AppendProperty(propName, value, asString);
    }
  }

  std::vector<std::string> GetCacheEntryKeys() const
  {
    std::vector<std::string> definitions;
    definitions.reserve(this->Cache.size());
    for (auto const& i : this->Cache) {
      definitions.push_back(i.first);
    }
    return definitions;
  }

  /** Get the version of CMake that wrote the cache.  */
  unsigned int GetCacheMajorVersion() const { return this->CacheMajorVersion; }
  unsigned int GetCacheMinorVersion() const { return this->CacheMinorVersion; }

  //! Add an entry into the cache
  void AddCacheEntry(std::string const& key, std::string const& value,
                     std::string const& helpString,
                     cmStateEnums::CacheEntryType type)
  {
    this->AddCacheEntry(key, cmValue{ value }, cmValue{ helpString }, type);
  }
  void AddCacheEntry(std::string const& key, cmValue value,
                     std::string const& helpString,
                     cmStateEnums::CacheEntryType type)
  {
    this->AddCacheEntry(key, value, cmValue{ helpString }, type);
  }
  void AddCacheEntry(std::string const& key, cmValue value, cmValue helpString,
                     cmStateEnums::CacheEntryType type);

  //! Remove an entry from the cache
  void RemoveCacheEntry(std::string const& key);

private:
  //! Get a cache entry object for a key
  CacheEntry* GetCacheEntry(std::string const& key);
  CacheEntry const* GetCacheEntry(std::string const& key) const;

  //! Clean out the CMakeFiles directory if no CMakeCache.txt
  void CleanCMakeFiles(std::string const& path);

  static void OutputHelpString(std::ostream& fout,
                               std::string const& helpString);
  static void OutputWarningComment(std::ostream& fout,
                                   std::string const& message,
                                   bool wrapSpaces);
  static void OutputNewlineTruncationWarning(std::ostream& fout,
                                             std::string const& key,
                                             std::string const& value,
                                             cmMessenger* messenger);
  static void OutputKey(std::ostream& fout, std::string const& key);
  static void OutputValue(std::ostream& fout, std::string const& value);
  static void OutputValueNoNewlines(std::ostream& fout,
                                    std::string const& value);

  static char const* PersistentProperties[];
  bool ReadPropertyEntry(std::string const& key, CacheEntry const& e);
  void WritePropertyEntries(std::ostream& os, std::string const& entryKey,
                            CacheEntry const& e, cmMessenger* messenger) const;

  std::map<std::string, CacheEntry> Cache;
  bool CacheLoaded = false;

  // Cache version info
  unsigned int CacheMajorVersion = 0;
  unsigned int CacheMinorVersion = 0;
};
