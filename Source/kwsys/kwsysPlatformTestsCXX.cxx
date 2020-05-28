/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */

#ifdef TEST_KWSYS_CXX_STAT_HAS_ST_MTIM
#  include <sys/types.h>

#  include <sys/stat.h>
#  include <unistd.h>
int main()
{
  struct stat stat1;
  (void)stat1.st_mtim.tv_sec;
  (void)stat1.st_mtim.tv_nsec;
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_STAT_HAS_ST_MTIMESPEC
#  include <sys/types.h>

#  include <sys/stat.h>
#  include <unistd.h>
int main()
{
  struct stat stat1;
  (void)stat1.st_mtimespec.tv_sec;
  (void)stat1.st_mtimespec.tv_nsec;
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_SETENV
#  include <stdlib.h>
int main()
{
  return setenv("A", "B", 1);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_UNSETENV
#  include <stdlib.h>
int main()
{
  unsetenv("A");
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H
#  include <stdlib.h>
int main()
{
  char* e = environ[0];
  return e ? 0 : 1;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_GETLOADAVG
// Match feature definitions from SystemInformation.cxx
#  if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
#    define _GNU_SOURCE
#  endif
#  include <stdlib.h>
int main()
{
  double loadavg[3] = { 0.0, 0.0, 0.0 };
  return getloadavg(loadavg, 3);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_RLIMIT64
#  include <sys/resource.h>
int main()
{
  struct rlimit64 rlim;
  return getrlimit64(0, &rlim);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_UTIMES
#  include <sys/time.h>
int main()
{
  struct timeval* current_time = 0;
  return utimes("/example", current_time);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_UTIMENSAT
#  include <fcntl.h>
#  include <sys/stat.h>
#  if defined(__APPLE__)
#    include <AvailabilityMacros.h>
#    if MAC_OS_X_VERSION_MIN_REQUIRED < 101300
#      error "utimensat not available on macOS < 10.13"
#    endif
#  endif
int main()
{
  struct timespec times[2] = { { 0, UTIME_OMIT }, { 0, UTIME_NOW } };
  return utimensat(AT_FDCWD, "/example", times, AT_SYMLINK_NOFOLLOW);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_BACKTRACE
#  if defined(__PATHSCALE__) || defined(__PATHCC__) ||                        \
    (defined(__LSB_VERSION__) && (__LSB_VERSION__ < 41))
backtrace does not work with this compiler or os
#  endif
#  if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
#    define _GNU_SOURCE
#  endif
#  include <execinfo.h>
int main()
{
  void* stackSymbols[256];
  backtrace(stackSymbols, 256);
  backtrace_symbols(&stackSymbols[0], 1);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_DLADDR
#  if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
#    define _GNU_SOURCE
#  endif
#  include <dlfcn.h>
int main()
{
  Dl_info info;
  int ierr = dladdr((void*)main, &info);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_CXXABI
#  if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
#    define _GNU_SOURCE
#  endif
#  if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5130 && __linux &&             \
    __SUNPRO_CC_COMPAT == 'G'
#    include <iostream>
#  endif
#  include <cxxabi.h>
int main()
{
  int status = 0;
  size_t bufferLen = 512;
  char buffer[512] = { '\0' };
  const char* function = "_ZN5kwsys17SystemInformation15GetProgramStackEii";
  char* demangledFunction =
    abi::__cxa_demangle(function, buffer, &bufferLen, &status);
  return status;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_WSTRING
#  include <string>
void f(std::wstring*)
{
}
int main()
{
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_EXT_STDIO_FILEBUF_H
#  include <ext/stdio_filebuf.h>
int main()
{
  return 0;
}
#endif
