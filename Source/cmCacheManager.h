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
#ifndef cmCacheManager_h
#define cmCacheManager_h

#include "cmStandardIncludes.h"
class cmMakefile;

/** \class cmCacheManager
 * \brief Control class for cmake's cache
 *
 * Load and Save CMake cache files.
 * 
 */
class cmCacheManager
{
public:
  class CacheIterator;
  friend class cmCacheManager::CacheIterator;
  enum CacheEntryType{ BOOL=0, PATH, FILEPATH, STRING, INTERNAL,STATIC,UNINITIALIZED };

private:
  struct CacheEntry
  {
    std::string m_Value;
    CacheEntryType m_Type;
    std::map<cmStdString,cmStdString> m_Properties;
  };

public:
  class CacheIterator
  {
  public:
    void Begin();
    bool Find(const char*);
    bool IsAtEnd() const;
    void Next();
    const char *GetName() const {
      return m_Position->first.c_str(); } 
    const char* GetProperty(const char*) const ;
    bool GetPropertyAsBool(const char*) const ;
    bool PropertyExists(const char*) const;
    void SetProperty(const char* property, const char* value);
    void SetProperty(const char* property, bool value);
    const char* GetValue() const { return this->GetEntry().m_Value.c_str(); }
    void SetValue(const char*);
    CacheEntryType GetType() const { return this->GetEntry().m_Type; }
    cmCacheManager &m_Container;
    std::map<cmStdString, CacheEntry>::iterator m_Position;
    CacheIterator(cmCacheManager &cm) : m_Container(cm) {
      this->Begin();
    }
    CacheIterator(cmCacheManager &cm, const char* key) : m_Container(cm) 
      {
      if ( key )
        {
        this->Find(key);
        }
    }
  private:
    CacheEntry const& GetEntry() const { return m_Position->second; }
    CacheEntry& GetEntry() { return m_Position->second; }
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
  
  ///! Load a cache for given makefile.  Loads from ouput home.
  bool LoadCache(cmMakefile*); 
  ///! Load a cache for given makefile.  Loads from path/CMakeCache.txt.
  bool LoadCache(const char* path);
  bool LoadCache(const char* path, bool internal);
  bool LoadCache(const char* path, bool internal, 
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);

  ///! Save cache for given makefile.  Saves to ouput home CMakeCache.txt.
  bool SaveCache(cmMakefile*) ;
  ///! Save cache for given makefile.  Saves to ouput path/CMakeCache.txt
  bool SaveCache(const char* path) ;

  ///! Print the cache to a stream
  void PrintCache(std::ostream&) const;
  
  ///! Get the iterator for an entry with a given key.
  cmCacheManager::CacheIterator GetCacheIterator(const char *key=0);
  
  ///! Remove an entry from the cache
  void RemoveCacheEntry(const char* key);
  
  ///! Get the number of entries in the cache
  int GetSize() {
    return static_cast<int>(m_Cache.size()); }
  
  ///! Break up a line like VAR:type="value" into var, type and value
  static bool ParseEntry(const char* entry, 
                         std::string& var,
                         std::string& value,
                         CacheEntryType& type);

  ///! Get a value from the cache given a key
  const char* GetCacheValue(const char* key) const;

protected:
  ///! Add an entry into the cache
  void AddCacheEntry(const char* key, const char* value, 
                     const char* helpString, CacheEntryType type);

  ///! Add a BOOL entry into the cache
  void AddCacheEntry(const char* key, bool, const char* helpString);

  ///! Get a cache entry object for a key
  CacheEntry *GetCacheEntry(const char *key);

private:
  typedef  std::map<cmStdString, CacheEntry> CacheEntryMap;
  static void OutputHelpString(std::ofstream& fout, 
                               const std::string& helpString);
  CacheEntryMap m_Cache;
  // Only cmake and cmMakefile should be able to add cache values
  // the commands should never use the cmCacheManager directly
  friend class cmMakefile; // allow access to add cache values
  friend class cmake; // allow access to add cache values
  friend class cmakewizard; // allow access to add cache values
};

#endif
