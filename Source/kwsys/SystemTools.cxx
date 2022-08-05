/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#ifdef __osf__
#  define _OSF_SOURCE
#  define _POSIX_C_SOURCE 199506L
#  define _XOPEN_SOURCE_EXTENDED
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__))
#  define KWSYS_WINDOWS_DIRS
#else
#  if defined(__SUNPRO_CC)
#    include <fcntl.h>
#  endif
#endif

#if defined(_WIN32) && !defined(_WIN32_WINNT)
#  define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#include "kwsysPrivate.h"
#include KWSYS_HEADER(RegularExpression.hxx)
#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(Directory.hxx)
#include KWSYS_HEADER(FStream.hxx)
#include KWSYS_HEADER(Encoding.h)
#include KWSYS_HEADER(Encoding.hxx)

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#ifdef _WIN32
#  include <cwchar>
#endif

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "Directory.hxx.in"
#  include "Encoding.hxx.in"
#  include "FStream.hxx.in"
#  include "RegularExpression.hxx.in"
#  include "SystemTools.hxx.in"
#endif

#ifdef _MSC_VER
#  pragma warning(disable : 4786)
#endif

#if defined(__sgi) && !defined(__GNUC__)
#  pragma set woff 1375 /* base class destructor not virtual */
#endif

#include <cctype>
#include <cerrno>
#ifdef __QNX__
#  include <malloc.h> /* for malloc/free on QNX */
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined(_WIN32) && !defined(_MSC_VER) && defined(__GNUC__)
#  include <strings.h> /* for strcasecmp */
#endif

#ifdef _MSC_VER
#  define umask _umask
#endif

// support for realpath call
#ifndef _WIN32
#  include <climits>
#  include <pwd.h>
#  include <sys/ioctl.h>
#  include <sys/time.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  include <utime.h>
#  ifndef __VMS
#    include <sys/param.h>
#    include <termios.h>
#  endif
#  include <csignal> /* sigprocmask */
#endif

#ifdef __linux
#  include <linux/fs.h>
#endif

#if defined(__APPLE__) &&                                                     \
  (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ - 0 >= 101200)
#  define KWSYS_SYSTEMTOOLS_HAVE_MACOS_COPYFILE_CLONE
#  include <copyfile.h>
#  include <sys/stat.h>
#endif

// Windows API.
#if defined(_WIN32)
#  include <windows.h>
#  include <winioctl.h>
#  ifndef INVALID_FILE_ATTRIBUTES
#    define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#  endif
#  ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#    define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE (0x2)
#  endif
#  if defined(_MSC_VER) && _MSC_VER >= 1800
#    define KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#  endif
#  ifndef IO_REPARSE_TAG_APPEXECLINK
#    define IO_REPARSE_TAG_APPEXECLINK (0x8000001BL)
#  endif
// from ntifs.h, which can only be used by drivers
typedef struct _REPARSE_DATA_BUFFER
{
  ULONG ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  union
  {
    struct
    {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      ULONG Flags;
      WCHAR PathBuffer[1];
    } SymbolicLinkReparseBuffer;
    struct
    {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      WCHAR PathBuffer[1];
    } MountPointReparseBuffer;
    struct
    {
      UCHAR DataBuffer[1];
    } GenericReparseBuffer;
    struct
    {
      ULONG Version;
      WCHAR StringList[1];
      // In version 3, there are 4 NUL-terminated strings:
      // * Package ID
      // * Entry Point
      // * Executable Path
      // * Application Type
    } AppExecLinkReparseBuffer;
  } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

namespace {
WCHAR* GetAppExecLink(PREPARSE_DATA_BUFFER data, size_t& len)
{
  // We only know the layout of version 3.
  if (data->AppExecLinkReparseBuffer.Version != 3) {
    return nullptr;
  }

  WCHAR* pstr = data->AppExecLinkReparseBuffer.StringList;

  // Skip the package id and entry point strings.
  for (int i = 0; i < 2; ++i) {
    len = std::wcslen(pstr);
    if (len == 0) {
      return nullptr;
    }
    pstr += len + 1;
  }

  // The third string is the executable path.
  len = std::wcslen(pstr);
  if (len == 0) {
    return nullptr;
  }
  return pstr;
}
}
#endif

#if !KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H
extern char** environ;
#endif

// getpwnam doesn't exist on Windows and Cray Xt3/Catamount
// same for TIOCGWINSZ
#if defined(_WIN32) || defined(__LIBCATAMOUNT__) ||                           \
  (defined(HAVE_GETPWNAM) && HAVE_GETPWNAM == 0)
#  undef HAVE_GETPWNAM
#  undef HAVE_TTY_INFO
#else
#  define HAVE_GETPWNAM 1
#  define HAVE_TTY_INFO 1
#endif

#define VTK_URL_PROTOCOL_REGEX "([a-zA-Z0-9]*)://(.*)"
#define VTK_URL_REGEX                                                         \
  "([a-zA-Z0-9]*)://(([A-Za-z0-9]+)(:([^:@]+))?@)?([^:@/]*)(:([0-9]+))?/"     \
  "(.+)?"
#define VTK_URL_BYTE_REGEX "%[0-9a-fA-F][0-9a-fA-F]"
#ifdef _MSC_VER
#  include <sys/utime.h>
#else
#  include <utime.h>
#endif

// This is a hack to prevent warnings about these functions being
// declared but not referenced.
#if defined(__sgi) && !defined(__GNUC__)
#  include <sys/termios.h>
namespace KWSYS_NAMESPACE {
class SystemToolsHack
{
public:
  enum
  {
    Ref1 = sizeof(cfgetospeed(0)),
    Ref2 = sizeof(cfgetispeed(0)),
    Ref3 = sizeof(tcgetattr(0, 0)),
    Ref4 = sizeof(tcsetattr(0, 0, 0)),
    Ref5 = sizeof(cfsetospeed(0, 0)),
    Ref6 = sizeof(cfsetispeed(0, 0))
  };
};
}
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__))
#  include <direct.h>
#  include <io.h>
#  define _unlink unlink
#endif

/* The maximum length of a file name.  */
#if defined(PATH_MAX)
#  define KWSYS_SYSTEMTOOLS_MAXPATH PATH_MAX
#elif defined(MAXPATHLEN)
#  define KWSYS_SYSTEMTOOLS_MAXPATH MAXPATHLEN
#else
#  define KWSYS_SYSTEMTOOLS_MAXPATH 16384
#endif

#if defined(__BEOS__) && !defined(__ZETA__)
#  include <be/kernel/OS.h>
#  include <be/storage/Path.h>

// BeOS 5 doesn't have usleep(), but it has snooze(), which is identical.
static inline void usleep(unsigned int msec)
{
  ::snooze(msec);
}

// BeOS 5 also doesn't have realpath(), but its C++ API offers something close.
static inline char* realpath(const char* path, char* resolved_path)
{
  const size_t maxlen = KWSYS_SYSTEMTOOLS_MAXPATH;
  snprintf(resolved_path, maxlen, "%s", path);
  BPath normalized(resolved_path, nullptr, true);
  const char* resolved = normalized.Path();
  if (resolved != nullptr) // nullptr == No such file.
  {
    if (snprintf(resolved_path, maxlen, "%s", resolved) < maxlen) {
      return resolved_path;
    }
  }
  return nullptr; // something went wrong.
}
#endif

#ifdef _WIN32
static time_t windows_filetime_to_posix_time(const FILETIME& ft)
{
  LARGE_INTEGER date;
  date.HighPart = ft.dwHighDateTime;
  date.LowPart = ft.dwLowDateTime;

  // removes the diff between 1970 and 1601
  date.QuadPart -= ((LONGLONG)(369 * 365 + 89) * 24 * 3600 * 10000000);

  // converts back from 100-nanoseconds to seconds
  return date.QuadPart / 10000000;
}
#endif

#ifdef KWSYS_WINDOWS_DIRS
#  include <wctype.h>
#  ifdef _MSC_VER
typedef KWSYS_NAMESPACE::SystemTools::mode_t mode_t;
#  endif

inline int Mkdir(const std::string& dir, const mode_t* mode)
{
  int ret =
    _wmkdir(KWSYS_NAMESPACE::Encoding::ToWindowsExtendedPath(dir).c_str());
  if (ret == 0 && mode)
    KWSYS_NAMESPACE::SystemTools::SetPermissions(dir, *mode);
  return ret;
}
inline int Rmdir(const std::string& dir)
{
  return _wrmdir(
    KWSYS_NAMESPACE::Encoding::ToWindowsExtendedPath(dir).c_str());
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  std::vector<wchar_t> w_buf(len);
  if (_wgetcwd(&w_buf[0], len)) {
    size_t nlen = kwsysEncoding_wcstombs(buf, &w_buf[0], len);
    if (nlen == static_cast<size_t>(-1)) {
      return 0;
    }
    if (nlen < len) {
      // make sure the drive letter is capital
      if (nlen > 1 && buf[1] == ':') {
        buf[0] = toupper(buf[0]);
      }
      return buf;
    }
  }
  return 0;
}
inline int Chdir(const std::string& dir)
{
  return _wchdir(KWSYS_NAMESPACE::Encoding::ToWide(dir).c_str());
}
inline void Realpath(const std::string& path, std::string& resolved_path,
                     std::string* errorMessage = nullptr)
{
  std::wstring tmp = KWSYS_NAMESPACE::Encoding::ToWide(path);
  wchar_t* ptemp;
  wchar_t fullpath[MAX_PATH];
  DWORD bufferLen = GetFullPathNameW(
    tmp.c_str(), sizeof(fullpath) / sizeof(fullpath[0]), fullpath, &ptemp);
  if (bufferLen < sizeof(fullpath) / sizeof(fullpath[0])) {
    resolved_path = KWSYS_NAMESPACE::Encoding::ToNarrow(fullpath);
    KWSYS_NAMESPACE::SystemTools::ConvertToUnixSlashes(resolved_path);
  } else if (errorMessage) {
    if (bufferLen) {
      *errorMessage = "Destination path buffer size too small.";
    } else if (unsigned int errorId = GetLastError()) {
      LPSTR message = nullptr;
      DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message, 0, nullptr);
      *errorMessage = std::string(message, size);
      LocalFree(message);
    } else {
      *errorMessage = "Unknown error.";
    }

    resolved_path = "";
  } else {
    resolved_path = path;
  }
}
#else
#  include <sys/types.h>

#  include <fcntl.h>
#  include <unistd.h>
inline int Mkdir(const std::string& dir, const mode_t* mode)
{
  return mkdir(dir.c_str(), mode ? *mode : 00777);
}
inline int Rmdir(const std::string& dir)
{
  return rmdir(dir.c_str());
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  return getcwd(buf, len);
}

inline int Chdir(const std::string& dir)
{
  return chdir(dir.c_str());
}
inline void Realpath(const std::string& path, std::string& resolved_path,
                     std::string* errorMessage = nullptr)
{
  char resolved_name[KWSYS_SYSTEMTOOLS_MAXPATH];

  errno = 0;
  char* ret = realpath(path.c_str(), resolved_name);
  if (ret) {
    resolved_path = ret;
  } else if (errorMessage) {
    if (errno) {
      *errorMessage = strerror(errno);
    } else {
      *errorMessage = "Unknown error.";
    }

    resolved_path = "";
  } else {
    // if path resolution fails, return what was passed in
    resolved_path = path;
  }
}
#endif

#if !defined(_WIN32) && defined(__COMO__)
// Hack for como strict mode to avoid defining _SVID_SOURCE or _BSD_SOURCE.
extern "C" {
extern FILE* popen(__const char* __command, __const char* __modes) __THROW;
extern int pclose(FILE* __stream) __THROW;
extern char* realpath(__const char* __restrict __name,
                      char* __restrict __resolved) __THROW;
extern char* strdup(__const char* __s) __THROW;
extern int putenv(char* __string) __THROW;
}
#endif

namespace KWSYS_NAMESPACE {

double SystemTools::GetTime()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  return (429.4967296 * ft.dwHighDateTime + 0.0000001 * ft.dwLowDateTime -
          11644473600.0);
#else
  struct timeval t;
  gettimeofday(&t, nullptr);
  return 1.0 * double(t.tv_sec) + 0.000001 * double(t.tv_usec);
#endif
}

/* Type of character storing the environment.  */
#if defined(_WIN32)
typedef wchar_t envchar;
#else
using envchar = char;
#endif

/* Order by environment key only (VAR from VAR=VALUE).  */
struct kwsysEnvCompare
{
  bool operator()(const envchar* l, const envchar* r) const
  {
#if defined(_WIN32)
    const wchar_t* leq = wcschr(l, L'=');
    const wchar_t* req = wcschr(r, L'=');
    size_t llen = leq ? (leq - l) : wcslen(l);
    size_t rlen = req ? (req - r) : wcslen(r);
    if (llen == rlen) {
      return wcsncmp(l, r, llen) < 0;
    } else {
      return wcscmp(l, r) < 0;
    }
#else
    const char* leq = strchr(l, '=');
    const char* req = strchr(r, '=');
    size_t llen = leq ? static_cast<size_t>(leq - l) : strlen(l);
    size_t rlen = req ? static_cast<size_t>(req - r) : strlen(r);
    if (llen == rlen) {
      return strncmp(l, r, llen) < 0;
    } else {
      return strcmp(l, r) < 0;
    }
#endif
  }
};

class kwsysEnvSet : public std::set<const envchar*, kwsysEnvCompare>
{
public:
  class Free
  {
    const envchar* Env;

  public:
    Free(const envchar* env)
      : Env(env)
    {
    }
    ~Free() { free(const_cast<envchar*>(this->Env)); }

    Free(const Free&) = delete;
    Free& operator=(const Free&) = delete;
  };

  const envchar* Release(const envchar* env)
  {
    const envchar* old = nullptr;
    auto i = this->find(env);
    if (i != this->end()) {
      old = *i;
      this->erase(i);
    }
    return old;
  }
};

#ifdef _WIN32
struct SystemToolsPathCaseCmp
{
  bool operator()(std::string const& l, std::string const& r) const
  {
#  ifdef _MSC_VER
    return _stricmp(l.c_str(), r.c_str()) < 0;
#  elif defined(__GNUC__)
    return strcasecmp(l.c_str(), r.c_str()) < 0;
#  else
    return SystemTools::Strucmp(l.c_str(), r.c_str()) < 0;
#  endif
  }
};
#endif

/**
 * SystemTools static variables singleton class.
 */
class SystemToolsStatic
{
public:
  using StringMap = std::map<std::string, std::string>;
#if KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP
  /**
   * Path translation table from dir to refdir
   * Each time 'dir' will be found it will be replace by 'refdir'
   */
  StringMap TranslationMap;
#endif
#ifdef _WIN32
  static std::string GetCasePathName(std::string const& pathIn);
  static std::string GetActualCaseForPathCached(std::string const& path);
  static const char* GetEnvBuffered(const char* key);
  std::map<std::string, std::string, SystemToolsPathCaseCmp> PathCaseMap;
  std::map<std::string, std::string> EnvMap;
#endif
#ifdef __CYGWIN__
  StringMap Cyg2Win32Map;
#endif

  /**
   * Actual implementation of ReplaceString.
   */
  static void ReplaceString(std::string& source, const char* replace,
                            size_t replaceSize, const std::string& with);

  /**
   * Actual implementation of FileIsFullPath.
   */
  static bool FileIsFullPath(const char*, size_t);

  /**
   * Find a filename (file or directory) in the system PATH, with
   * optional extra paths.
   */
  static std::string FindName(
    const std::string& name,
    const std::vector<std::string>& userPaths = std::vector<std::string>(),
    bool no_system_path = false);
};

// Do NOT initialize.  Default initialization to zero is necessary.
static SystemToolsStatic* SystemToolsStatics;

#ifdef _WIN32
std::string SystemToolsStatic::GetCasePathName(std::string const& pathIn)
{
  std::string casePath;

  // First check if the file is relative. We don't fix relative paths since the
  // real case depends on the root directory and the given path fragment may
  // have meaning elsewhere in the project.
  if (!SystemTools::FileIsFullPath(pathIn)) {
    // This looks unnecessary, but it allows for the return value optimization
    // since all return paths return the same local variable.
    casePath = pathIn;
    return casePath;
  }

  std::vector<std::string> path_components;
  SystemTools::SplitPath(pathIn, path_components);

  // Start with root component.
  std::vector<std::string>::size_type idx = 0;
  casePath = path_components[idx++];
  // make sure drive letter is always upper case
  if (casePath.size() > 1 && casePath[1] == ':') {
    casePath[0] = toupper(casePath[0]);
  }
  const char* sep = "";

  // If network path, fill casePath with server/share so FindFirstFile
  // will work after that.  Maybe someday call other APIs to get
  // actual case of servers and shares.
  if (path_components.size() > 2 && path_components[0] == "//") {
    casePath += path_components[idx++];
    casePath += "/";
    casePath += path_components[idx++];
    sep = "/";
  }

  // Convert case of all components that exist.
  bool converting = true;
  for (; idx < path_components.size(); idx++) {
    casePath += sep;
    sep = "/";

    if (converting) {
      // If path component contains wildcards, we skip matching
      // because these filenames are not allowed on windows,
      // and we do not want to match a different file.
      if (path_components[idx].find('*') != std::string::npos ||
          path_components[idx].find('?') != std::string::npos) {
        converting = false;
      } else {
        std::string test_str = casePath;
        test_str += path_components[idx];
        WIN32_FIND_DATAW findData;
        HANDLE hFind =
          ::FindFirstFileW(Encoding::ToWide(test_str).c_str(), &findData);
        if (INVALID_HANDLE_VALUE != hFind) {
          path_components[idx] = Encoding::ToNarrow(findData.cFileName);
          ::FindClose(hFind);
        } else {
          converting = false;
        }
      }
    }

    casePath += path_components[idx];
  }
  return casePath;
}

