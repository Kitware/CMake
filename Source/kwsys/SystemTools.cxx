/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifdef __osf__
#  define _OSF_SOURCE
#  define _POSIX_C_SOURCE 199506L
#  define _XOPEN_SOURCE_EXTENDED
#endif

#include "kwsysPrivate.h"
#include KWSYS_HEADER(RegularExpression.hxx)
#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(Directory.hxx)

#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(ios/fstream)
#include KWSYS_HEADER(ios/sstream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemTools.hxx.in"
# include "Directory.hxx.in"
# include "kwsys_ios_iostream.h.in"
# include "kwsys_ios_fstream.h.in"
# include "kwsys_ios_sstream.h.in"
#endif

#ifdef _MSC_VER
# pragma warning (disable: 4786)
#endif

#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 1375 /* base class destructor not virtual */
#endif

#include <ctype.h>
#include <errno.h>
#ifdef __QNX__
# include <malloc.h> /* for malloc/free on QNX */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

// support for realpath call
#ifndef _WIN32
#include <sys/time.h>
#include <utime.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>
#ifndef __VMS
#include <sys/param.h>
#include <termios.h>
#endif
#include <signal.h>    /* sigprocmask */
#endif

// Windows API.
#if defined(_WIN32)
# include <windows.h>
#elif defined (__CYGWIN__)
# include <windows.h>
# undef _WIN32
#endif

#ifdef __CYGWIN__
extern "C" void cygwin_conv_to_win32_path(const char *path, char *win32_path);
#endif

// getpwnam doesn't exist on Windows and Cray Xt3/Catamount
// same for TIOCGWINSZ
#if defined(_WIN32) || defined (__LIBCATAMOUNT__)
# undef HAVE_GETPWNAM
# undef HAVE_TTY_INFO
#else
# define HAVE_GETPWNAM 1
# define HAVE_TTY_INFO 1
#endif

#define VTK_URL_PROTOCOL_REGEX "([a-zA-Z0-9]*)://(.*)"
#define VTK_URL_REGEX "([a-zA-Z0-9]*)://(([A-Za-z0-9]+)(:([^:@]+))?@)?([^:@/]+)(:([0-9]+))?/(.+)?"

#ifdef _MSC_VER
#include <sys/utime.h>
#else
#include <utime.h>
#endif


// This is a hack to prevent warnings about these functions being
// declared but not referenced.
#if defined(__sgi) && !defined(__GNUC__)
# include <sys/termios.h>
namespace KWSYS_NAMESPACE
{
class SystemToolsHack
{
public:
  enum
  {
    Ref1 = sizeof(cfgetospeed(0)),
    Ref2 = sizeof(cfgetispeed(0)),
    Ref3 = sizeof(tcgetattr(0, 0)),
    Ref4 = sizeof(tcsetattr(0, 0, 0)),
    Ref5 = sizeof(cfsetospeed(0,0)),
    Ref6 = sizeof(cfsetispeed(0,0))
  };
};
}
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__WATCOMC__) ||defined(__BORLANDC__) || defined(__MINGW32__))
#include <io.h>
#include <direct.h>
#define _unlink unlink
#endif

/* The maximum length of a file name.  */
#if defined(PATH_MAX)
# define KWSYS_SYSTEMTOOLS_MAXPATH PATH_MAX
#elif defined(MAXPATHLEN)
# define KWSYS_SYSTEMTOOLS_MAXPATH MAXPATHLEN
#else
# define KWSYS_SYSTEMTOOLS_MAXPATH 16384
#endif
#if defined(__WATCOMC__)
#include <direct.h>
#define _mkdir mkdir
#define _rmdir rmdir
#define _getcwd getcwd
#define _chdir chdir
#endif

#if defined(__HAIKU__)
#include <os/kernel/OS.h>
#include <os/storage/Path.h>
#endif

#if defined(__BEOS__) && !defined(__ZETA__) && !defined(__HAIKU__)
#include <be/kernel/OS.h>
#include <be/storage/Path.h>

// BeOS 5 doesn't have usleep(), but it has snooze(), which is identical.
static inline void usleep(unsigned int msec)
{
  ::snooze(msec);
}

// BeOS 5 also doesn't have realpath(), but its C++ API offers something close.
static inline char *realpath(const char *path, char *resolved_path)
{
  const size_t maxlen = KWSYS_SYSTEMTOOLS_MAXPATH;
  snprintf(resolved_path, maxlen, "%s", path);
  BPath normalized(resolved_path, NULL, true);
  const char *resolved = normalized.Path();
  if (resolved != NULL)   // NULL == No such file.
    {
    if (snprintf(resolved_path, maxlen, "%s", resolved) < maxlen)
      {
      return resolved_path;
      }
    }
  return NULL;   // something went wrong.
}
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__MINGW32__))
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
  const char* ret = _getcwd(buf, len);
  if(!ret)
    {
    fprintf(stderr, "No current working directory.\n");
    abort();
    }
  // make sure the drive letter is capital
  if(strlen(buf) > 1 && buf[1] == ':')
    {
    buf[0] = toupper(buf[0]);
    }
  return ret;
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
  else
    {
    resolved_path = path;
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
  const char* ret = getcwd(buf, len);
  if(!ret)
    {
    fprintf(stderr, "No current working directory\n");
    abort();
    }
  return ret;
}

inline int Chdir(const char* dir)
{
  return chdir(dir);
}
inline void Realpath(const char *path, kwsys_stl::string & resolved_path)
{
  char resolved_name[KWSYS_SYSTEMTOOLS_MAXPATH];

  char *ret = realpath(path, resolved_name);
  if(ret)
    {
    resolved_path = ret;
    }
  else
    {
    // if path resolution fails, return what was passed in
    resolved_path = path;
    }
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

namespace KWSYS_NAMESPACE
{

double SystemTools::GetTime(void)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  return (429.4967296*ft.dwHighDateTime
          + 0.0000001*ft.dwLowDateTime
          - 11644473600.0);
#else
  struct timeval t;
  gettimeofday(&t, 0);
  return 1.0*double(t.tv_sec) + 0.000001*double(t.tv_usec);
#endif
}

class SystemToolsTranslationMap :
    public kwsys_stl::map<kwsys_stl::string,kwsys_stl::string>
{
};

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

#ifdef __INTEL_COMPILER
#pragma warning disable 444
#endif

class kwsysDeletingCharVector : public kwsys_stl::vector<char*>
{
public:
  ~kwsysDeletingCharVector();
};

kwsysDeletingCharVector::~kwsysDeletingCharVector()
{
#ifndef KWSYS_DO_NOT_CLEAN_PUTENV
  for(kwsys_stl::vector<char*>::iterator i = this->begin();
      i != this->end(); ++i)
    {
    delete []*i;
    }
#endif
}
bool SystemTools::PutEnv(const char* value)
{
  static kwsysDeletingCharVector localEnvironment;
  char* envVar = new char[strlen(value)+1];
  strcpy(envVar, value);
  int ret = putenv(envVar);
  // save the pointer in the static vector so that it can
  // be deleted on exit
  localEnvironment.push_back(envVar);
  return ret == 0;
}

const char* SystemTools::GetExecutableExtension()
{
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__VMS)
  return ".exe";
#else
  return "";
#endif
}


