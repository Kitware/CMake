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
#include <SystemTools.hxx>
#include <RegularExpression.hxx>
#include <Directory.hxx>

#include <fstream>

#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

// support for realpath call
#ifndef _WIN32
#include <limits.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/wait.h>
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__BORLANDC__))
#include <string.h>
#include <windows.h>
#include <direct.h>
#define _unlink unlink
inline int Mkdir(const char* dir)
{
  return _mkdir(dir);
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  return _getcwd(buf, len);
}
inline int Chdir(const char* dir)
{
  #if defined(__BORLANDC__)
  return chdir(dir);
  #else
  return _chdir(dir);
  #endif
}
#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
inline int Mkdir(const char* dir)
{
  return mkdir(dir, 00777);
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  return getcwd(buf, len);
}
inline int Chdir(const char* dir)
{
  return chdir(dir);
}
#endif


/* Implement floattime() for various platforms */
// Taken from Python 2.1.3

#if defined( _WIN32 ) && !defined( __CYGWIN__ )
#  include <sys/timeb.h>
#  define HAVE_FTIME
#  if defined( __BORLANDC__)
#    define FTIME ftime
#    define TIMEB timeb
#  else // Visual studio?
#    define FTIME _ftime
#    define TIMEB _timeb
#  endif
#elif defined( __CYGWIN__ ) || defined( __linux__ )
#  include <sys/time.h>
#  include <time.h>
#  define HAVE_GETTIMEOFDAY
#endif