std::string SystemToolsStatic::GetActualCaseForPathCached(std::string const& p)
{
  // Check to see if actual case has already been called
  // for this path, and the result is stored in the PathCaseMap
  auto& pcm = SystemToolsStatics->PathCaseMap;
  {
    auto itr = pcm.find(p);
    if (itr != pcm.end()) {
      return itr->second;
    }
  }
  std::string casePath = SystemToolsStatic::GetCasePathName(p);
  if (casePath.size() <= MAX_PATH) {
    pcm[p] = casePath;
  }
  return casePath;
}
#endif

// adds the elements of the env variable path to the arg passed in
void SystemTools::GetPath(std::vector<std::string>& path, const char* env)
{
  size_t const old_size = path.size();
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char pathSep = ';';
#else
  const char pathSep = ':';
#endif
  if (!env) {
    env = "PATH";
  }
  std::string pathEnv;
  if (!SystemTools::GetEnv(env, pathEnv)) {
    return;
  }

  // A hack to make the below algorithm work.
  if (!pathEnv.empty() && pathEnv.back() != pathSep) {
    pathEnv += pathSep;
  }
  std::string::size_type start = 0;
  bool done = false;
  while (!done) {
    std::string::size_type endpos = pathEnv.find(pathSep, start);
    if (endpos != std::string::npos) {
      path.push_back(pathEnv.substr(start, endpos - start));
      start = endpos + 1;
    } else {
      done = true;
    }
  }
  for (auto i = path.begin() + old_size; i != path.end(); ++i) {
    SystemTools::ConvertToUnixSlashes(*i);
  }
}

#if defined(_WIN32)
const char* SystemToolsStatic::GetEnvBuffered(const char* key)
{
  std::string env;
  if (SystemTools::GetEnv(key, env)) {
    std::string& menv = SystemToolsStatics->EnvMap[key];
    if (menv != env) {
      menv = std::move(env);
    }
    return menv.c_str();
  }
  return nullptr;
}
#endif

const char* SystemTools::GetEnv(const char* key)
{
#if defined(_WIN32)
  return SystemToolsStatic::GetEnvBuffered(key);
#else
  return getenv(key);
#endif
}

const char* SystemTools::GetEnv(const std::string& key)
{
#if defined(_WIN32)
  return SystemToolsStatic::GetEnvBuffered(key.c_str());
#else
  return getenv(key.c_str());
#endif
}

bool SystemTools::GetEnv(const char* key, std::string& result)
{
#if defined(_WIN32)
  const std::wstring wkey = Encoding::ToWide(key);
  const wchar_t* wv = _wgetenv(wkey.c_str());
  if (wv) {
    result = Encoding::ToNarrow(wv);
    return true;
  }
#else
  const char* v = getenv(key);
  if (v) {
    result = v;
    return true;
  }
#endif
  return false;
}

bool SystemTools::GetEnv(const std::string& key, std::string& result)
{
  return SystemTools::GetEnv(key.c_str(), result);
}

bool SystemTools::HasEnv(const char* key)
{
#if defined(_WIN32)
  const std::wstring wkey = Encoding::ToWide(key);
  const wchar_t* v = _wgetenv(wkey.c_str());
#else
  const char* v = getenv(key);
#endif
  return v != nullptr;
}

bool SystemTools::HasEnv(const std::string& key)
{
  return SystemTools::HasEnv(key.c_str());
}

#if KWSYS_CXX_HAS_UNSETENV
/* unsetenv("A") removes A from the environment.
   On older platforms it returns void instead of int.  */
static int kwsysUnPutEnv(const std::string& env)
{
  size_t pos = env.find('=');
  if (pos != std::string::npos) {
    std::string name = env.substr(0, pos);
    unsetenv(name.c_str());
  } else {
    unsetenv(env.c_str());
  }
  return 0;
}

#elif defined(__CYGWIN__) || defined(__GLIBC__)
/* putenv("A") removes A from the environment.  It must not put the
   memory in the environment because it does not have any "=" syntax.  */
static int kwsysUnPutEnv(const std::string& env)
{
  int err = 0;
  size_t pos = env.find('=');
  size_t const len = pos == std::string::npos ? env.size() : pos;
  size_t const sz = len + 1;
  char local_buf[256];
  char* buf = sz > sizeof(local_buf) ? (char*)malloc(sz) : local_buf;
  if (!buf) {
    return -1;
  }
  strncpy(buf, env.c_str(), len);
  buf[len] = 0;
  if (putenv(buf) < 0 && errno != EINVAL) {
    err = errno;
  }
  if (buf != local_buf) {
    free(buf);
  }
  if (err) {
    errno = err;
    return -1;
  }
  return 0;
}

#elif defined(_WIN32)
/* putenv("A=") places "A=" in the environment, which is as close to
   removal as we can get with the putenv API.  We have to leak the
   most recent value placed in the environment for each variable name
   on program exit in case exit routines access it.  */

static kwsysEnvSet kwsysUnPutEnvSet;

static int kwsysUnPutEnv(std::string const& env)
{
  std::wstring wEnv = Encoding::ToWide(env);
  size_t const pos = wEnv.find('=');
  size_t const len = pos == std::string::npos ? wEnv.size() : pos;
  wEnv.resize(len + 1, L'=');
  wchar_t* newEnv = _wcsdup(wEnv.c_str());
  if (!newEnv) {
    return -1;
  }
  kwsysEnvSet::Free oldEnv(kwsysUnPutEnvSet.Release(newEnv));
  kwsysUnPutEnvSet.insert(newEnv);
  return _wputenv(newEnv);
}

#else
/* Manipulate the "environ" global directly.  */
static int kwsysUnPutEnv(const std::string& env)
{
  size_t pos = env.find('=');
  size_t const len = pos == std::string::npos ? env.size() : pos;
  int in = 0;
  int out = 0;
  while (environ[in]) {
    if (strlen(environ[in]) > len && environ[in][len] == '=' &&
        strncmp(env.c_str(), environ[in], len) == 0) {
      ++in;
    } else {
      environ[out++] = environ[in++];
    }
  }
  while (out < in) {
    environ[out++] = 0;
  }
  return 0;
}
#endif

#if KWSYS_CXX_HAS_SETENV

/* setenv("A", "B", 1) will set A=B in the environment and makes its
   own copies of the strings.  */
bool SystemTools::PutEnv(const std::string& env)
{
  size_t pos = env.find('=');
  if (pos != std::string::npos) {
    std::string name = env.substr(0, pos);
    return setenv(name.c_str(), env.c_str() + pos + 1, 1) == 0;
  } else {
    return kwsysUnPutEnv(env) == 0;
  }
}

bool SystemTools::UnPutEnv(const std::string& env)
{
  return kwsysUnPutEnv(env) == 0;
}

#else

/* putenv("A=B") will set A=B in the environment.  Most putenv implementations
   put their argument directly in the environment.  They never free the memory
   on program exit.  Keep an active set of pointers to memory we allocate and
   pass to putenv, one per environment key.  At program exit remove any
   environment values that may still reference memory we allocated.  Then free
   the memory.  This will not affect any environment values we never set.  */

#  ifdef __INTEL_COMPILER
#    pragma warning disable 444 /* base has non-virtual destructor */
#  endif

class kwsysEnv : public kwsysEnvSet
{
public:
  ~kwsysEnv()
  {
    for (iterator i = this->begin(); i != this->end(); ++i) {
#  if defined(_WIN32)
      const std::string s = Encoding::ToNarrow(*i);
      kwsysUnPutEnv(s);
#  else
      kwsysUnPutEnv(*i);
#  endif
      free(const_cast<envchar*>(*i));
    }
  }
  bool Put(const char* env)
  {
#  if defined(_WIN32)
    const std::wstring wEnv = Encoding::ToWide(env);
    wchar_t* newEnv = _wcsdup(wEnv.c_str());
#  else
    char* newEnv = strdup(env);
#  endif
    Free oldEnv(this->Release(newEnv));
    this->insert(newEnv);
#  if defined(_WIN32)
    return _wputenv(newEnv) == 0;
#  else
    return putenv(newEnv) == 0;
#  endif
  }
  bool UnPut(const char* env)
  {
#  if defined(_WIN32)
    const std::wstring wEnv = Encoding::ToWide(env);
    Free oldEnv(this->Release(wEnv.c_str()));
#  else
    Free oldEnv(this->Release(env));
#  endif
    return kwsysUnPutEnv(env) == 0;
  }
};

static kwsysEnv kwsysEnvInstance;

bool SystemTools::PutEnv(const std::string& env)
{
  return kwsysEnvInstance.Put(env.c_str());
}

bool SystemTools::UnPutEnv(const std::string& env)
{
  return kwsysEnvInstance.UnPut(env.c_str());
}

#endif

const char* SystemTools::GetExecutableExtension()
{
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__VMS)
  return ".exe";
#else
  return "";
#endif
}

FILE* SystemTools::Fopen(const std::string& file, const char* mode)
{
#ifdef _WIN32
  // Remove any 'e', which is supported on UNIX, but not Windows.
  std::wstring trimmedMode = Encoding::ToWide(mode);
  trimmedMode.erase(std::remove(trimmedMode.begin(), trimmedMode.end(), L'e'),
                    trimmedMode.end());
  return _wfopen(Encoding::ToWindowsExtendedPath(file).c_str(),
                 trimmedMode.c_str());
#else
  return fopen(file.c_str(), mode);
#endif
}

Status SystemTools::MakeDirectory(const char* path, const mode_t* mode)
{
  if (!path) {
    return Status::POSIX(EINVAL);
  }
  return SystemTools::MakeDirectory(std::string(path), mode);
}

Status SystemTools::MakeDirectory(std::string const& path, const mode_t* mode)
{
  if (path.empty()) {
    return Status::POSIX(EINVAL);
  }
  if (SystemTools::PathExists(path)) {
    if (SystemTools::FileIsDirectory(path)) {
      return Status::Success();
    }
    return Status::POSIX(EEXIST);
  }
  std::string dir = path;
  SystemTools::ConvertToUnixSlashes(dir);

  std::string::size_type pos = 0;
  std::string topdir;
  while ((pos = dir.find('/', pos)) != std::string::npos) {
    // all underlying functions use C strings, so temporarily
    // end the string here
    dir[pos] = '\0';

    Mkdir(dir, mode);
    dir[pos] = '/';

    ++pos;
  }
  topdir = dir;
  if (Mkdir(topdir, mode) != 0 && errno != EEXIST) {
    return Status::POSIX_errno();
  }

  return Status::Success();
}

// replace replace with with as many times as it shows up in source.
// write the result into source.
void SystemTools::ReplaceString(std::string& source,
                                const std::string& replace,
                                const std::string& with)
{
  // do while hangs if replaceSize is 0
  if (replace.empty()) {
    return;
  }

  SystemToolsStatic::ReplaceString(source, replace.c_str(), replace.size(),
                                   with);
}

void SystemTools::ReplaceString(std::string& source, const char* replace,
                                const char* with)
{
  // do while hangs if replaceSize is 0
  if (!*replace) {
    return;
  }

  SystemToolsStatic::ReplaceString(source, replace, strlen(replace),
                                   with ? with : "");
}

void SystemToolsStatic::ReplaceString(std::string& source, const char* replace,
                                      size_t replaceSize,
                                      const std::string& with)
{
  const char* src = source.c_str();
  char* searchPos = const_cast<char*>(strstr(src, replace));

  // get out quick if string is not found
  if (!searchPos) {
    return;
  }

  // perform replacements until done
  char* orig = strdup(src);
  char* currentPos = orig;
  searchPos = searchPos - src + orig;

  // initialize the result
  source.erase(source.begin(), source.end());
  do {
    *searchPos = '\0';
    source += currentPos;
    currentPos = searchPos + replaceSize;
    // replace
    source += with;
    searchPos = strstr(currentPos, replace);
  } while (searchPos);

  // copy any trailing text
  source += currentPos;
  free(orig);
}

#if defined(_WIN32) && !defined(__CYGWIN__)

#  if defined(KEY_WOW64_32KEY) && defined(KEY_WOW64_64KEY)
#    define KWSYS_ST_KEY_WOW64_32KEY KEY_WOW64_32KEY
#    define KWSYS_ST_KEY_WOW64_64KEY KEY_WOW64_64KEY
#  else
#    define KWSYS_ST_KEY_WOW64_32KEY 0x0200
#    define KWSYS_ST_KEY_WOW64_64KEY 0x0100
#  endif

static bool hasPrefix(const std::string& s, const char* pattern,
                      std::string::size_type spos)
{
  size_t plen = strlen(pattern);
  if (spos != plen)
    return false;
  return s.compare(0, plen, pattern) == 0;
}

static bool SystemToolsParseRegistryKey(const std::string& key,
                                        HKEY& primaryKey, std::wstring& second,
                                        std::string* valuename)
{
  size_t start = key.find('\\');
  if (start == std::string::npos) {
    return false;
  }

  size_t valuenamepos = key.find(';');
  if (valuenamepos != std::string::npos && valuename) {
    *valuename = key.substr(valuenamepos + 1);
  }

  second = Encoding::ToWide(key.substr(start + 1, valuenamepos - start - 1));

  if (hasPrefix(key, "HKEY_CURRENT_USER", start)) {
    primaryKey = HKEY_CURRENT_USER;
  } else if (hasPrefix(key, "HKEY_CURRENT_CONFIG", start)) {
    primaryKey = HKEY_CURRENT_CONFIG;
  } else if (hasPrefix(key, "HKEY_CLASSES_ROOT", start)) {
    primaryKey = HKEY_CLASSES_ROOT;
  } else if (hasPrefix(key, "HKEY_LOCAL_MACHINE", start)) {
    primaryKey = HKEY_LOCAL_MACHINE;
  } else if (hasPrefix(key, "HKEY_USERS", start)) {
    primaryKey = HKEY_USERS;
  }

  return true;
}

