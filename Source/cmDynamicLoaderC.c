/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// This file is actually 4 different implementations.
// 1. HP machines which uses shl_load
// 2. Apple OSX which uses NSLinkModule
// 3. Windows which uses LoadLibrary
// 4. Most unix systems which use dlopen (default )
// Each part of the ifdef contains a complete implementation for
// the static methods of cmDynamicLoader.  

// Ugly stuff for library handles
// They are different on several different OS's
#if defined(__hpux)
# include <dl.h>
  typedef shl_t cmLibHandle;
#elif defined(_WIN32)
  #include <windows.h>
  typedef HMODULE cmLibHandle;
#else
  typedef void* cmLibHandle;
#endif

typedef void (*cmDynamicLoaderFunction)();

// ---------------------------------------------------------------
// 1. Implementation for HPUX  machines
#ifdef __hpux
#define CMDYNAMICLOADER_DEFINED 1
#include <dl.h>

cmDynamicLoaderFunction cmDynamicLoaderGetSymbolAddress(cmLibHandle lib,
                                                        const char* sym)
{ 
  void* addr;
  int status;
  
  status = shl_findsym (&lib, sym, TYPE_PROCEDURE, &addr);
  return (cmDynamicLoaderFunction)((status < 0) ? (void*)0 : addr);
}
#endif


// ---------------------------------------------------------------
// 2. Implementation for Darwin (including OSX) Machines

#ifdef __APPLE__
#define CMDYNAMICLOADER_DEFINED
#include <mach-o/dyld.h>

cmDynamicLoaderFunction cmDynamicLoaderGetSymbolAddress(cmLibHandle lib,
                                                        const char* sym)
{ 
  void *result=0;
  if(NSIsSymbolNameDefined(sym))
    {
    NSSymbol symbol= NSLookupAndBindSymbol(sym);
    if(symbol)
      {
      result = NSAddressOfSymbol(symbol);
      }
    }
  return (cmDynamicLoaderFunction)result;
}
#endif


// ---------------------------------------------------------------
// 3. Implementation for Windows win32 code
#ifdef _WIN32
#include <windows.h>
#define CMDYNAMICLOADER_DEFINED 1

cmDynamicLoaderFunction cmDynamicLoaderGetSymbolAddress(cmLibHandle lib,
                                                        const char* sym)
{ 
#ifdef UNICODE
        wchar_t *wsym = new wchar_t [mbstowcs(NULL, sym, 32000)];
        mbstowcs(wsym, sym, 32000);
        void *ret = GetProcAddress(lib, wsym);
        delete [] wsym;
        return (cmDynamicLoaderFunction)ret;
#else
  return (cmDynamicLoaderFunction)GetProcAddress(lib, sym);
#endif
}
#endif

// ---------------------------------------------------------------
// 4. Implementation for default UNIX machines.
// if nothing has been defined then use this
#ifndef CMDYNAMICLOADER_DEFINED
#define CMDYNAMICLOADER_DEFINED
// Setup for most unix machines
#include <dlfcn.h>

cmDynamicLoaderFunction cmDynamicLoaderGetSymbolAddress(cmLibHandle lib,
                                                        const char* sym)
{ 
  return (cmDynamicLoaderFunction)dlsym(lib, sym);
}
#endif