namespace KWSYS_NAMESPACE
{

double
SystemTools::GetTime(void)
{
  /* There are three ways to get the time:
     (1) gettimeofday() -- resolution in microseconds
     (2) ftime() -- resolution in milliseconds
     (3) time() -- resolution in seconds
     In all cases the return value is a float in seconds.
     Since on some systems (e.g. SCO ODT 3.0) gettimeofday() may
     fail, so we fall back on ftime() or time().
     Note: clock resolution does not imply clock accuracy! */
#ifdef HAVE_GETTIMEOFDAY
  {
  struct timeval t;
#ifdef GETTIMEOFDAY_NO_TZ
  if (gettimeofday(&t) == 0)
    return (double)t.tv_sec + t.tv_usec*0.000001;
#else /* !GETTIMEOFDAY_NO_TZ */
  if (gettimeofday(&t, (struct timezone *)NULL) == 0)
    return (double)t.tv_sec + t.tv_usec*0.000001;
#endif /* !GETTIMEOFDAY_NO_TZ */
  }
#endif /* !HAVE_GETTIMEOFDAY */
  {
#if defined(HAVE_FTIME)
  struct TIMEB t;
  FTIME(&t);
  return (double)t.time + (double)t.millitm * (double)0.001;
#else /* !HAVE_FTIME */
  time_t secs;
  time(&secs);
  return (double)secs;
#endif /* !HAVE_FTIME */
  }
}

// adds the elements of the env variable path to the arg passed in
void SystemTools::GetPath(kwsys_std::vector<kwsys_std::string>& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* pathSep = ";";
#else
  const char* pathSep = ":";
#endif
  kwsys_std::string pathEnv = getenv("PATH");
  // A hack to make the below algorithm work.  
  if(pathEnv[pathEnv.length()-1] != ':')
    {
    pathEnv += pathSep;
    }
  kwsys_std::string::size_type start =0;
  bool done = false;
  while(!done)
    {
    kwsys_std::string::size_type endpos = pathEnv.find(pathSep, start);
    if(endpos != kwsys_std::string::npos)
      {
      path.push_back(pathEnv.substr(start, endpos-start));
      start = endpos+1;
      }
    else
      {
      done = true;
      }
    }
  for(kwsys_std::vector<kwsys_std::string>::iterator i = path.begin();
      i != path.end(); ++i)
    {
    SystemTools::ConvertToUnixSlashes(*i);
    }
}


const char* SystemTools::GetExecutableExtension()
{
#if defined(_WIN32) || defined(__CYGWIN__)
  return ".exe";
#else
  return "";
#endif  
}


bool SystemTools::MakeDirectory(const char* path)
{
  if(SystemTools::FileExists(path))
    {
    return true;
    }
  kwsys_std::string dir = path;
  if(dir.size() == 0)
    {
    return false;
    }
  SystemTools::ConvertToUnixSlashes(dir);

  kwsys_std::string::size_type pos = dir.find(':');
  if(pos == kwsys_std::string::npos)
    {
    pos = 0;
    }
  kwsys_std::string topdir;
  while((pos = dir.find('/', pos)) != kwsys_std::string::npos)
    {
    topdir = dir.substr(0, pos);
    Mkdir(topdir.c_str());
    pos++;
    }
  if(dir[dir.size()-1] == '/')
    {
    topdir = dir.substr(0, dir.size());
    }
  else
    {
    topdir = dir;
    }
  if(Mkdir(topdir.c_str()) != 0)
    {
    // There is a bug in the Borland Run time library which makes MKDIR
    // return EACCES when it should return EEXISTS
    // if it is some other error besides directory exists
    // then return false
    if( (errno != EEXIST) 
#ifdef __BORLANDC__
        && (errno != EACCES) 
#endif      
      )
      {
      return false;
      }
    }
  return true;
}


// replace replace with with as many times as it shows up in source.
// write the result into source.
void SystemTools::ReplaceString(kwsys_std::string& source,
                                   const char* replace,
                                   const char* with)
{
  // get out quick if string is not found
  kwsys_std::string::size_type start = source.find(replace);
  if(start ==  kwsys_std::string::npos)
    {
    return;
    }
  kwsys_std::string rest;
  kwsys_std::string::size_type lengthReplace = strlen(replace);
  kwsys_std::string::size_type lengthWith = strlen(with);
  while(start != kwsys_std::string::npos)
    {
    rest = source.substr(start+lengthReplace);
    source = source.substr(0, start);
    source += with;
    source += rest;
    start = source.find(replace, start + lengthWith );
    }
}

// Read a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will return the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will return the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::ReadRegistryValue(const char *key, kwsys_std::string &value)
{

  kwsys_std::string primary = key;
  kwsys_std::string second;
  kwsys_std::string valuename;
 
  size_t start = primary.find("\\");
  if (start == kwsys_std::string::npos)
    {
    return false;
    }

  size_t valuenamepos = primary.find(";");
  if (valuenamepos != kwsys_std::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }

  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey;
  if (primary == "HKEY_CURRENT_USER")
    {
    primaryKey = HKEY_CURRENT_USER;
    }
  if (primary == "HKEY_CURRENT_CONFIG")
    {
    primaryKey = HKEY_CURRENT_CONFIG;
    }
  if (primary == "HKEY_CLASSES_ROOT")
    {
    primaryKey = HKEY_CLASSES_ROOT;
    }
  if (primary == "HKEY_LOCAL_MACHINE")
    {
    primaryKey = HKEY_LOCAL_MACHINE;
    }
  if (primary == "HKEY_USERS")
    {
    primaryKey = HKEY_USERS;
    }
  
  HKEY hKey;
  if(RegOpenKeyEx(primaryKey, 
                  second.c_str(), 
                  0, 
                  KEY_READ, 
                  &hKey) != ERROR_SUCCESS)
    {
    return false;
    }
  else
    {
    DWORD dwType, dwSize;
    dwSize = 1023;
    char data[1024];
    if(RegQueryValueEx(hKey, 
                       (LPTSTR)valuename.c_str(), 
                       NULL, 
                       &dwType, 
                       (BYTE *)data, 
                       &dwSize) == ERROR_SUCCESS)
      {
      if (dwType == REG_SZ)
        {
        value = data;
        return true;
        }
      }
    }
  return false;
}
#else
bool SystemTools::ReadRegistryValue(const char *, kwsys_std::string &)
{
  return false;
}
#endif


// Write a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will set the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will set the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::WriteRegistryValue(const char *key, const char *value)
{
  kwsys_std::string primary = key;
  kwsys_std::string second;
  kwsys_std::string valuename;
 
  size_t start = primary.find("\\");
  if (start == kwsys_std::string::npos)
    {
    return false;
    }

  size_t valuenamepos = primary.find(";");
  if (valuenamepos != kwsys_std::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }

  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey;
  if (primary == "HKEY_CURRENT_USER")
    {
    primaryKey = HKEY_CURRENT_USER;
    }
  if (primary == "HKEY_CURRENT_CONFIG")
    {
    primaryKey = HKEY_CURRENT_CONFIG;
    }
  if (primary == "HKEY_CLASSES_ROOT")
    {
    primaryKey = HKEY_CLASSES_ROOT;
    }
  if (primary == "HKEY_LOCAL_MACHINE")
    {
    primaryKey = HKEY_LOCAL_MACHINE;
    }
  if (primary == "HKEY_USERS")
    {
    primaryKey = HKEY_USERS;
    }
  
  HKEY hKey;
  DWORD dwDummy;
  if(RegCreateKeyEx(primaryKey, 
                    second.c_str(), 
                    0, 
                    "",
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hKey,
                    &dwDummy) != ERROR_SUCCESS)
    {
    return false;
    }

  if(RegSetValueEx(hKey, 
                   (LPTSTR)valuename.c_str(), 
                   0, 
                   REG_SZ, 
                   (CONST BYTE *)value, 
                   (DWORD)(strlen(value) + 1)) == ERROR_SUCCESS)
    {
    return true;
    }
  return false;
}
#else
bool SystemTools::WriteRegistryValue(const char *, const char *)
{
  return false;
}
#endif

// Delete a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will delete the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will delete  the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::DeleteRegistryValue(const char *key)
{
  kwsys_std::string primary = key;
  kwsys_std::string second;
  kwsys_std::string valuename;
 
  size_t start = primary.find("\\");
  if (start == kwsys_std::string::npos)
    {
    return false;
    }

  size_t valuenamepos = primary.find(";");
  if (valuenamepos != kwsys_std::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }

  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey;
  if (primary == "HKEY_CURRENT_USER")
    {
    primaryKey = HKEY_CURRENT_USER;
    }
  if (primary == "HKEY_CURRENT_CONFIG")
    {
    primaryKey = HKEY_CURRENT_CONFIG;
    }
  if (primary == "HKEY_CLASSES_ROOT")
    {
    primaryKey = HKEY_CLASSES_ROOT;
    }
  if (primary == "HKEY_LOCAL_MACHINE")
    {
    primaryKey = HKEY_LOCAL_MACHINE;
    }
  if (primary == "HKEY_USERS")
    {
    primaryKey = HKEY_USERS;
    }
  
  HKEY hKey;
  if(RegOpenKeyEx(primaryKey, 
                  second.c_str(), 
                  0, 
                  KEY_WRITE, 
                  &hKey) != ERROR_SUCCESS)
    {
    return false;
    }
  else
    {
    if(RegDeleteValue(hKey, 
                      (LPTSTR)valuename.c_str()) == ERROR_SUCCESS)
      {
      return true;
      }
    }
  return false;
}
#else
bool SystemTools::DeleteRegistryValue(const char *)
{
  return false;
}
#endif

// replace replace with with as many times as it shows up in source.
// write the result into source.
#if defined(_WIN32) && !defined(__CYGWIN__)
void SystemTools::ExpandRegistryValues(kwsys_std::string& source)
{
  // Regular expression to match anything inside [...] that begins in HKEY.
  // Note that there is a special rule for regular expressions to match a
  // close square-bracket inside a list delimited by square brackets.
  // The "[^]]" part of this expression will match any character except
  // a close square-bracket.  The ']' character must be the first in the
  // list of characters inside the [^...] block of the expression.
  cmRegularExpression regEntry("\\[(HKEY[^]]*)\\]");
  
  // check for black line or comment
  while (regEntry.find(source))
    {
    // the arguments are the second match
    kwsys_std::string key = regEntry.match(1);
    kwsys_std::string val;
    if (ReadRegistryValue(key.c_str(), val))
      {
      kwsys_std::string reg = "[";
      reg += key + "]";
      SystemTools::ReplaceString(source, reg.c_str(), val.c_str());
      }
    else
      {
      kwsys_std::string reg = "[";
      reg += key + "]";
      SystemTools::ReplaceString(source, reg.c_str(), "/registry");
      }
    }
}
#else
void SystemTools::ExpandRegistryValues(kwsys_std::string&)
{
}
#endif  


kwsys_std::string SystemTools::EscapeQuotes(const char* str)
{
  kwsys_std::string result = "";
  for(const char* ch = str; *ch != '\0'; ++ch)
    {
    if(*ch == '"')
      {
      result += '\\';
      }
    result += *ch;
    }
  return result;
}

bool SystemTools::SameFile(const char* file1, const char* file2)
{
#ifdef _WIN32
  HANDLE hFile1, hFile2;

  hFile1 = CreateFile( file1, 
                      GENERIC_READ, 
                      FILE_SHARE_READ ,
                      NULL,
                      OPEN_EXISTING,
                      FILE_FLAG_BACKUP_SEMANTICS,
                      NULL
    );
  hFile2 = CreateFile( file2, 
                      GENERIC_READ, 
                      FILE_SHARE_READ, 
                      NULL,
                      OPEN_EXISTING,
                      FILE_FLAG_BACKUP_SEMANTICS,
                      NULL
    );
  if( hFile1 == INVALID_HANDLE_VALUE || hFile2 == INVALID_HANDLE_VALUE)
    {
    if(hFile1 != INVALID_HANDLE_VALUE)
      {
      CloseHandle(hFile1);
      }
    if(hFile2 != INVALID_HANDLE_VALUE)
      {
      CloseHandle(hFile2);
      }
    return false;
    }

   BY_HANDLE_FILE_INFORMATION fiBuf1;
   BY_HANDLE_FILE_INFORMATION fiBuf2;
   GetFileInformationByHandle( hFile1, &fiBuf1 );
   GetFileInformationByHandle( hFile2, &fiBuf2 );
   CloseHandle(hFile1);
   CloseHandle(hFile2);
   return (fiBuf1.nFileIndexHigh == fiBuf2.nFileIndexHigh &&
           fiBuf1.nFileIndexLow == fiBuf2.nFileIndexLow);
#else
  struct stat fileStat1, fileStat2;
  if (stat(file1, &fileStat1) == 0 && stat(file2, &fileStat2) == 0)
    {
    // see if the files are the same file
    // check the device inode and size
    if(memcmp(&fileStat2.st_dev, &fileStat1.st_dev, sizeof(fileStat1.st_dev)) == 0 && 
       memcmp(&fileStat2.st_ino, &fileStat1.st_ino, sizeof(fileStat1.st_ino)) == 0 &&
       fileStat2.st_size == fileStat1.st_size 
      ) 
      {
      return true;
      }
    }
  return false;
#endif
}


// return true if the file exists
bool SystemTools::FileExists(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0) 
    {
    return false;
    }
  else
    {
    return true;
    }
}


