/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
public:
  typedef  std::map<cmStdString, CacheEntry> CacheEntryMap;
  /**
   * Types for the cache entries.  These are useful as
   * hints for a cache editor program.  Path should bring
   * up a file chooser, BOOL a check box, and STRING a 
   * text entry box, FILEPATH is a full path to a file which
   * can be different than just a path input
   */
  static CacheEntryType StringToType(const char*);
  ///! Singleton pattern get instance of the cmCacheManager.
  static cmCacheManager* GetInstance();
  
  ///! Load a cache for given makefile.  Loads from ouput home.
  bool LoadCache(cmMakefile*); 
  ///! Load a cache for given makefile.  Loads from path/CMakeCache.txt.
  bool LoadCache(const char* path);
  bool LoadCache(const char* path, bool internal);
  bool LoadCache(const char* path, bool internal, 
		 std::set<std::string>& excludes,
		 std::set<std::string>& includes);

  ///! Put cache definitions into makefile
  void DefineCache(cmMakefile*); 
  
  ///! Save cache for given makefile.  Saves to ouput home CMakeCache.txt.
  bool SaveCache(cmMakefile*) ;
  ///! Save cache for given makefile.  Saves to ouput path/CMakeCache.txt
  bool SaveCache(const char* path) ;

  ///! Print the cache to a stream
  void PrintCache(std::ostream&) const;
  
  ///! Get the cache map ivar.
  const CacheEntryMap &GetCacheMap() const { return m_Cache; }

  ///! Get a cache entry object for a key
  CacheEntry *GetCacheEntry(const char *key);
  
  ///! Remove an entry from the cache
  void RemoveCacheEntry(const char* key);
  
protected:
  ///! Add an entry into the cache
  void AddCacheEntry(const char* key, const char* value, 
                     const char* helpString, CacheEntryType type);

  ///! Add a BOOL entry into the cache
  void AddCacheEntry(const char* key, bool, const char* helpString);
  
  ///! Get a value from the cache given a key
  const char* GetCacheValue(const char* key) const;

private:
  static void OutputHelpString(std::ofstream& fout, 
                               const std::string& helpString);
  static cmCacheManager* s_Instance;
  CacheEntryMap m_Cache;
  // Only cmake and cmMakefile should be able to add cache values
  // the commands should never use the cmCacheManager directly
  friend class cmMakefile; // allow access to add cache values
  friend class cmake; // allow access to add cache values
};

#endif
