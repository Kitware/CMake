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
  cmListFileArgument(): Value(), Quoted(false) {}
  cmListFileArgument(const cmListFileArgument& r): Value(r.Value), Quoted(r.Quoted) {}
  cmListFileArgument(const std::string& v, bool q): Value(v), Quoted(q) {}
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
};

struct cmListFileFunction
{
  std::string m_Name;
  std::vector<cmListFileArgument> m_Arguments;
  std::string m_FilePath;
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
    
  /**
   * Read a CMake command (or function) from an input file.  This
   * returns the name of the function and a list of its 
   * arguments.  The last argument is the name of the file that 
   * the ifstream points to, and is used for debug info only.
   */
  static bool ParseFunction(std::ifstream&, cmListFileFunction& function,
                            const char* filename, bool& parseError,
                            long& line);

  /**
   *  Extract white-space separated arguments from a string.
   *  Double quoted strings are accepted with spaces.
   *  This is called by ParseFunction.
   */
  static void GetArguments(std::string& line,
                           std::vector<cmListFileArgument>& arguments);
  
private:
  // Cache the file
  bool CacheFile(const char* path, bool requireProjectCommand);
  // private data
  typedef std::map<cmStdString, cmListFile> ListFileMap;
  ListFileMap m_ListFileCache;  // file name to ListFile map
  static cmListFileCache* Instance; // singelton pointer
};


#endif