static DWORD SystemToolsMakeRegistryMode(DWORD mode,
                                         SystemTools::KeyWOW64 view)
{
  // only add the modes when on a system that supports Wow64.
  static FARPROC wow64p =
    GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process");
  if (wow64p == nullptr) {
    return mode;
  }

  if (view == SystemTools::KeyWOW64_32) {
    return mode | KWSYS_ST_KEY_WOW64_32KEY;
  } else if (view == SystemTools::KeyWOW64_64) {
    return mode | KWSYS_ST_KEY_WOW64_64KEY;
  }
  return mode;
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::GetRegistrySubKeys(const std::string& key,
                                     std::vector<std::string>& subkeys,
                                     KeyWOW64 view)
{
  HKEY primaryKey = HKEY_CURRENT_USER;
  std::wstring second;
  if (!SystemToolsParseRegistryKey(key, primaryKey, second, nullptr)) {
    return false;
  }

  HKEY hKey;
  if (RegOpenKeyExW(primaryKey, second.c_str(), 0,
                    SystemToolsMakeRegistryMode(KEY_READ, view),
                    &hKey) != ERROR_SUCCESS) {
    return false;
  } else {
    wchar_t name[1024];
    DWORD dwNameSize = sizeof(name) / sizeof(name[0]);

    DWORD i = 0;
    while (RegEnumKeyW(hKey, i, name, dwNameSize) == ERROR_SUCCESS) {
      subkeys.push_back(Encoding::ToNarrow(name));
      ++i;
    }

    RegCloseKey(hKey);
  }

  return true;
}
#else
bool SystemTools::GetRegistrySubKeys(const std::string&,
                                     std::vector<std::string>&, KeyWOW64)
{
  return false;
}
#endif

// Read a registry value.
// Example :
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will return the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will return the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::ReadRegistryValue(const std::string& key, std::string& value,
                                    KeyWOW64 view)
{
  bool valueset = false;
  HKEY primaryKey = HKEY_CURRENT_USER;
  std::wstring second;
  std::string valuename;
  if (!SystemToolsParseRegistryKey(key, primaryKey, second, &valuename)) {
    return false;
  }

  HKEY hKey;
  if (RegOpenKeyExW(primaryKey, second.c_str(), 0,
                    SystemToolsMakeRegistryMode(KEY_READ, view),
                    &hKey) != ERROR_SUCCESS) {
    return false;
  } else {
    DWORD dwType, dwSize;
    dwSize = 1023;
    wchar_t data[1024];
    if (RegQueryValueExW(hKey, Encoding::ToWide(valuename).c_str(), nullptr,
                         &dwType, (BYTE*)data, &dwSize) == ERROR_SUCCESS) {
      if (dwType == REG_SZ) {
        value = Encoding::ToNarrow(data);
        valueset = true;
      } else if (dwType == REG_EXPAND_SZ) {
        wchar_t expanded[1024];
        DWORD dwExpandedSize = sizeof(expanded) / sizeof(expanded[0]);
        if (ExpandEnvironmentStringsW(data, expanded, dwExpandedSize)) {
          value = Encoding::ToNarrow(expanded);
          valueset = true;
        }
      }
    }

    RegCloseKey(hKey);
  }

  return valueset;
}
#else
bool SystemTools::ReadRegistryValue(const std::string&, std::string&, KeyWOW64)
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
bool SystemTools::WriteRegistryValue(const std::string& key,
                                     const std::string& value, KeyWOW64 view)
{
  HKEY primaryKey = HKEY_CURRENT_USER;
  std::wstring second;
  std::string valuename;
  if (!SystemToolsParseRegistryKey(key, primaryKey, second, &valuename)) {
    return false;
  }

  HKEY hKey;
  DWORD dwDummy;
  wchar_t lpClass[] = L"";
  if (RegCreateKeyExW(primaryKey, second.c_str(), 0, lpClass,
                      REG_OPTION_NON_VOLATILE,
                      SystemToolsMakeRegistryMode(KEY_WRITE, view), nullptr,
                      &hKey, &dwDummy) != ERROR_SUCCESS) {
    return false;
  }

  std::wstring wvalue = Encoding::ToWide(value);
  if (RegSetValueExW(hKey, Encoding::ToWide(valuename).c_str(), 0, REG_SZ,
                     (CONST BYTE*)wvalue.c_str(),
                     (DWORD)(sizeof(wchar_t) * (wvalue.size() + 1))) ==
      ERROR_SUCCESS) {
    return true;
  }
  return false;
}
#else
bool SystemTools::WriteRegistryValue(const std::string&, const std::string&,
                                     KeyWOW64)
{
  return false;
}
#endif

// Delete a registry value.
// Example :
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will delete the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will delete the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool SystemTools::DeleteRegistryValue(const std::string& key, KeyWOW64 view)
{
  HKEY primaryKey = HKEY_CURRENT_USER;
  std::wstring second;
  std::string valuename;
  if (!SystemToolsParseRegistryKey(key, primaryKey, second, &valuename)) {
    return false;
  }

  HKEY hKey;
  if (RegOpenKeyExW(primaryKey, second.c_str(), 0,
                    SystemToolsMakeRegistryMode(KEY_WRITE, view),
                    &hKey) != ERROR_SUCCESS) {
    return false;
  } else {
    if (RegDeleteValue(hKey, (LPTSTR)valuename.c_str()) == ERROR_SUCCESS) {
      RegCloseKey(hKey);
      return true;
    }
  }
  return false;
}
#else
bool SystemTools::DeleteRegistryValue(const std::string&, KeyWOW64)
{
  return false;
}
#endif

bool SystemTools::SameFile(const std::string& file1, const std::string& file2)
{
#ifdef _WIN32
  HANDLE hFile1, hFile2;

  hFile1 =
    CreateFileW(Encoding::ToWide(file1).c_str(), GENERIC_READ, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  hFile2 =
    CreateFileW(Encoding::ToWide(file2).c_str(), GENERIC_READ, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (hFile1 == INVALID_HANDLE_VALUE || hFile2 == INVALID_HANDLE_VALUE) {
    if (hFile1 != INVALID_HANDLE_VALUE) {
      CloseHandle(hFile1);
    }
    if (hFile2 != INVALID_HANDLE_VALUE) {
      CloseHandle(hFile2);
    }
    return false;
  }

  BY_HANDLE_FILE_INFORMATION fiBuf1;
  BY_HANDLE_FILE_INFORMATION fiBuf2;
  GetFileInformationByHandle(hFile1, &fiBuf1);
  GetFileInformationByHandle(hFile2, &fiBuf2);
  CloseHandle(hFile1);
  CloseHandle(hFile2);
  return (fiBuf1.dwVolumeSerialNumber == fiBuf2.dwVolumeSerialNumber &&
          fiBuf1.nFileIndexHigh == fiBuf2.nFileIndexHigh &&
          fiBuf1.nFileIndexLow == fiBuf2.nFileIndexLow);
#else
  struct stat fileStat1, fileStat2;
  if (stat(file1.c_str(), &fileStat1) == 0 &&
      stat(file2.c_str(), &fileStat2) == 0) {
    // see if the files are the same file
    // check the device inode and size
    if (memcmp(&fileStat2.st_dev, &fileStat1.st_dev,
               sizeof(fileStat1.st_dev)) == 0 &&
        memcmp(&fileStat2.st_ino, &fileStat1.st_ino,
               sizeof(fileStat1.st_ino)) == 0 &&
        fileStat2.st_size == fileStat1.st_size) {
      return true;
    }
  }
  return false;
#endif
}

bool SystemTools::PathExists(const std::string& path)
{
  if (path.empty()) {
    return false;
  }
#if defined(_WIN32)
  return (GetFileAttributesW(Encoding::ToWindowsExtendedPath(path).c_str()) !=
          INVALID_FILE_ATTRIBUTES);
#else
  struct stat st;
  return lstat(path.c_str(), &st) == 0;
#endif
}

bool SystemTools::FileExists(const char* filename)
{
  if (!filename) {
    return false;
  }
  return SystemTools::FileExists(std::string(filename));
}

bool SystemTools::FileExists(const std::string& filename)
{
  if (filename.empty()) {
    return false;
  }
#if defined(_WIN32)
  const std::wstring path = Encoding::ToWindowsExtendedPath(filename);
  DWORD attr = GetFileAttributesW(path.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES) {
    return false;
  }

  if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
    // Using 0 instead of GENERIC_READ as it allows reading of file attributes
    // even if we do not have permission to read the file itself
    HANDLE handle = CreateFileW(path.c_str(), 0, 0, nullptr, OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS, nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
      // A reparse point may be an execution alias (Windows Store app), which
      // is similar to a symlink but it cannot be opened as a regular file.
      // We must look at the reparse point data explicitly.
      handle = CreateFileW(
        path.c_str(), 0, 0, nullptr, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);

      if (handle == INVALID_HANDLE_VALUE) {
        return false;
      }

      byte buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
      DWORD bytesReturned = 0;

      if (!DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0, buffer,
                           MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned,
                           nullptr)) {
        CloseHandle(handle);
        return false;
      }

      CloseHandle(handle);

      PREPARSE_DATA_BUFFER data =
        reinterpret_cast<PREPARSE_DATA_BUFFER>(&buffer[0]);

      // Assume that file exists if it is an execution alias.
      return data->ReparseTag == IO_REPARSE_TAG_APPEXECLINK;
    }

    CloseHandle(handle);
  }

  return true;
#else
// SCO OpenServer 5.0.7/3.2's command has 711 permission.
#  if defined(_SCO_DS)
  return access(filename.c_str(), F_OK) == 0;
#  else
  return access(filename.c_str(), R_OK) == 0;
#  endif
#endif
}

bool SystemTools::FileExists(const char* filename, bool isFile)
{
  if (!filename) {
    return false;
  }
  return SystemTools::FileExists(std::string(filename), isFile);
}

bool SystemTools::FileExists(const std::string& filename, bool isFile)
{
  if (SystemTools::FileExists(filename)) {
    // If isFile is set return not FileIsDirectory,
    // so this will only be true if it is a file
    return !isFile || !SystemTools::FileIsDirectory(filename);
  }
  return false;
}

bool SystemTools::TestFileAccess(const char* filename,
                                 TestFilePermissions permissions)
{
  if (!filename) {
    return false;
  }
  return SystemTools::TestFileAccess(std::string(filename), permissions);
}

bool SystemTools::TestFileAccess(const std::string& filename,
                                 TestFilePermissions permissions)
{
  if (filename.empty()) {
    return false;
  }
#if defined(_WIN32) && !defined(__CYGWIN__)
  // If execute set, change to read permission (all files on Windows
  // are executable if they are readable).  The CRT will always fail
  // if you pass an execute bit.
  if (permissions & TEST_FILE_EXECUTE) {
    permissions &= ~TEST_FILE_EXECUTE;
    permissions |= TEST_FILE_READ;
  }
  return _waccess(Encoding::ToWindowsExtendedPath(filename).c_str(),
                  permissions) == 0;
#else
  return access(filename.c_str(), permissions) == 0;
#endif
}

int SystemTools::Stat(const char* path, SystemTools::Stat_t* buf)
{
  if (!path) {
    errno = EFAULT;
    return -1;
  }
  return SystemTools::Stat(std::string(path), buf);
}

int SystemTools::Stat(const std::string& path, SystemTools::Stat_t* buf)
{
  if (path.empty()) {
    errno = ENOENT;
    return -1;
  }
#if defined(_WIN32) && !defined(__CYGWIN__)
  // Ideally we should use Encoding::ToWindowsExtendedPath to support
  // long paths, but _wstat64 rejects paths with '?' in them, thinking
  // they are wildcards.
  std::wstring const& wpath = Encoding::ToWide(path);
  return _wstat64(wpath.c_str(), buf);
#else
  return stat(path.c_str(), buf);
#endif
}

Status SystemTools::Touch(std::string const& filename, bool create)
{
  if (!SystemTools::FileExists(filename)) {
    if (create) {
      FILE* file = Fopen(filename, "a+b");
      if (file) {
        fclose(file);
        return Status::Success();
      }
      return Status::POSIX_errno();
    } else {
      return Status::Success();
    }
  }
#if defined(_WIN32) && !defined(__CYGWIN__)
  HANDLE h = CreateFileW(Encoding::ToWindowsExtendedPath(filename).c_str(),
                         FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, 0,
                         OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
  if (!h) {
    return Status::Windows_GetLastError();
  }
  FILETIME mtime;
  GetSystemTimeAsFileTime(&mtime);
  if (!SetFileTime(h, 0, 0, &mtime)) {
    Status status = Status::Windows_GetLastError();
    CloseHandle(h);
    return status;
  }
  CloseHandle(h);
#elif KWSYS_CXX_HAS_UTIMENSAT
  // utimensat is only available on newer Unixes and macOS 10.13+
  if (utimensat(AT_FDCWD, filename.c_str(), nullptr, 0) < 0) {
    return Status::POSIX_errno();
  }
#else
  // fall back to utimes
  if (utimes(filename.c_str(), nullptr) < 0) {
    return Status::POSIX_errno();
  }
#endif
  return Status::Success();
}

Status SystemTools::FileTimeCompare(std::string const& f1,
                                    std::string const& f2, int* result)
{
  // Default to same time.
  *result = 0;
#if !defined(_WIN32) || defined(__CYGWIN__)
  // POSIX version.  Use stat function to get file modification time.
  struct stat s1;
  if (stat(f1.c_str(), &s1) != 0) {
    return Status::POSIX_errno();
  }
  struct stat s2;
  if (stat(f2.c_str(), &s2) != 0) {
    return Status::POSIX_errno();
  }
#  if KWSYS_CXX_STAT_HAS_ST_MTIM
  // Compare using nanosecond resolution.
  if (s1.st_mtim.tv_sec < s2.st_mtim.tv_sec) {
    *result = -1;
  } else if (s1.st_mtim.tv_sec > s2.st_mtim.tv_sec) {
    *result = 1;
  } else if (s1.st_mtim.tv_nsec < s2.st_mtim.tv_nsec) {
    *result = -1;
  } else if (s1.st_mtim.tv_nsec > s2.st_mtim.tv_nsec) {
    *result = 1;
  }
#  elif KWSYS_CXX_STAT_HAS_ST_MTIMESPEC
  // Compare using nanosecond resolution.
  if (s1.st_mtimespec.tv_sec < s2.st_mtimespec.tv_sec) {
    *result = -1;
  } else if (s1.st_mtimespec.tv_sec > s2.st_mtimespec.tv_sec) {
    *result = 1;
  } else if (s1.st_mtimespec.tv_nsec < s2.st_mtimespec.tv_nsec) {
    *result = -1;
  } else if (s1.st_mtimespec.tv_nsec > s2.st_mtimespec.tv_nsec) {
    *result = 1;
  }
#  else
  // Compare using 1 second resolution.
  if (s1.st_mtime < s2.st_mtime) {
    *result = -1;
  } else if (s1.st_mtime > s2.st_mtime) {
    *result = 1;
  }
#  endif
#else
  // Windows version.  Get the modification time from extended file attributes.
  WIN32_FILE_ATTRIBUTE_DATA f1d;
  WIN32_FILE_ATTRIBUTE_DATA f2d;
  if (!GetFileAttributesExW(Encoding::ToWindowsExtendedPath(f1).c_str(),
                            GetFileExInfoStandard, &f1d)) {
    return Status::Windows_GetLastError();
  }
  if (!GetFileAttributesExW(Encoding::ToWindowsExtendedPath(f2).c_str(),
                            GetFileExInfoStandard, &f2d)) {
    return Status::Windows_GetLastError();
  }

  // Compare the file times using resolution provided by system call.
  *result = (int)CompareFileTime(&f1d.ftLastWriteTime, &f2d.ftLastWriteTime);
#endif
  return Status::Success();
}

// Return a capitalized string (i.e the first letter is uppercased, all other
// are lowercased)
std::string SystemTools::Capitalized(const std::string& s)
{
  std::string n;
  if (s.empty()) {
    return n;
  }
  n.resize(s.size());
  n[0] = static_cast<std::string::value_type>(toupper(s[0]));
  for (size_t i = 1; i < s.size(); i++) {
    n[i] = static_cast<std::string::value_type>(tolower(s[i]));
  }
  return n;
}

// Return capitalized words
std::string SystemTools::CapitalizedWords(const std::string& s)
{
  std::string n(s);
  for (size_t i = 0; i < s.size(); i++) {
#if defined(_MSC_VER) && defined(_MT) && defined(_DEBUG)
    // MS has an assert that will fail if s[i] < 0; setting
    // LC_CTYPE using setlocale() does *not* help. Painful.
    if ((int)s[i] >= 0 && isalpha(s[i]) &&
        (i == 0 || ((int)s[i - 1] >= 0 && isspace(s[i - 1]))))
#else
    if (isalpha(s[i]) && (i == 0 || isspace(s[i - 1])))
#endif
    {
      n[i] = static_cast<std::string::value_type>(toupper(s[i]));
    }
  }
  return n;
}

// Return uncapitalized words
std::string SystemTools::UnCapitalizedWords(const std::string& s)
{
  std::string n(s);
  for (size_t i = 0; i < s.size(); i++) {
#if defined(_MSC_VER) && defined(_MT) && defined(_DEBUG)
    // MS has an assert that will fail if s[i] < 0; setting
    // LC_CTYPE using setlocale() does *not* help. Painful.
    if ((int)s[i] >= 0 && isalpha(s[i]) &&
        (i == 0 || ((int)s[i - 1] >= 0 && isspace(s[i - 1]))))
#else
    if (isalpha(s[i]) && (i == 0 || isspace(s[i - 1])))
#endif
    {
      n[i] = static_cast<std::string::value_type>(tolower(s[i]));
    }
  }
  return n;
}

// only works for words with at least two letters
std::string SystemTools::AddSpaceBetweenCapitalizedWords(const std::string& s)
{
  std::string n;
  if (!s.empty()) {
    n.reserve(s.size());
    n += s[0];
    for (size_t i = 1; i < s.size(); i++) {
      if (isupper(s[i]) && !isspace(s[i - 1]) && !isupper(s[i - 1])) {
        n += ' ';
      }
      n += s[i];
    }
  }
  return n;
}

char* SystemTools::AppendStrings(const char* str1, const char* str2)
{
  if (!str1) {
    return SystemTools::DuplicateString(str2);
  }
  if (!str2) {
    return SystemTools::DuplicateString(str1);
  }
  size_t len1 = strlen(str1);
  char* newstr = new char[len1 + strlen(str2) + 1];
  if (!newstr) {
    return nullptr;
  }
  strcpy(newstr, str1);
  strcat(newstr + len1, str2);
  return newstr;
}

char* SystemTools::AppendStrings(const char* str1, const char* str2,
                                 const char* str3)
{
  if (!str1) {
    return SystemTools::AppendStrings(str2, str3);
  }
  if (!str2) {
    return SystemTools::AppendStrings(str1, str3);
  }
  if (!str3) {
    return SystemTools::AppendStrings(str1, str2);
  }

  size_t len1 = strlen(str1), len2 = strlen(str2);
  char* newstr = new char[len1 + len2 + strlen(str3) + 1];
  if (!newstr) {
    return nullptr;
  }
  strcpy(newstr, str1);
  strcat(newstr + len1, str2);
  strcat(newstr + len1 + len2, str3);
  return newstr;
}

// Return a lower case string
std::string SystemTools::LowerCase(const std::string& s)
{
  std::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++) {
    n[i] = static_cast<std::string::value_type>(tolower(s[i]));
  }
  return n;
}

// Return a lower case string
std::string SystemTools::UpperCase(const std::string& s)
{
  std::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++) {
    n[i] = static_cast<std::string::value_type>(toupper(s[i]));
  }
  return n;
}

