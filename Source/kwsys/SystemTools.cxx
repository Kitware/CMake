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
#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(Directory.hxx)

#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(ios/fstream)
#include KWSYS_HEADER(ios/sstream)

#ifdef _MSC_VER
# pragma warning (disable: 4786)
#endif

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
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__))
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#define _unlink unlink
inline int Mkdir(const char* dir)
{
  return _mkdir(dir);
}
inline int Rmdir(const char* dir)
{
  return _rmdir(dir);
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
inline void Realpath(const char *path, kwsys_stl::string & resolved_path)
{
  char *ptemp;
  char fullpath[MAX_PATH];
  if( GetFullPathName(path, sizeof(fullpath), fullpath, &ptemp) )
    {
    resolved_path = fullpath;
    KWSYS_NAMESPACE::SystemTools::ConvertToUnixSlashes(resolved_path);
    }
}
#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
inline int Mkdir(const char* dir)
{
  return mkdir(dir, 00777);
}
inline int Rmdir(const char* dir)
{
  return rmdir(dir);
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  return getcwd(buf, len);
}
inline int Chdir(const char* dir)
{
  return chdir(dir);
}
inline void Realpath(const char *path, kwsys_stl::string & resolved_path)
{
# ifdef MAXPATHLEN
  char resolved_name[MAXPATHLEN];
# else
#  ifdef PATH_MAX
  char resolved_name[PATH_MAX];
#  else
  char resolved_name[5024];
#  endif  //PATH_MAX
# endif //MAXPATHLEN

  realpath(path, resolved_name);
  resolved_path = resolved_name;
}
#endif

#if !defined(_WIN32) && defined(__COMO__)
// Hack for como strict mode to avoid defining _SVID_SOURCE or _BSD_SOURCE.
extern "C"
{
extern FILE *popen (__const char *__command, __const char *__modes) __THROW;
extern int pclose (FILE *__stream) __THROW;
extern char *realpath (__const char *__restrict __name,
                       char *__restrict __resolved) __THROW;
extern char *strdup (__const char *__s) __THROW;
extern int putenv (char *__string) __THROW;
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

class SystemToolsTranslationMap : 
    public kwsys_stl::map<kwsys_stl::string,kwsys_stl::string>
{
};

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
  ::FTIME(&t);
  return (double)t.time + (double)t.millitm * (double)0.001;
#else /* !HAVE_FTIME */
  time_t secs;
  time(&secs);
  return (double)secs;
#endif /* !HAVE_FTIME */
  }
}

// adds the elements of the env variable path to the arg passed in
void SystemTools::GetPath(kwsys_stl::vector<kwsys_stl::string>& path, const char* env)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* pathSep = ";";
#else
  const char* pathSep = ":";
#endif
  if(!env)
    {
    env = "PATH";
    }
  const char* cpathEnv = SystemTools::GetEnv(env);
  if ( !cpathEnv )
    {
    return;
    }

  kwsys_stl::string pathEnv = cpathEnv;
   
  // A hack to make the below algorithm work.  
  if(pathEnv[pathEnv.length()-1] != ':')
    {
    pathEnv += pathSep;
    }
  kwsys_stl::string::size_type start =0;
  bool done = false;
  while(!done)
    {
    kwsys_stl::string::size_type endpos = pathEnv.find(pathSep, start);
    if(endpos != kwsys_stl::string::npos)
      {
      path.push_back(pathEnv.substr(start, endpos-start));
      start = endpos+1;
      }
    else
      {
      done = true;
      }
    }
  for(kwsys_stl::vector<kwsys_stl::string>::iterator i = path.begin();
      i != path.end(); ++i)
    {
    SystemTools::ConvertToUnixSlashes(*i);
    }
}

const char* SystemTools::GetEnv(const char* key)
{
  return getenv(key);
}

