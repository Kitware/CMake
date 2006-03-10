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
#include KWSYS_HEADER(DynamicLoader.hxx)

#include KWSYS_HEADER(Configure.hxx)

#ifdef __APPLE__
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1030
#include <string.h> // for strlen
#endif //MAC_OS_X_VERSION_MIN_REQUIRED < 1030
#endif // __APPLE__

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "DynamicLoader.hxx.in"
# include "Configure.hxx.in"
#endif

// This file is actually 3 different implementations.
// 1. HP machines which uses shl_load
// 2. Mac OS X 10.2.x and earlier which uses NSLinkModule
// 3. Windows which uses LoadLibrary
// 4. Most unix systems (including Mac OS X 10.3 and later) which use dlopen (default)
// Each part of the ifdef contains a complete implementation for
// the static methods of DynamicLoader.

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
DynamicLoader::DynamicLoader()
{
}

//----------------------------------------------------------------------------
DynamicLoader::~DynamicLoader()
{
}

}

// ---------------------------------------------------------------
// 1. Implementation for HPUX  machines
#ifdef __hpux
#include <dl.h>
#define DYNAMICLOADER_DEFINED 1

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
LibHandle DynamicLoader::OpenLibrary(const char* libname )
{
  return shl_load(libname, BIND_DEFERRED | DYNAMIC_PATH, 0L);
}

//----------------------------------------------------------------------------
int DynamicLoader::CloseLibrary(LibHandle lib)
{
  return !shl_unload(lib);
}