// Count char in string
size_t SystemTools::CountChar(const char* str, char c)
{
  size_t count = 0;

  if (str) {
    while (*str) {
      if (*str == c) {
        ++count;
      }
      ++str;
    }
  }
  return count;
}

// Remove chars in string
char* SystemTools::RemoveChars(const char* str, const char* toremove)
{
  if (!str) {
    return nullptr;
  }
  char* clean_str = new char[strlen(str) + 1];
  char* ptr = clean_str;
  while (*str) {
    const char* str2 = toremove;
    while (*str2 && *str != *str2) {
      ++str2;
    }
    if (!*str2) {
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
  if (!str) {
    return nullptr;
  }
  char* clean_str = new char[strlen(str) + 1];
  char* ptr = clean_str;
  while (*str) {
    if ((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F')) {
      *ptr++ = *str;
    }
    ++str;
  }
  *ptr = '\0';
  return clean_str;
}

// Replace chars in string
char* SystemTools::ReplaceChars(char* str, const char* toreplace,
                                char replacement)
{
  if (str) {
    char* ptr = str;
    while (*ptr) {
      const char* ptr2 = toreplace;
      while (*ptr2) {
        if (*ptr == *ptr2) {
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
  if (!str1 || !str2) {
    return false;
  }
  size_t len1 = strlen(str1), len2 = strlen(str2);
  return len1 >= len2 && !strncmp(str1, str2, len2) ? true : false;
}

// Returns if string starts with another string
bool SystemTools::StringStartsWith(const std::string& str1, const char* str2)
{
  if (!str2) {
    return false;
  }
  size_t len1 = str1.size(), len2 = strlen(str2);
  return len1 >= len2 && !strncmp(str1.c_str(), str2, len2) ? true : false;
}

// Returns if string ends with another string
bool SystemTools::StringEndsWith(const char* str1, const char* str2)
{
  if (!str1 || !str2) {
    return false;
  }
  size_t len1 = strlen(str1), len2 = strlen(str2);
  return len1 >= len2 && !strncmp(str1 + (len1 - len2), str2, len2) ? true
                                                                    : false;
}

// Returns if string ends with another string
bool SystemTools::StringEndsWith(const std::string& str1, const char* str2)
{
  if (!str2) {
    return false;
  }
  size_t len1 = str1.size(), len2 = strlen(str2);
  return len1 >= len2 && !strncmp(str1.c_str() + (len1 - len2), str2, len2)
    ? true
    : false;
}

// Returns a pointer to the last occurrence of str2 in str1
const char* SystemTools::FindLastString(const char* str1, const char* str2)
{
  if (!str1 || !str2) {
    return nullptr;
  }

  size_t len1 = strlen(str1), len2 = strlen(str2);
  if (len1 >= len2) {
    const char* ptr = str1 + len1 - len2;
    do {
      if (!strncmp(ptr, str2, len2)) {
        return ptr;
      }
    } while (ptr-- != str1);
  }

  return nullptr;
}

// Duplicate string
char* SystemTools::DuplicateString(const char* str)
{
  if (str) {
    char* newstr = new char[strlen(str) + 1];
    return strcpy(newstr, str);
  }
  return nullptr;
}

// Return a cropped string
std::string SystemTools::CropString(const std::string& s, size_t max_len)
{
  if (s.empty() || max_len == 0 || max_len >= s.size()) {
    return s;
  }

  std::string n;
  n.reserve(max_len);

  size_t middle = max_len / 2;

  n.assign(s, 0, middle);
  n += s.substr(s.size() - (max_len - middle));

  if (max_len > 2) {
    n[middle] = '.';
    if (max_len > 3) {
      n[middle - 1] = '.';
      if (max_len > 4) {
        n[middle + 1] = '.';
      }
    }
  }

  return n;
}

std::vector<std::string> SystemTools::SplitString(const std::string& p,
                                                  char sep, bool isPath)
{
  std::string path = p;
  std::vector<std::string> paths;
  if (path.empty()) {
    return paths;
  }
  if (isPath && path[0] == '/') {
    path.erase(path.begin());
    paths.emplace_back("/");
  }
  std::string::size_type pos1 = 0;
  std::string::size_type pos2 = path.find(sep, pos1);
  while (pos2 != std::string::npos) {
    paths.push_back(path.substr(pos1, pos2 - pos1));
    pos1 = pos2 + 1;
    pos2 = path.find(sep, pos1 + 1);
  }
  paths.push_back(path.substr(pos1, pos2 - pos1));

  return paths;
}

int SystemTools::EstimateFormatLength(const char* format, va_list ap)
{
  if (!format) {
    return 0;
  }

  // Quick-hack attempt at estimating the length of the string.
  // Should never under-estimate.

  // Start with the length of the format string itself.

  size_t length = strlen(format);

  // Increase the length for every argument in the format.

  const char* cur = format;
  while (*cur) {
    if (*cur++ == '%') {
      // Skip "%%" since it doesn't correspond to a va_arg.
      if (*cur != '%') {
        while (!int(isalpha(*cur))) {
          ++cur;
        }
        switch (*cur) {
          case 's': {
            // Check the length of the string.
            char* s = va_arg(ap, char*);
            if (s) {
              length += strlen(s);
            }
          } break;
          case 'e':
          case 'f':
          case 'g': {
            // Assume the argument contributes no more than 64 characters.
            length += 64;

            // Eat the argument.
            static_cast<void>(va_arg(ap, double));
          } break;
          default: {
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

std::string SystemTools::EscapeChars(const char* str,
                                     const char* chars_to_escape,
                                     char escape_char)
{
  std::string n;
  if (str) {
    if (!chars_to_escape || !*chars_to_escape) {
      n.append(str);
    } else {
      n.reserve(strlen(str));
      while (*str) {
        const char* ptr = chars_to_escape;
        while (*ptr) {
          if (*str == *ptr) {
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
static void ConvertVMSToUnix(std::string& path)
{
  std::string::size_type rootEnd = path.find(":[");
  std::string::size_type pathEnd = path.find("]");
  if (rootEnd != std::string::npos) {
    std::string root = path.substr(0, rootEnd);
    std::string pathPart = path.substr(rootEnd + 2, pathEnd - rootEnd - 2);
    const char* pathCString = pathPart.c_str();
    const char* pos0 = pathCString;
    for (std::string::size_type pos = 0; *pos0; ++pos) {
      if (*pos0 == '.') {
        pathPart[pos] = '/';
      }
      pos0++;
    }
    path = "/" + root + "/" + pathPart;
  }
}
#endif

// convert windows slashes to unix slashes
void SystemTools::ConvertToUnixSlashes(std::string& path)
{
  if (path.empty()) {
    return;
  }

  const char* pathCString = path.c_str();
  bool hasDoubleSlash = false;
#ifdef __VMS
  ConvertVMSToUnix(path);
#else
  const char* pos0 = pathCString;
  for (std::string::size_type pos = 0; *pos0; ++pos) {
    if (*pos0 == '\\') {
      path[pos] = '/';
    }

    // Also, reuse the loop to check for slash followed by another slash
    if (!hasDoubleSlash && *(pos0 + 1) == '/' && *(pos0 + 2) == '/') {
#  ifdef _WIN32
      // However, on windows if the first characters are both slashes,
      // then keep them that way, so that network paths can be handled.
      if (pos > 0) {
        hasDoubleSlash = true;
      }
#  else
      hasDoubleSlash = true;
#  endif
    }

    pos0++;
  }

  if (hasDoubleSlash) {
    SystemTools::ReplaceString(path, "//", "/");
  }
#endif

  // remove any trailing slash
  // if there is a tilda ~ then replace it with HOME
  pathCString = path.c_str();
  if (pathCString[0] == '~' &&
      (pathCString[1] == '/' || pathCString[1] == '\0')) {
    std::string homeEnv;
    if (SystemTools::GetEnv("HOME", homeEnv)) {
      path.replace(0, 1, homeEnv);
    }
  }
#ifdef HAVE_GETPWNAM
  else if (pathCString[0] == '~') {
    std::string::size_type idx = path.find_first_of("/\0");
    char oldch = path[idx];
    path[idx] = '\0';
    passwd* pw = getpwnam(path.c_str() + 1);
    path[idx] = oldch;
    if (pw) {
      path.replace(0, idx, pw->pw_dir);
    }
  }
#endif
  // remove trailing slash if the path is more than
  // a single /
  pathCString = path.c_str();
  size_t size = path.size();
  if (size > 1 && path.back() == '/') {
    // if it is c:/ then do not remove the trailing slash
    if (!((size == 3 && pathCString[1] == ':'))) {
      path.resize(size - 1);
    }
  }
}

#ifdef _WIN32
std::wstring SystemTools::ConvertToWindowsExtendedPath(
  const std::string& source)
{
  return Encoding::ToWindowsExtendedPath(source);
}
#endif

// change // to /, and escape any spaces in the path
std::string SystemTools::ConvertToUnixOutputPath(const std::string& path)
{
  std::string ret = path;

  // remove // except at the beginning might be a cygwin drive
  std::string::size_type pos = 1;
  while ((pos = ret.find("//", pos)) != std::string::npos) {
    ret.erase(pos, 1);
  }
  // escape spaces and () in the path
  if (ret.find_first_of(' ') != std::string::npos) {
    std::string result;
    char lastch = 1;
    for (const char* ch = ret.c_str(); *ch != '\0'; ++ch) {
      // if it is already escaped then don't try to escape it again
      if ((*ch == ' ') && lastch != '\\') {
        result += '\\';
      }
      result += *ch;
      lastch = *ch;
    }
    ret = result;
  }
  return ret;
}

std::string SystemTools::ConvertToOutputPath(const std::string& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  return SystemTools::ConvertToWindowsOutputPath(path);
#else
  return SystemTools::ConvertToUnixOutputPath(path);
#endif
}

// remove double slashes not at the start
std::string SystemTools::ConvertToWindowsOutputPath(const std::string& path)
{
  std::string ret;
  // make it big enough for all of path and double quotes
  ret.reserve(path.size() + 3);
  // put path into the string
  ret = path;
  std::string::size_type pos = 0;
  // first convert all of the slashes
  while ((pos = ret.find('/', pos)) != std::string::npos) {
    ret[pos] = '\\';
    pos++;
  }
  // check for really small paths
  if (ret.size() < 2) {
    return ret;
  }
  // now clean up a bit and remove double slashes
  // Only if it is not the first position in the path which is a network
  // path on windows
  pos = 1; // start at position 1
  if (ret[0] == '\"') {
    pos = 2; // if the string is already quoted then start at 2
    if (ret.size() < 3) {
      return ret;
    }
  }
  while ((pos = ret.find("\\\\", pos)) != std::string::npos) {
    ret.erase(pos, 1);
  }
  // now double quote the path if it has spaces in it
  // and is not already double quoted
  if (ret.find(' ') != std::string::npos && ret[0] != '\"') {
    ret.insert(static_cast<std::string::size_type>(0),
               static_cast<std::string::size_type>(1), '\"');
    ret.append(1, '\"');
  }
  return ret;
}

/**
 * Append the filename from the path source to the directory name dir.
 */
static std::string FileInDir(const std::string& source, const std::string& dir)
{
  std::string new_destination = dir;
  SystemTools::ConvertToUnixSlashes(new_destination);
  return new_destination + '/' + SystemTools::GetFilenameName(source);
}

Status SystemTools::CopyFileIfDifferent(std::string const& source,
                                        std::string const& destination)
{
  // special check for a destination that is a directory
  // FilesDiffer does not handle file to directory compare
  if (SystemTools::FileIsDirectory(destination)) {
    const std::string new_destination = FileInDir(source, destination);
    if (!SystemTools::ComparePath(new_destination, destination)) {
      return SystemTools::CopyFileIfDifferent(source, new_destination);
    }
  } else {
    // source and destination are files so do a copy if they
    // are different
    if (SystemTools::FilesDiffer(source, destination)) {
      return SystemTools::CopyFileAlways(source, destination);
    }
  }
  // at this point the files must be the same so return true
  return Status::Success();
}

#define KWSYS_ST_BUFFER 4096

bool SystemTools::FilesDiffer(const std::string& source,
                              const std::string& destination)
{

#if defined(_WIN32)
  WIN32_FILE_ATTRIBUTE_DATA statSource;
  if (GetFileAttributesExW(Encoding::ToWindowsExtendedPath(source).c_str(),
                           GetFileExInfoStandard, &statSource) == 0) {
    return true;
  }

  WIN32_FILE_ATTRIBUTE_DATA statDestination;
  if (GetFileAttributesExW(
        Encoding::ToWindowsExtendedPath(destination).c_str(),
        GetFileExInfoStandard, &statDestination) == 0) {
    return true;
  }

  if (statSource.nFileSizeHigh != statDestination.nFileSizeHigh ||
      statSource.nFileSizeLow != statDestination.nFileSizeLow) {
    return true;
  }

  if (statSource.nFileSizeHigh == 0 && statSource.nFileSizeLow == 0) {
    return false;
  }
  auto nleft =
    ((__int64)statSource.nFileSizeHigh << 32) + statSource.nFileSizeLow;

#else

  struct stat statSource;
  if (stat(source.c_str(), &statSource) != 0) {
    return true;
  }

  struct stat statDestination;
  if (stat(destination.c_str(), &statDestination) != 0) {
    return true;
  }

  if (statSource.st_size != statDestination.st_size) {
    return true;
  }

  if (statSource.st_size == 0) {
    return false;
  }
  off_t nleft = statSource.st_size;
#endif

#if defined(_WIN32)
  kwsys::ifstream finSource(source.c_str(), (std::ios::binary | std::ios::in));
  kwsys::ifstream finDestination(destination.c_str(),
                                 (std::ios::binary | std::ios::in));
#else
  kwsys::ifstream finSource(source.c_str());
  kwsys::ifstream finDestination(destination.c_str());
#endif
  if (!finSource || !finDestination) {
    return true;
  }

  // Compare the files a block at a time.
  char source_buf[KWSYS_ST_BUFFER];
  char dest_buf[KWSYS_ST_BUFFER];
  while (nleft > 0) {
    // Read a block from each file.
    std::streamsize nnext = (nleft > KWSYS_ST_BUFFER)
      ? KWSYS_ST_BUFFER
      : static_cast<std::streamsize>(nleft);
    finSource.read(source_buf, nnext);
    finDestination.read(dest_buf, nnext);

    // If either failed to read assume they are different.
    if (static_cast<std::streamsize>(finSource.gcount()) != nnext ||
        static_cast<std::streamsize>(finDestination.gcount()) != nnext) {
      return true;
    }

    // If this block differs the file differs.
    if (memcmp(static_cast<const void*>(source_buf),
               static_cast<const void*>(dest_buf),
               static_cast<size_t>(nnext)) != 0) {
      return true;
    }

    // Update the byte count remaining.
    nleft -= nnext;
  }

  // No differences found.
  return false;
}

bool SystemTools::TextFilesDiffer(const std::string& path1,
                                  const std::string& path2)
{
  kwsys::ifstream if1(path1.c_str());
  kwsys::ifstream if2(path2.c_str());
  if (!if1 || !if2) {
    return true;
  }

  for (;;) {
    std::string line1, line2;
    bool hasData1 = GetLineFromStream(if1, line1);
    bool hasData2 = GetLineFromStream(if2, line2);
    if (hasData1 != hasData2) {
      return true;
    }
    if (!hasData1) {
      break;
    }
    if (line1 != line2) {
      return true;
    }
  }
  return false;
}

Status SystemTools::CopyFileContentBlockwise(std::string const& source,
                                             std::string const& destination)
{
  // Open files
  kwsys::ifstream fin(source.c_str(), std::ios::in | std::ios::binary);
  if (!fin) {
    return Status::POSIX_errno();
  }

  // try and remove the destination file so that read only destination files
  // can be written to.
  // If the remove fails continue so that files in read only directories
  // that do not allow file removal can be modified.
  SystemTools::RemoveFile(destination);

  kwsys::ofstream fout(destination.c_str(),
                       std::ios::out | std::ios::trunc | std::ios::binary);
  if (!fout) {
    return Status::POSIX_errno();
  }

  // This copy loop is very sensitive on certain platforms with
  // slightly broken stream libraries (like HPUX).  Normally, it is
  // incorrect to not check the error condition on the fin.read()
  // before using the data, but the fin.gcount() will be zero if an
  // error occurred.  Therefore, the loop should be safe everywhere.
  while (fin) {
    const int bufferSize = 4096;
    char buffer[bufferSize];

    fin.read(buffer, bufferSize);
    if (fin.gcount()) {
      fout.write(buffer, fin.gcount());
    } else {
      break;
    }
  }

  // Make sure the operating system has finished writing the file
  // before closing it.  This will ensure the file is finished before
  // the check below.
  fout.flush();

  fin.close();
  fout.close();

  if (!fout) {
    return Status::POSIX_errno();
  }

  return Status::Success();
}

/**
 * Clone the source file to the destination file
 *
 * If available, the Linux FICLONE ioctl is used to create a check
 * copy-on-write clone of the source file.
 *
 * The method returns false for the following cases:
 * - The code has not been compiled on Linux or the ioctl was unknown
 * - The source and destination is on different file systems
 * - The underlying filesystem does not support file cloning
 * - An unspecified error occurred
 */
Status SystemTools::CloneFileContent(std::string const& source,
                                     std::string const& destination)
{
#if defined(__linux) && defined(FICLONE)
  int in = open(source.c_str(), O_RDONLY);
  if (in < 0) {
    return Status::POSIX_errno();
  }

  SystemTools::RemoveFile(destination);

  int out =
    open(destination.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (out < 0) {
    Status status = Status::POSIX_errno();
    close(in);
    return status;
  }

  Status status = Status::Success();
  if (ioctl(out, FICLONE, in) < 0) {
    status = Status::POSIX_errno();
  }
  close(in);
  close(out);

  return status;
#elif defined(__APPLE__) &&                                                   \
  defined(KWSYS_SYSTEMTOOLS_HAVE_MACOS_COPYFILE_CLONE)
  // NOTE: we cannot use `clonefile` as the {a,c,m}time for the file needs to
  // be updated by `copy_file_if_different` and `copy_file`.
  if (copyfile(source.c_str(), destination.c_str(), nullptr,
               COPYFILE_METADATA | COPYFILE_CLONE) < 0) {
    return Status::POSIX_errno();
  }
#  if KWSYS_CXX_HAS_UTIMENSAT
  // utimensat is only available on newer Unixes and macOS 10.13+
  if (utimensat(AT_FDCWD, destination.c_str(), nullptr, 0) < 0) {
    return Status::POSIX_errno();
  }
#  else
  // fall back to utimes
  if (utimes(destination.c_str(), nullptr) < 0) {
    return Status::POSIX_errno();
  }
#  endif
  return Status::Success();
#else
  (void)source;
  (void)destination;
  return Status::POSIX(ENOSYS);
#endif
}

/**
 * Copy a file named by "source" to the file named by "destination".
 */
Status SystemTools::CopyFileAlways(std::string const& source,
                                   std::string const& destination)
{
  Status status;
  mode_t perm = 0;
  Status perms = SystemTools::GetPermissions(source, perm);
  std::string real_destination = destination;

  if (SystemTools::FileIsDirectory(source)) {
    status = SystemTools::MakeDirectory(destination);
    if (!status.IsSuccess()) {
      return status;
    }
  } else {
    // If destination is a directory, try to create a file with the same
    // name as the source in that directory.

    std::string destination_dir;
    if (SystemTools::FileIsDirectory(destination)) {
      destination_dir = real_destination;
      SystemTools::ConvertToUnixSlashes(real_destination);
      real_destination += '/';
      std::string source_name = source;
      real_destination += SystemTools::GetFilenameName(source_name);
    } else {
      destination_dir = SystemTools::GetFilenamePath(destination);
    }
    // If files are the same do not copy
    if (SystemTools::SameFile(source, real_destination)) {
      return status;
    }

    // Create destination directory
    if (!destination_dir.empty()) {
      status = SystemTools::MakeDirectory(destination_dir);
      if (!status.IsSuccess()) {
        return status;
      }
    }

    status = SystemTools::CloneFileContent(source, real_destination);
    // if cloning did not succeed, fall back to blockwise copy
    if (!status.IsSuccess()) {
      status = SystemTools::CopyFileContentBlockwise(source, real_destination);
    }
    if (!status.IsSuccess()) {
      return status;
    }
  }
  if (perms) {
    status = SystemTools::SetPermissions(real_destination, perm);
  }
  return status;
}

Status SystemTools::CopyAFile(std::string const& source,
                              std::string const& destination, bool always)
{
  if (always) {
    return SystemTools::CopyFileAlways(source, destination);
  } else {
    return SystemTools::CopyFileIfDifferent(source, destination);
  }
}

/**
 * Copy a directory content from "source" directory to the directory named by
 * "destination".
 */
Status SystemTools::CopyADirectory(std::string const& source,
                                   std::string const& destination, bool always)
{
  Status status;
  Directory dir;
  status = dir.Load(source);
  if (!status.IsSuccess()) {
    return status;
  }
  status = SystemTools::MakeDirectory(destination);
  if (!status.IsSuccess()) {
    return status;
  }

  for (size_t fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum) {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)), ".") != 0 &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)), "..") != 0) {
      std::string fullPath = source;
      fullPath += "/";
      fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
      if (SystemTools::FileIsDirectory(fullPath)) {
        std::string fullDestPath = destination;
        fullDestPath += "/";
        fullDestPath += dir.GetFile(static_cast<unsigned long>(fileNum));
        status = SystemTools::CopyADirectory(fullPath, fullDestPath, always);
        if (!status.IsSuccess()) {
          return status;
        }
      } else {
        status = SystemTools::CopyAFile(fullPath, destination, always);
        if (!status.IsSuccess()) {
          return status;
        }
      }
    }
  }

  return status;
}

// return size of file; also returns zero if no file exists
unsigned long SystemTools::FileLength(const std::string& filename)
{
  unsigned long length = 0;
#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA fs;
  if (GetFileAttributesExW(Encoding::ToWindowsExtendedPath(filename).c_str(),
                           GetFileExInfoStandard, &fs) != 0) {
    /* To support the full 64-bit file size, use fs.nFileSizeHigh
     * and fs.nFileSizeLow to construct the 64 bit size

    length = ((__int64)fs.nFileSizeHigh << 32) + fs.nFileSizeLow;
     */
    length = static_cast<unsigned long>(fs.nFileSizeLow);
  }
#else
  struct stat fs;
  if (stat(filename.c_str(), &fs) == 0) {
    length = static_cast<unsigned long>(fs.st_size);
  }
#endif
  return length;
}

int SystemTools::Strucmp(const char* l, const char* r)
{
  int lc;
  int rc;
  do {
    lc = tolower(*l++);
    rc = tolower(*r++);
  } while (lc == rc && lc);
  return lc - rc;
}

// return file's modified time
long int SystemTools::ModifiedTime(const std::string& filename)
{
  long int mt = 0;
#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA fs;
  if (GetFileAttributesExW(Encoding::ToWindowsExtendedPath(filename).c_str(),
                           GetFileExInfoStandard, &fs) != 0) {
    mt = windows_filetime_to_posix_time(fs.ftLastWriteTime);
  }
#else
  struct stat fs;
  if (stat(filename.c_str(), &fs) == 0) {
    mt = static_cast<long int>(fs.st_mtime);
  }
#endif
  return mt;
}

// return file's creation time
long int SystemTools::CreationTime(const std::string& filename)
{
  long int ct = 0;
#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA fs;
  if (GetFileAttributesExW(Encoding::ToWindowsExtendedPath(filename).c_str(),
                           GetFileExInfoStandard, &fs) != 0) {
    ct = windows_filetime_to_posix_time(fs.ftCreationTime);
  }
#else
  struct stat fs;
  if (stat(filename.c_str(), &fs) == 0) {
    ct = fs.st_ctime >= 0 ? static_cast<long int>(fs.st_ctime) : 0;
  }
#endif
  return ct;
}

std::string SystemTools::GetLastSystemError()
{
  int e = errno;
  return strerror(e);
}

Status SystemTools::RemoveFile(std::string const& source)
{
#ifdef _WIN32
  std::wstring const& ws = Encoding::ToWindowsExtendedPath(source);
  if (DeleteFileW(ws.c_str())) {
    return Status::Success();
  }
  DWORD err = GetLastError();
  if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
    return Status::Success();
  }
  if (err != ERROR_ACCESS_DENIED) {
    return Status::Windows(err);
  }
  /* The file may be read-only.  Try adding write permission.  */
  mode_t mode;
  if (!SystemTools::GetPermissions(source, mode) ||
      !SystemTools::SetPermissions(source, S_IWRITE)) {
    SetLastError(err);
    return Status::Windows(err);
  }

  const DWORD DIRECTORY_SOFT_LINK_ATTRS =
    FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT;
  DWORD attrs = GetFileAttributesW(ws.c_str());
  if (attrs != INVALID_FILE_ATTRIBUTES &&
      (attrs & DIRECTORY_SOFT_LINK_ATTRS) == DIRECTORY_SOFT_LINK_ATTRS &&
      RemoveDirectoryW(ws.c_str())) {
    return Status::Success();
  }
  if (DeleteFileW(ws.c_str()) || GetLastError() == ERROR_FILE_NOT_FOUND ||
      GetLastError() == ERROR_PATH_NOT_FOUND) {
    return Status::Success();
  }
  /* Try to restore the original permissions.  */
  SystemTools::SetPermissions(source, mode);
  SetLastError(err);
  return Status::Windows(err);
#else
  if (unlink(source.c_str()) != 0 && errno != ENOENT) {
    return Status::POSIX_errno();
  }
  return Status::Success();
#endif
}

Status SystemTools::RemoveADirectory(std::string const& source)
{
  // Add write permission to the directory so we can modify its
  // content to remove files and directories from it.
  mode_t mode = 0;
  if (SystemTools::GetPermissions(source, mode)) {
#if defined(_WIN32) && !defined(__CYGWIN__)
    mode |= S_IWRITE;
#else
    mode |= S_IWUSR;
#endif
    SystemTools::SetPermissions(source, mode);
  }

  Status status;
  Directory dir;
  status = dir.Load(source);
  if (!status.IsSuccess()) {
    return status;
  }

  size_t fileNum;
  for (fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum) {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)), ".") != 0 &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)), "..") != 0) {
      std::string fullPath = source;
      fullPath += "/";
      fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
      if (SystemTools::FileIsDirectory(fullPath) &&
          !SystemTools::FileIsSymlink(fullPath)) {
        status = SystemTools::RemoveADirectory(fullPath);
        if (!status.IsSuccess()) {
          return status;
        }
      } else {
        status = SystemTools::RemoveFile(fullPath);
        if (!status.IsSuccess()) {
          return status;
        }
      }
    }
  }

  if (Rmdir(source) != 0) {
    status = Status::POSIX_errno();
  }
  return status;
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
std::string SystemToolsStatic::FindName(
  const std::string& name, const std::vector<std::string>& userPaths,
  bool no_system_path)
{
  // Add the system search path to our path first
  std::vector<std::string> path;
  if (!no_system_path) {
    SystemTools::GetPath(path, "CMAKE_FILE_PATH");
    SystemTools::GetPath(path);
  }
  // now add the additional paths
  path.reserve(path.size() + userPaths.size());
  path.insert(path.end(), userPaths.begin(), userPaths.end());
  // now look for the file
  std::string tryPath;
  for (std::string const& p : path) {
    tryPath = p;
    if (tryPath.empty() || tryPath.back() != '/') {
      tryPath += '/';
    }
    tryPath += name;
    if (SystemTools::FileExists(tryPath)) {
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
std::string SystemTools::FindFile(const std::string& name,
                                  const std::vector<std::string>& userPaths,
                                  bool no_system_path)
{
  std::string tryPath =
    SystemToolsStatic::FindName(name, userPaths, no_system_path);
  if (!tryPath.empty() && !SystemTools::FileIsDirectory(tryPath)) {
    return SystemTools::CollapseFullPath(tryPath);
  }
  // Couldn't find the file.
  return "";
}

/**
 * Find the directory the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the directory if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string SystemTools::FindDirectory(
  const std::string& name, const std::vector<std::string>& userPaths,
  bool no_system_path)
{
  std::string tryPath =
    SystemToolsStatic::FindName(name, userPaths, no_system_path);
  if (!tryPath.empty() && SystemTools::FileIsDirectory(tryPath)) {
    return SystemTools::CollapseFullPath(tryPath);
  }
  // Couldn't find the file.
  return "";
}

/**
 * Find the executable with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the executable if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string SystemTools::FindProgram(const char* nameIn,
                                     const std::vector<std::string>& userPaths,
                                     bool no_system_path)
{
  if (!nameIn || !*nameIn) {
    return "";
  }
  return SystemTools::FindProgram(std::string(nameIn), userPaths,
                                  no_system_path);
}

std::string SystemTools::FindProgram(const std::string& name,
                                     const std::vector<std::string>& userPaths,
                                     bool no_system_path)
{
  std::string tryPath;

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  std::vector<std::string> extensions;
  // check to see if the name already has a .xxx at
  // the end of it
  // on windows try .com then .exe
  if (name.size() <= 3 || name[name.size() - 4] != '.') {
    extensions.emplace_back(".com");
    extensions.emplace_back(".exe");

    // first try with extensions if the os supports them
    for (std::string const& ext : extensions) {
      tryPath = name;
      tryPath += ext;
      if (SystemTools::FileIsExecutable(tryPath)) {
        return SystemTools::CollapseFullPath(tryPath);
      }
    }
  }
#endif

  // now try just the name
  if (SystemTools::FileIsExecutable(name)) {
    return SystemTools::CollapseFullPath(name);
  }
  // now construct the path
  std::vector<std::string> path;
  // Add the system search path to our path.
  if (!no_system_path) {
    SystemTools::GetPath(path);
  }
  // now add the additional paths
  path.reserve(path.size() + userPaths.size());
  path.insert(path.end(), userPaths.begin(), userPaths.end());
  // Add a trailing slash to all paths to aid the search process.
  for (std::string& p : path) {
    if (p.empty() || p.back() != '/') {
      p += '/';
    }
  }
  // Try each path
  for (std::string& p : path) {
#ifdef _WIN32
    // Remove double quotes from the path on windows
    SystemTools::ReplaceString(p, "\"", "");
#endif
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
    // first try with extensions
    for (std::string const& ext : extensions) {
      tryPath = p;
      tryPath += name;
      tryPath += ext;
      if (SystemTools::FileIsExecutable(tryPath)) {
        return SystemTools::CollapseFullPath(tryPath);
      }
    }
#endif
    // now try it without them
    tryPath = p;
    tryPath += name;
    if (SystemTools::FileIsExecutable(tryPath)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
  }
  // Couldn't find the program.
  return "";
}

std::string SystemTools::FindProgram(const std::vector<std::string>& names,
                                     const std::vector<std::string>& path,
                                     bool noSystemPath)
{
  for (std::string const& name : names) {
    // Try to find the program.
    std::string result = SystemTools::FindProgram(name, path, noSystemPath);
    if (!result.empty()) {
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
std::string SystemTools::FindLibrary(const std::string& name,
                                     const std::vector<std::string>& userPaths)
{
  // See if the executable exists as written.
  if (SystemTools::FileExists(name, true)) {
    return SystemTools::CollapseFullPath(name);
  }

  // Add the system search path to our path.
  std::vector<std::string> path;
  SystemTools::GetPath(path);
  // now add the additional paths
  path.reserve(path.size() + userPaths.size());
  path.insert(path.end(), userPaths.begin(), userPaths.end());
  // Add a trailing slash to all paths to aid the search process.
  for (std::string& p : path) {
    if (p.empty() || p.back() != '/') {
      p += '/';
    }
  }
  std::string tryPath;
  for (std::string const& p : path) {
#if defined(__APPLE__)
    tryPath = p;
    tryPath += name;
    tryPath += ".framework";
    if (SystemTools::FileIsDirectory(tryPath)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
#endif
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    tryPath = p;
    tryPath += name;
    tryPath += ".lib";
    if (SystemTools::FileExists(tryPath, true)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
#else
    tryPath = p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".so";
    if (SystemTools::FileExists(tryPath, true)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
    tryPath = p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".a";
    if (SystemTools::FileExists(tryPath, true)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
    tryPath = p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".sl";
    if (SystemTools::FileExists(tryPath, true)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
    tryPath = p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".dylib";
    if (SystemTools::FileExists(tryPath, true)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
    tryPath = p;
    tryPath += "lib";
    tryPath += name;
    tryPath += ".dll";
    if (SystemTools::FileExists(tryPath, true)) {
      return SystemTools::CollapseFullPath(tryPath);
    }
#endif
  }

  // Couldn't find the library.
  return "";
}

std::string SystemTools::GetRealPath(const std::string& path,
                                     std::string* errorMessage)
{
  std::string ret;
  Realpath(path, ret, errorMessage);
  return ret;
}

bool SystemTools::FileIsDirectory(const std::string& inName)
{
  if (inName.empty()) {
    return false;
  }
  size_t length = inName.size();
  const char* name = inName.c_str();

  // Remove any trailing slash from the name except in a root component.
  char local_buffer[KWSYS_SYSTEMTOOLS_MAXPATH];
  std::string string_buffer;
  size_t last = length - 1;
  if (last > 0 && (name[last] == '/' || name[last] == '\\') &&
      strcmp(name, "/") != 0 && name[last - 1] != ':') {
    if (last < sizeof(local_buffer)) {
      memcpy(local_buffer, name, last);
      local_buffer[last] = '\0';
      name = local_buffer;
    } else {
      string_buffer.append(name, last);
      name = string_buffer.c_str();
    }
  }

// Now check the file node type.
#if defined(_WIN32)
  DWORD attr =
    GetFileAttributesW(Encoding::ToWindowsExtendedPath(name).c_str());
  if (attr != INVALID_FILE_ATTRIBUTES) {
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  struct stat fs;
  if (stat(name, &fs) == 0) {
    return S_ISDIR(fs.st_mode);
#endif
  } else {
    return false;
  }
}

bool SystemTools::FileIsExecutable(const std::string& name)
{
  return !FileIsDirectory(name) && TestFileAccess(name, TEST_FILE_EXECUTE);
}

#if defined(_WIN32)
bool SystemTools::FileIsSymlinkWithAttr(const std::wstring& path,
                                        unsigned long attr)
{
  if (attr != INVALID_FILE_ATTRIBUTES) {
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
      // FILE_ATTRIBUTE_REPARSE_POINT means:
      // * a file or directory that has an associated reparse point, or
      // * a file that is a symbolic link.
      HANDLE hFile = CreateFileW(
        path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
      if (hFile == INVALID_HANDLE_VALUE) {
        return false;
      }
      byte buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
      DWORD bytesReturned = 0;
      if (!DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, nullptr, 0, buffer,
                           MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned,
                           nullptr)) {
        CloseHandle(hFile);
        // Since FILE_ATTRIBUTE_REPARSE_POINT is set this file must be
        // a symbolic link if it is not a reparse point.
        return GetLastError() == ERROR_NOT_A_REPARSE_POINT;
      }
      CloseHandle(hFile);
      ULONG reparseTag =
        reinterpret_cast<PREPARSE_DATA_BUFFER>(&buffer[0])->ReparseTag;
      return (reparseTag == IO_REPARSE_TAG_SYMLINK) ||
        (reparseTag == IO_REPARSE_TAG_MOUNT_POINT);
    }
    return false;
  }

  return false;
}
#endif

bool SystemTools::FileIsSymlink(const std::string& name)
{
#if defined(_WIN32)
  std::wstring path = Encoding::ToWindowsExtendedPath(name);
  return FileIsSymlinkWithAttr(path, GetFileAttributesW(path.c_str()));
#else
  struct stat fs;
  if (lstat(name.c_str(), &fs) == 0) {
    return S_ISLNK(fs.st_mode);
  } else {
    return false;
  }
#endif
}

bool SystemTools::FileIsFIFO(const std::string& name)
{
#if defined(_WIN32)
  HANDLE hFile =
    CreateFileW(Encoding::ToWide(name).c_str(), GENERIC_READ, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }
  const DWORD type = GetFileType(hFile);
  CloseHandle(hFile);
  return type == FILE_TYPE_PIPE;
#else
  struct stat fs;
  if (lstat(name.c_str(), &fs) == 0) {
    return S_ISFIFO(fs.st_mode);
  } else {
    return false;
  }
#endif
}

Status SystemTools::CreateSymlink(std::string const& origName,
                                  std::string const& newName)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  DWORD flags;
  if (FileIsDirectory(origName)) {
    flags = SYMBOLIC_LINK_FLAG_DIRECTORY;
  } else {
    flags = 0;
  }

  std::wstring origPath = Encoding::ToWindowsExtendedPath(origName);
  std::wstring newPath = Encoding::ToWindowsExtendedPath(newName);

  Status status;
  if (!CreateSymbolicLinkW(newPath.c_str(), origPath.c_str(),
                           flags |
                             SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) {
    status = Status::Windows_GetLastError();
  }
  // Older Windows versions do not understand
  // SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
  if (status.GetWindows() == ERROR_INVALID_PARAMETER) {
    status = Status::Success();
    if (!CreateSymbolicLinkW(newPath.c_str(), origPath.c_str(), flags)) {
      status = Status::Windows_GetLastError();
    }
  }

  return status;
#else
  if (symlink(origName.c_str(), newName.c_str()) < 0) {
    return Status::POSIX_errno();
  }
  return Status::Success();
#endif
}

Status SystemTools::ReadSymlink(std::string const& newName,
                                std::string& origName)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::wstring newPath = Encoding::ToWindowsExtendedPath(newName);
  // FILE_ATTRIBUTE_REPARSE_POINT means:
  // * a file or directory that has an associated reparse point, or
  // * a file that is a symbolic link.
  HANDLE hFile = CreateFileW(
    newPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
    FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (hFile == INVALID_HANDLE_VALUE) {
    return Status::Windows_GetLastError();
  }
  byte buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  DWORD bytesReturned = 0;
  Status status;
  if (!DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, nullptr, 0, buffer,
                       MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned,
                       nullptr)) {
    status = Status::Windows_GetLastError();
  }
  CloseHandle(hFile);
  if (!status.IsSuccess()) {
    return status;
  }
  PREPARSE_DATA_BUFFER data =
    reinterpret_cast<PREPARSE_DATA_BUFFER>(&buffer[0]);
  USHORT substituteNameLength;
  PCWSTR substituteNameData;
  if (data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
    substituteNameLength =
      data->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
    substituteNameData = data->SymbolicLinkReparseBuffer.PathBuffer +
      data->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR);
  } else if (data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
    substituteNameLength =
      data->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
    substituteNameData = data->MountPointReparseBuffer.PathBuffer +
      data->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR);
  } else if (data->ReparseTag == IO_REPARSE_TAG_APPEXECLINK) {
    // The reparse buffer is a list of 0-terminated non-empty strings,
    // terminated by an empty string (0-0).  We need the third string.
    size_t destLen;
    substituteNameData = GetAppExecLink(data, destLen);
    if (substituteNameData == nullptr || destLen == 0) {
      return Status::Windows(ERROR_SYMLINK_NOT_SUPPORTED);
    }
    substituteNameLength = static_cast<USHORT>(destLen);
  } else {
    return Status::Windows(ERROR_REPARSE_TAG_MISMATCH);
  }
  std::wstring substituteName(substituteNameData, substituteNameLength);
  origName = Encoding::ToNarrow(substituteName);
#else
  char buf[KWSYS_SYSTEMTOOLS_MAXPATH + 1];
  int count = static_cast<int>(
    readlink(newName.c_str(), buf, KWSYS_SYSTEMTOOLS_MAXPATH));
  if (count < 0) {
    return Status::POSIX_errno();
  }
  // Add null-terminator.
  buf[count] = 0;
  origName = buf;
#endif
  return Status::Success();
}

Status SystemTools::ChangeDirectory(std::string const& dir)
{
  if (Chdir(dir) < 0) {
    return Status::POSIX_errno();
  }
  return Status::Success();
}

std::string SystemTools::GetCurrentWorkingDirectory()
{
  char buf[2048];
  const char* cwd = Getcwd(buf, 2048);
  std::string path;
  if (cwd) {
    path = cwd;
    SystemTools::ConvertToUnixSlashes(path);
  }
  return path;
}

std::string SystemTools::GetProgramPath(const std::string& in_name)
{
  std::string dir, file;
  SystemTools::SplitProgramPath(in_name, dir, file);
  return dir;
}

bool SystemTools::SplitProgramPath(const std::string& in_name,
                                   std::string& dir, std::string& file, bool)
{
  dir = in_name;
  file.clear();
  SystemTools::ConvertToUnixSlashes(dir);

  if (!SystemTools::FileIsDirectory(dir)) {
    std::string::size_type slashPos = dir.rfind('/');
    if (slashPos != std::string::npos) {
      file = dir.substr(slashPos + 1);
      dir.resize(slashPos);
    } else {
      file = dir;
      dir.clear();
    }
  }
  if (!(dir.empty()) && !SystemTools::FileIsDirectory(dir)) {
    std::string oldDir = in_name;
    SystemTools::ConvertToUnixSlashes(oldDir);
    dir = in_name;
    return false;
  }
  return true;
}

bool SystemTools::FindProgramPath(const char* argv0, std::string& pathOut,
                                  std::string& errorMsg, const char* exeName,
                                  const char* buildDir,
                                  const char* installPrefix)
{
  std::vector<std::string> failures;
  std::string self = argv0 ? argv0 : "";
  failures.push_back(self);
  SystemTools::ConvertToUnixSlashes(self);
  self = SystemTools::FindProgram(self);
  if (!SystemTools::FileIsExecutable(self)) {
    if (buildDir) {
      std::string intdir = ".";
#ifdef CMAKE_INTDIR
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
  if (installPrefix) {
    if (!SystemTools::FileIsExecutable(self)) {
      failures.push_back(self);
      self = installPrefix;
      self += "/bin/";
      self += exeName;
    }
  }
  if (!SystemTools::FileIsExecutable(self)) {
    failures.push_back(self);
    std::ostringstream msg;
    msg << "Can not find the command line program ";
    if (exeName) {
      msg << exeName;
    }
    msg << "\n";
    if (argv0) {
      msg << "  argv[0] = \"" << argv0 << "\"\n";
    }
    msg << "  Attempted paths:\n";
    for (std::string const& ff : failures) {
      msg << "    \"" << ff << "\"\n";
    }
    errorMsg = msg.str();
    return false;
  }
  pathOut = self;
  return true;
}

#if KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP
void SystemTools::AddTranslationPath(const std::string& a,
                                     const std::string& b)
{
  std::string path_a = a;
  std::string path_b = b;
  SystemTools::ConvertToUnixSlashes(path_a);
  SystemTools::ConvertToUnixSlashes(path_b);
  // First check this is a directory path, since we don't want the table to
  // grow too fat
  if (SystemTools::FileIsDirectory(path_a)) {
    // Make sure the path is a full path and does not contain no '..'
    // Ken--the following code is incorrect. .. can be in a valid path
    // for example  /home/martink/MyHubba...Hubba/Src
    if (SystemTools::FileIsFullPath(path_b) &&
        path_b.find("..") == std::string::npos) {
      // Before inserting make sure path ends with '/'
      if (!path_a.empty() && path_a.back() != '/') {
        path_a += '/';
      }
      if (!path_b.empty() && path_b.back() != '/') {
        path_b += '/';
      }
      if (!(path_a == path_b)) {
        SystemToolsStatics->TranslationMap.insert(
          SystemToolsStatic::StringMap::value_type(std::move(path_a),
                                                   std::move(path_b)));
      }
    }
  }
}

void SystemTools::AddKeepPath(const std::string& dir)
{
  std::string cdir;
  Realpath(SystemTools::CollapseFullPath(dir), cdir);
  SystemTools::AddTranslationPath(cdir, dir);
}

void SystemTools::CheckTranslationPath(std::string& path)
{
  // Do not translate paths that are too short to have meaningful
  // translations.
  if (path.size() < 2) {
    return;
  }

  // Always add a trailing slash before translation.  It does not
  // matter if this adds an extra slash, but we do not want to
  // translate part of a directory (like the foo part of foo-dir).
  path += '/';

  // In case a file was specified we still have to go through this:
  // Now convert any path found in the table back to the one desired:
  for (auto const& pair : SystemToolsStatics->TranslationMap) {
    // We need to check of the path is a substring of the other path
    if (path.compare(0, pair.first.size(), pair.first) == 0) {
      path = path.replace(0, pair.first.size(), pair.second);
    }
  }

  // Remove the trailing slash we added before.
  path.pop_back();
}
#endif

static void SystemToolsAppendComponents(
  std::vector<std::string>& out_components,
  std::vector<std::string>::iterator first,
  std::vector<std::string>::iterator last)
{
  static const std::string up = "..";
  static const std::string cur = ".";
  for (std::vector<std::string>::const_iterator i = first; i != last; ++i) {
    if (*i == up) {
      // Remove the previous component if possible.  Ignore ../ components
      // that try to go above the root.  Keep ../ components if they are
      // at the beginning of a relative path (base path is relative).
      if (out_components.size() > 1 && out_components.back() != up) {
        out_components.resize(out_components.size() - 1);
      } else if (!out_components.empty() && out_components[0].empty()) {
        out_components.emplace_back(std::move(*i));
      }
    } else if (!i->empty() && *i != cur) {
      out_components.emplace_back(std::move(*i));
    }
  }
}

namespace {

std::string CollapseFullPathImpl(std::string const& in_path,
                                 std::string const* in_base)
{
  // Collect the output path components.
  std::vector<std::string> out_components;

  // Split the input path components.
  std::vector<std::string> path_components;
  SystemTools::SplitPath(in_path, path_components);
  out_components.reserve(path_components.size());

  // If the input path is relative, start with a base path.
  if (path_components[0].empty()) {
    std::vector<std::string> base_components;

    if (in_base) {
      // Use the given base path.
      SystemTools::SplitPath(*in_base, base_components);
    } else {
      // Use the current working directory as a base path.
      std::string cwd = SystemTools::GetCurrentWorkingDirectory();
      SystemTools::SplitPath(cwd, base_components);
    }

    // Append base path components to the output path.
    out_components.push_back(base_components[0]);
    SystemToolsAppendComponents(out_components, base_components.begin() + 1,
                                base_components.end());
  }

  // Append input path components to the output path.
  SystemToolsAppendComponents(out_components, path_components.begin(),
                              path_components.end());

  // Transform the path back to a string.
  std::string newPath = SystemTools::JoinPath(out_components);

#if KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP
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
  // SystemTools::AddTranslationPath(newPath, in_path);

  SystemTools::CheckTranslationPath(newPath);
#endif
#ifdef _WIN32
  newPath = SystemToolsStatics->GetActualCaseForPathCached(newPath);
  SystemTools::ConvertToUnixSlashes(newPath);
#endif
  // Return the reconstructed path.
  return newPath;
}
}

std::string SystemTools::CollapseFullPath(std::string const& in_path)
{
  return CollapseFullPathImpl(in_path, nullptr);
}

std::string SystemTools::CollapseFullPath(std::string const& in_path,
                                          const char* in_base)
{
  if (!in_base) {
    return CollapseFullPathImpl(in_path, nullptr);
  }
  std::string tmp_base = in_base;
  return CollapseFullPathImpl(in_path, &tmp_base);
}

std::string SystemTools::CollapseFullPath(std::string const& in_path,
                                          std::string const& in_base)
{
  return CollapseFullPathImpl(in_path, &in_base);
}

// compute the relative path from here to there
std::string SystemTools::RelativePath(const std::string& local,
                                      const std::string& remote)
{
  if (!SystemTools::FileIsFullPath(local)) {
    return "";
  }
  if (!SystemTools::FileIsFullPath(remote)) {
    return "";
  }

  std::string l = SystemTools::CollapseFullPath(local);
  std::string r = SystemTools::CollapseFullPath(remote);

  // split up both paths into arrays of strings using / as a separator
  std::vector<std::string> localSplit = SystemTools::SplitString(l, '/', true);
  std::vector<std::string> remoteSplit =
    SystemTools::SplitString(r, '/', true);
  std::vector<std::string>
    commonPath; // store shared parts of path in this array
  std::vector<std::string> finalPath; // store the final relative path here
  // count up how many matching directory names there are from the start
  unsigned int sameCount = 0;
  while (((sameCount <= (localSplit.size() - 1)) &&
          (sameCount <= (remoteSplit.size() - 1))) &&
// for Windows and Apple do a case insensitive string compare
#if defined(_WIN32) || defined(__APPLE__)
         SystemTools::Strucmp(localSplit[sameCount].c_str(),
                              remoteSplit[sameCount].c_str()) == 0
#else
         localSplit[sameCount] == remoteSplit[sameCount]
#endif
  ) {
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
  if (sameCount == 0) {
    return remote;
  }

  // for each entry that is not common in the local path
  // add a ../ to the finalpath array, this gets us out of the local
  // path into the remote dir
  for (std::string const& lp : localSplit) {
    if (!lp.empty()) {
      finalPath.emplace_back("../");
    }
  }
  // for each entry that is not common in the remote path add it
  // to the final path.
  for (std::string const& rp : remoteSplit) {
    if (!rp.empty()) {
      finalPath.push_back(rp);
    }
  }
  std::string relativePath; // result string
  // now turn the array of directories into a unix path by puttint /
  // between each entry that does not already have one
  for (std::string const& fp : finalPath) {
    if (!relativePath.empty() && relativePath.back() != '/') {
      relativePath += '/';
    }
    relativePath += fp;
  }
  return relativePath;
}

std::string SystemTools::GetActualCaseForPath(const std::string& p)
{
#ifdef _WIN32
  return SystemToolsStatic::GetCasePathName(p);
#else
  return p;
#endif
}

const char* SystemTools::SplitPathRootComponent(const std::string& p,
                                                std::string* root)
{
  // Identify the root component.
  const char* c = p.c_str();
  if ((c[0] == '/' && c[1] == '/') || (c[0] == '\\' && c[1] == '\\')) {
    // Network path.
    if (root) {
      *root = "//";
    }
    c += 2;
  } else if (c[0] == '/' || c[0] == '\\') {
    // Unix path (or Windows path w/out drive letter).
    if (root) {
      *root = "/";
    }
    c += 1;
  } else if (c[0] && c[1] == ':' && (c[2] == '/' || c[2] == '\\')) {
    // Windows path.
    if (root) {
      (*root) = "_:/";
      (*root)[0] = c[0];
    }
    c += 3;
  } else if (c[0] && c[1] == ':') {
    // Path relative to a windows drive working directory.
    if (root) {
      (*root) = "_:";
      (*root)[0] = c[0];
    }
    c += 2;
  } else if (c[0] == '~') {
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
    while (c[n] && c[n] != '/') {
      ++n;
    }
    if (root) {
      root->assign(c, n);
      *root += '/';
    }
    if (c[n] == '/') {
      ++n;
    }
    c += n;
  } else {
    // Relative path.
    if (root) {
      *root = "";
    }
  }

  // Return the remaining path.
  return c;
}

void SystemTools::SplitPath(const std::string& p,
                            std::vector<std::string>& components,
                            bool expand_home_dir)
{
  const char* c;
  components.clear();

  // Identify the root component.
  {
    std::string root;
    c = SystemTools::SplitPathRootComponent(p, &root);

    // Expand home directory references if requested.
    if (expand_home_dir && !root.empty() && root[0] == '~') {
      std::string homedir;
      root.resize(root.size() - 1);
      if (root.size() == 1) {
#if defined(_WIN32) && !defined(__CYGWIN__)
        if (!SystemTools::GetEnv("USERPROFILE", homedir))
#endif
          SystemTools::GetEnv("HOME", homedir);
      }
#ifdef HAVE_GETPWNAM
      else if (passwd* pw = getpwnam(root.c_str() + 1)) {
        if (pw->pw_dir) {
          homedir = pw->pw_dir;
        }
      }
#endif
      if (!homedir.empty() &&
          (homedir.back() == '/' || homedir.back() == '\\')) {
        homedir.resize(homedir.size() - 1);
      }
      SystemTools::SplitPath(homedir, components);
    } else {
      components.push_back(root);
    }
  }

  // Parse the remaining components.
  const char* first = c;
  const char* last = first;
  for (; *last; ++last) {
    if (*last == '/' || *last == '\\') {
      // End of a component.  Save it.
      components.emplace_back(first, last);
      first = last + 1;
    }
  }

  // Save the last component unless there were no components.
  if (last != c) {
    components.emplace_back(first, last);
  }
}

std::string SystemTools::JoinPath(const std::vector<std::string>& components)
{
  return SystemTools::JoinPath(components.begin(), components.end());
}

std::string SystemTools::JoinPath(
  std::vector<std::string>::const_iterator first,
  std::vector<std::string>::const_iterator last)
{
  // Construct result in a single string.
  std::string result;
  size_t len = 0;
  for (auto i = first; i != last; ++i) {
    len += 1 + i->size();
  }
  result.reserve(len);

  // The first two components do not add a slash.
  if (first != last) {
    result.append(*first++);
  }
  if (first != last) {
    result.append(*first++);
  }

  // All remaining components are always separated with a slash.
  while (first != last) {
    result.push_back('/');
    result.append((*first++));
  }

  // Return the concatenated result.
  return result;
}

bool SystemTools::ComparePath(const std::string& c1, const std::string& c2)
{
#if defined(_WIN32) || defined(__APPLE__)
#  ifdef _MSC_VER
  return _stricmp(c1.c_str(), c2.c_str()) == 0;
#  elif defined(__APPLE__) || defined(__GNUC__)
  return strcasecmp(c1.c_str(), c2.c_str()) == 0;
#  else
  return SystemTools::Strucmp(c1.c_str(), c2.c_str()) == 0;
#  endif
#else
  return c1 == c2;
#endif
}

bool SystemTools::Split(const std::string& str,
                        std::vector<std::string>& lines, char separator)
{
  std::string data(str);
  std::string::size_type lpos = 0;
  while (lpos < data.length()) {
    std::string::size_type rpos = data.find_first_of(separator, lpos);
    if (rpos == std::string::npos) {
      // String ends at end of string without a separator.
      lines.push_back(data.substr(lpos));
      return false;
    } else {
      // String ends in a separator, remove the character.
      lines.push_back(data.substr(lpos, rpos - lpos));
    }
    lpos = rpos + 1;
  }
  return true;
}

bool SystemTools::Split(const std::string& str,
                        std::vector<std::string>& lines)
{
  std::string data(str);
  std::string::size_type lpos = 0;
  while (lpos < data.length()) {
    std::string::size_type rpos = data.find_first_of('\n', lpos);
    if (rpos == std::string::npos) {
      // Line ends at end of string without a newline.
      lines.push_back(data.substr(lpos));
      return false;
    }
    if ((rpos > lpos) && (data[rpos - 1] == '\r')) {
      // Line ends in a "\r\n" pair, remove both characters.
      lines.push_back(data.substr(lpos, (rpos - 1) - lpos));
    } else {
      // Line ends in a "\n", remove the character.
      lines.push_back(data.substr(lpos, rpos - lpos));
    }
    lpos = rpos + 1;
  }
  return true;
}

std::string SystemTools::Join(const std::vector<std::string>& list,
                              const std::string& separator)
{
  std::string result;
  if (list.empty()) {
    return result;
  }

  size_t total_size = separator.size() * (list.size() - 1);
  for (const std::string& string : list) {
    total_size += string.size();
  }

  result.reserve(total_size);
  bool needs_separator = false;
  for (const std::string& string : list) {
    if (needs_separator) {
      result += separator;
    }
    result += string;
    needs_separator = true;
  }

  return result;
}

/**
 * Return path of a full filename (no trailing slashes).
 * Warning: returned path is converted to Unix slashes format.
 */
std::string SystemTools::GetFilenamePath(const std::string& filename)
{
  std::string fn = filename;
  SystemTools::ConvertToUnixSlashes(fn);

  std::string::size_type slash_pos = fn.rfind('/');
  if (slash_pos == 0) {
    return "/";
  }
  if (slash_pos == 2 && fn[1] == ':') {
    // keep the / after a drive letter
    fn.resize(3);
    return fn;
  }
  if (slash_pos == std::string::npos) {
    return "";
  }
  fn.resize(slash_pos);
  return fn;
}

/**
 * Return file name of a full filename (i.e. file name without path).
 */
std::string SystemTools::GetFilenameName(const std::string& filename)
{
#if defined(_WIN32) || defined(KWSYS_SYSTEMTOOLS_SUPPORT_WINDOWS_SLASHES)
  const char* separators = "/\\";
#else
  char separators = '/';
#endif
  std::string::size_type slash_pos = filename.find_last_of(separators);
  if (slash_pos != std::string::npos) {
    return filename.substr(slash_pos + 1);
  } else {
    return filename;
  }
}

/**
 * Return file extension of a full filename (dot included).
 * Warning: this is the longest extension (for example: .tar.gz)
 */
std::string SystemTools::GetFilenameExtension(const std::string& filename)
{
  std::string name = SystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.find('.');
  if (dot_pos != std::string::npos) {
    name.erase(0, dot_pos);
    return name;
  } else {
    return "";
  }
}

/**
 * Return file extension of a full filename (dot included).
 * Warning: this is the shortest extension (for example: .gz of .tar.gz)
 */
std::string SystemTools::GetFilenameLastExtension(const std::string& filename)
{
  std::string name = SystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.rfind('.');
  if (dot_pos != std::string::npos) {
    name.erase(0, dot_pos);
    return name;
  } else {
    return "";
  }
}

/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the longest extension (for example: .tar.gz)
 */
std::string SystemTools::GetFilenameWithoutExtension(
  const std::string& filename)
{
  std::string name = SystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.find('.');
  if (dot_pos != std::string::npos) {
    name.resize(dot_pos);
  }
  return name;
}

/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the last extension (for example: removes .gz
 * from .tar.gz)
 */
std::string SystemTools::GetFilenameWithoutLastExtension(
  const std::string& filename)
{
  std::string name = SystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.rfind('.');
  if (dot_pos != std::string::npos) {
    name.resize(dot_pos);
  }
  return name;
}

bool SystemTools::FileHasSignature(const char* filename, const char* signature,
                                   long offset)
{
  if (!filename || !signature) {
    return false;
  }

  FILE* fp = Fopen(filename, "rb");
  if (!fp) {
    return false;
  }

  fseek(fp, offset, SEEK_SET);

  bool res = false;
  size_t signature_len = strlen(signature);
  char* buffer = new char[signature_len];

  if (fread(buffer, 1, signature_len, fp) == signature_len) {
    res = (!strncmp(buffer, signature, signature_len) ? true : false);
  }

  delete[] buffer;

  fclose(fp);
  return res;
}

SystemTools::FileTypeEnum SystemTools::DetectFileType(const char* filename,
                                                      unsigned long length,
                                                      double percent_bin)
{
  if (!filename || percent_bin < 0) {
    return SystemTools::FileTypeUnknown;
  }

  if (SystemTools::FileIsDirectory(filename)) {
    return SystemTools::FileTypeUnknown;
  }

  FILE* fp = Fopen(filename, "rb");
  if (!fp) {
    return SystemTools::FileTypeUnknown;
  }

  // Allocate buffer and read bytes

  auto* buffer = new unsigned char[length];
  size_t read_length = fread(buffer, 1, length, fp);
  fclose(fp);
  if (read_length == 0) {
    delete[] buffer;
    return SystemTools::FileTypeUnknown;
  }

  // Loop over contents and count

  size_t text_count = 0;

  const unsigned char* ptr = buffer;
  const unsigned char* buffer_end = buffer + read_length;

  while (ptr != buffer_end) {
    if ((*ptr >= 0x20 && *ptr <= 0x7F) || *ptr == '\n' || *ptr == '\r' ||
        *ptr == '\t') {
      text_count++;
    }
    ptr++;
  }

  delete[] buffer;

  double current_percent_bin = (static_cast<double>(read_length - text_count) /
                                static_cast<double>(read_length));

  if (current_percent_bin >= percent_bin) {
    return SystemTools::FileTypeBinary;
  }

  return SystemTools::FileTypeText;
}

bool SystemTools::LocateFileInDir(const char* filename, const char* dir,
                                  std::string& filename_found,
                                  int try_filename_dirs)
{
  if (!filename || !dir) {
    return false;
  }

  // Get the basename of 'filename'

  std::string filename_base = SystemTools::GetFilenameName(filename);

  // Check if 'dir' is really a directory
  // If win32 and matches something like C:, accept it as a dir

  std::string real_dir;
  if (!SystemTools::FileIsDirectory(dir)) {
#if defined(_WIN32)
    size_t dir_len = strlen(dir);
    if (dir_len < 2 || dir[dir_len - 1] != ':') {
#endif
      real_dir = SystemTools::GetFilenamePath(dir);
      dir = real_dir.c_str();
#if defined(_WIN32)
    }
#endif
  }

  // Try to find the file in 'dir'

  bool res = false;
  if (!filename_base.empty() && dir) {
    size_t dir_len = strlen(dir);
    int need_slash =
      (dir_len && dir[dir_len - 1] != '/' && dir[dir_len - 1] != '\\');

    std::string temp = dir;
    if (need_slash) {
      temp += "/";
    }
    temp += filename_base;

    if (SystemTools::FileExists(temp)) {
      res = true;
      filename_found = temp;
    }

    // If not found, we can try harder by appending part of the file to
    // to the directory to look inside.
    // Example: if we were looking for /foo/bar/yo.txt in /d1/d2, then
    // try to find yo.txt in /d1/d2/bar, then /d1/d2/foo/bar, etc.

    else if (try_filename_dirs) {
      std::string filename_dir(filename);
      std::string filename_dir_base;
      std::string filename_dir_bases;
      do {
        filename_dir = SystemTools::GetFilenamePath(filename_dir);
        filename_dir_base = SystemTools::GetFilenameName(filename_dir);
#if defined(_WIN32)
        if (filename_dir_base.empty() || filename_dir_base.back() == ':')
#else
        if (filename_dir_base.empty())
#endif
        {
          break;
        }

        filename_dir_bases = filename_dir_base + "/" + filename_dir_bases;

        temp = dir;
        if (need_slash) {
          temp += "/";
        }
        temp += filename_dir_bases;

        res = SystemTools::LocateFileInDir(filename_base.c_str(), temp.c_str(),
                                           filename_found, 0);

      } while (!res && !filename_dir_base.empty());
    }
  }

  return res;
}

bool SystemTools::FileIsFullPath(const std::string& in_name)
{
  return SystemToolsStatic::FileIsFullPath(in_name.c_str(), in_name.size());
}

bool SystemTools::FileIsFullPath(const char* in_name)
{
  return SystemToolsStatic::FileIsFullPath(
    in_name, in_name[0] ? (in_name[1] ? 2 : 1) : 0);
}

bool SystemToolsStatic::FileIsFullPath(const char* in_name, size_t len)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  // On Windows, the name must be at least two characters long.
  if (len < 2) {
    return false;
  }
  if (in_name[1] == ':') {
    return true;
  }
  if (in_name[0] == '\\') {
    return true;
  }
#else
  // On UNIX, the name must be at least one character long.
  if (len < 1) {
    return false;
  }
#endif
#if !defined(_WIN32)
  if (in_name[0] == '~') {
    return true;
  }
#endif
  // On UNIX, the name must begin in a '/'.
  // On Windows, if the name begins in a '/', then it is a full
  // network path.
  if (in_name[0] == '/') {
    return true;
  }
  return false;
}

Status SystemTools::GetShortPath(std::string const& path,
                                 std::string& shortPath)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string tempPath = path; // create a buffer

  // if the path passed in has quotes around it, first remove the quotes
  if (!path.empty() && path[0] == '"' && path.back() == '"') {
    tempPath.resize(path.length() - 1);
    tempPath.erase(0, 1);
  }

  std::wstring wtempPath = Encoding::ToWide(tempPath);
  DWORD ret = GetShortPathNameW(wtempPath.c_str(), nullptr, 0);
  std::vector<wchar_t> buffer(ret);
  if (ret != 0) {
    ret = GetShortPathNameW(wtempPath.c_str(), &buffer[0],
                            static_cast<DWORD>(buffer.size()));
  }

  if (ret == 0) {
    return Status::Windows_GetLastError();
  } else {
    shortPath = Encoding::ToNarrow(&buffer[0]);
    return Status::Success();
  }
#else
  shortPath = path;
  return Status::Success();
#endif
}

std::string SystemTools::GetCurrentDateTime(const char* format)
{
  char buf[1024];
  time_t t;
  time(&t);
  strftime(buf, sizeof(buf), format, localtime(&t));
  return std::string(buf);
}

std::string SystemTools::MakeCidentifier(const std::string& s)
{
  std::string str(s);
  if (str.find_first_of("0123456789") == 0) {
    str = "_" + str;
  }

  std::string permited_chars("_"
                             "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "0123456789");
  std::string::size_type pos = 0;
  while ((pos = str.find_first_not_of(permited_chars, pos)) !=
         std::string::npos) {
    str[pos] = '_';
  }
  return str;
}

// Convenience function around std::getline which removes a trailing carriage
// return and can truncate the buffer as needed.  Returns true
// if any data were read before the end-of-file was reached.
bool SystemTools::GetLineFromStream(
  std::istream& is, std::string& line, bool* has_newline /* = 0 */,
  std::string::size_type sizeLimit /* = std::string::npos */)
{
  // Start with an empty line.
  line = "";

  // Early short circuit return if stream is no good. Just return
  // false and the empty line. (Probably means caller tried to
  // create a file stream with a non-existent file name...)
  //
  if (!is) {
    if (has_newline) {
      *has_newline = false;
    }
    return false;
  }

  std::getline(is, line);
  bool haveData = !line.empty() || !is.eof();
  if (!line.empty()) {
    // Avoid storing a carriage return character.
    if (line.back() == '\r') {
      line.resize(line.size() - 1);
    }

    // if we read too much then truncate the buffer
    if (sizeLimit != std::string::npos && line.size() > sizeLimit) {
      line.resize(sizeLimit);
    }
  }

  // Return the results.
  if (has_newline) {
    *has_newline = !is.eof();
  }
  return haveData;
}

int SystemTools::GetTerminalWidth()
{
  int width = -1;
#ifdef HAVE_TTY_INFO
  struct winsize ws;
  std::string columns; /* Unix98 environment variable */
  if (ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col > 0 && ws.ws_row > 0) {
    width = ws.ws_col;
  }
  if (!isatty(STDOUT_FILENO)) {
    width = -1;
  }
  if (SystemTools::GetEnv("COLUMNS", columns) && !columns.empty()) {
    long t;
    char* endptr;
    t = strtol(columns.c_str(), &endptr, 0);
    if (endptr && !*endptr && (t > 0) && (t < 1000)) {
      width = static_cast<int>(t);
    }
  }
  if (width < 9) {
    width = -1;
  }
#endif
  return width;
}

Status SystemTools::GetPermissions(const char* file, mode_t& mode)
{
  if (!file) {
    return Status::POSIX(EINVAL);
  }
  return SystemTools::GetPermissions(std::string(file), mode);
}

Status SystemTools::GetPermissions(std::string const& file, mode_t& mode)
{
#if defined(_WIN32)
  DWORD attr =
    GetFileAttributesW(Encoding::ToWindowsExtendedPath(file).c_str());
  if (attr == INVALID_FILE_ATTRIBUTES) {
    return Status::Windows_GetLastError();
  }
  if ((attr & FILE_ATTRIBUTE_READONLY) != 0) {
    mode = (_S_IREAD | (_S_IREAD >> 3) | (_S_IREAD >> 6));
  } else {
    mode = (_S_IWRITE | (_S_IWRITE >> 3) | (_S_IWRITE >> 6)) |
      (_S_IREAD | (_S_IREAD >> 3) | (_S_IREAD >> 6));
  }
  if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
    mode |= S_IFDIR | (_S_IEXEC | (_S_IEXEC >> 3) | (_S_IEXEC >> 6));
  } else {
    mode |= S_IFREG;
  }
  size_t dotPos = file.rfind('.');
  const char* ext = dotPos == std::string::npos ? 0 : (file.c_str() + dotPos);
  if (ext &&
      (Strucmp(ext, ".exe") == 0 || Strucmp(ext, ".com") == 0 ||
       Strucmp(ext, ".cmd") == 0 || Strucmp(ext, ".bat") == 0)) {
    mode |= (_S_IEXEC | (_S_IEXEC >> 3) | (_S_IEXEC >> 6));
  }
#else
  struct stat st;
  if (stat(file.c_str(), &st) < 0) {
    return Status::POSIX_errno();
  }
  mode = st.st_mode;
#endif
  return Status::Success();
}

Status SystemTools::SetPermissions(const char* file, mode_t mode,
                                   bool honor_umask)
{
  if (!file) {
    return Status::POSIX(EINVAL);
  }
  return SystemTools::SetPermissions(std::string(file), mode, honor_umask);
}

Status SystemTools::SetPermissions(std::string const& file, mode_t mode,
                                   bool honor_umask)
{
  if (!SystemTools::PathExists(file)) {
    return Status::POSIX(ENOENT);
  }
  if (honor_umask) {
    mode_t currentMask = umask(0);
    umask(currentMask);
    mode &= ~currentMask;
  }
#ifdef _WIN32
  if (_wchmod(Encoding::ToWindowsExtendedPath(file).c_str(), mode) < 0)
#else
  if (chmod(file.c_str(), mode) < 0)
#endif
  {
    return Status::POSIX_errno();
  }

  return Status::Success();
}

std::string SystemTools::GetParentDirectory(const std::string& fileOrDir)
{
  return SystemTools::GetFilenamePath(fileOrDir);
}

bool SystemTools::IsSubDirectory(const std::string& cSubdir,
                                 const std::string& cDir)
{
  if (cDir.empty()) {
    return false;
  }
  std::string subdir = cSubdir;
  std::string dir = cDir;
  SystemTools::ConvertToUnixSlashes(subdir);
  SystemTools::ConvertToUnixSlashes(dir);
  if (subdir.size() <= dir.size() || dir.empty()) {
    return false;
  }
  bool isRootPath = dir.back() == '/'; // like "/" or "C:/"
  size_t expectedSlashPosition = isRootPath ? dir.size() - 1u : dir.size();
  if (subdir[expectedSlashPosition] != '/') {
    return false;
  }
  subdir.resize(dir.size());
  return SystemTools::ComparePath(subdir, dir);
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
  if (msec >= 1000) {
    sleep(msec / 1000);
    usleep((msec % 1000) * 1000);
  } else {
    usleep(msec * 1000);
  }
#endif
}

std::string SystemTools::GetOperatingSystemNameAndVersion()
{
  std::string res;

#ifdef _WIN32
  char buffer[256];

  OSVERSIONINFOEXA osvi;
  BOOL bOsVersionInfoEx;

  ZeroMemory(&osvi, sizeof(osvi));
  osvi.dwOSVersionInfoSize = sizeof(osvi);

#  ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#    pragma warning(push)
#    ifdef __INTEL_COMPILER
#      pragma warning(disable : 1478)
#    elif defined __clang__
#      pragma clang diagnostic push
#      pragma clang diagnostic ignored "-Wdeprecated-declarations"
#    else
#      pragma warning(disable : 4996)
#    endif
#  endif
  bOsVersionInfoEx = GetVersionExA((OSVERSIONINFOA*)&osvi);
  if (!bOsVersionInfoEx) {
    return "";
  }
#  ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#    ifdef __clang__
#      pragma clang diagnostic pop
#    else
#      pragma warning(pop)
#    endif
#  endif

  switch (osvi.dwPlatformId) {
      // Test for the Windows NT product family.

    case VER_PLATFORM_WIN32_NT:

      // Test for the specific product family.
      if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
          res += "Microsoft Windows 10";
        } else {
          res += "Microsoft Windows Server 2016 family";
        }
      }

      if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
          res += "Microsoft Windows 8.1";
        } else {
          res += "Microsoft Windows Server 2012 R2 family";
        }
      }

      if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
          res += "Microsoft Windows 8";
        } else {
          res += "Microsoft Windows Server 2012 family";
        }
      }

      if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
          res += "Microsoft Windows 7";
        } else {
          res += "Microsoft Windows Server 2008 R2 family";
        }
      }

      if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
          res += "Microsoft Windows Vista";
        } else {
          res += "Microsoft Windows Server 2008 family";
        }
      }

      if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
        res += "Microsoft Windows Server 2003 family";
      }

      if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
        res += "Microsoft Windows XP";
      }

      if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
        res += "Microsoft Windows 2000";
      }

      if (osvi.dwMajorVersion <= 4) {
        res += "Microsoft Windows NT";
      }

      // Test for specific product on Windows NT 4.0 SP6 and later.

      if (bOsVersionInfoEx) {
        // Test for the workstation type.

        if (osvi.wProductType == VER_NT_WORKSTATION) {
          if (osvi.dwMajorVersion == 4) {
            res += " Workstation 4.0";
          } else if (osvi.dwMajorVersion == 5) {
            if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
              res += " Home Edition";
            } else {
              res += " Professional";
            }
          }
        }

        // Test for the server type.

        else if (osvi.wProductType == VER_NT_SERVER) {
          if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
            if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
              res += " Datacenter Edition";
            } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
              res += " Enterprise Edition";
            } else if (osvi.wSuiteMask == VER_SUITE_BLADE) {
              res += " Web Edition";
            } else {
              res += " Standard Edition";
            }
          }

          else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
            if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
              res += " Datacenter Server";
            } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
              res += " Advanced Server";
            } else {
              res += " Server";
            }
          }

          else if (osvi.dwMajorVersion <= 4) // Windows NT 4.0
          {
            if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
              res += " Server 4.0, Enterprise Edition";
            } else {
              res += " Server 4.0";
            }
          }
        }
      }

      // Test for specific product on Windows NT 4.0 SP5 and earlier

      else {
        HKEY hKey;
#  define BUFSIZE 80
        wchar_t szProductType[BUFSIZE];
        DWORD dwBufLen = BUFSIZE;
        LONG lRet;

        lRet =
          RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                        0, KEY_QUERY_VALUE, &hKey);
        if (lRet != ERROR_SUCCESS) {
          return "";
        }

        lRet = RegQueryValueExW(hKey, L"ProductType", nullptr, nullptr,
                                (LPBYTE)szProductType, &dwBufLen);

        if ((lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE)) {
          return "";
        }

        RegCloseKey(hKey);

        if (lstrcmpiW(L"WINNT", szProductType) == 0) {
          res += " Workstation";
        }
        if (lstrcmpiW(L"LANMANNT", szProductType) == 0) {
          res += " Server";
        }
        if (lstrcmpiW(L"SERVERNT", szProductType) == 0) {
          res += " Advanced Server";
        }

        res += " ";
        snprintf(buffer, sizeof(buffer), "%ld", osvi.dwMajorVersion);
        res += buffer;
        res += ".";
        snprintf(buffer, sizeof(buffer), "%ld", osvi.dwMinorVersion);
        res += buffer;
      }

      // Display service pack (if any) and build number.

      if (osvi.dwMajorVersion == 4 &&
          lstrcmpiA(osvi.szCSDVersion, "Service Pack 6") == 0) {
        HKEY hKey;
        LONG lRet;

        // Test for SP6 versus SP6a.

        lRet = RegOpenKeyExW(
          HKEY_LOCAL_MACHINE,
          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
          0, KEY_QUERY_VALUE, &hKey);

        if (lRet == ERROR_SUCCESS) {
          res += " Service Pack 6a (Build ";
          snprintf(buffer, sizeof(buffer), "%ld", osvi.dwBuildNumber & 0xFFFF);
          res += buffer;
          res += ")";
        } else // Windows NT 4.0 prior to SP6a
        {
          res += " ";
          res += osvi.szCSDVersion;
          res += " (Build ";
          snprintf(buffer, sizeof(buffer), "%ld", osvi.dwBuildNumber & 0xFFFF);
          res += buffer;
          res += ")";
        }

        RegCloseKey(hKey);
      } else // Windows NT 3.51 and earlier or Windows 2000 and later
      {
        res += " ";
        res += osvi.szCSDVersion;
        res += " (Build ";
        snprintf(buffer, sizeof(buffer), "%ld", osvi.dwBuildNumber & 0xFFFF);
        res += buffer;
        res += ")";
      }

      break;

      // Test for the Windows 95 product family.

    case VER_PLATFORM_WIN32_WINDOWS:

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
        res += "Microsoft Windows 95";
        if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B') {
          res += " OSR2";
        }
      }

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
        res += "Microsoft Windows 98";
        if (osvi.szCSDVersion[1] == 'A') {
          res += " SE";
        }
      }

      if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
        res += "Microsoft Windows Millennium Edition";
      }
      break;

    case VER_PLATFORM_WIN32s:

      res += "Microsoft Win32s";
      break;
  }
