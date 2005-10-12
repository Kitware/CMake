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
#include "cmFileTimeComparison.h"

#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/SystemTools.hxx>

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cmsys/hash_map.hxx>
#endif

#include <ctype.h>
#include <sys/stat.h>

#if !defined(_WIN32) || defined(__CYGWIN__)
#  define cmFileTimeComparison_Type struct stat
#else
#  define cmFileTimeComparison_Type FILETIME
#endif

class cmFileTimeComparisonInternal
{
public:
  inline bool FileTimeCompare(const char* f1, const char* f2, int* result);

private:
#if defined(CMAKE_BUILD_WITH_CMAKE)
  class HashString
    {
  public:
    size_t operator()(const cmStdString& s) const
      {
      return h(s.c_str());
      }
    cmsys::hash<const char*> h;
    };
  typedef cmsys::hash_map<cmStdString, cmFileTimeComparison_Type, HashString> FileStatsMap;
  FileStatsMap Files;
#endif

  inline bool Stat(const char* fname, cmFileTimeComparison_Type* st);
  inline int Compare(cmFileTimeComparison_Type* st1, cmFileTimeComparison_Type* st2);
};

bool cmFileTimeComparisonInternal::Stat(const char* fname, cmFileTimeComparison_Type* st)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  cmFileTimeComparisonInternal::FileStatsMap::iterator fit = this->Files.find(fname);
  if ( fit != this->Files.end() )
    {
    *st = fit->second;
    return 0;
    }
#endif
#if !defined(_WIN32) || defined(__CYGWIN__)
  int res = ::stat(fname, st);
  if ( res != 0 )
    {
    return false;
    }
#else
  // Windows version.  Create file handles and get the modification times.
  HANDLE hf1 = CreateFile(f1, GENERIC_READ, FILE_SHARE_READ,
                          NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
                          NULL);
  if(hf1 == INVALID_HANDLE_VALUE)
    {
    return false;
    }
  if(!GetFileTime(hf1, 0, 0, st))
    {
    CloseHandle(hf1);
    return false;
    }
  CloseHandle(hf1);
#endif
#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->Files[fname] = *st;
#endif
  return true;
}

cmFileTimeComparison::cmFileTimeComparison()
{
  m_Internals = new cmFileTimeComparisonInternal;
}

cmFileTimeComparison::~cmFileTimeComparison()
{
  delete m_Internals;
}

bool cmFileTimeComparison::FileTimeCompare(const char* f1, const char* f2, int* result)
{
  return m_Internals->FileTimeCompare(f1, f2, result);
}

int cmFileTimeComparisonInternal::Compare(cmFileTimeComparison_Type* s1, cmFileTimeComparison_Type* s2)
{
#if !defined(_WIN32) || defined(__CYGWIN__)
# if KWSYS_STAT_HAS_ST_MTIM
  // Compare using nanosecond resolution.
  if(s1->st_mtim.tv_sec < s2->st_mtim.tv_sec)
    {
    return -1;
    }
  else if(s1->st_mtim.tv_sec > s2->st_mtim.tv_sec)
    {
    return 1;
    }
  else if(s1->st_mtim.tv_nsec < s2->st_mtim.tv_nsec)
    {
    return -1;
    }
  else if(s1->st_mtim.tv_nsec > s2->st_mtim.tv_nsec)
    {
    return 1;
    }
# else
  // Compare using 1 second resolution.
  if(s1->st_mtime < s2->st_mtime)
    {
    return -1;
    }
  else if(s1->st_mtime > s2->st_mtime)
    {
    return 1;
    }
  return 0;
# endif
#else
  return (int)CompareFileTime(s1, s2);
#endif
}

bool cmFileTimeComparisonInternal::FileTimeCompare(const char* f1, const char* f2, int* result)
{
  // Default to same time.
  *result = 0;
  cmFileTimeComparison_Type s1;
  if(!this->Stat(f1, &s1))
    {
    return false;
    }
  cmFileTimeComparison_Type s2;
  if(!this->Stat(f2, &s2))
    {
    return false;
    }
  *result = this->Compare(&s1, &s2);
  return true;
}

