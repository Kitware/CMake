/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

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
  enum CacheEntryType{ BOOL=0, PATH, FILEPATH, STRING, INTERNAL,STATIC };
  struct CacheEntry
  {
    std::string m_Value;
    std::string m_HelpString;
    CacheEntryType m_Type;
  };
  class CacheIterator
  {
  public:
    void Begin();
    bool IsAtEnd();
    void Next();
    const char *GetName() {
      return position->first.c_str(); } 
    CacheEntry const &GetEntry() {
      return position->second; }
    cmCacheManager const &m_Container;
    std::map<cmStdString, CacheEntry>::const_iterator position;
    CacheIterator(cmCacheManager const &foo) : m_Container(foo) {
      this->Begin();
    }
  };
  friend class cmCacheManager::CacheIterator;
  
  ///! return an iterator to iterate through the cache map
  cmCacheManager::CacheIterator NewIterator() 
    {
      return CacheIterator(*this);
    } 
  
  typedef  std::map<cmStdString, CacheEntry> CacheEntryMap;
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
  
  ///! Get a cache entry object for a key
  CacheEntry *GetCacheEntry(const char *key);
  
  bool IsAdvanced(const char* key);
  
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

private:
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