bool SystemTools::MakeDirectory(const char* path)
{
  if(!path)
    {
    return false;
    }
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
  // do while hangs if replaceSize is 0
  if(replaceSize == 0)
    {
    return;
    }
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

#if defined(KEY_WOW64_32KEY) && defined(KEY_WOW64_64KEY)
# define KWSYS_ST_KEY_WOW64_32KEY KEY_WOW64_32KEY
# define KWSYS_ST_KEY_WOW64_64KEY KEY_WOW64_64KEY
#else
# define KWSYS_ST_KEY_WOW64_32KEY 0x0200
# define KWSYS_ST_KEY_WOW64_64KEY 0x0100
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
static DWORD SystemToolsMakeRegistryMode(DWORD mode,
                                         SystemTools::KeyWOW64 view)
{
  // only add the modes when on a system that supports Wow64.
  static FARPROC wow64p = GetProcAddress(GetModuleHandle("kernel32"),
                                         "IsWow64Process");
  if(wow64p == NULL)
    {
    return mode;
    }

  if(view == SystemTools::KeyWOW64_32)
    {
    return mode | KWSYS_ST_KEY_WOW64_32KEY;
    }
  else if(view == SystemTools::KeyWOW64_64)
    {
    return mode | KWSYS_ST_KEY_WOW64_64KEY;
    }
  return mode;
}
#endif

// Read a registry value.
// Example :
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will return the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will return the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::ReadRegistryValue(const char *key, kwsys_stl::string &value,
                                    KeyWOW64 view)
{
  bool valueset = false;
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
                  SystemToolsMakeRegistryMode(KEY_READ, view),
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
        valueset = true;
        }
      else if (dwType == REG_EXPAND_SZ)
        {
        char expanded[1024];
        DWORD dwExpandedSize = sizeof(expanded)/sizeof(expanded[0]);
        if(ExpandEnvironmentStrings(data, expanded, dwExpandedSize))
          {
          value = expanded;
          valueset = true;
          }
        }
      }

    RegCloseKey(hKey);
    }

  return valueset;
}
#else
bool SystemTools::ReadRegistryValue(const char *, kwsys_stl::string &,
                                    KeyWOW64)
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
bool SystemTools::WriteRegistryValue(const char *key, const char *value,
                                     KeyWOW64 view)
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
  char lpClass[] = "";
  if(RegCreateKeyEx(primaryKey,
                    second.c_str(),
                    0,
                    lpClass,
                    REG_OPTION_NON_VOLATILE,
                    SystemToolsMakeRegistryMode(KEY_WRITE, view),
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
bool SystemTools::WriteRegistryValue(const char *, const char *, KeyWOW64)
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
bool SystemTools::DeleteRegistryValue(const char *key, KeyWOW64 view)
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
                  SystemToolsMakeRegistryMode(KEY_WRITE, view),
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
bool SystemTools::DeleteRegistryValue(const char *, KeyWOW64)
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

//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__CYGWIN__)
static bool WindowsFileExists(const char* filename)
{
  WIN32_FILE_ATTRIBUTE_DATA fd;
  return GetFileAttributesExA(filename, GetFileExInfoStandard, &fd) != 0;
}
#endif

//----------------------------------------------------------------------------
bool SystemTools::FileExists(const char* filename)
{
  if(!(filename && *filename))
    {
    return false;
    }
#if defined(__CYGWIN__)
  // Convert filename to native windows path if possible.
  char winpath[MAX_PATH];
  if(SystemTools::PathCygwinToWin32(filename, winpath))
    {
    return WindowsFileExists(winpath);
    }
  return access(filename, R_OK) == 0;
#elif defined(_WIN32)
  return WindowsFileExists(filename);
#else
  return access(filename, R_OK) == 0;
#endif
}

//----------------------------------------------------------------------------
bool SystemTools::FileExists(const char* filename, bool isFile)
{
  if(SystemTools::FileExists(filename))
    {
    // If isFile is set return not FileIsDirectory,
    // so this will only be true if it is a file
    return !isFile || !SystemTools::FileIsDirectory(filename);
    }
  return false;
}

//----------------------------------------------------------------------------
#ifdef __CYGWIN__
bool SystemTools::PathCygwinToWin32(const char *path, char *win32_path)
{
  SystemToolsTranslationMap::iterator i =
    SystemTools::Cyg2Win32Map->find(path);

  if (i != SystemTools::Cyg2Win32Map->end())
    {
    strncpy(win32_path, i->second.c_str(), MAX_PATH);
    }
  else
    {
    cygwin_conv_to_win32_path(path, win32_path);
    SystemToolsTranslationMap::value_type entry(path, win32_path);
    SystemTools::Cyg2Win32Map->insert(entry);
    }
  return win32_path[0] != 0;
}
#endif

bool SystemTools::Touch(const char* filename, bool create)
{
  if(create && !SystemTools::FileExists(filename))
    {
    FILE* file = fopen(filename, "a+b");
    if(file)
      {
      fclose(file);
      return true;
      }
    return false;
    }
#ifdef _MSC_VER
#define utime _utime
#define utimbuf _utimbuf
#endif
  struct stat fromStat;
  if(stat(filename, &fromStat) < 0)
    {
    return false;
    }
  struct utimbuf buf;
  buf.actime = fromStat.st_atime;
  buf.modtime = static_cast<time_t>(SystemTools::GetTime());
  if(utime(filename, &buf) < 0)
    {
    return false;
    }
  return true;
}

bool SystemTools::FileTimeCompare(const char* f1, const char* f2,
                                  int* result)
{
  // Default to same time.
  *result = 0;
#if !defined(_WIN32) || defined(__CYGWIN__)
  // POSIX version.  Use stat function to get file modification time.
  struct stat s1;
  if(stat(f1, &s1) != 0)
    {
    return false;
    }
  struct stat s2;
  if(stat(f2, &s2) != 0)
    {
    return false;
    }
# if KWSYS_STAT_HAS_ST_MTIM
  // Compare using nanosecond resolution.
  if(s1.st_mtim.tv_sec < s2.st_mtim.tv_sec)
    {
    *result = -1;
    }
  else if(s1.st_mtim.tv_sec > s2.st_mtim.tv_sec)
    {
    *result = 1;
    }
  else if(s1.st_mtim.tv_nsec < s2.st_mtim.tv_nsec)
    {
    *result = -1;
    }
  else if(s1.st_mtim.tv_nsec > s2.st_mtim.tv_nsec)
    {
    *result = 1;
    }
# else
  // Compare using 1 second resolution.
  if(s1.st_mtime < s2.st_mtime)
    {
    *result = -1;
    }
  else if(s1.st_mtime > s2.st_mtime)
    {
    *result = 1;
    }
# endif
#else
  // Windows version.  Get the modification time from extended file attributes.
  WIN32_FILE_ATTRIBUTE_DATA f1d;
  WIN32_FILE_ATTRIBUTE_DATA f2d;
  if(!GetFileAttributesEx(f1, GetFileExInfoStandard, &f1d))
    {
    return false;
    }
  if(!GetFileAttributesEx(f2, GetFileExInfoStandard, &f2d))
    {
    return false;
    }

  // Compare the file times using resolution provided by system call.
  *result = (int)CompareFileTime(&f1d.ftLastWriteTime, &f2d.ftLastWriteTime);
#endif
  return true;
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

// Return capitalized words
kwsys_stl::string SystemTools::CapitalizedWords(const kwsys_stl::string& s)
{
  kwsys_stl::string n(s);
  for (size_t i = 0; i < s.size(); i++)
    {
#if defined(_MSC_VER) && defined (_MT) && defined (_DEBUG)
    // MS has an assert that will fail if s[i] < 0; setting
    // LC_CTYPE using setlocale() does *not* help. Painful.
    if ((int)s[i] >= 0 && isalpha(s[i]) &&
        (i == 0 || ((int)s[i - 1] >= 0 && isspace(s[i - 1]))))
#else
    if (isalpha(s[i]) && (i == 0 || isspace(s[i - 1])))
#endif
      {
      n[i] = static_cast<kwsys_stl::string::value_type>(toupper(s[i]));
      }
    }
  return n;
}

// Return uncapitalized words
kwsys_stl::string SystemTools::UnCapitalizedWords(const kwsys_stl::string& s)
{
  kwsys_stl::string n(s);
  for (size_t i = 0; i < s.size(); i++)
    {
#if defined(_MSC_VER) && defined (_MT) && defined (_DEBUG)
    // MS has an assert that will fail if s[i] < 0; setting
    // LC_CTYPE using setlocale() does *not* help. Painful.
    if ((int)s[i] >= 0 && isalpha(s[i]) &&
        (i == 0 || ((int)s[i - 1] >= 0 && isspace(s[i - 1]))))
#else
    if (isalpha(s[i]) && (i == 0 || isspace(s[i - 1])))
#endif
      {
      n[i] = static_cast<kwsys_stl::string::value_type>(tolower(s[i]));
      }
    }
  return n;
}

// only works for words with at least two letters
kwsys_stl::string SystemTools::AddSpaceBetweenCapitalizedWords(
  const kwsys_stl::string& s)
{
  kwsys_stl::string n;
  if (s.size())
    {
    n.reserve(s.size());
    n += s[0];
    for (size_t i = 1; i < s.size(); i++)
      {
      if (isupper(s[i]) && !isspace(s[i - 1]) && !isupper(s[i - 1]))
        {
        n += ' ';
        }
      n += s[i];
      }
    }
  return n;
}

char* SystemTools::AppendStrings(const char* str1, const char* str2)
{
  if (!str1)
    {
    return SystemTools::DuplicateString(str2);
    }
  if (!str2)
    {
    return SystemTools::DuplicateString(str1);
    }
  size_t len1 = strlen(str1);
  char *newstr = new char[len1 + strlen(str2) + 1];
  if (!newstr)
    {
    return 0;
    }
  strcpy(newstr, str1);
  strcat(newstr + len1, str2);
  return newstr;
}

char* SystemTools::AppendStrings(
  const char* str1, const char* str2, const char* str3)
{
  if (!str1)
    {
    return SystemTools::AppendStrings(str2, str3);
    }
  if (!str2)
    {
    return SystemTools::AppendStrings(str1, str3);
    }
  if (!str3)
    {
    return SystemTools::AppendStrings(str1, str2);
    }

  size_t len1 = strlen(str1), len2 = strlen(str2);
  char *newstr = new char[len1 + len2 + strlen(str3) + 1];
  if (!newstr)
    {
    return 0;
    }
  strcpy(newstr, str1);
  strcat(newstr + len1, str2);
  strcat(newstr + len1 + len2, str3);
  return newstr;
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

// Count char in string
size_t SystemTools::CountChar(const char* str, char c)
{
  size_t count = 0;

  if (str)
    {
    while (*str)
      {
      if (*str == c)
        {
        ++count;
        }
      ++str;
      }
    }
  return count;
}

// Remove chars in string
char* SystemTools::RemoveChars(const char* str, const char *toremove)
{
  if (!str)
    {
    return NULL;
    }
  char *clean_str = new char [strlen(str) + 1];
  char *ptr = clean_str;
  while (*str)
    {
    const char *str2 = toremove;
    while (*str2 && *str != *str2)
      {
      ++str2;
      }
    if (!*str2)
      {
      *ptr++ = *str;
      }
    ++str;
    }
  *ptr = '\0';
  return clean_str;
}

// Remove chars in string
char* SystemTools::RemoveCharsButUpperHex(const char* str)
{
  if (!str)
    {
    return 0;
    }
  char *clean_str = new char [strlen(str) + 1];
  char *ptr = clean_str;
  while (*str)
    {
    if ((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F'))
      {
      *ptr++ = *str;
      }
    ++str;
    }
  *ptr = '\0';
  return clean_str;
}

// Replace chars in string
char* SystemTools::ReplaceChars(char* str, const char *toreplace, char replacement)
{
  if (str)
    {
    char *ptr = str;
    while (*ptr)
      {
      const char *ptr2 = toreplace;
      while (*ptr2)
        {
        if (*ptr == *ptr2)
          {
          *ptr = replacement;
          }
        ++ptr2;
        }
      ++ptr;
      }
    }
  return str;
}

// Returns if string starts with another string
bool SystemTools::StringStartsWith(const char* str1, const char* str2)
{
  if (!str1 || !str2)
    {
    return false;
    }
  size_t len1 = strlen(str1), len2 = strlen(str2);
  return len1 >= len2 && !strncmp(str1, str2, len2) ? true : false;
}

// Returns if string ends with another string
bool SystemTools::StringEndsWith(const char* str1, const char* str2)
{
  if (!str1 || !str2)
    {
    return false;
    }
  size_t len1 = strlen(str1), len2 = strlen(str2);
  return len1 >= len2 &&  !strncmp(str1 + (len1 - len2), str2, len2) ? true : false;
}

// Returns a pointer to the last occurence of str2 in str1
const char* SystemTools::FindLastString(const char* str1, const char* str2)
{
  if (!str1 || !str2)
    {
    return NULL;
    }

  size_t len1 = strlen(str1), len2 = strlen(str2);
  if (len1 >= len2)
    {
    const char *ptr = str1 + len1 - len2;
    do
      {
      if (!strncmp(ptr, str2, len2))
        {
        return ptr;
        }
      } while (ptr-- != str1);
    }

  return NULL;
}

// Duplicate string
char* SystemTools::DuplicateString(const char* str)
{
  if (str)
    {
    char *newstr = new char [strlen(str) + 1];
    return strcpy(newstr, str);
    }
  return NULL;
}

// Return a cropped string
kwsys_stl::string SystemTools::CropString(const kwsys_stl::string& s,
                                          size_t max_len)
{
  if (!s.size() || max_len == 0 || max_len >= s.size())
    {
    return s;
    }

  kwsys_stl::string n;
  n.reserve(max_len);

  size_t middle = max_len / 2;

  n += s.substr(0, middle);
  n += s.substr(s.size() - (max_len - middle), kwsys_stl::string::npos);

  if (max_len > 2)
    {
    n[middle] = '.';
    if (max_len > 3)
      {
      n[middle - 1] = '.';
      if (max_len > 4)
        {
        n[middle + 1] = '.';
        }
      }
    }

  return n;
}

//----------------------------------------------------------------------------
kwsys_stl::vector<kwsys::String> SystemTools::SplitString(const char* p, char sep, bool isPath)
{
  kwsys_stl::string path = p;
  kwsys_stl::vector<kwsys::String> paths;
  if(path.empty())
    {
    return paths;
    }
  if(isPath && path[0] == '/')
    {
    path.erase(path.begin());
    paths.push_back("/");
    }
  kwsys_stl::string::size_type pos1 = 0;
  kwsys_stl::string::size_type pos2 = path.find(sep, pos1+1);
  while(pos2 != kwsys_stl::string::npos)
    {
    paths.push_back(path.substr(pos1, pos2-pos1));
    pos1 = pos2+1;
    pos2 = path.find(sep, pos1+1);
    }
  paths.push_back(path.substr(pos1, pos2-pos1));

  return paths;
}

//----------------------------------------------------------------------------
int SystemTools::EstimateFormatLength(const char *format, va_list ap)
{
  if (!format)
    {
    return 0;
    }

  // Quick-hack attempt at estimating the length of the string.
  // Should never under-estimate.

  // Start with the length of the format string itself.

  size_t length = strlen(format);

  // Increase the length for every argument in the format.

  const char* cur = format;
  while(*cur)
    {
    if(*cur++ == '%')
      {
      // Skip "%%" since it doesn't correspond to a va_arg.
      if(*cur != '%')
        {
        while(!int(isalpha(*cur)))
          {
          ++cur;
          }
        switch (*cur)
          {
          case 's':
          {
          // Check the length of the string.
          char* s = va_arg(ap, char*);
          if(s)
            {
            length += strlen(s);
            }
          } break;
          case 'e':
          case 'f':
          case 'g':
          {
          // Assume the argument contributes no more than 64 characters.
          length += 64;

          // Eat the argument.
          static_cast<void>(va_arg(ap, double));
          } break;
          default:
          {
          // Assume the argument contributes no more than 64 characters.
          length += 64;

          // Eat the argument.
          static_cast<void>(va_arg(ap, int));
          } break;
          }
        }

      // Move past the characters just tested.
      ++cur;
      }
    }

  return static_cast<int>(length);
}

kwsys_stl::string SystemTools::EscapeChars(
  const char *str,
  const char *chars_to_escape,
  char escape_char)
{
  kwsys_stl::string n;
  if (str)
    {
    if (!chars_to_escape | !*chars_to_escape)
      {
      n.append(str);
      }
    else
      {
      n.reserve(strlen(str));
      while (*str)
        {
        const char *ptr = chars_to_escape;
        while (*ptr)
          {
          if (*str == *ptr)
            {
            n += escape_char;
            break;
            }
          ++ptr;
          }
        n += *str;
        ++str;
        }
      }
    }
  return n;
}

#ifdef __VMS
static void ConvertVMSToUnix(kwsys_stl::string& path)
{
  kwsys_stl::string::size_type rootEnd = path.find(":[");
  kwsys_stl::string::size_type pathEnd = path.find("]");
  if(rootEnd != path.npos)
    {
    kwsys_stl::string root = path.substr(0, rootEnd);
    kwsys_stl::string pathPart = path.substr(rootEnd+2, pathEnd - rootEnd-2);
    const char* pathCString = pathPart.c_str();
    const char* pos0 = pathCString;
    for (kwsys_stl::string::size_type pos = 0; *pos0; ++ pos )
      {
      if ( *pos0 == '.' )
        {
        pathPart[pos] = '/';
        }
      pos0 ++;
      }
    path = "/"+ root + "/" + pathPart;
    }
}
#endif

// convert windows slashes to unix slashes
void SystemTools::ConvertToUnixSlashes(kwsys_stl::string& path)
{
  const char* pathCString = path.c_str();
  bool hasDoubleSlash = false;
#ifdef __VMS
  ConvertVMSToUnix(path);
#else
  const char* pos0 = pathCString;
  const char* pos1 = pathCString+1;
  for (kwsys_stl::string::size_type pos = 0; *pos0; ++ pos )
    {
    // make sure we don't convert an escaped space to a unix slash
    if ( *pos0 == '\\' && *pos1 != ' ' )
      {
      path[pos] = '/';
      }

    // Also, reuse the loop to check for slash followed by another slash
    if (*pos1 == '/' && *(pos1+1) == '/' && !hasDoubleSlash)
      {
#ifdef _WIN32
      // However, on windows if the first characters are both slashes,
      // then keep them that way, so that network paths can be handled.
      if ( pos > 0)
        {
        hasDoubleSlash = true;
        }
#else
      hasDoubleSlash = true;
#endif
      }

    pos0 ++;
    pos1 ++;
    }

  if ( hasDoubleSlash )
    {
    SystemTools::ReplaceString(path, "//", "/");
    }
#endif
  // remove any trailing slash
  if(!path.empty())
    {
    // if there is a tilda ~ then replace it with HOME
    pathCString = path.c_str();
    if(pathCString[0] == '~' && (pathCString[1] == '/' || pathCString[1] == '\0'))
      {
      const char* homeEnv = SystemTools::GetEnv("HOME");
      if (homeEnv)
        {
        path.replace(0,1,homeEnv);
        }
      }
#ifdef HAVE_GETPWNAM
    else if(pathCString[0] == '~')
      {
      kwsys_stl::string::size_type idx = path.find_first_of("/\0");
      kwsys_stl::string user = path.substr(1, idx-1);
      passwd* pw = getpwnam(user.c_str());
      if(pw)
        {
        path.replace(0, idx, pw->pw_dir);
        }
      }
#endif
    // remove trailing slash if the path is more than
    // a single /
    pathCString = path.c_str();
    if(path.size() > 1 && *(pathCString+(path.size()-1)) == '/')
      {
      // if it is c:/ then do not remove the trailing slash
      if(!((path.size() == 3 && pathCString[1] == ':')))
        {
        path = path.substr(0, path.size()-1);
        }
      }
    }
}

// change // to /, and escape any spaces in the path
kwsys_stl::string SystemTools::ConvertToUnixOutputPath(const char* path)
{
  kwsys_stl::string ret = path;

  // remove // except at the beginning might be a cygwin drive
  kwsys_stl::string::size_type pos=1;
  while((pos = ret.find("//", pos)) != kwsys_stl::string::npos)
    {
    ret.erase(pos, 1);
    }
  // escape spaces and () in the path
  if(ret.find_first_of(" ") != kwsys_stl::string::npos)
    {
    kwsys_stl::string result = "";
    char lastch = 1;
    for(const char* ch = ret.c_str(); *ch != '\0'; ++ch)
      {
        // if it is already escaped then don't try to escape it again
      if((*ch == ' ') && lastch != '\\')
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
  kwsys_stl::string ret;
  // make it big enough for all of path and double quotes
  ret.reserve(strlen(path)+3);
  // put path into the string
  ret.assign(path);
  ret = path;
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
  if(ret.find(' ') != kwsys_stl::string::npos
     && ret[0] != '\"')
    {
    ret.insert(static_cast<kwsys_stl::string::size_type>(0),
               static_cast<kwsys_stl::string::size_type>(1), '\"');
    ret.append(1, '\"');
    }
  return ret;
}

bool SystemTools::CopyFileIfDifferent(const char* source,
                                      const char* destination)
{
  // special check for a destination that is a directory
  // FilesDiffer does not handle file to directory compare
  if(SystemTools::FileIsDirectory(destination))
    {
    kwsys_stl::string new_destination = destination;
    SystemTools::ConvertToUnixSlashes(new_destination);
    new_destination += '/';
    kwsys_stl::string source_name = source;
    new_destination += SystemTools::GetFilenameName(source_name);
    if(SystemTools::FilesDiffer(source, new_destination.c_str()))
      {
      return SystemTools::CopyFileAlways(source, destination);
      }
    else
      {
      // the files are the same so the copy is done return
      // true
      return true;
      }
    }
  // source and destination are files so do a copy if they
  // are different
  if(SystemTools::FilesDiffer(source, destination))
    {
    return SystemTools::CopyFileAlways(source, destination);
    }
  // at this point the files must be the same so return true
  return true;
}

#define KWSYS_ST_BUFFER 4096

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

  // Compare the files a block at a time.
  char source_buf[KWSYS_ST_BUFFER];
  char dest_buf[KWSYS_ST_BUFFER];
  off_t nleft = statSource.st_size;
  while(nleft > 0)
    {
    // Read a block from each file.
    kwsys_ios::streamsize nnext = (nleft > KWSYS_ST_BUFFER)? KWSYS_ST_BUFFER : static_cast<kwsys_ios::streamsize>(nleft);
    finSource.read(source_buf, nnext);
    finDestination.read(dest_buf, nnext);

    // If either failed to read assume they are different.
    if(static_cast<kwsys_ios::streamsize>(finSource.gcount()) != nnext ||
       static_cast<kwsys_ios::streamsize>(finDestination.gcount()) != nnext)
      {
      return true;
      }

    // If this block differs the file differs.
    if(memcmp(static_cast<const void*>(source_buf),
              static_cast<const void*>(dest_buf),
              static_cast<size_t>(nnext)) != 0)
      {
      return true;
      }

    // Update the byte count remaining.
    nleft -= nnext;
    }

  // No differences found.
  return false;
}


//----------------------------------------------------------------------------
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

  if(!fout)
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

//----------------------------------------------------------------------------
bool SystemTools::CopyAFile(const char* source, const char* destination,
                            bool always)
{
  if(always)
    {
    return SystemTools::CopyFileAlways(source, destination);
    }
  else
    {
    return SystemTools::CopyFileIfDifferent(source, destination);
    }
}

/**
 * Copy a directory content from "source" directory to the directory named by
 * "destination".
 */
bool SystemTools::CopyADirectory(const char* source, const char* destination,
                                 bool always)
{
  Directory dir;
  dir.Load(source);
  size_t fileNum;
  if ( !SystemTools::MakeDirectory(destination) )
    {
    return false;
    }
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
        kwsys_stl::string fullDestPath = destination;
        fullDestPath += "/";
        fullDestPath += dir.GetFile(static_cast<unsigned long>(fileNum));
        if (!SystemTools::CopyADirectory(fullPath.c_str(),
                                         fullDestPath.c_str(),
                                         always))
          {
          return false;
          }
        }
      else
        {
        if(!SystemTools::CopyAFile(fullPath.c_str(), destination, always))
          {
          return false;
          }
        }
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
      return static_cast<unsigned long>(fs.st_size);
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

// return file's modified time
long int SystemTools::ModifiedTime(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0)
    {
    return 0;
    }
  else
    {
    return static_cast<long int>(fs.st_mtime);
    }
}

// return file's creation time
long int SystemTools::CreationTime(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0)
    {
    return 0;
    }
  else
    {
    return fs.st_ctime >= 0 ? static_cast<long int>(fs.st_ctime) : 0;
    }
}

bool SystemTools::ConvertDateMacroString(const char *str, time_t *tmt)
{
  if (!str || !tmt || strlen(str) > 11)
    {
    return false;
    }

  struct tm tmt2;

  // __DATE__
  // The compilation date of the current source file. The date is a string
  // literal of the form Mmm dd yyyy. The month name Mmm is the same as for
  // dates generated by the library function asctime declared in TIME.H.

  // index:   012345678901
  // format:  Mmm dd yyyy
  // example: Dec 19 2003

  static char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  char buffer[12];
  strcpy(buffer, str);

  buffer[3] = 0;
  char *ptr = strstr(month_names, buffer);
  if (!ptr)
    {
    return false;
    }

  int month = static_cast<int>((ptr - month_names) / 3);
  int day = atoi(buffer + 4);
  int year = atoi(buffer + 7);

  tmt2.tm_isdst = -1;
  tmt2.tm_hour  = 0;
  tmt2.tm_min   = 0;
  tmt2.tm_sec   = 0;
  tmt2.tm_wday  = 0;
  tmt2.tm_yday  = 0;
  tmt2.tm_mday  = day;
  tmt2.tm_mon   = month;
  tmt2.tm_year  = year - 1900;

  *tmt = mktime(&tmt2);
  return true;
}

bool SystemTools::ConvertTimeStampMacroString(const char *str, time_t *tmt)
{
  if (!str || !tmt || strlen(str) > 26)
    {
    return false;
    }

  struct tm tmt2;

  // __TIMESTAMP__
  // The date and time of the last modification of the current source file,
  // expressed as a string literal in the form Ddd Mmm Date hh:mm:ss yyyy,
  /// where Ddd is the abbreviated day of the week and Date is an integer
  // from 1 to 31.

  // index:   0123456789
  //                    0123456789
  //                              0123456789
  // format:  Ddd Mmm Date hh:mm:ss yyyy
  // example: Fri Dec 19 14:34:58 2003

  static char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  char buffer[27];
  strcpy(buffer, str);

  buffer[7] = 0;
  char *ptr = strstr(month_names, buffer + 4);
  if (!ptr)
    {
    return false;
    }

  int month = static_cast<int>((ptr - month_names) / 3);
  int day = atoi(buffer + 8);
  int hour = atoi(buffer + 11);
  int min = atoi(buffer + 14);
  int sec = atoi(buffer + 17);
  int year = atoi(buffer + 20);

  tmt2.tm_isdst = -1;
  tmt2.tm_hour  = hour;
  tmt2.tm_min   = min;
  tmt2.tm_sec   = sec;
  tmt2.tm_wday  = 0;
  tmt2.tm_yday  = 0;
  tmt2.tm_mday  = day;
  tmt2.tm_mon   = month;
  tmt2.tm_year  = year - 1900;

  *tmt = mktime(&tmt2);
  return true;
}

kwsys_stl::string SystemTools::GetLastSystemError()
{
  int e = errno;
  return strerror(e);
}

bool SystemTools::RemoveFile(const char* source)
{
#ifdef _WIN32
  mode_t mode;
  if ( !SystemTools::GetPermissions(source, mode) )
    {
    return false;
    }
  /* Win32 unlink is stupid --- it fails if the file is read-only  */
  SystemTools::SetPermissions(source, S_IWRITE);
#endif
  bool res = unlink(source) != 0 ? false : true;
#ifdef _WIN32
  if ( !res )
    {
    SystemTools::SetPermissions(source, mode);
    }
#endif
  return res;
}

bool SystemTools::RemoveADirectory(const char* source)
{
  // Add write permission to the directory so we can modify its
  // content to remove files and directories from it.
  mode_t mode;
  if(SystemTools::GetPermissions(source, mode))
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
    mode |= S_IWRITE;
#else
    mode |= S_IWUSR;
#endif
    SystemTools::SetPermissions(source, mode);
    }

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
      if(SystemTools::FileIsDirectory(fullPath.c_str()) &&
        !SystemTools::FileIsSymlink(fullPath.c_str()))
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
 */
size_t SystemTools::GetMaximumFilePathLength()
{
  return KWSYS_SYSTEMTOOLS_MAXPATH;
}

/**
 * Find the file the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the file if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_stl::string SystemTools
::FindName(const char* name,
           const kwsys_stl::vector<kwsys_stl::string>& userPaths,
           bool no_system_path)
{
  // Add the system search path to our path first
  kwsys_stl::vector<kwsys_stl::string> path;
  if (!no_system_path)
    {
    SystemTools::GetPath(path, "CMAKE_FILE_PATH");
    SystemTools::GetPath(path);
    }
  // now add the additional paths
  {
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator i = userPaths.begin();
        i != userPaths.end(); ++i)
    {
    path.push_back(*i);
    }
  }
  // Add a trailing slash to all paths to aid the search process.
  {
  for(kwsys_stl::vector<kwsys_stl::string>::iterator i = path.begin();
      i != path.end(); ++i)
    {
    kwsys_stl::string& p = *i;
    if(p.empty() || p[p.size()-1] != '/')
      {
      p += "/";
      }
    }
  }
  // now look for the file
  kwsys_stl::string tryPath;
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
    tryPath = *p;
    tryPath += name;
    if(SystemTools::FileExists(tryPath.c_str()))
      {
      return tryPath;
      }
    }
  // Couldn't find the file.
  return "";
}

/**
 * Find the file the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the file if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_stl::string SystemTools
::FindFile(const char* name,
           const kwsys_stl::vector<kwsys_stl::string>& userPaths,
           bool no_system_path)
{
  kwsys_stl::string tryPath = SystemTools::FindName(name, userPaths, no_system_path);
  if(tryPath != "" && !SystemTools::FileIsDirectory(tryPath.c_str()))
    {
    return SystemTools::CollapseFullPath(tryPath.c_str());
    }
  // Couldn't find the file.
  return "";
}

/**
 * Find the directory the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the directory if it is
 * found.  Otherwise, the empty string is returned.
 */
kwsys_stl::string SystemTools
::FindDirectory(const char* name,
                const kwsys_stl::vector<kwsys_stl::string>& userPaths,
                bool no_system_path)
{
  kwsys_stl::string tryPath = SystemTools::FindName(name, userPaths, no_system_path);
  if(tryPath != "" && SystemTools::FileIsDirectory(tryPath.c_str()))
    {
    return SystemTools::CollapseFullPath(tryPath.c_str());
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
  const char* nameIn,
  const kwsys_stl::vector<kwsys_stl::string>& userPaths,
  bool no_system_path)
{
  if(!nameIn || !*nameIn)
    {
    return "";
    }
  kwsys_stl::string name = nameIn;
  kwsys_stl::vector<kwsys_stl::string> extensions;
#if defined (_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  bool hasExtension = false;
  // check to see if the name already has a .xxx at
  // the end of it
  if(name.size() > 3 && name[name.size()-4] == '.')
    {
    hasExtension = true;
    }
  // on windows try .com then .exe
  if(!hasExtension)
    {
    extensions.push_back(".com");
    extensions.push_back(".exe");
    }
#endif
  kwsys_stl::string tryPath;

  // first try with extensions if the os supports them
  if(extensions.size())
    {
    for(kwsys_stl::vector<kwsys_stl::string>::iterator i =
          extensions.begin(); i != extensions.end(); ++i)
      {
      tryPath = name;
      tryPath += *i;
      if(SystemTools::FileExists(tryPath.c_str()) &&
         !SystemTools::FileIsDirectory(tryPath.c_str()))
        {
        return SystemTools::CollapseFullPath(tryPath.c_str());
        }
      }
    }
  // now try just the name
  tryPath = name;
  if(SystemTools::FileExists(tryPath.c_str()) &&
     !SystemTools::FileIsDirectory(tryPath.c_str()))
    {
    return SystemTools::CollapseFullPath(tryPath.c_str());
    }
  // now construct the path
  kwsys_stl::vector<kwsys_stl::string> path;
  // Add the system search path to our path.
  if (!no_system_path)
    {
    SystemTools::GetPath(path);
    }
  // now add the additional paths
  {
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator i =
        userPaths.begin();  i != userPaths.end(); ++i)
    {
    path.push_back(*i);
    }
  }
  // Add a trailing slash to all paths to aid the search process.
  {
  for(kwsys_stl::vector<kwsys_stl::string>::iterator i = path.begin();
      i != path.end(); ++i)
    {
    kwsys_stl::string& p = *i;
    if(p.empty() || p[p.size()-1] != '/')
      {
      p += "/";
      }
    }
  }
  // Try each path
  for(kwsys_stl::vector<kwsys_stl::string>::iterator p = path.begin();
      p != path.end(); ++p)
    {
#ifdef _WIN32
    // Remove double quotes from the path on windows
    SystemTools::ReplaceString(*p, "\"", "");
#endif
    // first try with extensions
    if(extensions.size())
      {
      for(kwsys_stl::vector<kwsys_stl::string>::iterator ext
            = extensions.begin(); ext != extensions.end(); ++ext)
        {
        tryPath = *p;
        tryPath += name;
        tryPath += *ext;
        if(SystemTools::FileExists(tryPath.c_str()) &&
           !SystemTools::FileIsDirectory(tryPath.c_str()))
          {
          return SystemTools::CollapseFullPath(tryPath.c_str());
          }
        }
      }
    // now try it without them
    tryPath = *p;
    tryPath += name;
    if(SystemTools::FileExists(tryPath.c_str()) &&
       !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    }
  // Couldn't find the program.
  return "";
}

kwsys_stl::string SystemTools::FindProgram(
  const kwsys_stl::vector<kwsys_stl::string>& names,
  const kwsys_stl::vector<kwsys_stl::string>& path,
  bool noSystemPath)
{
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator it = names.begin();
      it != names.end() ; ++it)
    {
    // Try to find the program.
    kwsys_stl::string result = SystemTools::FindProgram(it->c_str(),
                                                  path,
                                                  noSystemPath);
    if ( !result.empty() )
      {
      return result;
      }
    }
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
  {
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator i = userPaths.begin();
        i != userPaths.end(); ++i)
    {
    path.push_back(*i);
    }
  }
  // Add a trailing slash to all paths to aid the search process.
  {
  for(kwsys_stl::vector<kwsys_stl::string>::iterator i = path.begin();
      i != path.end(); ++i)
    {
    kwsys_stl::string& p = *i;
    if(p.empty() || p[p.size()-1] != '/')
      {
      p += "/";
      }
    }
  }
  kwsys_stl::string tryPath;
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
#if defined(__APPLE__)
    tryPath = *p;
    tryPath += name;
    tryPath += ".framework";
    if(SystemTools::FileExists(tryPath.c_str())
       && SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
#endif
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    tryPath = *p;
    tryPath += name;
    tryPath += ".lib";
    if(SystemTools::FileExists(tryPath.c_str())
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
#else
    tryPath = *p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".so";
    if(SystemTools::FileExists(tryPath.c_str())
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".a";
    if(SystemTools::FileExists(tryPath.c_str())
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".sl";
    if(SystemTools::FileExists(tryPath.c_str())
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".dylib";
    if(SystemTools::FileExists(tryPath.c_str())
       && !SystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return SystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "lib";
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

kwsys_stl::string SystemTools::GetRealPath(const char* path)
{
  kwsys_stl::string ret;
  Realpath(path, ret);
  return ret;
}

bool SystemTools::FileIsDirectory(const char* name)
{
  // Remove any trailing slash from the name.
  char buffer[KWSYS_SYSTEMTOOLS_MAXPATH];
  size_t last = strlen(name)-1;
  if(last > 0 && (name[last] == '/' || name[last] == '\\')
    && strcmp(name, "/") !=0)
    {
    memcpy(buffer, name, last);
    buffer[last] = 0;
    name = buffer;
    }

  // Now check the file node type.
  struct stat fs;
  if(stat(name, &fs) == 0)
    {
#if defined( _WIN32 ) && !defined(__CYGWIN__)
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

bool SystemTools::FileIsSymlink(const char* name)
{
#if defined( _WIN32 )
  (void)name;
  return false;
#else
  struct stat fs;
  if(lstat(name, &fs) == 0)
    {
    return S_ISLNK(fs.st_mode);
    }
  else
    {
    return false;
    }
#endif
}

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::CreateSymlink(const char*, const char*)
{
  return false;
}
#else
bool SystemTools::CreateSymlink(const char* origName, const char* newName)
{
  return symlink(origName, newName) >= 0;
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::ReadSymlink(const char*, kwsys_stl::string&)
{
  return false;
}
#else
bool SystemTools::ReadSymlink(const char* newName,
                              kwsys_stl::string& origName)
{
  char buf[KWSYS_SYSTEMTOOLS_MAXPATH+1];
  int count =
    static_cast<int>(readlink(newName, buf, KWSYS_SYSTEMTOOLS_MAXPATH));
  if(count >= 0)
    {
    // Add null-terminator.
    buf[count] = 0;
    origName = buf;
    return true;
    }
  else
    {
    return false;
    }
}
#endif

int SystemTools::ChangeDirectory(const char *dir)
{
  return Chdir(dir);
}

kwsys_stl::string SystemTools::GetCurrentWorkingDirectory(bool collapse)
{
  char buf[2048];
  const char* cwd = Getcwd(buf, 2048);
  kwsys_stl::string path;
  if ( cwd )
    {
    path = cwd;
    }
  if(collapse)
    {
    return SystemTools::CollapseFullPath(path.c_str());
    }
  return path;
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
  kwsys_stl::string self = argv0 ? argv0 : "";
  failures.push_back(self);
  SystemTools::ConvertToUnixSlashes(self);
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
    msg << "Can not find the command line program ";
    if (exeName)
      {
      msg << exeName;
      }
    msg << "\n";
    if (argv0)
      {
      msg << "  argv[0] = \"" << argv0 << "\"\n";
      }
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
    // Ken--the following code is incorrect. .. can be in a valid path
    // for example  /home/martink/MyHubba...Hubba/Src
    if( SystemTools::FileIsFullPath(path_b.c_str()) && path_b.find("..")
        == kwsys_stl::string::npos )
      {
      // Before inserting make sure path ends with '/'
      if(path_a.size() && path_a[path_a.size() -1] != '/')
        {
        path_a += '/';
        }
      if(path_b.size() && path_b[path_b.size() -1] != '/')
        {
        path_b += '/';
        }
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
  kwsys_stl::string cdir;
  Realpath(SystemTools::CollapseFullPath(dir).c_str(), cdir);
  SystemTools::AddTranslationPath(cdir.c_str(), dir);
}

void SystemTools::CheckTranslationPath(kwsys_stl::string & path)
{
  // Do not translate paths that are too short to have meaningful
  // translations.
  if(path.size() < 2)
    {
    return;
    }

  // Always add a trailing slash before translation.  It does not
  // matter if this adds an extra slash, but we do not want to
  // translate part of a directory (like the foo part of foo-dir).
  path += "/";

  // In case a file was specified we still have to go through this:
  // Now convert any path found in the table back to the one desired:
  kwsys_stl::map<kwsys_stl::string,kwsys_stl::string>::const_iterator it;
  for(it  = SystemTools::TranslationMap->begin();
      it != SystemTools::TranslationMap->end();
      ++it )
    {
    // We need to check of the path is a substring of the other path
    if(path.find( it->first ) == 0)
      {
      path = path.replace( 0, it->first.size(), it->second);
      }
    }

  // Remove the trailing slash we added before.
  path.erase(path.end()-1, path.end());
}

void
SystemToolsAppendComponents(
  kwsys_stl::vector<kwsys_stl::string>& out_components,
  kwsys_stl::vector<kwsys_stl::string>::const_iterator first,
  kwsys_stl::vector<kwsys_stl::string>::const_iterator last)
{
  for(kwsys_stl::vector<kwsys_stl::string>::const_iterator i = first;
      i != last; ++i)
    {
    if(*i == "..")
      {
      if(out_components.begin() != out_components.end())
        {
        out_components.erase(out_components.end()-1, out_components.end());
        }
      }
    else if(!(*i == ".") && !(*i == ""))
      {
      out_components.push_back(*i);
      }
    }
}

kwsys_stl::string SystemTools::CollapseFullPath(const char* in_path,
                                                const char* in_base)
{
  // Collect the output path components.
  kwsys_stl::vector<kwsys_stl::string> out_components;

  // Split the input path components.
  kwsys_stl::vector<kwsys_stl::string> path_components;
  SystemTools::SplitPath(in_path, path_components);

  // If the input path is relative, start with a base path.
  if(path_components[0].length() == 0)
    {
    kwsys_stl::vector<kwsys_stl::string> base_components;
    if(in_base)
      {
      // Use the given base path.
      SystemTools::SplitPath(in_base, base_components);
      }
    else
      {
      // Use the current working directory as a base path.
      char buf[2048];
      if(const char* cwd = Getcwd(buf, 2048))
        {
        SystemTools::SplitPath(cwd, base_components);
        }
      else
        {
        // ??
        }
      }

    // Append base path components to the output path.
    out_components.push_back(base_components[0]);
    SystemToolsAppendComponents(out_components,
                                base_components.begin()+1,
                                base_components.end());
    }

  // Append input path components to the output path.
  SystemToolsAppendComponents(out_components,
                              path_components.begin(),
                              path_components.end());

  // Transform the path back to a string.
  kwsys_stl::string newPath = SystemTools::JoinPath(out_components);

  // Update the translation table with this potentially new path.  I am not
  // sure why this line is here, it seems really questionable, but yet I
  // would put good money that if I remove it something will break, basically
  // from what I can see it created a mapping from the collapsed path, to be
  // replaced by the input path, which almost completely does the opposite of
  // this function, the only thing preventing this from happening a lot is
  // that if the in_path has a .. in it, then it is not added to the
  // translation table. So for most calls this either does nothing due to the
  // ..  or it adds a translation between identical paths as nothing was
  // collapsed, so I am going to try to comment it out, and see what hits the
  // fan, hopefully quickly.
  // Commented out line below:
  //SystemTools::AddTranslationPath(newPath.c_str(), in_path);

  SystemTools::CheckTranslationPath(newPath);
#ifdef _WIN32
  newPath = SystemTools::GetActualCaseForPath(newPath.c_str());
  SystemTools::ConvertToUnixSlashes(newPath);
#endif
  // Return the reconstructed path.
  return newPath;
}

// compute the relative path from here to there
kwsys_stl::string SystemTools::RelativePath(const char* local, const char* remote)
{
  if(!SystemTools::FileIsFullPath(local))
    {
    return "";
    }
  if(!SystemTools::FileIsFullPath(remote))
    {
    return "";
    }

  // split up both paths into arrays of strings using / as a separator
  kwsys_stl::vector<kwsys::String> localSplit = SystemTools::SplitString(local, '/', true);
  kwsys_stl::vector<kwsys::String> remoteSplit = SystemTools::SplitString(remote, '/', true);
  kwsys_stl::vector<kwsys::String> commonPath; // store shared parts of path in this array
  kwsys_stl::vector<kwsys::String> finalPath;  // store the final relative path here
  // count up how many matching directory names there are from the start
  unsigned int sameCount = 0;
  while(
    ((sameCount <= (localSplit.size()-1)) && (sameCount <= (remoteSplit.size()-1)))
    &&
// for windows and apple do a case insensitive string compare
#if defined(_WIN32) || defined(__APPLE__)
    SystemTools::Strucmp(localSplit[sameCount].c_str(),
                         remoteSplit[sameCount].c_str()) == 0
#else
    localSplit[sameCount] == remoteSplit[sameCount]
#endif
    )
    {
    // put the common parts of the path into the commonPath array
    commonPath.push_back(localSplit[sameCount]);
    // erase the common parts of the path from the original path arrays
    localSplit[sameCount] = "";
    remoteSplit[sameCount] = "";
    sameCount++;
    }

  // If there is nothing in common at all then just return the full
  // path.  This is the case only on windows when the paths have
  // different drive letters.  On unix two full paths always at least
  // have the root "/" in common so we will return a relative path
  // that passes through the root directory.
  if(sameCount == 0)
    {
    return remote;
    }

  // for each entry that is not common in the local path
  // add a ../ to the finalpath array, this gets us out of the local
  // path into the remote dir
  for(unsigned int i = 0; i < localSplit.size(); ++i)
    {
    if(localSplit[i].size())
      {
      finalPath.push_back("../");
      }
    }
  // for each entry that is not common in the remote path add it
  // to the final path.
  for(kwsys_stl::vector<String>::iterator vit = remoteSplit.begin();
      vit != remoteSplit.end(); ++vit)
    {
    if(vit->size())
      {
      finalPath.push_back(*vit);
      }
    }
  kwsys_stl::string relativePath;     // result string
  // now turn the array of directories into a unix path by puttint /
  // between each entry that does not already have one
  for(kwsys_stl::vector<String>::iterator vit1 = finalPath.begin();
      vit1 != finalPath.end(); ++vit1)
    {
    if(relativePath.size() && relativePath[relativePath.size()-1] != '/')
      {
      relativePath += "/";
      }
    relativePath += *vit1;
    }
  return relativePath;
}

#ifdef _WIN32
static int GetCasePathName(const kwsys_stl::string & pathIn,
                            kwsys_stl::string & casePath)
{
  kwsys_stl::vector<kwsys_stl::string> path_components;
  SystemTools::SplitPath(pathIn.c_str(), path_components);
  if(path_components[0].empty()) // First component always exists.
    {
    // Relative paths cannot be converted.
    casePath = "";
    return 0;
    }

  // Start with root component.
  kwsys_stl::vector<kwsys_stl::string>::size_type idx = 0;
  casePath = path_components[idx++];
  const char* sep = "";

  // If network path, fill casePath with server/share so FindFirstFile
  // will work after that.  Maybe someday call other APIs to get
  // actual case of servers and shares.
  if(path_components.size() > 2 && path_components[0] == "//")
    {
    casePath += path_components[idx++];
    casePath += "/";
    casePath += path_components[idx++];
    sep = "/";
    }

  for(; idx < path_components.size(); idx++)
    {
    casePath += sep;
    sep = "/";
    kwsys_stl::string test_str = casePath;
    test_str += path_components[idx];

    WIN32_FIND_DATA findData;
    HANDLE hFind = ::FindFirstFile(test_str.c_str(), &findData);
    if (INVALID_HANDLE_VALUE != hFind)
      {
      casePath += findData.cFileName;
      ::FindClose(hFind);
      }
    else
      {
      casePath = "";
      return 0;
      }
    }
  return (int)casePath.size();
}
#endif


//----------------------------------------------------------------------------
kwsys_stl::string SystemTools::GetActualCaseForPath(const char* p)
{
#ifndef _WIN32
  return p;
#else
  kwsys_stl::string casePath = p;
  // make sure drive letter is always upper case
  if(casePath.size() > 1 && casePath[1] == ':')
    {
    casePath[0] = toupper(casePath[0]);
    }

  // Check to see if actual case has already been called
  // for this path, and the result is stored in the LongPathMap
  SystemToolsTranslationMap::iterator i =
    SystemTools::LongPathMap->find(casePath);
  if(i != SystemTools::LongPathMap->end())
    {
    return i->second;
    }
  int len = GetCasePathName(p, casePath);
  if(len == 0 || len > MAX_PATH+1)
    {
    return p;
    }
  (*SystemTools::LongPathMap)[p] = casePath;
  return casePath;
#endif
}

//----------------------------------------------------------------------------
const char* SystemTools::SplitPathRootComponent(const char* p,
                                                kwsys_stl::string* root)
{
  // Identify the root component.
  const char* c = p;
  if((c[0] == '/' && c[1] == '/') || (c[0] == '\\' && c[1] == '\\'))
    {
    // Network path.
    if(root)
      {
      *root = "//";
      }
    c += 2;
    }
  else if(c[0] == '/' || c[0] == '\\')
    {
    // Unix path (or Windows path w/out drive letter).
    if(root)
      {
      *root = "/";
      }
    c += 1;
    }
  else if(c[0] && c[1] == ':' && (c[2] == '/' || c[2] == '\\'))
    {
    // Windows path.
    if(root)
      {
      (*root) = "_:/";
      (*root)[0] = c[0];
      }
    c += 3;
    }
  else if(c[0] && c[1] == ':')
    {
    // Path relative to a windows drive working directory.
    if(root)
      {
      (*root) = "_:";
      (*root)[0] = c[0];
      }
    c += 2;
    }
  else if(c[0] == '~')
    {
    // Home directory.  The returned root should always have a
    // trailing slash so that appending components as
    // c[0]c[1]/c[2]/... works.  The remaining path returned should
    // skip the first slash if it exists:
    //
    //   "~"    : root = "~/" , return ""
    //   "~/    : root = "~/" , return ""
    //   "~/x   : root = "~/" , return "x"
    //   "~u"   : root = "~u/", return ""
    //   "~u/"  : root = "~u/", return ""
    //   "~u/x" : root = "~u/", return "x"
    size_t n = 1;
    while(c[n] && c[n] != '/')
      {
      ++n;
      }
    if(root)
      {
      root->assign(c, n);
      *root += '/';
      }
    if(c[n] == '/')
      {
      ++n;
      }
    c += n;
    }
  else
    {
    // Relative path.
    if(root)
      {
      *root = "";
      }
    }

  // Return the remaining path.
  return c;
}

//----------------------------------------------------------------------------
void SystemTools::SplitPath(const char* p,
                            kwsys_stl::vector<kwsys_stl::string>& components,
                            bool expand_home_dir)
{
  const char* c = p;
  components.clear();

  // Identify the root component.
  {
  kwsys_stl::string root;
  c = SystemTools::SplitPathRootComponent(c, &root);

  // Expand home directory references if requested.
  if(expand_home_dir && !root.empty() && root[0] == '~')
    {
    kwsys_stl::string homedir;
    root = root.substr(0, root.size()-1);
    if(root.size() == 1)
      {
#if defined(_WIN32) && !defined(__CYGWIN__)
      if(const char* userp = getenv("USERPROFILE"))
        {
        homedir = userp;
        }
      else
#endif
      if(const char* h = getenv("HOME"))
        {
        homedir = h;
        }
      }
#ifdef HAVE_GETPWNAM
    else if(passwd* pw = getpwnam(root.c_str()+1))
      {
      if(pw->pw_dir)
        {
        homedir = pw->pw_dir;
        }
      }
#endif
    if(!homedir.empty() && (homedir[homedir.size()-1] == '/' ||
                            homedir[homedir.size()-1] == '\\'))
      {
      homedir = homedir.substr(0, homedir.size()-1);
      }
    SystemTools::SplitPath(homedir.c_str(), components);
    }
  else
    {
    components.push_back(root);
    }
  }

  // Parse the remaining components.
  const char* first = c;
  const char* last = first;
  for(;*last; ++last)
    {
    if(*last == '/' || *last == '\\')
      {
      // End of a component.  Save it.
      components.push_back(
        kwsys_stl::string(first,static_cast<kwsys_stl::string::size_type>(
                            last-first)));
      first = last+1;
      }
    }

  // Save the last component unless there were no components.
  if(last != c)
    {
    components.push_back(
      kwsys_stl::string(first,static_cast<kwsys_stl::string::size_type>(
                          last-first)));
    }
}

//----------------------------------------------------------------------------
kwsys_stl::string
SystemTools::JoinPath(const kwsys_stl::vector<kwsys_stl::string>& components)
{
  return SystemTools::JoinPath(components.begin(), components.end());
}

//----------------------------------------------------------------------------
kwsys_stl::string
SystemTools
::JoinPath(kwsys_stl::vector<kwsys_stl::string>::const_iterator first,
           kwsys_stl::vector<kwsys_stl::string>::const_iterator last)
{
  // Construct result in a single string.
  kwsys_stl::string result;

  // The first two components do not add a slash.
  if(first != last)
    {
    result.append(*first++);
    }
  if(first != last)
    {
    result.append(*first++);
    }

  // All remaining components are always separated with a slash.
  while(first != last)
    {
    result.append("/");
    result.append((*first++));
    }

  // Return the concatenated result.
  return result;
}

//----------------------------------------------------------------------------
bool SystemTools::ComparePath(const char* c1, const char* c2)
{
#if defined(_WIN32) || defined(__APPLE__)
# ifdef _MSC_VER
  return _stricmp(c1, c2) == 0;
# elif defined(__APPLE__) || defined(__GNUC__)
  return strcasecmp(c1, c2) == 0;
#else
  return SystemTools::Strucmp(c1, c2) == 0;
# endif
#else
  return strcmp(c1, c2) == 0;
#endif
}

//----------------------------------------------------------------------------
bool SystemTools::Split(const char* str, kwsys_stl::vector<kwsys_stl::string>& lines, char separator)
{
  kwsys_stl::string data(str);
  kwsys_stl::string::size_type lpos = 0;
  while(lpos < data.length())
    {
    kwsys_stl::string::size_type rpos = data.find_first_of(separator, lpos);
    if(rpos == kwsys_stl::string::npos)
      {
      // Line ends at end of string without a newline.
      lines.push_back(data.substr(lpos));
      return false;
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

//----------------------------------------------------------------------------
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
    kwsys_stl::string  ret = fn.substr(0, slash_pos);
    if(ret.size() == 2 && ret[1] == ':')
      {
      return ret + '/';
      }
    if(ret.size() == 0)
      {
      return "/";
      }
    return ret;
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
#if defined(_WIN32)
  kwsys_stl::string::size_type slash_pos = filename.find_last_of("/\\");
#else
  kwsys_stl::string::size_type slash_pos = filename.find_last_of("/");
#endif
  if(slash_pos != kwsys_stl::string::npos)
    {
    return filename.substr(slash_pos + 1);
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
 * Warning: this is the shortest extension (for example: .gz of .tar.gz)
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

bool SystemTools::FileHasSignature(const char *filename,
                                   const char *signature,
                                   long offset)
{
  if (!filename || !signature)
    {
    return false;
    }

  FILE *fp;
  fp = fopen(filename, "rb");
  if (!fp)
    {
    return false;
    }

  fseek(fp, offset, SEEK_SET);

  bool res = false;
  size_t signature_len = strlen(signature);
  char *buffer = new char [signature_len];

  if (fread(buffer, 1, signature_len, fp) == signature_len)
    {
    res = (!strncmp(buffer, signature, signature_len) ? true : false);
    }

  delete [] buffer;

  fclose(fp);
  return res;
}

SystemTools::FileTypeEnum
SystemTools::DetectFileType(const char *filename,
                            unsigned long length,
                            double percent_bin)
{
  if (!filename || percent_bin < 0)
    {
    return SystemTools::FileTypeUnknown;
    }

  FILE *fp;
  fp = fopen(filename, "rb");
  if (!fp)
    {
    return SystemTools::FileTypeUnknown;
    }

  // Allocate buffer and read bytes

  unsigned char *buffer = new unsigned char [length];
  size_t read_length = fread(buffer, 1, length, fp);
  fclose(fp);
  if (read_length == 0)
    {
    return SystemTools::FileTypeUnknown;
    }

  // Loop over contents and count

  size_t text_count = 0;

  const unsigned char *ptr = buffer;
  const unsigned char *buffer_end = buffer + read_length;

  while (ptr != buffer_end)
    {
    if ((*ptr >= 0x20 && *ptr <= 0x7F) ||
        *ptr == '\n' ||
        *ptr == '\r' ||
        *ptr == '\t')
      {
      text_count++;
      }
    ptr++;
    }

  delete [] buffer;

  double current_percent_bin =
    (static_cast<double>(read_length - text_count) /
     static_cast<double>(read_length));

  if (current_percent_bin >= percent_bin)
    {
    return SystemTools::FileTypeBinary;
    }

  return SystemTools::FileTypeText;
}

bool SystemTools::LocateFileInDir(const char *filename,
                                  const char *dir,
                                  kwsys_stl::string& filename_found,
                                  int try_filename_dirs)
{
  if (!filename || !dir)
    {
    return false;
    }

  // Get the basename of 'filename'

  kwsys_stl::string filename_base = SystemTools::GetFilenameName(filename);

  // Check if 'dir' is really a directory
  // If win32 and matches something like C:, accept it as a dir

  kwsys_stl::string real_dir;
  if (!SystemTools::FileIsDirectory(dir))
    {
#if defined( _WIN32 )
    size_t dir_len = strlen(dir);
    if (dir_len < 2 || dir[dir_len - 1] != ':')
      {
#endif
      real_dir = SystemTools::GetFilenamePath(dir);
      dir = real_dir.c_str();
#if defined( _WIN32 )
      }
#endif
    }

  // Try to find the file in 'dir'

  bool res = false;
  if (filename_base.size() && dir)
    {
    size_t dir_len = strlen(dir);
    int need_slash =
      (dir_len && dir[dir_len - 1] != '/' && dir[dir_len - 1] != '\\');

    kwsys_stl::string temp = dir;
    if (need_slash)
      {
      temp += "/";
      }
    temp += filename_base;

    if (SystemTools::FileExists(temp.c_str()))
      {
      res = true;
      filename_found = temp;
      }

    // If not found, we can try harder by appending part of the file to
    // to the directory to look inside.
    // Example: if we were looking for /foo/bar/yo.txt in /d1/d2, then
    // try to find yo.txt in /d1/d2/bar, then /d1/d2/foo/bar, etc.

    else if (try_filename_dirs)
      {
      kwsys_stl::string filename_dir(filename);
      kwsys_stl::string filename_dir_base;
      kwsys_stl::string filename_dir_bases;
      do
        {
        filename_dir = SystemTools::GetFilenamePath(filename_dir);
        filename_dir_base = SystemTools::GetFilenameName(filename_dir);
#if defined( _WIN32 )
        if (!filename_dir_base.size() ||
            filename_dir_base[filename_dir_base.size() - 1] == ':')
#else
        if (!filename_dir_base.size())
#endif
          {
          break;
          }

        filename_dir_bases = filename_dir_base + "/" + filename_dir_bases;

        temp = dir;
        if (need_slash)
          {
          temp += "/";
          }
        temp += filename_dir_bases;

        res = SystemTools::LocateFileInDir(
          filename_base.c_str(), temp.c_str(), filename_found, 0);

        } while (!res && filename_dir_base.size());
      }
    }

  return res;
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
#if !defined(_WIN32)
  if(name[0] == '~')
    {
    return true;
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
  return kwsys_stl::string(buf);
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

// Due to a buggy stream library on the HP and another on Mac OS X, we
// need this very carefully written version of getline.  Returns true
// if any data were read before the end-of-file was reached.
bool SystemTools::GetLineFromStream(kwsys_ios::istream& is,
                                    kwsys_stl::string& line,
                                    bool* has_newline /* = 0 */,
                                    long sizeLimit /* = -1 */)
{
  const int bufferSize = 1024;
  char buffer[bufferSize];
  bool haveData = false;
  bool haveNewline = false;

  // Start with an empty line.
  line = "";

  long leftToRead = sizeLimit;

  // Early short circuit return if stream is no good. Just return
  // false and the empty line. (Probably means caller tried to
  // create a file stream with a non-existent file name...)
  //
  if(!is)
    {
    if(has_newline)
      {
      *has_newline = false;
      }
    return false;
    }

  // If no characters are read from the stream, the end of file has
  // been reached.  Clear the fail bit just before reading.
  while(!haveNewline &&
        leftToRead != 0 &&
        (is.clear(is.rdstate() & ~kwsys_ios::ios::failbit),
         is.getline(buffer, bufferSize), is.gcount() > 0))
    {
    // We have read at least one byte.
    haveData = true;

    // If newline character was read the gcount includes the character
    // but the buffer does not: the end of line has been reached.
    size_t length = strlen(buffer);
    if(length < static_cast<size_t>(is.gcount()))
      {
      haveNewline = true;
      }

    // Avoid storing a carriage return character.
    if(length > 0 && buffer[length-1] == '\r')
      {
      buffer[length-1] = 0;
      }

    // if we read too much then truncate the buffer
    if (leftToRead > 0)
      {
      if (static_cast<long>(length) > leftToRead)
        {
        buffer[leftToRead-1] = 0;
        leftToRead = 0;
        }
      else
        {
        leftToRead -= static_cast<long>(length);
        }
      }

    // Append the data read to the line.
    line.append(buffer);
    }

  // Return the results.
  if(has_newline)
    {
    *has_newline = haveNewline;
    }
  return haveData;
}

int SystemTools::GetTerminalWidth()
{
  int width = -1;
#ifdef HAVE_TTY_INFO
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
      width = static_cast<int>(t);
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
  return SystemTools::GetFilenamePath(fileOrDir);
}

bool SystemTools::IsSubDirectory(const char* cSubdir, const char* cDir)
{
  if(!*cDir)
    {
    return false;
    }
  kwsys_stl::string subdir = cSubdir;
  kwsys_stl::string dir = cDir;
  SystemTools::ConvertToUnixSlashes(dir);
  kwsys_stl::string path = subdir;
  do
    {
    path = SystemTools::GetParentDirectory(path.c_str());
    if(SystemTools::ComparePath(dir.c_str(), path.c_str()))
      {
      return true;
      }
    }
  while ( path.size() > dir.size() );
  return false;
}

void SystemTools::Delay(unsigned int msec)
{
#ifdef _WIN32
  Sleep(msec);
#else
  // The sleep function gives 1 second resolution and the usleep
  // function gives 1e-6 second resolution but on some platforms has a
  // maximum sleep time of 1 second.  This could be re-implemented to
  // use select with masked signals or pselect to mask signals
  // atomically.  If select is given empty sets and zero as the max
  // file descriptor but a non-zero timeout it can be used to block
  // for a precise amount of time.
  if(msec >= 1000)
    {
    sleep(msec / 1000);
    usleep((msec % 1000) * 1000);
    }
  else
    {
    usleep(msec * 1000);
    }
#endif
}

void SystemTools::ConvertWindowsCommandLineToUnixArguments(
  const char *cmd_line, int *argc, char ***argv)
{
  if (!cmd_line || !argc || !argv)
    {
    return;
    }

  // A space delimites an argument except when it is inside a quote

  (*argc) = 1;

  size_t cmd_line_len = strlen(cmd_line);

  size_t i;
  for (i = 0; i < cmd_line_len; i++)
    {
    while (isspace(cmd_line[i]) && i < cmd_line_len)
      {
      i++;
      }
    if (i < cmd_line_len)
      {
      if (cmd_line[i] == '\"')
        {
        i++;
        while (cmd_line[i] != '\"' && i < cmd_line_len)
          {
          i++;
          }
        (*argc)++;
        }
      else
        {
        while (!isspace(cmd_line[i]) && i < cmd_line_len)
          {
          i++;
          }
        (*argc)++;
        }
      }
    }

  (*argv) = new char* [(*argc) + 1];
  (*argv)[(*argc)] = NULL;

  // Set the first arg to be the exec name

  (*argv)[0] = new char [1024];
#ifdef _WIN32
  ::GetModuleFileName(0, (*argv)[0], 1024);
#else
  (*argv)[0][0] = '\0';
#endif

  // Allocate the others

  int j;
  for (j = 1; j < (*argc); j++)
    {
    (*argv)[j] = new char [cmd_line_len + 10];
    }

  // Grab the args

  size_t pos;
  int argc_idx = 1;

  for (i = 0; i < cmd_line_len; i++)
    {
    while (isspace(cmd_line[i]) && i < cmd_line_len)
      {
      i++;
      }
    if (i < cmd_line_len)
      {
      if (cmd_line[i] == '\"')
        {
        i++;
        pos = i;
        while (cmd_line[i] != '\"' && i < cmd_line_len)
          {
          i++;
          }
        memcpy((*argv)[argc_idx], &cmd_line[pos], i - pos);
        (*argv)[argc_idx][i - pos] = '\0';
        argc_idx++;
        }
      else
        {
        pos = i;
        while (!isspace(cmd_line[i]) && i < cmd_line_len)
          {
          i++;
          }
        memcpy((*argv)[argc_idx], &cmd_line[pos], i - pos);
        (*argv)[argc_idx][i - pos] = '\0';
        argc_idx++;
        }
      }
    }
 }

kwsys_stl::string SystemTools::GetOperatingSystemNameAndVersion()
{
  kwsys_stl::string res;

#ifdef _WIN32
  char buffer[256];

  OSVERSIONINFOEX osvi;
  BOOL bOsVersionInfoEx;

  // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
  // If that fails, try using the OSVERSIONINFO structure.

  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
  if (!bOsVersionInfoEx)
    {
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx((OSVERSIONINFO *)&osvi))
      {
      return 0;
      }
    }

  switch (osvi.dwPlatformId)
    {
    // Test for the Windows NT product family.

    case VER_PLATFORM_WIN32_NT:

      // Test for the specific product family.

      if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
        {
#if (_MSC_VER >= 1300)
        if (osvi.wProductType == VER_NT_WORKSTATION)
          {
          res += "Microsoft Windows Vista";
          }
        else
          {
          res += "Microsoft Windows Server 2008 family";
          }
#else
        res += "Microsoft Windows Vista or Windows Server 2008";
#endif
        }

      if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
        {
        res += "Microsoft Windows Server 2003 family";
        }

      if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
        {
        res += "Microsoft Windows XP";
        }

      if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
        {
        res += "Microsoft Windows 2000";
        }

      if (osvi.dwMajorVersion <= 4)
        {
        res += "Microsoft Windows NT";
        }

      // Test for specific product on Windows NT 4.0 SP6 and later.

      if (bOsVersionInfoEx)
        {
        // Test for the workstation type.

#if (_MSC_VER >= 1300)
        if (osvi.wProductType == VER_NT_WORKSTATION)
          {
          if (osvi.dwMajorVersion == 4)
            {
            res += " Workstation 4.0";
            }
          else if (osvi.dwMajorVersion == 5)
            {
            if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
              {
              res += " Home Edition";
              }
            else
              {
              res += " Professional";
              }
            }
          }

        // Test for the server type.

        else if (osvi.wProductType == VER_NT_SERVER)
          {
          if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
            {
            if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
              {
              res += " Datacenter Edition";
              }
            else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
              {
              res += " Enterprise Edition";
              }
            else if (osvi.wSuiteMask == VER_SUITE_BLADE)
              {
              res += " Web Edition";
              }
            else
              {
              res += " Standard Edition";
              }
            }

          else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
            {
            if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
              {
              res += " Datacenter Server";
              }
            else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
              {
              res += " Advanced Server";
              }
            else
              {
              res += " Server";
              }
            }

          else if (osvi.dwMajorVersion <= 4)  // Windows NT 4.0
            {
            if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
              {
              res += " Server 4.0, Enterprise Edition";
              }
            else
              {
              res += " Server 4.0";
              }
            }
          }
#endif // Visual Studio 7 and up
        }

      // Test for specific product on Windows NT 4.0 SP5 and earlier

      else
        {
        HKEY hKey;
        #define BUFSIZE 80
        char szProductType[BUFSIZE];
        DWORD dwBufLen=BUFSIZE;
        LONG lRet;

        lRet = RegOpenKeyEx(
          HKEY_LOCAL_MACHINE,
          "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
          0, KEY_QUERY_VALUE, &hKey);
        if (lRet != ERROR_SUCCESS)
          {
          return 0;
          }

        lRet = RegQueryValueEx(hKey, "ProductType", NULL, NULL,
                               (LPBYTE) szProductType, &dwBufLen);

        if ((lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE))
          {
          return 0;
          }

        RegCloseKey(hKey);

        if (lstrcmpi("WINNT", szProductType) == 0)
          {
          res += " Workstation";
          }
        if (lstrcmpi("LANMANNT", szProductType) == 0)
          {
          res += " Server";
          }
        if (lstrcmpi("SERVERNT", szProductType) == 0)
          {
          res += " Advanced Server";
          }

        res += " ";
        sprintf(buffer, "%ld", osvi.dwMajorVersion);
        res += buffer;
        res += ".";
        sprintf(buffer, "%ld", osvi.dwMinorVersion);
        res += buffer;
        }

      // Display service pack (if any) and build number.

      if (osvi.dwMajorVersion == 4 &&
          lstrcmpi(osvi.szCSDVersion, "Service Pack 6") == 0)
        {
        HKEY hKey;
        LONG lRet;

        // Test for SP6 versus SP6a.

        lRet = RegOpenKeyEx(
          HKEY_LOCAL_MACHINE,
          "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
          0, KEY_QUERY_VALUE, &hKey);

        if (lRet == ERROR_SUCCESS)
          {
          res += " Service Pack 6a (Build ";
          sprintf(buffer, "%ld", osvi.dwBuildNumber & 0xFFFF);
          res += buffer;
          res += ")";
          }
        else // Windows NT 4.0 prior to SP6a
          {
          res += " ";
          res += osvi.szCSDVersion;
          res += " (Build ";
          sprintf(buffer, "%ld", osvi.dwBuildNumber & 0xFFFF);
          res += buffer;
          res += ")";
          }

        RegCloseKey(hKey);
        }
      else // Windows NT 3.51 and earlier or Windows 2000 and later
        {
        res += " ";
        res += osvi.szCSDVersion;
        res += " (Build ";
        sprintf(buffer, "%ld", osvi.dwBuildNumber & 0xFFFF);
        res += buffer;
        res += ")";
        }

      break;

      // Test for the Windows 95 product family.

    case VER_PLATFORM_WIN32_WINDOWS:

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
        {
        res += "Microsoft Windows 95";
        if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B')
          {
          res += " OSR2";
          }
        }

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
        {
        res += "Microsoft Windows 98";
        if (osvi.szCSDVersion[1] == 'A')
          {
          res += " SE";
          }
        }

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
        {
        res += "Microsoft Windows Millennium Edition";
        }
      break;

    case VER_PLATFORM_WIN32s:

      res +=  "Microsoft Win32s";
      break;
    }
#endif

  return res;
}

// ----------------------------------------------------------------------
bool SystemTools::ParseURLProtocol( const kwsys_stl::string& URL,
                                    kwsys_stl::string& protocol,
                                    kwsys_stl::string& dataglom )
{
  // match 0 entire url
  // match 1 protocol
  // match 2 dataglom following protocol://
  kwsys::RegularExpression urlRe( VTK_URL_PROTOCOL_REGEX );

  if ( ! urlRe.find( URL ) ) return false;

  protocol = urlRe.match( 1 );
  dataglom = urlRe.match( 2 );

  return true;
}

// ----------------------------------------------------------------------
bool SystemTools::ParseURL( const kwsys_stl::string& URL,
                            kwsys_stl::string& protocol,
                            kwsys_stl::string& username,
                            kwsys_stl::string& password,
                            kwsys_stl::string& hostname,
                            kwsys_stl::string& dataport,
                            kwsys_stl::string& database )
{
  kwsys::RegularExpression urlRe( VTK_URL_REGEX );
  if ( ! urlRe.find( URL ) ) return false;

  // match 0 URL
  // match 1 protocol
  // match 2 mangled user
  // match 3 username
  // match 4 mangled password
  // match 5 password
  // match 6 hostname
  // match 7 mangled port
  // match 8 dataport
  // match 9 database name

  protocol = urlRe.match( 1 );
  username = urlRe.match( 3 );
  password = urlRe.match( 5 );
  hostname = urlRe.match( 6 );
  dataport = urlRe.match( 8 );
  database = urlRe.match( 9 );

  return true;
}

// ----------------------------------------------------------------------
// These must NOT be initialized.  Default initialization to zero is
// necessary.
unsigned int SystemToolsManagerCount;
SystemToolsTranslationMap *SystemTools::TranslationMap;
SystemToolsTranslationMap *SystemTools::LongPathMap;
#ifdef __CYGWIN__
SystemToolsTranslationMap *SystemTools::Cyg2Win32Map;
#endif

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

#if defined(__VMS)
// On VMS we configure the run time C library to be more UNIX like.
// http://h71000.www7.hp.com/doc/732final/5763/5763pro_004.html
extern "C" int decc$feature_get_index(char *name);
extern "C" int decc$feature_set_value(int index, int mode, int value);
static int SetVMSFeature(char* name, int value)
{
  int i;
  errno = 0;
  i = decc$feature_get_index(name);
  return i >= 0 && (decc$feature_set_value(i, 1, value) >= 0 || errno == 0);
}
#endif

void SystemTools::ClassInitialize()
{
#ifdef __VMS
  SetVMSFeature("DECC$FILENAME_UNIX_ONLY", 1);
#endif
  // Allocate the translation map first.
  SystemTools::TranslationMap = new SystemToolsTranslationMap;
  SystemTools::LongPathMap = new SystemToolsTranslationMap;
#ifdef __CYGWIN__
  SystemTools::Cyg2Win32Map = new SystemToolsTranslationMap;
#endif

  // Add some special translation paths for unix.  These are not added
  // for windows because drive letters need to be maintained.  Also,
  // there are not sym-links and mount points on windows anyway.
#if !defined(_WIN32) || defined(__CYGWIN__)
  // The tmp path is frequently a logical path so always keep it:
  SystemTools::AddKeepPath("/tmp/");

  // If the current working directory is a logical path then keep the
  // logical name.
  if(const char* pwd = getenv("PWD"))
    {
    char buf[2048];
    if(const char* cwd = Getcwd(buf, 2048))
      {
      // The current working directory may be a logical path.  Find
      // the shortest logical path that still produces the correct
      // physical path.
      kwsys_stl::string cwd_changed;
      kwsys_stl::string pwd_changed;

      // Test progressively shorter logical-to-physical mappings.
      kwsys_stl::string pwd_str = pwd;
      kwsys_stl::string cwd_str = cwd;
      kwsys_stl::string pwd_path;
      Realpath(pwd, pwd_path);
      while(cwd_str == pwd_path && cwd_str != pwd_str)
        {
        // The current pair of paths is a working logical mapping.
        cwd_changed = cwd_str;
        pwd_changed = pwd_str;

        // Strip off one directory level and see if the logical
        // mapping still works.
        pwd_str = SystemTools::GetFilenamePath(pwd_str.c_str());
        cwd_str = SystemTools::GetFilenamePath(cwd_str.c_str());
        Realpath(pwd_str.c_str(), pwd_path);
        }

      // Add the translation to keep the logical path name.
      if(!cwd_changed.empty() && !pwd_changed.empty())
        {
        SystemTools::AddTranslationPath(cwd_changed.c_str(),
                                        pwd_changed.c_str());
        }
      }
    }
#endif
}

void SystemTools::ClassFinalize()
{
  delete SystemTools::TranslationMap;
  delete SystemTools::LongPathMap;
#ifdef __CYGWIN__
  delete SystemTools::Cyg2Win32Map;
#endif
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
  fprintf(stderr, "%s", message);
  fflush(stderr);
  return 1; // no further reporting required
}

void SystemTools::EnableMSVCDebugHook()
{
  if (getenv("DART_TEST_FROM_DART"))
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
