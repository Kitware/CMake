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
#ifndef cmListFileCache_h
#define cmListFileCache_h

#include "cmStandardIncludes.h"

/** \class cmListFileCache
 * \brief A class to cache list file contents.
 *
 * cmListFileCache is a class used to cache the contents of parsed
 * cmake list files.
 */

struct cmListFileFunction
{
  std::string m_Name;
  std::vector<std::string> m_Arguments;
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
   *  NULL is returned.
   */
  cmListFile* GetFileCache(const char* path);
private:
  // Cache the file
  bool CacheFile(const char* path);
  // private data
  typedef std::map<cmStdString, cmListFile> ListFileMap;
  ListFileMap m_ListFileCache;  // file name to ListFile map
  static cmListFileCache* Instance; // singelton pointer
};


#endif
