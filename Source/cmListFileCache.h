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
#ifndef cmListFileCache_h
#define cmListFileCache_h

#include "cmStandardIncludes.h"

/** \class cmListFileCache
 * \brief A class to cache list file contents.
 *
 * cmListFileCache is a class used to cache the contents of parsed
 * cmake list files.
 */

struct cmListFileArgument
{
  cmListFileArgument(): Value(), Quoted(false), FilePath(0), Line(0) {}
  cmListFileArgument(const cmListFileArgument& r):
    Value(r.Value), Quoted(r.Quoted), FilePath(r.FilePath), Line(r.Line) {}
  cmListFileArgument(const std::string& v, bool q, const char* file,
                     long line): Value(v), Quoted(q),
                                 FilePath(file), Line(line) {}
  bool operator == (const cmListFileArgument& r) const
    {
    return (this->Value == r.Value) && (this->Quoted == r.Quoted);
    }
  bool operator != (const cmListFileArgument& r) const
    {
    return !(*this == r);
    }
  std::string Value;
  bool Quoted;
  const char* FilePath;
  long Line;
};

struct cmListFileFunction
{
  std::string m_Name;
  std::vector<cmListFileArgument> m_Arguments;
  const char* m_FilePath;
  long m_Line;
};

struct cmListFile
{
  cmListFile() 
    :m_ModifiedTime(0) 
    {
    }
  long int m_ModifiedTime;
  std::vector<cmListFileFunction> m_Functions;
};

class cmListFileCache
{
public:
  static cmListFileCache* GetInstance();
  static void ClearCache();


  /** Return the cached version of the given file.
   *  If the file is not already in the cache, a cache entry
   *  will be made.  If there is an error loading the file,
   *  NULL is returned.  If requireProjectCommand is true,
   *  then a PROJECT(Project) command will be added to the file
   *  if it does not have a PROJECT command in it.
   */
  cmListFile* GetFileCache(const char* path, bool requireProjectCommand);

  //! Flush cache file out of cache.
  void FlushCache(const char* path);

  ~cmListFileCache();
private:
  // Cache the file
  bool CacheFile(const char* path, bool requireProjectCommand);
  // private data
  typedef std::map<cmStdString, cmListFile> ListFileMap;
  ListFileMap m_ListFileCache;  // file name to ListFile map

  typedef std::map<cmStdString, char*> UniqueStrings;
  UniqueStrings m_UniqueStrings;
  const char* GetUniqueStringPointer(const char* name);

  static cmListFileCache* Instance; // singelton pointer
};

#endif
