/* Distributed under the OSI-approved BSD 3-Clause License.
   See https://cmake.org/licensing#kwsys for details.  */
#ifndef DynamicLoader_hxx
#define DynamicLoader_hxx

#include <string>

#if defined(__hpux)
#  include <dl.h>
#elif defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>
#elif defined(__APPLE__)
#  include <AvailabilityMacros.h>
#  if MAC_OS_X_VERSION_MAX_ALLOWED < 1030
#    include <mach-o/dyld.h>
#  endif
#elif defined(__BEOS__)
#  include <be/kernel/image.h>
#endif

class DynamicLoader
{
public:
#if defined(__hpux)
  typedef shl_t LibraryHandle;
#elif defined(_WIN32) && !defined(__CYGWIN__)
  typedef HMODULE LibraryHandle;
#elif defined(__APPLE__)
#  if MAC_OS_X_VERSION_MAX_ALLOWED < 1030
  typedef NSModule LibraryHandle;
#  else
  typedef void* LibraryHandle;
#  endif
#elif defined(__BEOS__)
  typedef image_id LibraryHandle;
#else // POSIX
  typedef void* LibraryHandle;
#endif

  typedef void (*SymbolPointer)();

  static LibraryHandle OpenLibrary(const std::string&);

  static int CloseLibrary(LibraryHandle);

  static SymbolPointer GetSymbolAddress(LibraryHandle, const std::string&);
};

#endif