// Return a capitalized string (i.e the first letter is uppercased, all other
// are lowercased)
kwsys_std::string SystemTools::Capitalized(const kwsys_std::string& s)
{
  kwsys_std::string n;
  n.resize(s.size());
  n[0] = toupper(s[0]);
  for (size_t i = 1; i < s.size(); i++)
    {
    n[i] = tolower(s[i]);
    }
  return n;
}


// Return a lower case string 
kwsys_std::string SystemTools::LowerCase(const kwsys_std::string& s)
{
  kwsys_std::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    {
    n[i] = tolower(s[i]);
    }
  return n;
}

// Return a lower case string 
kwsys_std::string SystemTools::UpperCase(const kwsys_std::string& s)
{
  kwsys_std::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    {
    n[i] = toupper(s[i]);
    }
  return n;
}


// convert windows slashes to unix slashes 
void SystemTools::ConvertToUnixSlashes(kwsys_std::string& path)
{
  kwsys_std::string::size_type pos = 0;
  while((pos = path.find('\\', pos)) != kwsys_std::string::npos)
    {
    path[pos] = '/';
    pos++;
    }
  // Remove all // from the path just like most unix shells
  int start_find = 0;

#ifdef _WIN32
  // However, on windows if the first characters are both slashes,
  // then keep them that way, so that network paths can be handled.
  start_find = 1;
#endif

  while((pos = path.find("//", start_find)) != kwsys_std::string::npos)
    {
    SystemTools::ReplaceString(path, "//", "/");
    }
  
  // remove any trailing slash
  if(path.size() && path[path.size()-1] == '/')
    {
    path = path.substr(0, path.size()-1);
    }

  // if there is a tilda ~ then replace it with HOME
  if(path.find("~") == 0)
    {
    if (getenv("HOME"))
      {
      path = kwsys_std::string(getenv("HOME")) + path.substr(1);
      }
    }
  
  // if there is a /tmp_mnt in a path get rid of it!
  // stupid sgi's 
  if(path.find("/tmp_mnt") == 0)
    {
    path = path.substr(8);
    }
}