//----------------------------------------------------------------------------
DynamicLoaderFunction
DynamicLoader::GetSymbolAddress(LibHandle lib, const char* sym)
{
  void* addr;
  int status;

  status = shl_findsym (&lib, sym, TYPE_PROCEDURE, &addr);
  void* result = (status < 0) ? (void*)0 : addr;

  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<DynamicLoaderFunction*>(&result);
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibPrefix()
{
  return "lib";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibExtension()
{
  return ".sl";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LastError()
{
  // TODO: Need implementation with errno/strerror
  /* If successful, shl_findsym returns an integer (int) value zero. If
   * shl_findsym cannot find sym, it returns -1 and sets errno to zero.
   * If any other errors occur, shl_findsym returns -1 and sets errno to one
   * of these values (defined in <errno.h>):
   * ENOEXEC
   * A format error was detected in the specified library.
   * ENOSYM
   * A symbol on which sym depends could not be found.
   * EINVAL
   * The specified handle is invalid.
   */

  return 0;
}

} // namespace KWSYS_NAMESPACE

#endif //__hpux


// ---------------------------------------------------------------
// 2. Implementation for Mac OS X 10.2.x and earlier
#ifdef __APPLE__
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1030
#include <mach-o/dyld.h>
#define DYNAMICLOADER_DEFINED 1

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
LibHandle DynamicLoader::OpenLibrary(const char* libname )
{
  NSObjectFileImageReturnCode rc;
  NSObjectFileImage image = 0;

  rc = NSCreateObjectFileImageFromFile(libname, &image);
  // rc == NSObjectFileImageInappropriateFile when trying to load a dylib file
  if( rc != NSObjectFileImageSuccess )
    {
    return 0;
    }
  return NSLinkModule(image, libname,
    NSLINKMODULE_OPTION_PRIVATE|NSLINKMODULE_OPTION_BINDNOW);
}

//----------------------------------------------------------------------------
int DynamicLoader::CloseLibrary( LibHandle lib)
{
  bool success = NSUnLinkModule(lib, NSUNLINKMODULE_OPTION_NONE);
  return success;
}

//----------------------------------------------------------------------------
DynamicLoaderFunction DynamicLoader::GetSymbolAddress(LibHandle lib, const char* sym)
{
  void *result=0;
  // Need to prepend symbols with '_' on Apple-gcc compilers
  size_t len = strlen(sym);
  char *rsym = new char[len + 1 + 1];
  strcpy(rsym, "_");
  strcat(rsym+1, sym);

  NSSymbol symbol = NSLookupSymbolInModule(lib, rsym);
  if(symbol)
    {
    result = NSAddressOfSymbol(symbol);
    }

  delete[] rsym;
  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<DynamicLoaderFunction*>(&result);
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibPrefix()
{
  return "lib";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibExtension()
{
  // NSCreateObjectFileImageFromFile fail when dealing with dylib image
  // it returns NSObjectFileImageInappropriateFile
  //return ".dylib";
  return ".so";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LastError()
{
  return 0;
}

} // namespace KWSYS_NAMESPACE

#endif //MAC_OS_X_VERSION_MIN_REQUIRED < 1030
#endif // __APPLE__

// ---------------------------------------------------------------
// 3. Implementation for Windows win32 code
#ifdef _WIN32
#include <windows.h>
#define DYNAMICLOADER_DEFINED 1

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
LibHandle DynamicLoader::OpenLibrary(const char* libname)
{
  LibHandle lh;
#ifdef UNICODE
  wchar_t libn[MB_CUR_MAX];
  mbstowcs(libn, libname, MB_CUR_MAX);
  lh = LoadLibrary(libn);
#else
  lh = LoadLibrary(libname);
#endif
  return lh;
}

//----------------------------------------------------------------------------
int DynamicLoader::CloseLibrary(LibHandle lib)
{
  return (int)FreeLibrary(lib);
}

//----------------------------------------------------------------------------
DynamicLoaderFunction DynamicLoader::GetSymbolAddress(LibHandle lib, const char* sym)
{
  void *result;
#ifdef __BORLANDC__
  // Need to prepend symbols with '_' on borland compilers
  size_t len = strlen(sym);
  char *rsym = new char[len + 1 + 1];
  strcpy(rsym, "_");
  strcat(rsym+1, sym);
#else
  const char *rsym = sym;
#endif
#ifdef UNICODE
  wchar_t wsym[MB_CUR_MAX];
  mbstowcs(wsym, rsym, MB_CUR_MAX);
  result = GetProcAddress(lib, wsym);
#else
  result = (void*)GetProcAddress(lib, rsym);
#endif
#ifdef __BORLANDC__
  delete[] rsym;
#endif
  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<DynamicLoaderFunction*>(&result);
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibPrefix()
{
  return "";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibExtension()
{
  return ".dll";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LastError()
{
  LPVOID lpMsgBuf;

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
    );

  static char* str = 0;
  delete [] str;
  str = strcpy(new char[strlen((char*)lpMsgBuf)+1], (char*)lpMsgBuf);
  // Free the buffer.
  LocalFree( lpMsgBuf );
  return str;
}

} // namespace KWSYS_NAMESPACE

#endif //_WIN32

// ---------------------------------------------------------------
// 4. Implementation for default UNIX machines.
// if nothing has been defined then use this
#ifndef DYNAMICLOADER_DEFINED
#define DYNAMICLOADER_DEFINED 1
// Setup for most unix machines
#include <dlfcn.h>

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
LibHandle DynamicLoader::OpenLibrary(const char* libname )
{
  return dlopen(libname, RTLD_LAZY);
}

//----------------------------------------------------------------------------
int DynamicLoader::CloseLibrary(LibHandle lib)
{
  if (lib)
    {
    // The function dlclose() returns 0 on success, and non-zero on error.
    return !dlclose(lib);
    }
  // else
  return 0;
}

//----------------------------------------------------------------------------
DynamicLoaderFunction DynamicLoader::GetSymbolAddress(LibHandle lib, const char* sym)
{
  void* result = dlsym(lib, sym);

  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<DynamicLoaderFunction*>(&result);
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibPrefix()
{
  return "lib";
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LibExtension()
{
#ifdef __CYGWIN__
  return ".dll";
#else
  return ".so";
#endif
}

//----------------------------------------------------------------------------
const char* DynamicLoader::LastError()
{
  return dlerror();
}

} // namespace KWSYS_NAMESPACE

#endif
