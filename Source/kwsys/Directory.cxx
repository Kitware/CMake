/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <Directory.hxx>

#include <Configure.hxx>

#include <std/string>
#include <std/vector>

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
class DirectoryInternals
{
public:
  // Array of Files
  kwsys_std::vector<kwsys_std::string> Files;
  
  // Path to Open'ed directory
  kwsys_std::string Path;
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
unsigned long Directory::GetNumberOfFiles()
{
  return static_cast<unsigned long>(this->Internal->Files.size());
}

//----------------------------------------------------------------------------
const char* Directory::GetFile(unsigned long dindex)
{
  if ( dindex >= this->Internal->Files.size() )
    {
    return 0;
    }
  return this->Internal->Files[dindex].c_str();
}

} // namespace KWSYS_NAMESPACE

// First microsoft compilers

#ifdef _MSC_VER
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
  size_t srchHandle = _findfirst(buf, &data);
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