// change // to /, and escape any spaces in the path
kwsys_std::string SystemTools::ConvertToUnixOutputPath(const char* path)
{
  kwsys_std::string ret = path;
  
  // remove // except at the beginning might be a cygwin drive
  kwsys_std::string::size_type pos = 1;
  while((pos = ret.find("//", pos)) != kwsys_std::string::npos)
    {
    ret.erase(pos, 1);
    }
  // now escape spaces if there is a space in the path
  if(ret.find(" ") != kwsys_std::string::npos)
    {
    kwsys_std::string result = "";
    char lastch = 1;
    for(const char* ch = ret.c_str(); *ch != '\0'; ++ch)
      {
        // if it is already escaped then don't try to escape it again
      if(*ch == ' ' && lastch != '\\')
        {
        result += '\\';
        }
      result += *ch;
      lastch = *ch;
      }
    ret = result;
    }
  return ret;
}



kwsys_std::string SystemTools::EscapeSpaces(const char* str)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  kwsys_std::string result;
  
  // if there are spaces
  kwsys_std::string temp = str;
  if (temp.find(" ") != kwsys_std::string::npos && 
      temp.find("\"")==kwsys_std::string::npos)
    {
    result = "\"";
    result += str;
    result += "\"";
    return result;
    }
  return str;
#else
  kwsys_std::string result = "";
  for(const char* ch = str; *ch != '\0'; ++ch)
    {
    if(*ch == ' ')
      {
      result += '\\';
      }
    result += *ch;
    }
  return result;
#endif
}

kwsys_std::string SystemTools::ConvertToOutputPath(const char* path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  return SystemTools::ConvertToWindowsOutputPath(path);
#else
  return SystemTools::ConvertToUnixOutputPath(path);
#endif
}


// remove double slashes not at the start
kwsys_std::string SystemTools::ConvertToWindowsOutputPath(const char* path)
{  
  kwsys_std::string ret = path;
  kwsys_std::string::size_type pos = 0;
  // first convert all of the slashes
  while((pos = ret.find('/', pos)) != kwsys_std::string::npos)
    {
    ret[pos] = '\\';
    pos++;
    }
  // check for really small paths
  if(ret.size() < 2)
    {
    return ret;
    }
  // now clean up a bit and remove double slashes
  // Only if it is not the first position in the path which is a network
  // path on windows
  pos = 1; // start at position 1
  if(ret[0] == '\"')
    {
    pos = 2;  // if the string is already quoted then start at 2
    if(ret.size() < 3)
      {
      return ret;
      }
    }
  while((pos = ret.find("\\\\", pos)) != kwsys_std::string::npos)
    {
    ret.erase(pos, 1);
    }
  // now double quote the path if it has spaces in it
  // and is not already double quoted
  if(ret.find(" ") != kwsys_std::string::npos
     && ret[0] != '\"')
    {
    kwsys_std::string result;
    result = "\"" + ret + "\"";
    ret = result;
    }
  return ret;
}

