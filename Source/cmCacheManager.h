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

/** \class cmCacheManager
 * \brief Control class for cmake's cache
 *
 * Load and Save CMake cache files.
 * 
 */
class cmCacheManager
{
public:
  /**
   * Types for the cache entries.  These are useful as
   * hints for a cache editor program.  Path should bring
   * up a file chooser, BOOL a check box, and STRING a 
   * text entry box.
   */
  enum CacheEntryType{ BOOL=0, PATH, STRING  };
  static CacheEntryType StringToType(const char*);
  /**
   * Singleton pattern get instance of the cmCacheManager.
   */
  static cmCacheManager* GetInstance();
  /**
   * Load a cache from file
   */
  bool LoadCache(const char* path);
  
  /**
   * Save the cache to a file
   */
  bool SaveCache(const char* path);
  
  /**
   * Add an entry into the cache
   */
  void AddCacheEntry(const char* key, const char* value, CacheEntryType type);
  
  /**
   * Get a value from the cache given a key
   */
  const char* GetCacheValue(const char* key);
private:
  static cmCacheManager* s_Instance;
  class CacheEntry
  {
  public:
    std::string m_Value;
    CacheEntryType m_Type;
  };
  std::map<std::string, CacheEntry> m_Cache;
};

#endif
