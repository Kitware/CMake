/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCacheManager_h
#define cmCacheManager_h

#include "cmStandardIncludes.h"
#include "cmPropertyMap.h"
class cmMakefile;
class cmMarkAsAdvancedCommand;
class cmake;

/** \class cmCacheManager
 * \brief Control class for cmake's cache
 *
 * Load and Save CMake cache files.
 *
 */
class cmCacheManager
{
public:
  cmCacheManager(cmake* cm);
  class CacheIterator;
  friend class cmCacheManager::CacheIterator;
  enum CacheEntryType{ BOOL=0, PATH, FILEPATH, STRING, INTERNAL,STATIC,
                       UNINITIALIZED };

private:
  struct CacheEntry
  {
    std::string Value;
    CacheEntryType Type;
    cmPropertyMap Properties;
    const char* GetProperty(const std::string&) const;
    void SetProperty(const std::string& property, const char* value);
    void AppendProperty(const std::string& property, const char* value,
                        bool asString=false);
    bool Initialized;
    CacheEntry() : Value(""), Type(UNINITIALIZED), Initialized(false)
      {}
  };

public:
  class CacheIterator
  {
  public:
    void Begin();
    bool Find(const std::string&);
    bool IsAtEnd() const;
    void Next();
    std::string GetName() const {
      return this->Position->first; }
    const char* GetProperty(const std::string&) const ;
    bool GetPropertyAsBool(const std::string&) const ;
    bool PropertyExists(const std::string&) const;
    void SetProperty(const std::string& property, const char* value);
    void AppendProperty(const std::string& property, const char* value,
                        bool asString=false);
    void SetProperty(const std::string& property, bool value);
    std::string GetValue() const { return this->GetEntry().Value; }
    bool GetValueAsBool() const;
    void SetValue(const char*);
    CacheEntryType GetType() const { return this->GetEntry().Type; }
    void SetType(CacheEntryType ty) { this->GetEntry().Type = ty; }
    bool Initialized() { return this->GetEntry().Initialized; }
    cmCacheManager &Container;
    std::map<std::string, CacheEntry>::iterator Position;
    CacheIterator(cmCacheManager &cm) : Container(cm) {
      this->Begin();
    }
    CacheIterator(cmCacheManager &cm, const char* key) : Container(cm)
      {
      if ( key )
        {
        this->Find(key);
        }
      }
  private:
    CacheEntry const& GetEntry() const { return this->Position->second; }
    CacheEntry& GetEntry() { return this->Position->second; }
  };

  ///! return an iterator to iterate through the cache map
  cmCacheManager::CacheIterator NewIterator()
    {
      return CacheIterator(*this);
    }

  /**
   * Types for the cache entries.  These are useful as
   * hints for a cache editor program.  Path should bring
   * up a file chooser, BOOL a check box, and STRING a
   * text entry box, FILEPATH is a full path to a file which
   * can be different than just a path input
   */
  static CacheEntryType StringToType(const char*);
  static const char* TypeToString(CacheEntryType);
  static bool IsType(const char*);

  ///! Load a cache for given makefile.  Loads from ouput home.
  bool LoadCache(cmMakefile*);
  ///! Load a cache for given makefile.  Loads from path/CMakeCache.txt.
  bool LoadCache(const std::string& path);
  bool LoadCache(const std::string& path, bool internal);
  bool LoadCache(const std::string& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);

  ///! Save cache for given makefile.  Saves to ouput home CMakeCache.txt.
  bool SaveCache(cmMakefile*) ;
  ///! Save cache for given makefile.  Saves to ouput path/CMakeCache.txt
  bool SaveCache(const std::string& path) ;

  ///! Delete the cache given
  bool DeleteCache(const std::string& path);

  ///! Print the cache to a stream
  void PrintCache(std::ostream&) const;

  ///! Get the iterator for an entry with a given key.
  cmCacheManager::CacheIterator GetCacheIterator(const char *key=0);

  ///! Remove an entry from the cache
  void RemoveCacheEntry(const std::string& key);

  ///! Get the number of entries in the cache
  int GetSize() {
    return static_cast<int>(this->Cache.size()); }

  ///! Break up a line like VAR:type="value" into var, type and value
  static bool ParseEntry(const std::string& entry,
                         std::string& var,
                         std::string& value,
                         CacheEntryType& type);

  ///! Get a value from the cache given a key
  const char* GetCacheValue(const std::string& key) const;

  /** Get the version of CMake that wrote the cache.  */
  unsigned int GetCacheMajorVersion() const
    { return this->CacheMajorVersion; }
  unsigned int GetCacheMinorVersion() const
    { return this->CacheMinorVersion; }
  bool NeedCacheCompatibility(int major, int minor);

protected:
  ///! Add an entry into the cache
  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString, CacheEntryType type);

  ///! Get a cache entry object for a key
  CacheEntry *GetCacheEntry(const std::string& key);
  ///! Clean out the CMakeFiles directory if no CMakeCache.txt
  void CleanCMakeFiles(const std::string& path);

  // Cache version info
  unsigned int CacheMajorVersion;
  unsigned int CacheMinorVersion;
private:
  cmake* CMakeInstance;
  typedef  std::map<std::string, CacheEntry> CacheEntryMap;
  static void OutputHelpString(std::ostream& fout,
                               const std::string& helpString);
  static void OutputKey(std::ostream& fout, std::string const& key);
  static void OutputValue(std::ostream& fout, std::string const& value);

  static const char* PersistentProperties[];
  bool ReadPropertyEntry(std::string const& key, CacheEntry& e);
  void WritePropertyEntries(std::ostream& os, CacheIterator const& i);

  CacheEntryMap Cache;
  // Only cmake and cmMakefile should be able to add cache values
  // the commands should never use the cmCacheManager directly
  friend class cmMakefile; // allow access to add cache values
  friend class cmake; // allow access to add cache values
  friend class cmMarkAsAdvancedCommand; // allow access to add cache values
};

#endif