bool SystemTools::CopyFileIfDifferent(const char* source,
                                        const char* destination)
{
  if(SystemTools::FilesDiffer(source, destination))
    {
    SystemTools::CopyFileAlways(source, destination);
    return true;
    }
  return false;
}

  
bool SystemTools::FilesDiffer(const char* source,
                                const char* destination)
{
  struct stat statSource;
  if (stat(source, &statSource) != 0) 
    {
    return true;
    }

  struct stat statDestination;
  if (stat(destination, &statDestination) != 0) 
    {
    return true;
    }

  if(statSource.st_size != statDestination.st_size)
    {
    return true;
    }

  if(statSource.st_size == 0)
    {
    return false;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
  kwsys_std::ifstream finSource(source, 
                          kwsys_std::ios::binary | kwsys_std::ios::in);
  kwsys_std::ifstream finDestination(destination, 
                               kwsys_std::ios::binary | kwsys_std::ios::in);
#else
  kwsys_std::ifstream finSource(source);
  kwsys_std::ifstream finDestination(destination);
#endif
  if(!finSource || !finDestination)
    {
    return true;
    }

  char* source_buf = new char[statSource.st_size];
  char* dest_buf = new char[statSource.st_size];

  finSource.read(source_buf, statSource.st_size);
  finDestination.read(dest_buf, statSource.st_size);

  if(statSource.st_size != static_cast<long>(finSource.gcount()) ||
     statSource.st_size != static_cast<long>(finDestination.gcount()))
    {
    // Failed to read files.
    delete [] source_buf;
    delete [] dest_buf;
    return true;
    }
  int ret = memcmp((const void*)source_buf, 
                   (const void*)dest_buf, 
                   statSource.st_size);

  delete [] dest_buf;
  delete [] source_buf;

  return ret != 0;
}


/**
 * Copy a file named by "source" to the file named by "destination".
 */
bool SystemTools::CopyFileAlways(const char* source, const char* destination)
{
  const int bufferSize = 4096;
  char buffer[bufferSize];

  // If destination is a directory, try to create a file with the same
  // name as the source in that directory.

  kwsys_std::string new_destination;
  if(SystemTools::FileExists(destination) &&
     SystemTools::FileIsDirectory(destination))
    {
    new_destination = destination;
    SystemTools::ConvertToUnixSlashes(new_destination);
    new_destination += '/';
    kwsys_std::string source_name = source;
    new_destination += SystemTools::GetFilenameName(source_name);
    destination = new_destination.c_str();
    }

  // Create destination directory

  kwsys_std::string destination_dir = destination;
  destination_dir = SystemTools::GetFilenamePath(destination_dir);
  SystemTools::MakeDirectory(destination_dir.c_str());

  // Open files

#if defined(_WIN32) || defined(__CYGWIN__)
  kwsys_std::ifstream fin(source, 
                    kwsys_std::ios::binary | kwsys_std::ios::in);
#else
  kwsys_std::ifstream fin(source);
#endif
  if(!fin)
    {
    return false;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
  kwsys_std::ofstream fout(destination, 
                     kwsys_std::ios::binary | kwsys_std::ios::out | kwsys_std::ios::trunc);
#else
  kwsys_std::ofstream fout(destination, 
                     kwsys_std::ios::out | kwsys_std::ios::trunc);
#endif
  if(!fout)
    {
    return false;
    }
  
  // This copy loop is very sensitive on certain platforms with
  // slightly broken stream libraries (like HPUX).  Normally, it is
  // incorrect to not check the error condition on the fin.read()
  // before using the data, but the fin.gcount() will be zero if an
  // error occurred.  Therefore, the loop should be safe everywhere.
  while(fin)
    {
    fin.read(buffer, bufferSize);
    if(fin.gcount())
      {
      fout.write(buffer, fin.gcount());
      }
    }
  
  // Make sure the operating system has finished writing the file
  // before closing it.  This will ensure the file is finished before
  // the check below.
  fout.flush();
  
  fin.close();
  fout.close();

  // More checks.
  struct stat statSource, statDestination;
  statSource.st_size = 12345;
  statDestination.st_size = 12345;
  if(stat(source, &statSource) != 0)
    {
    return false;
    }
  else if(stat(destination, &statDestination) != 0)
    {
    return false;
    }
  else if(statSource.st_size != statDestination.st_size)
    {
    return false;
    }
  return true;
}

// return true if the file exists
long int SystemTools::ModifiedTime(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0) 
    {
    return 0;
    }
  else
    {
    return (long int)fs.st_mtime;
    }
}


kwsys_std::string SystemTools::GetLastSystemError()
{
  int e = errno;
  return strerror(e);
}

bool SystemTools::RemoveFile(const char* source)
{
  return unlink(source) != 0 ? false : true;
}

/**
 * Find the file the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the file if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_std::string SystemTools::FindFile(const char* name, 
                                       const kwsys_std::vector<kwsys_std::string>& userPaths)
{
  // Add the system search path to our path.
  kwsys_std::vector<kwsys_std::string> path = userPaths;
  SystemTools::GetPath(path);

  kwsys_std::string tryPath;
  for(kwsys_std::vector<kwsys_std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    if(SystemTools::FileExists(tryPath.c_str()) &&
      !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    }
  // Couldn't find the file.
  return "";
}

/**
 * Find the executable with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the executable if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_std::string SystemTools::FindProgram(const char* name,
                                       const kwsys_std::vector<kwsys_std::string>& userPaths,
                                       bool no_system_path)
{
  if(!name)
    {
    return "";
    }
  // See if the executable exists as written.
  if(SystemTools::FileExists(name) &&
      !SystemTools::FileIsDirectory(name))
    {
    return SystemTools::CollapseFullPath(name);
    }
  kwsys_std::string tryPath = name;
  tryPath += SystemTools::GetExecutableExtension();
  if(SystemTools::FileExists(tryPath.c_str()) &&
     !SystemTools::FileIsDirectory(tryPath.c_str()))
    {
    return SystemTools::CollapseFullPath(tryPath.c_str());
    }

  // Add the system search path to our path.
  kwsys_std::vector<kwsys_std::string> path = userPaths;
  if (!no_system_path)
    {
    SystemTools::GetPath(path);
    }

  for(kwsys_std::vector<kwsys_std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    if(SystemTools::FileExists(tryPath.c_str()) &&
      !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
#ifdef _WIN32
    tryPath += ".com";
    if(SystemTools::FileExists(tryPath.c_str()) &&
       !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
#endif
    tryPath += SystemTools::GetExecutableExtension();
    if(SystemTools::FileExists(tryPath.c_str()) &&
       !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    }

  // Couldn't find the program.
  return "";
}


/**
 * Find the library with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the library if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_std::string SystemTools::FindLibrary(const char* name,
                                       const kwsys_std::vector<kwsys_std::string>& userPaths)
{
  // See if the executable exists as written.
  if(SystemTools::FileExists(name) &&
     !SystemTools::FileIsDirectory(name))
    {
    return SystemTools::CollapseFullPath(name);
    }
  
  // Add the system search path to our path.
  kwsys_std::vector<kwsys_std::string> path = userPaths;
  SystemTools::GetPath(path);
  
  kwsys_std::string tryPath;
  for(kwsys_std::vector<kwsys_std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    tryPath += ".lib";
    if(SystemTools::FileExists(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
#else
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".so";
    if(SystemTools::FileExists(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".a";
    if(SystemTools::FileExists(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".sl";
    if(SystemTools::FileExists(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".dylib";
    if(SystemTools::FileExists(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
#endif
    }
  
  // Couldn't find the library.
  return "";
}

bool SystemTools::FileIsDirectory(const char* name)
{  
  struct stat fs;
  if(stat(name, &fs) == 0)
    {
#if _WIN32
    return ((fs.st_mode & _S_IFDIR) != 0);
#else
    return S_ISDIR(fs.st_mode);
#endif
    }
  else
    {
    return false;
    }
}

int SystemTools::ChangeDirectory(const char *dir)
{
  return Chdir(dir);
}

kwsys_std::string SystemTools::GetCurrentWorkingDirectory()
{
  char buf[2048];
  kwsys_std::string path = Getcwd(buf, 2048);
  return path;
}

kwsys_std::string SystemTools::GetProgramPath(const char* in_name)
{
  kwsys_std::string dir, file;
  SystemTools::SplitProgramPath(in_name, dir, file);
  return dir;
}

bool SystemTools::SplitProgramPath(const char* in_name,
                                   kwsys_std::string& dir,
                                   kwsys_std::string& file,
                                   bool errorReport)
{
  dir = in_name;
  file = "";
  SystemTools::ConvertToUnixSlashes(dir);
  
  if(!SystemTools::FileIsDirectory(dir.c_str()))
    {
    kwsys_std::string::size_type slashPos = dir.rfind("/");
    if(slashPos != kwsys_std::string::npos)
      {
      file = dir.substr(slashPos+1);
      dir = dir.substr(0, slashPos);
      }
    else
      {
      file = dir;
      dir = "";
      }
    }
  if((dir != "") && !SystemTools::FileIsDirectory(dir.c_str()))
    {
    kwsys_std::string oldDir = in_name;
    SystemTools::ConvertToUnixSlashes(oldDir);
    dir = in_name;
    return false;
    }
  return true;
}

kwsys_std::string SystemTools::CollapseFullPath(const char* in_relative)
{
  return SystemTools::CollapseFullPath(in_relative, 0);
}

kwsys_std::string SystemTools::CollapseFullPath(const char* in_relative,
                                            const char* in_base)
{
  kwsys_std::string dir, file;
  SystemTools::SplitProgramPath(in_relative, dir, file, false);
  
  // Save original working directory.
  kwsys_std::string orig = SystemTools::GetCurrentWorkingDirectory();
  
  // Change to base of relative path.
  if(in_base)
    {
    Chdir(in_base);
    }
  
#ifdef _WIN32
  // Follow relative path.
  if(dir != "")
    {
    Chdir(dir.c_str());
    }
  
  // Get the resulting directory.
  kwsys_std::string newDir = SystemTools::GetCurrentWorkingDirectory();
  
  // Add the file back on to the directory.
  SystemTools::ConvertToUnixSlashes(newDir);
#else
# ifdef MAXPATHLEN
  char resolved_name[MAXPATHLEN];
# else
#  ifdef PATH_MAX
  char resolved_name[PATH_MAX];
#  else
  char resolved_name[5024];
#  endif
# endif
  
  // Resolve relative path.
  kwsys_std::string newDir;
  if(dir != "")
    {
    realpath(dir.c_str(), resolved_name);
    newDir = resolved_name;
    }
  else
    {
    newDir = SystemTools::GetCurrentWorkingDirectory();
    }
#endif
  
  // Restore original working directory.
  Chdir(orig.c_str());
  
  // Construct and return the full path.
  kwsys_std::string newPath = newDir;
  if(file != "")
    {
    newPath += "/";
    newPath += file;
    }
  return newPath;
}

bool SystemTools::Split(const char* str, kwsys_std::vector<kwsys_std::string>& lines)
{
  kwsys_std::string data(str);
  kwsys_std::string::size_type lpos = 0;
  while(lpos < data.length())
    {
    kwsys_std::string::size_type rpos = data.find_first_of("\n", lpos);
    if(rpos == kwsys_std::string::npos)
      {
      // Line ends at end of string without a newline.
      lines.push_back(data.substr(lpos));
      return false;
      }
    if((rpos > lpos) && (data[rpos-1] == '\r'))
      {
      // Line ends in a "\r\n" pair, remove both characters.
      lines.push_back(data.substr(lpos, (rpos-1)-lpos));
      }
    else
      {
      // Line ends in a "\n", remove the character.
      lines.push_back(data.substr(lpos, rpos-lpos));
      }
    lpos = rpos+1;
    }
  return true;
}

/**
 * Return path of a full filename (no trailing slashes).
 * Warning: returned path is converted to Unix slashes format.
 */
kwsys_std::string SystemTools::GetFilenamePath(const kwsys_std::string& filename)
{
  kwsys_std::string fn = filename;
  SystemTools::ConvertToUnixSlashes(fn);
  
  kwsys_std::string::size_type slash_pos = fn.rfind("/");
  if(slash_pos != kwsys_std::string::npos)
    {
    return fn.substr(0, slash_pos);
    }
  else
    {
    return "";
    }
}


/**
 * Return file name of a full filename (i.e. file name without path).
 */
kwsys_std::string SystemTools::GetFilenameName(const kwsys_std::string& filename)
{
  kwsys_std::string fn = filename;
  SystemTools::ConvertToUnixSlashes(fn);
  
  kwsys_std::string::size_type slash_pos = fn.rfind("/");
  if(slash_pos != kwsys_std::string::npos)
    {
    return fn.substr(slash_pos + 1);
    }
  else
    {
    return filename;
    }
}


/**
 * Return file extension of a full filename (dot included).
 * Warning: this is the longest extension (for example: .tar.gz)
 */
kwsys_std::string SystemTools::GetFilenameExtension(const kwsys_std::string& filename)
{
  kwsys_std::string name = SystemTools::GetFilenameName(filename);
  kwsys_std::string::size_type dot_pos = name.find(".");
  if(dot_pos != kwsys_std::string::npos)
    {
    return name.substr(dot_pos);
    }
  else
    {
    return "";
    }
}


/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the longest extension (for example: .tar.gz)
 */
kwsys_std::string SystemTools::GetFilenameWithoutExtension(const kwsys_std::string& filename)
{
  kwsys_std::string name = SystemTools::GetFilenameName(filename);
  kwsys_std::string::size_type dot_pos = name.find(".");
  if(dot_pos != kwsys_std::string::npos)
    {
    return name.substr(0, dot_pos);
    }
  else
    {
    return name;
    }
}


/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the last extension (for example: removes .gz
 * from .tar.gz)
 */
kwsys_std::string
SystemTools::GetFilenameWithoutLastExtension(const kwsys_std::string& filename)
{
  kwsys_std::string name = SystemTools::GetFilenameName(filename);
  kwsys_std::string::size_type dot_pos = name.rfind(".");
  if(dot_pos != kwsys_std::string::npos)
    {
    return name.substr(0, dot_pos);
    }
  else
    {
    return name;
    }
}

bool SystemTools::FileIsFullPath(const char* in_name)
{
  kwsys_std::string name = in_name;
#if defined(_WIN32)
  // On Windows, the name must be at least two characters long.
  if(name.length() < 2)
    {
    return false;
    }
  if(name[1] == ':')
    {
    return true;
    }
  if(name[0] == '\\')
    {
    return true;
    }
#else
  // On UNIX, the name must be at least one character long.
  if(name.length() < 1)
    {
    return false;
    }
#endif
  // On UNIX, the name must begin in a '/'.
  // On Windows, if the name begins in a '/', then it is a full
  // network path.
  if(name[0] == '/')
    {
    return true;
    }
  return false;
}

void SystemTools::Glob(const char *directory, const char *regexp,
                         kwsys_std::vector<kwsys_std::string>& files)
{
  Directory d;
  RegularExpression reg(regexp);
  
  if (d.Load(directory))
    {
    size_t numf;
        unsigned int i;
    numf = d.GetNumberOfFiles();
    for (i = 0; i < numf; i++)
      {
      kwsys_std::string fname = d.GetFile(i);
      if (reg.find(fname))
        {
        files.push_back(fname);
        }
      }
    }
}


void SystemTools::GlobDirs(const char *fullPath,
                             kwsys_std::vector<kwsys_std::string>& files)
{
  kwsys_std::string path = fullPath;
  kwsys_std::string::size_type pos = path.find("/*");
  if(pos == kwsys_std::string::npos)
    {
    files.push_back(fullPath);
    return;
    }
  kwsys_std::string startPath = path.substr(0, pos);
  kwsys_std::string finishPath = path.substr(pos+2);

  Directory d;
  if (d.Load(startPath.c_str()))
    {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i)
      {
      if((kwsys_std::string(d.GetFile(i)) != ".")
         && (kwsys_std::string(d.GetFile(i)) != ".."))
        {
        kwsys_std::string fname = startPath;
        fname +="/";
        fname += d.GetFile(i);
        if(SystemTools::FileIsDirectory(fname.c_str()))
          {
          fname += finishPath;
          SystemTools::GlobDirs(fname.c_str(), files);
          }
        }
      }
    }
}