#endif

  return res;
}

bool SystemTools::ParseURLProtocol(const std::string& URL,
                                   std::string& protocol,
                                   std::string& dataglom, bool decode)
{
  // match 0 entire url
  // match 1 protocol
  // match 2 dataglom following protocol://
  kwsys::RegularExpression urlRe(VTK_URL_PROTOCOL_REGEX);

  if (!urlRe.find(URL))
    return false;

  protocol = urlRe.match(1);
  dataglom = urlRe.match(2);

  if (decode) {
    dataglom = DecodeURL(dataglom);
  }

  return true;
}

bool SystemTools::ParseURL(const std::string& URL, std::string& protocol,
                           std::string& username, std::string& password,
                           std::string& hostname, std::string& dataport,
                           std::string& database, bool decode)
{
  kwsys::RegularExpression urlRe(VTK_URL_REGEX);
  if (!urlRe.find(URL))
    return false;

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

  protocol = urlRe.match(1);
  username = urlRe.match(3);
  password = urlRe.match(5);
  hostname = urlRe.match(6);
  dataport = urlRe.match(8);
  database = urlRe.match(9);

  if (decode) {
    username = DecodeURL(username);
    password = DecodeURL(password);
    hostname = DecodeURL(hostname);
    dataport = DecodeURL(dataport);
    database = DecodeURL(database);
  }

  return true;
}