bool SystemTools::GetEnv(const char* key, kwsys_stl::string& result)
{
  const char* v = getenv(key);
  if(v)
    {
    result = v;
    return true;
    }
  else
    {
    return false;
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
  kwsys_stl::string dir = path;
  if(dir.size() == 0)
    {
    return false;
    }
  SystemTools::ConvertToUnixSlashes(dir);

  kwsys_stl::string::size_type pos = dir.find(':');
  if(pos == kwsys_stl::string::npos)
    {
    pos = 0;
    }
  kwsys_stl::string topdir;
  while((pos = dir.find('/', pos)) != kwsys_stl::string::npos)
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
void SystemTools::ReplaceString(kwsys_stl::string& source,
                                   const char* replace,
                                   const char* with)
{
  const char *src = source.c_str();
  char *searchPos = const_cast<char *>(strstr(src,replace));
  
  // get out quick if string is not found
  if (!searchPos)
    {
    return;
    }

  // perform replacements until done
  size_t replaceSize = strlen(replace);
  char *orig = strdup(src);
  char *currentPos = orig;
  searchPos = searchPos - src + orig;
  
  // initialize the result
  source.erase(source.begin(),source.end());
  do
    {
    *searchPos = '\0';
    source += currentPos;
    currentPos = searchPos + replaceSize;
    // replace
    source += with;
    searchPos = strstr(currentPos,replace);
    }
  while (searchPos);

  // copy any trailing text
  source += currentPos;
  free(orig);
}

// Read a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will return the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will return the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::ReadRegistryValue(const char *key, kwsys_stl::string &value)
{

  kwsys_stl::string primary = key;
  kwsys_stl::string second;
  kwsys_stl::string valuename;
 
  size_t start = primary.find("\\");
  if (start == kwsys_stl::string::npos)
    {
    return false;
    }

  size_t valuenamepos = primary.find(";");
  if (valuenamepos != kwsys_stl::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }

  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey = HKEY_CURRENT_USER;
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
        RegCloseKey(hKey);
        return true;
        }
      }
    }
  return false;
}
#else
bool SystemTools::ReadRegistryValue(const char *, kwsys_stl::string &)
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
  kwsys_stl::string primary = key;
  kwsys_stl::string second;
  kwsys_stl::string valuename;
 
  size_t start = primary.find("\\");
  if (start == kwsys_stl::string::npos)
    {
    return false;
    }

  size_t valuenamepos = primary.find(";");
  if (valuenamepos != kwsys_stl::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }

  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey = HKEY_CURRENT_USER;
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
  kwsys_stl::string primary = key;
  kwsys_stl::string second;
  kwsys_stl::string valuename;
 
  size_t start = primary.find("\\");
  if (start == kwsys_stl::string::npos)
    {
    return false;
    }

  size_t valuenamepos = primary.find(";");
  if (valuenamepos != kwsys_stl::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }

  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey = HKEY_CURRENT_USER;
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
      RegCloseKey(hKey);
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
   return (fiBuf1.dwVolumeSerialNumber == fiBuf2.dwVolumeSerialNumber &&
           fiBuf1.nFileIndexHigh == fiBuf2.nFileIndexHigh &&
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
#ifdef _MSC_VER
# define access _access
#endif
#ifndef R_OK
# define R_OK 04
#endif
  if ( access(filename, R_OK) != 0 )
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
kwsys_stl::string SystemTools::Capitalized(const kwsys_stl::string& s)
{
  kwsys_stl::string n;
  if(s.size() == 0)
    {
    return n;
    }
  n.resize(s.size());
  n[0] = static_cast<kwsys_stl::string::value_type>(toupper(s[0]));
  for (size_t i = 1; i < s.size(); i++)
    {
    n[i] = static_cast<kwsys_stl::string::value_type>(tolower(s[i]));
    }
  return n;
}


// Return a lower case string 
kwsys_stl::string SystemTools::LowerCase(const kwsys_stl::string& s)
{
  kwsys_stl::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    {
    n[i] = static_cast<kwsys_stl::string::value_type>(tolower(s[i]));
    }
  return n;
}

// Return a lower case string 
kwsys_stl::string SystemTools::UpperCase(const kwsys_stl::string& s)
{
  kwsys_stl::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    {
    n[i] = static_cast<kwsys_stl::string::value_type>(toupper(s[i]));
    }
  return n;
}


// convert windows slashes to unix slashes 
void SystemTools::ConvertToUnixSlashes(kwsys_stl::string& path)
{
  kwsys_stl::string::size_type pos = 0;
  while((pos = path.find('\\', pos)) != kwsys_stl::string::npos)
    {
    // make sure we don't convert an escaped space to a unix slash
    if(pos < path.size()-2)
      {
      if(path[pos+1] != ' ')
        {
        path[pos] = '/';
        }
      }
    pos++;
    }
  // Remove all // from the path just like most unix shells
  int start_find;

#ifdef _WIN32
  // However, on windows if the first characters are both slashes,
  // then keep them that way, so that network paths can be handled.
  start_find = 1;
#else
  start_find = 0;
#endif

  while((pos = path.find("//", start_find)) != kwsys_stl::string::npos)
    {
    SystemTools::ReplaceString(path, "//", "/");
    }
  
  // remove any trailing slash
  if(path.size() > 1 && path[path.size()-1] == '/')
    {
    path = path.substr(0, path.size()-1);
    }

  // if there is a tilda ~ then replace it with HOME
  if(path.find("~") == 0)
    {
    if (getenv("HOME"))
      {
      path = kwsys_stl::string(getenv("HOME")) + path.substr(1);
      }
    }
}

// change // to /, and escape any spaces in the path
kwsys_stl::string SystemTools::ConvertToUnixOutputPath(const char* path)
{
  kwsys_stl::string ret = path;
  
  // remove // except at the beginning might be a cygwin drive
  kwsys_stl::string::size_type pos = 1;
  while((pos = ret.find("//", pos)) != kwsys_stl::string::npos)
    {
    ret.erase(pos, 1);
    }
  // now escape spaces if there is a space in the path
  if(ret.find(" ") != kwsys_stl::string::npos)
    {
    kwsys_stl::string result = "";
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

kwsys_stl::string SystemTools::ConvertToOutputPath(const char* path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  return SystemTools::ConvertToWindowsOutputPath(path);
#else
  return SystemTools::ConvertToUnixOutputPath(path);
#endif
}

// remove double slashes not at the start
kwsys_stl::string SystemTools::ConvertToWindowsOutputPath(const char* path)
{  
  kwsys_stl::string ret = path;
  kwsys_stl::string::size_type pos = 0;
  // first convert all of the slashes
  while((pos = ret.find('/', pos)) != kwsys_stl::string::npos)
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
  while((pos = ret.find("\\\\", pos)) != kwsys_stl::string::npos)
    {
    ret.erase(pos, 1);
    }
  // now double quote the path if it has spaces in it
  // and is not already double quoted
  if(ret.find(" ") != kwsys_stl::string::npos
     && ret[0] != '\"')
    {
    kwsys_stl::string result;
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
    return SystemTools::CopyFileAlways(source, destination);
    }
  return true;
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
  kwsys_ios::ifstream finSource(source, (kwsys_ios::ios::binary |
                                         kwsys_ios::ios::in));
  kwsys_ios::ifstream finDestination(destination, (kwsys_ios::ios::binary |
                                                   kwsys_ios::ios::in));
#else
  kwsys_ios::ifstream finSource(source);
  kwsys_ios::ifstream finDestination(destination);
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
  // If files are the same do not copy
  if ( SystemTools::SameFile(source, destination) )
    {
    return true;
    }

  mode_t perm = 0;
  bool perms = SystemTools::GetPermissions(source, perm);

  const int bufferSize = 4096;
  char buffer[bufferSize];

  // If destination is a directory, try to create a file with the same
  // name as the source in that directory.

  kwsys_stl::string new_destination;
  if(SystemTools::FileExists(destination) &&
     SystemTools::FileIsDirectory(destination))
    {
    new_destination = destination;
    SystemTools::ConvertToUnixSlashes(new_destination);
    new_destination += '/';
    kwsys_stl::string source_name = source;
    new_destination += SystemTools::GetFilenameName(source_name);
    destination = new_destination.c_str();
    }

  // Create destination directory

  kwsys_stl::string destination_dir = destination;
  destination_dir = SystemTools::GetFilenamePath(destination_dir);
  SystemTools::MakeDirectory(destination_dir.c_str());

  // Open files

#if defined(_WIN32) || defined(__CYGWIN__)
  kwsys_ios::ifstream fin(source, 
                    kwsys_ios::ios::binary | kwsys_ios::ios::in);
#else
  kwsys_ios::ifstream fin(source);
#endif
  if(!fin)
    {
    return false;
    }
 
  // try and remove the destination file so that read only destination files
  // can be written to.
  // If the remove fails continue so that files in read only directories
  // that do not allow file removal can be modified.
  SystemTools::RemoveFile(destination);

#if defined(_WIN32) || defined(__CYGWIN__)
  kwsys_ios::ofstream fout(destination, 
                     kwsys_ios::ios::binary | kwsys_ios::ios::out | kwsys_ios::ios::trunc);
#else
  kwsys_ios::ofstream fout(destination, 
                     kwsys_ios::ios::out | kwsys_ios::ios::trunc);
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
  if ( perms )
    {
    if ( !SystemTools::SetPermissions(destination, perm) )
      {
      return false;
      }
    }
  return true;
}

// return size of file; also returns zero if no file exists
unsigned long SystemTools::FileLength(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0) 
    {
      return 0;
    }
  else
    {
      return fs.st_size;
    }
}

int SystemTools::Strucmp(const char *s1, const char *s2)
{
// lifted from Graphvis http://www.graphviz.org 
  while ((*s1 != '\0') 
         && (tolower(*s1) == tolower(*s2)))
    {
      s1++;
      s2++;
    }
 
  return tolower(*s1) - tolower(*s2);

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


kwsys_stl::string SystemTools::GetLastSystemError()
{
  int e = errno;
  return strerror(e);
}

bool SystemTools::RemoveFile(const char* source)
{
  return unlink(source) != 0 ? false : true;
}

bool SystemTools::RemoveADirectory(const char* source)
{
  Directory dir;
  dir.Load(source);
  size_t fileNum;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
      {
      kwsys_stl::string fullPath = source;
      fullPath += "/";
      fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
      if(SystemTools::FileIsDirectory(fullPath.c_str()))
        {
        if (!SystemTools::RemoveADirectory(fullPath.c_str()))
          {
          return false;
          }
        }
      else
        {
        if(!SystemTools::RemoveFile(fullPath.c_str()))
          {
          return false;
          }
        }
      }
    }

  return (Rmdir(source) == 0);
}

/**
 * Find the file the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the file if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_stl::string SystemTools
::FindFile(const char* name, 
           const kwsys_stl::vector<kwsys_stl::string>& userPaths)
{
  // Add the system search path to our path first
  kwsys_stl::vector<kwsys_stl::string> path; 
  SystemTools::GetPath(path, "CMAKE_FILE_PATH");
  SystemTools::GetPath(path);
  // now add the additional paths
  path.insert(path.end(), userPaths.begin(), userPaths.end());
  // now look for the file
  kwsys_stl::string tryPath;
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator p = path.begin();
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
kwsys_stl::string SystemTools::FindProgram(
  const char* name,
  const kwsys_stl::vector<kwsys_stl::string>& userPaths,
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
  kwsys_stl::string tryPath = name;
  tryPath += SystemTools::GetExecutableExtension();
  if(SystemTools::FileExists(tryPath.c_str()) &&
     !SystemTools::FileIsDirectory(tryPath.c_str()))
    {
    return SystemTools::CollapseFullPath(tryPath.c_str());
    }
  kwsys_stl::vector<kwsys_stl::string> path;
  SystemTools::GetPath(path, "CMAKE_PROGRAM_PATH");
  // Add the system search path to our path.
  if (!no_system_path)
    {
    SystemTools::GetPath(path);
    }
  
  // now add the additional paths
  path.insert(path.end(), userPaths.begin(), userPaths.end());
  
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator p = path.begin();
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
kwsys_stl::string SystemTools
::FindLibrary(const char* name,
              const kwsys_stl::vector<kwsys_stl::string>& userPaths)
{
  // See if the executable exists as written.
  if(SystemTools::FileExists(name) &&
     !SystemTools::FileIsDirectory(name))
    {
    return SystemTools::CollapseFullPath(name);
    }
  
  // Add the system search path to our path.
  kwsys_stl::vector<kwsys_stl::string> path;
  SystemTools::GetPath(path);
   // now add the additional paths
  path.insert(path.end(), userPaths.begin(), userPaths.end());
  kwsys_stl::string tryPath;
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    tryPath += ".lib";
    if(SystemTools::FileExists(tryPath.c_str()) 
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
#else
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".so";
    if(SystemTools::FileExists(tryPath.c_str()) 
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".a";
    if(SystemTools::FileExists(tryPath.c_str()) 
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".sl";
    if(SystemTools::FileExists(tryPath.c_str()) 
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".dylib";
    if(SystemTools::FileExists(tryPath.c_str()) 
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".dll";
    if(SystemTools::FileExists(tryPath.c_str()) 
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
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

kwsys_stl::string SystemTools::GetCurrentWorkingDirectory()
{
  char buf[2048];
  const char* cwd = Getcwd(buf, 2048);
  kwsys_stl::string path;
  if ( cwd )
    {
    path = cwd;
    }

  return SystemTools::CollapseFullPath(path.c_str());
}

kwsys_stl::string SystemTools::GetProgramPath(const char* in_name)
{
  kwsys_stl::string dir, file;
  SystemTools::SplitProgramPath(in_name, dir, file);
  return dir;
}

bool SystemTools::SplitProgramPath(const char* in_name,
                                   kwsys_stl::string& dir,
                                   kwsys_stl::string& file,
                                   bool)
{
  dir = in_name;
  file = "";
  SystemTools::ConvertToUnixSlashes(dir);
  
  if(!SystemTools::FileIsDirectory(dir.c_str()))
    {
    kwsys_stl::string::size_type slashPos = dir.rfind("/");
    if(slashPos != kwsys_stl::string::npos)
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
  if(!(dir == "") && !SystemTools::FileIsDirectory(dir.c_str()))
    {
    kwsys_stl::string oldDir = in_name;
    SystemTools::ConvertToUnixSlashes(oldDir);
    dir = in_name;
    return false;
    }
  return true;
}

bool SystemTools::FindProgramPath(const char* argv0, 
                                  kwsys_stl::string& pathOut,
                                  kwsys_stl::string& errorMsg,
                                  const char* exeName,
                                  const char* buildDir,         
                                  const char* installPrefix )
{
  kwsys_stl::vector<kwsys_stl::string> failures;
  kwsys_stl::string self = argv0;
  SystemTools::ConvertToUnixSlashes(self);
  failures.push_back(argv0);
  self = SystemTools::FindProgram(self.c_str());
  if(!SystemTools::FileExists(self.c_str()))
    {
    if(buildDir)
      {
      kwsys_stl::string intdir = ".";
#ifdef  CMAKE_INTDIR
      intdir = CMAKE_INTDIR;
#endif
      self = buildDir;
      self += "/bin/";
      self += intdir;
      self += "/";
      self += exeName;
      self += SystemTools::GetExecutableExtension();
      }
    }
  if(installPrefix)
    {
    if(!SystemTools::FileExists(self.c_str()))
      {
      failures.push_back(self);
      self = installPrefix;
      self += "/bin/";
      self +=  exeName;
      }
    }
  if(!SystemTools::FileExists(self.c_str()))
    {
    failures.push_back(self);
    kwsys_ios::ostringstream msg;
    msg << "Can not find the command line program " << exeName << "\n";
    msg << "  argv[0] = \"" << argv0 << "\"\n";
    msg << "  Attempted paths:\n";
    kwsys_stl::vector<kwsys_stl::string>::iterator i;
    for(i=failures.begin(); i != failures.end(); ++i)
      {
      msg << "    \"" << i->c_str() << "\"\n";
      }
    errorMsg = msg.str();
    return false;
    }
  pathOut = self;
  return true;
}


kwsys_stl::string SystemTools::CollapseFullPath(const char* in_relative)
{
  return SystemTools::CollapseFullPath(in_relative, 0);
}

void SystemTools::AddTranslationPath(const char * a, const char * b)
{
  kwsys_stl::string path_a = a;
  kwsys_stl::string path_b = b;
  SystemTools::ConvertToUnixSlashes(path_a);
  SystemTools::ConvertToUnixSlashes(path_b);
  // First check this is a directory path, since we don't want the table to
  // grow too fat
  if( SystemTools::FileIsDirectory( path_a.c_str() ) )
    {
    // Make sure the path is a full path and does not contain no '..'
    if( SystemTools::FileIsFullPath(path_b.c_str()) && path_b.find("..") 
        == kwsys_stl::string::npos )
      {
      // Before inserting make sure path ends with '/'
      path_a += '/'; path_b += '/';
      if( !(path_a == path_b) )
        {
        SystemTools::TranslationMap->insert(
          SystemToolsTranslationMap::value_type(path_a, path_b));
        }
      }
    }
}

void SystemTools::AddKeepPath(const char* dir)
{
  kwsys_stl::string cdir = SystemTools::CollapseFullPath(dir);
  SystemTools::AddTranslationPath(cdir.c_str(), dir);
}

void SystemTools::CheckTranslationPath(kwsys_stl::string & path)
{
  // In case a file was specified we still have to go through this:
  // Now convert any path found in the table back to the one desired:
  kwsys_stl::map<kwsys_stl::string,kwsys_stl::string>::const_iterator it;
  for(it  = SystemTools::TranslationMap->begin();
      it != SystemTools::TranslationMap->end();
      ++it )
    {
    // We need to check of the path is a substring of the other path
    // But also check that the last character is a '/' otherwise we could
    // have some weird case such as /tmp/VTK and /tmp/VTK-bin
    if(path.size() > 1 && path[path.size()-1] != '/')
      {
      // Do not append '/' on a program name:
      if( SystemTools::FileIsDirectory( path.c_str() ) )
        {
        path += "/";
        }
      }
    if(path.find( it->first ) == 0)
      {
      path = path.replace( 0, it->first.size(), it->second);
      }
    }
}

kwsys_stl::string SystemTools::CollapseFullPath(const char* in_relative,
                                                const char* in_base)
{
  static int initialized = 0;

  kwsys_stl::string orig;
  
  // Change to base of relative path.
  if(in_base)
    {
    // Save original working directory.
    orig = SystemTools::GetCurrentWorkingDirectory();
    Chdir(in_base);
    }
  
  kwsys_stl::string dir, file;
  SystemTools::SplitProgramPath(in_relative, dir, file, false);
  if(dir.size() == 0 && 
      in_relative && strlen(in_relative) > 0 &&
      in_relative[0] == '/')
     {
     dir = "/";
     }

  // Resolve relative path.
  kwsys_stl::string newDir;
  if(!(dir == ""))
    {
    Realpath(dir.c_str(), newDir);    
    }
  else
    {
    newDir = SystemTools::GetCurrentWorkingDirectory();
    }

  if(in_base)
    {
    // Restore original working directory.
    Chdir(orig.c_str());
    }
  
  // Construct and return the full path.
  kwsys_stl::string newPath = newDir;
  if(!(file == ""))
    {
    if(!(newDir.size() == 1 && newDir[0] ==  '/'))
      {
      newPath += "/";
      }
    newPath += file;
    }

  // If the table has never been initialized, add default path:
  if(!initialized)
    {
    initialized = 1;
    //Also add some good default one:
    // This one should always be there it fix a bug on sgi
    SystemTools::AddTranslationPath("/tmp_mnt/", "/");
  
    //This is a good default also:
    SystemTools::AddKeepPath("/tmp/");
    }

  // Now we need to update the translation table with this potentially new path
  SystemTools::AddTranslationPath(newPath.c_str(), in_relative);
  SystemTools::CheckTranslationPath(newPath);

  return newPath;
}

bool SystemTools::Split(const char* str, kwsys_stl::vector<kwsys_stl::string>& lines)
{
  kwsys_stl::string data(str);
  kwsys_stl::string::size_type lpos = 0;
  while(lpos < data.length())
    {
    kwsys_stl::string::size_type rpos = data.find_first_of("\n", lpos);
    if(rpos == kwsys_stl::string::npos)
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
kwsys_stl::string SystemTools::GetFilenamePath(const kwsys_stl::string& filename)
{
  kwsys_stl::string fn = filename;
  SystemTools::ConvertToUnixSlashes(fn);
  
  kwsys_stl::string::size_type slash_pos = fn.rfind("/");
  if(slash_pos != kwsys_stl::string::npos)
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
kwsys_stl::string SystemTools::GetFilenameName(const kwsys_stl::string& filename)
{
  kwsys_stl::string fn = filename;
  SystemTools::ConvertToUnixSlashes(fn);
  
  kwsys_stl::string::size_type slash_pos = fn.rfind("/");
  if(slash_pos != kwsys_stl::string::npos)
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
kwsys_stl::string SystemTools::GetFilenameExtension(const kwsys_stl::string& filename)
{
  kwsys_stl::string name = SystemTools::GetFilenameName(filename);
  kwsys_stl::string::size_type dot_pos = name.find(".");
  if(dot_pos != kwsys_stl::string::npos)
    {
    return name.substr(dot_pos);
    }
  else
    {
    return "";
    }
}

/**
 * Return file extension of a full filename (dot included).
 * Warning: this is the shortest extension (for example: .tar.gz)
 */
kwsys_stl::string SystemTools::GetFilenameLastExtension(const kwsys_stl::string& filename)
{
  kwsys_stl::string name = SystemTools::GetFilenameName(filename);
  kwsys_stl::string::size_type dot_pos = name.rfind(".");
  if(dot_pos != kwsys_stl::string::npos)
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
kwsys_stl::string SystemTools::GetFilenameWithoutExtension(const kwsys_stl::string& filename)
{
  kwsys_stl::string name = SystemTools::GetFilenameName(filename);
  kwsys_stl::string::size_type dot_pos = name.find(".");
  if(dot_pos != kwsys_stl::string::npos)
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
kwsys_stl::string
SystemTools::GetFilenameWithoutLastExtension(const kwsys_stl::string& filename)
{
  kwsys_stl::string name = SystemTools::GetFilenameName(filename);
  kwsys_stl::string::size_type dot_pos = name.rfind(".");
  if(dot_pos != kwsys_stl::string::npos)
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
  kwsys_stl::string name = in_name;
#if defined(_WIN32) || defined(__CYGWIN__)
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

bool SystemTools::GetShortPath(const char* path, kwsys_stl::string& shortPath)
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

void SystemTools::SplitProgramFromArgs(const char* path, 
                                       kwsys_stl::string& program, kwsys_stl::string& args)
{
  // see if this is a full path to a program
  // if so then set program to path and args to nothing
  if(SystemTools::FileExists(path))
    {
    program = path;
    args = "";
    return;
    }
  // Try to find the program in the path, note the program
  // may have spaces in its name so we have to look for it 
  kwsys_stl::vector<kwsys_stl::string> e;
  kwsys_stl::string findProg = SystemTools::FindProgram(path, e);
  if(findProg.size())
    {
    program = findProg;
    args = "";
    return;
    }

  // Now try and peel off space separated chunks from the end of the string
  // so the largest path possible is found allowing for spaces in the path
  kwsys_stl::string dir = path;
  kwsys_stl::string::size_type spacePos = dir.rfind(' ');
  while(spacePos != kwsys_stl::string::npos)
    {
    kwsys_stl::string tryProg = dir.substr(0, spacePos);
    // See if the file exists
    if(SystemTools::FileExists(tryProg.c_str()))
      {
      program = tryProg;
      // remove trailing spaces from program
      kwsys_stl::string::size_type pos = program.size()-1;
      while(program[pos] == ' ')
        {
        program.erase(pos);
        pos--;
        }
      args = dir.substr(spacePos, dir.size()-spacePos);
      return;
      }
    // Now try and find the the program in the path 
    findProg = SystemTools::FindProgram(tryProg.c_str(), e);
    if(findProg.size())
      {
      program = findProg;
      // remove trailing spaces from program
      kwsys_stl::string::size_type pos = program.size()-1;
      while(program[pos] == ' ')
        {
        program.erase(pos);
        pos--;
        }
      args = dir.substr(spacePos, dir.size()-spacePos);
      return;
      }
    // move past the space for the next search
    spacePos--;
    spacePos = dir.rfind(' ', spacePos);
    }

  program = "";
  args = "";
}

kwsys_stl::string SystemTools::GetCurrentDateTime(const char* format)
{
  char buf[1024];
  time_t t;
  time(&t);
  strftime(buf, sizeof(buf), format, localtime(&t));
  return buf;
}

kwsys_stl::string SystemTools::MakeCindentifier(const char* s)
{
  kwsys_stl::string str(s);
  if (str.find_first_of("0123456789") == 0)
    {
    str = "_" + str;
    }

  kwsys_stl::string permited_chars("_"
                             "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "0123456789");
  kwsys_stl::string::size_type pos = 0;
  while ((pos = str.find_first_not_of(permited_chars, pos)) != kwsys_stl::string::npos)
    {
    str[pos] = '_';
    }
  return str;
}

// Due to a buggy stream library on the HP and another on Mac OSX, we
// need this very carefully written version of getline.  Returns true
// if any data were read before the end-of-file was reached.
bool SystemTools::GetLineFromStream(kwsys_ios::istream& is, kwsys_stl::string& line,
                                    bool *has_newline /* = 0 */)
{
  const int bufferSize = 1024;
  char buffer[bufferSize];
  line = "";
  bool haveData = false;
  if ( has_newline )
    {
    *has_newline = false;
    }

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
      if ( has_newline )
        {
        *has_newline = true;
        }
      break;
      }

    // The fail bit may be set.  Clear it.
    is.clear(is.rdstate() & ~kwsys_ios::ios::failbit);
    }
  return haveData;
}

int SystemTools::GetTerminalWidth()
{
  int width = -1;
#ifndef _WIN32
  struct winsize ws;
  char *columns; /* Unix98 environment variable */
  if(ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col>0 && ws.ws_row>0)
    {
    width = ws.ws_col;
    }
  if(!isatty(STDOUT_FILENO))
    {
    width = -1;
    }
  columns = getenv("COLUMNS");
  if(columns && *columns)
    {
    long t;
    char *endptr;
    t = strtol(columns, &endptr, 0);
    if(endptr && !*endptr && (t>0) && (t<1000))
      {
      width = (int)t;
      }
    }
  if ( width < 9 )
    {
    width = -1;
    }
#endif
  return width;
}

bool SystemTools::GetPermissions(const char* file, mode_t& mode)
{
  if ( !file )
    {
    return false;
    }

  struct stat st;
  if ( stat(file, &st) < 0 )
    {
    return false;
    }
  mode = st.st_mode;
  return true;
}

bool SystemTools::SetPermissions(const char* file, mode_t mode)
{
  if ( !file )
    {
    return false;
    }
  if ( !SystemTools::FileExists(file) )
    {
    return false;
    }
  if ( chmod(file, mode) < 0 )
    {
    return false;
    }

  return true;
}

kwsys_stl::string SystemTools::GetParentDirectory(const char* fileOrDir)
{
  if ( !fileOrDir || !*fileOrDir )
    {
    return "";
    }
  kwsys_stl::string res = fileOrDir;
  SystemTools::ConvertToUnixSlashes(res);
  kwsys_stl::string::size_type cc = res.size()-1;
  if ( res[cc] == '/' )
    {
    cc --;
    }
  for ( ; cc > 0; cc -- )
    {
    if ( res[cc] == '/' )
      {
      break;
      }
    }
  return res.substr(0, cc);
}

bool SystemTools::IsSubDirectory(const char* cSubdir, const char* cDir)
{
  kwsys_stl::string subdir = cSubdir;
  kwsys_stl::string dir = cDir;
  SystemTools::ConvertToUnixSlashes(dir);
  kwsys_stl::string path = subdir;
  do
    {
    path = SystemTools::GetParentDirectory(path.c_str());
    if ( dir == path )
      {
      return true;
      }
    }
  while ( path.size() > dir.size() );
  return false;
}

kwsys_stl::string SystemTools::FileExistsInParentDirectories(const char* fname,
  const char* directory, const char* toplevel)
{
  kwsys_stl::string file = fname;
  SystemTools::ConvertToUnixSlashes(file);
  kwsys_stl::string dir = directory;
  SystemTools::ConvertToUnixSlashes(dir);
  while ( 1 )
    {
    kwsys_stl::string path = dir + "/" + file;
    if ( SystemTools::FileExists(path.c_str()) )
      {
      return path;
      }
    if ( dir.size() < strlen(toplevel) )
      {
      break;
      }
    dir = SystemTools::GetParentDirectory(dir.c_str());
    }
  return "";
}

// These must NOT be initialized.  Default initialization to zero is
// necessary.
unsigned int SystemToolsManagerCount;
SystemToolsTranslationMap *SystemTools::TranslationMap;

// SystemToolsManager manages the SystemTools singleton.
// SystemToolsManager should be included in any translation unit
// that will use SystemTools or that implements the singleton
// pattern. It makes sure that the SystemTools singleton is created
// before and destroyed after all other singletons in CMake.

SystemToolsManager::SystemToolsManager()
{
  if(++SystemToolsManagerCount == 1)
    {
    SystemTools::ClassInitialize();
    }
}

SystemToolsManager::~SystemToolsManager()
{
  if(--SystemToolsManagerCount == 0)
    {
    SystemTools::ClassFinalize();
    }
}

void SystemTools::ClassInitialize()
{
  SystemTools::TranslationMap = new SystemToolsTranslationMap;
}

void SystemTools::ClassFinalize()
{
  delete SystemTools::TranslationMap;
}


} // namespace KWSYS_NAMESPACE

#if defined(_MSC_VER) && defined(_DEBUG)
# include <crtdbg.h>
# include <stdio.h>
# include <stdlib.h>
namespace KWSYS_NAMESPACE
{

static int SystemToolsDebugReport(int, char* message, int*)
{
  fprintf(stderr, message);
  exit(1);
}
void SystemTools::EnableMSVCDebugHook()
{
  if(getenv("DART_TEST_FROM_DART"))
    {
    _CrtSetReportHook(SystemToolsDebugReport);
    }
}

} // namespace KWSYS_NAMESPACE
#else
namespace KWSYS_NAMESPACE
{
void SystemTools::EnableMSVCDebugHook() {}
} // namespace KWSYS_NAMESPACE
#endif