bool SystemTools::GetShortPath(const char* path, kwsys_std::string& shortPath)
{
#if defined(WIN32) && !defined(__CYGWIN__)  
  const int size = int(strlen(path)) +1; // size of return
  char *buffer = new char[size];  // create a buffer
  char *tempPath = new char[size];  // create a buffer
  int ret;
  
  // if the path passed in has quotes around it, first remove the quotes
  if (path[0] == '"' && path[strlen(path)-1] == '"')
    {
    strcpy(tempPath,path+1);
    tempPath[strlen(tempPath)-1] = '\0';
    }
  else
    {
    strcpy(tempPath,path);
    }
  
  buffer[0] = 0;
  ret = GetShortPathName(tempPath, buffer, size);

  if(buffer[0] == 0 || ret > size)
    {
    delete [] buffer;
    delete [] tempPath;
    return false;
    }
  else
    {
    shortPath = buffer;
    delete [] buffer;
    delete [] tempPath;
    return true;
    }
#else
  shortPath = path;
  return true;
#endif
}

bool SystemTools::SimpleGlob(const kwsys_std::string& glob, 
                               kwsys_std::vector<kwsys_std::string>& files, 
                               int type /* = 0 */)
{
  files.clear();
  if ( glob[glob.size()-1] != '*' )
    {
    return false;
    }
  kwsys_std::string path = SystemTools::GetFilenamePath(glob);
  kwsys_std::string ppath = SystemTools::GetFilenameName(glob);
  ppath = ppath.substr(0, ppath.size()-1);
  if ( path.size() == 0 )
    {
    path = "/";
    }

  bool res = false;
  Directory d;
  if (d.Load(path.c_str()))
    {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i)
      {
      if((kwsys_std::string(d.GetFile(i)) != ".")
         && (kwsys_std::string(d.GetFile(i)) != ".."))
        {
        kwsys_std::string fname = path;
        if ( path[path.size()-1] != '/' )
          {
          fname +="/";
          }
        fname += d.GetFile(i);
        kwsys_std::string sfname = d.GetFile(i);
        if ( type > 0 && SystemTools::FileIsDirectory(fname.c_str()) )
          {
          continue;
          }
        if ( type < 0 && !SystemTools::FileIsDirectory(fname.c_str()) )
          {
          continue;
          }
        if ( sfname.size() >= ppath.size() && 
             sfname.substr(0, ppath.size()) == 
             ppath )
          {
          files.push_back(fname);
          res = true;
          }
        }
      }
    }
  return res;
}


