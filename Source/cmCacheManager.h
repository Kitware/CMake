/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

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
  enum CacheEntryType{ BOOL=0, PATH, FILEPATH, STRING, INTERNAL  };
  struct CacheEntry
  {
    std::string m_Value;
    std::string m_HelpString;
    CacheEntryType m_Type;
  };
public:
  typedef  std::map<std::string, CacheEntry> CacheEntryMap;
  /**
   * Types for the cache entries.  These are useful as
   * hints for a cache editor program.  Path should bring
   * up a file chooser, BOOL a check box, and STRING a 
   * text entry box, FILEPATH is a full path to a file which
   * can be different than just a path input
   */
  static CacheEntryType StringToType(const char*);
  //! Singleton pattern get instance of the cmCacheManager.
  static cmCacheManager* GetInstance();

  
  //! Load a cache for given makefile.  Loads from ouput home.
  bool LoadCache(cmMakefile*); 
  //! Load a cache for given makefile.  Loads from path/CMakeCache.txt.
  bool LoadCache(const char* path);
  
  //! Save cache for given makefile.  Saves to ouput home CMakeCache.txt.
  bool SaveCache(cmMakefile*) const;
  //! Save cache for given makefile.  Saves to ouput path/CMakeCache.txt
  bool SaveCache(const char* path) const;
  
  //! Add an entry into the cache
  void AddCacheEntry(const char* key, const char* value, 
                     const char* helpString, CacheEntryType type);

  //! Add a BOOL entry into the cache
  void AddCacheEntry(const char* key, bool, const char* helpString);
  
  //! Remove an entry from the cache
  void RemoveCacheEntry(const char* key);
  
  //! Print the cache to a stream
  CacheEntry *GetCacheEntry(const char *key);
  
  //! Get a value from the cache given a key
  const char* GetCacheValue(const char* key) const;

  //! Test a boolean cache entry to see if it is true or false, returns false 
  //  if no entry.
  bool IsOn(const char*) const;
  
  //! Print the cache to a stream
  void PrintCache(std::ostream&) const;
  
  //! Get the cache map ivar.
  const CacheEntryMap &GetCacheMap() const { return m_Cache; }
  
private:
  static void OutputHelpString(std::ofstream& fout, 
                               const std::string& helpString);
  static cmCacheManager* s_Instance;
  CacheEntryMap m_Cache;
};

#endif
