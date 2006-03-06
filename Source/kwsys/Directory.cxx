/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Directory.hxx)

#include KWSYS_HEADER(Configure.hxx)

#include KWSYS_HEADER(stl/string)
#include KWSYS_HEADER(stl/vector)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Directory.hxx.in"
# include "Configure.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_stl_string.hxx.in"
#endif

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
class DirectoryInternals
{
public:
  // Array of Files
  kwsys_stl::vector<kwsys_stl::string> Files;
  
  // Path to Open'ed directory
  kwsys_stl::string Path;
};

//----------------------------------------------------------------------------
Directory::Directory()
{
  this->Internal = new DirectoryInternals;
}

//----------------------------------------------------------------------------
Directory::~Directory()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
unsigned long Directory::GetNumberOfFiles() const
{
  return static_cast<unsigned long>(this->Internal->Files.size());
}

//----------------------------------------------------------------------------
const char* Directory::GetFile(unsigned long dindex) const
{
  if ( dindex >= this->Internal->Files.size() )
    {
    return 0;
    }
  return this->Internal->Files[dindex].c_str();
}

//----------------------------------------------------------------------------
const char* Directory::GetPath() const
{
  return this->Internal->Path.c_str();
}

//----------------------------------------------------------------------------
void Directory::Clear()
{
  //this->Internal->Path.clear();
  this->Internal->Path = "";
  this->Internal->Files.clear();
}

} // namespace KWSYS_NAMESPACE

// First microsoft compilers

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <windows.h>
#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace KWSYS_NAMESPACE
{

bool Directory::Load(const char* name)
{
  this->Clear();
#if _MSC_VER < 1300
  long srchHandle;
#else
  intptr_t srchHandle;
#endif
  char* buf;
  size_t n = strlen(name);
  if ( name[n - 1] == '/' ) 
    {
    buf = new char[n + 1 + 1];
    sprintf(buf, "%s*", name);
    } 
  else
    {
    buf = new char[n + 2 + 1];
    sprintf(buf, "%s/*", name);
    }
  struct _finddata_t data;      // data of current file
  
  // Now put them into the file array
  srchHandle = _findfirst(buf, &data);
  delete [] buf;
  
  if ( srchHandle == -1 )
    {
    return 0;
    }
  
  // Loop through names
  do 
    {
    this->Internal->Files.push_back(data.name);
    } 
  while ( _findnext(srchHandle, &data) != -1 );
  this->Internal->Path = name;
  return _findclose(srchHandle) != -1;
}

} // namespace KWSYS_NAMESPACE

#else

// Now the POSIX style directory access

#include <sys/types.h>
#include <dirent.h>

namespace KWSYS_NAMESPACE
{

bool Directory::Load(const char* name)
{
  this->Clear();
  DIR* dir = opendir(name);

  if (!dir)
    {
    return 0;
    }

  for (dirent* d = readdir(dir); d; d = readdir(dir) )
    {
    this->Internal->Files.push_back(d->d_name);
    }
  this->Internal->Path = name;
  closedir(dir);
  return 1;
}

} // namespace KWSYS_NAMESPACE

#endif