void SystemTools::SplitProgramFromArgs(const char* path, 
                                         kwsys_std::string& program, kwsys_std::string& args)
{
  if(SystemTools::FileExists(path))
    {
    program = path;
    args = "";
    return;
    }
  kwsys_std::vector<kwsys_std::string> e;
  kwsys_std::string findProg = SystemTools::FindProgram(path, e);
  if(findProg.size())
    {
    program = findProg;
    args = "";
    return;
    }
  kwsys_std::string dir = path;
  kwsys_std::string::size_type spacePos = dir.rfind(' ');
  if(spacePos == kwsys_std::string::npos)
    {
    program = "";
    args = "";
    return;
    }
  while(spacePos != kwsys_std::string::npos)
    {
    kwsys_std::string tryProg = dir.substr(0, spacePos);
    if(SystemTools::FileExists(tryProg.c_str()))
      {
      program = tryProg;
      args = dir.substr(spacePos, dir.size()-spacePos);
      return;
      } 
    findProg = SystemTools::FindProgram(tryProg.c_str(), e);
    if(findProg.size())
      {
      program = findProg;
      args = dir.substr(spacePos, dir.size()-spacePos);
      return;
      }
    spacePos = dir.rfind(' ', spacePos--);
    }
  program = "";
  args = "";
}

