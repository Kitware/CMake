/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCacheManager_h
#define cmCacheManager_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmPropertyMap.h"
#include "cmStateTypes.h"

class cmMessenger;

/** \class cmCacheManager
 * \brief Control class for cmake's cache
 *
 * Load and Save CMake cache files.
 *
 */
class cmCacheManager
{
public:
  cmCacheManager();
  class CacheIterator;
  friend class cmCacheManager::CacheIterator;

private:
  struct CacheEntry
  {
    std::string Value;
    cmStateEnums::CacheEntryType Type = cmStateEnums::UNINITIALIZED;
    cmPropertyMap Properties;
    std::vector<std::string> GetPropertyList() const;
    const char* GetProperty(const std::string&) const;
    void SetProperty(const std::string& property, const char* value);
    void AppendProperty(const std::string& property, const std::string& value,
                        bool asString = false);
    bool Initialized = false;
  };

public:
  class CacheIterator
  {
  public:
    void Begin();
    bool Find(const std::string&);
    bool IsAtEnd() const;
    void Next();
    std::string GetName() const { return this->Position->first; }
    std::vector<std::string> GetPropertyList() const;
    const char* GetProperty(const std::string&) const;
    bool GetPropertyAsBool(const std::string&) const;
    bool PropertyExists(const std::string&) const;
    void SetProperty(const std::string& property, const char* value);
    void AppendProperty(const std::string& property, const std::string& value,
                        bool asString = false);
    void SetProperty(const std::string& property, bool value);
    const std::string& GetValue() const { return this->GetEntry().Value; }
    bool GetValueAsBool() const;
    void SetValue(const char*);
    cmStateEnums::CacheEntryType GetType() const
    {
      return this->GetEntry().Type;
    }
    void SetType(cmStateEnums::CacheEntryType ty)
    {
      this->GetEntry().Type = ty;
    }
    bool Initialized() { return this->GetEntry().Initialized; }
    cmCacheManager& Container;
    std::map<std::string, CacheEntry>::iterator Position;
    CacheIterator(cmCacheManager& cm)
      : Container(cm)
    {
      this->Begin();
    }
    CacheIterator(cmCacheManager& cm, const char* key)
      : Container(cm)
    {
      if (key) {
        this->Find(key);
      }
    }

  private:
    CacheEntry const& GetEntry() const { return this->Position->second; }
    CacheEntry& GetEntry() { return this->Position->second; }
  };

  //! return an iterator to iterate through the cache map
  cmCacheManager::CacheIterator NewIterator() { return { *this }; }

  //! Load a cache for given makefile.  Loads from path/CMakeCache.txt.
  bool LoadCache(const std::string& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);

  //! Save cache for given makefile.  Saves to output path/CMakeCache.txt
  bool SaveCache(const std::string& path, cmMessenger* messenger);

  //! Delete the cache given
  bool DeleteCache(const std::string& path);

  //! Print the cache to a stream
  void PrintCache(std::ostream&) const;

  //! Get the iterator for an entry with a given key.
  cmCacheManager::CacheIterator GetCacheIterator(const std::string& key);
  cmCacheManager::CacheIterator GetCacheIterator();

  //! Remove an entry from the cache
  void RemoveCacheEntry(const std::string& key);

  //! Get the number of entries in the cache
  int GetSize() { return static_cast<int>(this->Cache.size()); }

  //! Get a value from the cache given a key
  const std::string* GetInitializedCacheValue(const std::string& key) const;

  const char* GetCacheEntryValue(const std::string& key)
  {
    cmCacheManager::CacheIterator it = this->GetCacheIterator(key);
    if (it.IsAtEnd()) {
      return nullptr;
    }
    return it.GetValue().c_str();
  }

  const char* GetCacheEntryProperty(std::string const& key,
                                    std::string const& propName)
  {
    return this->GetCacheIterator(key).GetProperty(propName);
  }

  cmStateEnums::CacheEntryType GetCacheEntryType(std::string const& key)
  {
    return this->GetCacheIterator(key).GetType();
  }

  bool GetCacheEntryPropertyAsBool(std::string const& key,
                                   std::string const& propName)
  {
    return this->GetCacheIterator(key).GetPropertyAsBool(propName);
  }

  void SetCacheEntryProperty(std::string const& key,
                             std::string const& propName,
                             std::string const& value)
  {
    this->GetCacheIterator(key).SetProperty(propName, value.c_str());
  }

  void SetCacheEntryBoolProperty(std::string const& key,
                                 std::string const& propName, bool value)
  {
    this->GetCacheIterator(key).SetProperty(propName, value);
  }

  void SetCacheEntryValue(std::string const& key, std::string const& value)
  {
    this->GetCacheIterator(key).SetValue(value.c_str());
  }

  void RemoveCacheEntryProperty(std::string const& key,
                                std::string const& propName)
  {
    this->GetCacheIterator(key).SetProperty(propName, nullptr);
  }

  void AppendCacheEntryProperty(std::string const& key,
                                std::string const& propName,
                                std::string const& value,
                                bool asString = false)
  {
    this->GetCacheIterator(key).AppendProperty(propName, value, asString);
  }

  std::vector<std::string> GetCacheEntryKeys()
  {
    std::vector<std::string> definitions;
    definitions.reserve(this->GetSize());
    cmCacheManager::CacheIterator cit = this->GetCacheIterator();
    for (cit.Begin(); !cit.IsAtEnd(); cit.Next()) {
      definitions.push_back(cit.GetName());
    }
    return definitions;
  }

  /** Get the version of CMake that wrote the cache.  */
  unsigned int GetCacheMajorVersion() const { return this->CacheMajorVersion; }
  unsigned int GetCacheMinorVersion() const { return this->CacheMinorVersion; }

protected:
  //! Add an entry into the cache
  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString,
                     cmStateEnums::CacheEntryType type);

  //! Get a cache entry object for a key
  CacheEntry* GetCacheEntry(const std::string& key);
  //! Clean out the CMakeFiles directory if no CMakeCache.txt
  void CleanCMakeFiles(const std::string& path);

  // Cache version info
  unsigned int CacheMajorVersion;
  unsigned int CacheMinorVersion;

private:
  using CacheEntryMap = std::map<std::string, CacheEntry>;
  static void OutputHelpString(std::ostream& fout,
                               const std::string& helpString);
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

  static const char* PersistentProperties[];
  bool ReadPropertyEntry(std::string const& key, CacheEntry& e);
  void WritePropertyEntries(std::ostream& os, CacheIterator i,
                            cmMessenger* messenger);

  CacheEntryMap Cache;
  // Only cmake and cmState should be able to add cache values
  // the commands should never use the cmCacheManager directly
  friend class cmState; // allow access to add cache values
  friend class cmake;   // allow access to add cache values
};

#endif