// ----------------------------------------------------------------------
std::string SystemTools::DecodeURL(const std::string& url)
{
  kwsys::RegularExpression urlByteRe(VTK_URL_BYTE_REGEX);
  std::string ret;
  for (size_t i = 0; i < url.length(); i++) {
    if (urlByteRe.find(url.substr(i, 3))) {
      char bytes[] = { url[i + 1], url[i + 2], '\0' };
      ret += static_cast<char>(strtoul(bytes, nullptr, 16));
      i += 2;
    } else {
      ret += url[i];
    }
  }
  return ret;
}

// ----------------------------------------------------------------------
// Do NOT initialize.  Default initialization to zero is necessary.
static unsigned int SystemToolsManagerCount;

// SystemToolsManager manages the SystemTools singleton.
// SystemToolsManager should be included in any translation unit
// that will use SystemTools or that implements the singleton
// pattern. It makes sure that the SystemTools singleton is created
// before and destroyed after all other singletons in CMake.

SystemToolsManager::SystemToolsManager()
{
  if (++SystemToolsManagerCount == 1) {
    SystemTools::ClassInitialize();
  }
}

SystemToolsManager::~SystemToolsManager()
{
  if (--SystemToolsManagerCount == 0) {
    SystemTools::ClassFinalize();
  }
}