kwsys_std::string SystemTools::GetCurrentDateTime(const char* format)
{
  char buf[1024];
  time_t t;
  time(&t);
  strftime(buf, sizeof(buf), format, localtime(&t));
  return buf;
}

kwsys_std::string SystemTools::MakeCindentifier(const char* s)
{
  kwsys_std::string str(s);
  if (str.find_first_of("0123456789") == 0)
    {
    str = "_" + str;
    }

  kwsys_std::string permited_chars("_"
                             "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "0123456789");
  kwsys_std::string::size_type pos = 0;
  while ((pos = str.find_first_not_of(permited_chars, pos)) != kwsys_std::string::npos)
    {
    str[pos] = '_';
    }
  return str;
}

// Due to a buggy stream library on the HP and another on Mac OSX, we
// need this very carefully written version of getline.  Returns true
// if any data were read before the end-of-file was reached.
bool SystemTools::GetLineFromStream(kwsys_std::istream& is, kwsys_std::string& line)
{
  const int bufferSize = 1024;
  char buffer[bufferSize];
  line = "";
  bool haveData = false;

  // If no characters are read from the stream, the end of file has
  // been reached.
  while((is.getline(buffer, bufferSize), is.gcount() > 0))
    {
    haveData = true;
    line.append(buffer);

    // If newline character was read, the gcount includes the
    // character, but the buffer does not.  The end of line has been
    // reached.
    if(strlen(buffer) < static_cast<size_t>(is.gcount()))
      {
      break;
      }

    // The fail bit may be set.  Clear it.
    is.clear(is.rdstate() & ~kwsys_std::ios::failbit);
    }
  return haveData;
}

#if defined(_MSC_VER) && defined(_DEBUG)
# include <crtdbg.h>
# include <stdio.h>
# include <stdlib.h>
static int SystemToolsDebugReport(int, char* message, int*)
{
  fprintf(stderr, message);
  exit(1);
  return 0;
}
void SystemTools::EnableMSVCDebugHook()
{
  if(getenv("DART_TEST_FROM_DART"))
    {
    _CrtSetReportHook(SystemToolsDebugReport);
    }
}
#else
void SystemTools::EnableMSVCDebugHook()
{
}
#endif

} // namespace KWSYS_NAMESPACE