#if defined(__VMS)
// On VMS we configure the run time C library to be more UNIX like.
// http://h71000.www7.hp.com/doc/732final/5763/5763pro_004.html
extern "C" int decc$feature_get_index(char* name);
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

  // Create statics singleton instance
  SystemToolsStatics = new SystemToolsStatic;

#if KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP
// Add some special translation paths for unix.  These are not added
// for windows because drive letters need to be maintained.  Also,
// there are not sym-links and mount points on windows anyway.
#  if !defined(_WIN32) || defined(__CYGWIN__)
  // The tmp path is frequently a logical path so always keep it:
  SystemTools::AddKeepPath("/tmp/");

  // If the current working directory is a logical path then keep the
  // logical name.
  std::string pwd_str;
  if (SystemTools::GetEnv("PWD", pwd_str)) {
    char buf[2048];
    if (const char* cwd = Getcwd(buf, 2048)) {
      // The current working directory may be a logical path.  Find
      // the shortest logical path that still produces the correct
      // physical path.
      std::string cwd_changed;
      std::string pwd_changed;

      // Test progressively shorter logical-to-physical mappings.
      std::string cwd_str = cwd;
      std::string pwd_path;
      Realpath(pwd_str, pwd_path);
      while (cwd_str == pwd_path && cwd_str != pwd_str) {
        // The current pair of paths is a working logical mapping.
        cwd_changed = cwd_str;
        pwd_changed = pwd_str;

        // Strip off one directory level and see if the logical
        // mapping still works.
        pwd_str = SystemTools::GetFilenamePath(pwd_str);
        cwd_str = SystemTools::GetFilenamePath(cwd_str);
        Realpath(pwd_str, pwd_path);
      }

      // Add the translation to keep the logical path name.
      if (!cwd_changed.empty() && !pwd_changed.empty()) {
        SystemTools::AddTranslationPath(cwd_changed, pwd_changed);
      }
    }
  }
#  endif
#endif
}

void SystemTools::ClassFinalize()
{
  delete SystemToolsStatics;
}

} // namespace KWSYS_NAMESPACE

#if defined(_MSC_VER) && defined(_DEBUG)
#  include <crtdbg.h>
#  include <stdio.h>
#  include <stdlib.h>
namespace KWSYS_NAMESPACE {

static int SystemToolsDebugReport(int, char* message, int* ret)
{
  if (ret) {
    // Pretend user clicked on Retry button in popup.
    *ret = 1;
  }
  fprintf(stderr, "%s", message);
  fflush(stderr);
  return 1; // no further reporting required
}

void SystemTools::EnableMSVCDebugHook()
{
  if (SystemTools::HasEnv("DART_TEST_FROM_DART") ||
      SystemTools::HasEnv("DASHBOARD_TEST_FROM_CTEST")) {
    _CrtSetReportHook(SystemToolsDebugReport);
  }
}

} // namespace KWSYS_NAMESPACE
#else
namespace KWSYS_NAMESPACE {
void SystemTools::EnableMSVCDebugHook()
{
}
} // namespace KWSYS_NAMESPACE
#endif
