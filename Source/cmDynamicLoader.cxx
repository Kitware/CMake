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
#include "cmDynamicLoader.h"

// This file is actually several different implementations.
// 1. HP machines which uses shl_load
// 2. Apple OSX which uses NSLinkModule
// 3. Windows which uses LoadLibrary
// 4. Most unix systems which use dlopen (default )
// Each part of the ifdef contains a complete implementation for
// the static methods of cmDynamicLoader.  


class cmDynamicLoaderCache 
{
public:
  ~cmDynamicLoaderCache();
  void CacheFile(const char* path, const cmLibHandle&);
  bool GetCacheFile(const char* path, cmLibHandle&);
  bool FlushCache(const char* path);
  void FlushCache();
  static cmDynamicLoaderCache* GetInstance();

private:
  std::map<std::string, cmLibHandle> m_CacheMap;
  static cmDynamicLoaderCache* Instance;
};

cmDynamicLoaderCache* cmDynamicLoaderCache::Instance = 0;

cmDynamicLoaderCache::~cmDynamicLoaderCache()
{
  this->FlushCache();
}

void cmDynamicLoaderCache::CacheFile(const char* path, const cmLibHandle& p)
{
  cmLibHandle h;
  if ( this->GetCacheFile(path, h) )
    {
    this->FlushCache(path);
    }
  this->m_CacheMap[path] = p;
}

bool cmDynamicLoaderCache::GetCacheFile(const char* path, cmLibHandle& p)
{
  std::map<std::string, cmLibHandle>::iterator it = m_CacheMap.find(path);
  if ( it != m_CacheMap.end() )
    {
    p = it->second;
    return true;
    }
  return false;
}

bool cmDynamicLoaderCache::FlushCache(const char* path)
{
  std::map<std::string, cmLibHandle>::iterator it = m_CacheMap.find(path);
  if ( it != m_CacheMap.end() )
    {
    cmDynamicLoader::CloseLibrary(it->second);
    m_CacheMap.erase(it);
    return true;
    }
  return false;
}

void cmDynamicLoaderCache::FlushCache()
{
  for ( std::map<std::string, cmLibHandle>::iterator it = m_CacheMap.begin();
        it != m_CacheMap.end(); it++ )
    {
    cmDynamicLoader::CloseLibrary(it->second);
    }
  m_CacheMap.erase(m_CacheMap.begin(), m_CacheMap.end());
}

cmDynamicLoaderCache* cmDynamicLoaderCache::GetInstance()
{
  if ( !cmDynamicLoaderCache::Instance )
    {
    cmDynamicLoaderCache::Instance = new cmDynamicLoaderCache;
    }
  return cmDynamicLoaderCache::Instance;
}

// ---------------------------------------------------------------
// 1. Implementation for HPUX  machines
#ifdef __hpux
#define CMDYNAMICLOADER_DEFINED 1
#include <dl.h>

cmLibHandle cmDynamicLoader::OpenLibrary(const char* libname )
{
  cmLibHandle lh;
  if ( cmDynamicLoaderCache::GetInstance()->GetCacheFile(libname, lh) )
    {
    return lh;
    }
  
  lh = shl_load(libname, BIND_DEFERRED | DYNAMIC_PATH, 0L);
  cmDynamicLoaderCache::GetInstance()->CacheFile(libname, lh);
  return lh;
}

int cmDynamicLoader::CloseLibrary(cmLibHandle lib)
{
  return 0;
}

cmDynamicLoaderFunction
cmDynamicLoader::GetSymbolAddress(cmLibHandle lib, const char* sym)
{ 
  void* addr;
  int status;
  
  status = shl_findsym (&lib, sym, TYPE_PROCEDURE, &addr);
  void* result = (status < 0) ? (void*)0 : addr;
  
  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<cmDynamicLoaderFunction*>(&result);
}

const char* cmDynamicLoader::LibPrefix()
{ 
  return "lib";
}

const char* cmDynamicLoader::LibExtension()
{
  return ".sl";
}

const char* cmDynamicLoader::LastError()
{
  return 0;
}
#endif



// ---------------------------------------------------------------
// 2. Implementation for Darwin (including OSX) Machines

#ifdef __APPLE__
#define CMDYNAMICLOADER_DEFINED
#include <mach-o/dyld.h>

cmLibHandle cmDynamicLoader::OpenLibrary(const char* libname )
{
  cmLibHandle lh;
  if ( cmDynamicLoaderCache::GetInstance()->GetCacheFile(libname, lh) )
    {
    return lh;
    }
  
  NSObjectFileImageReturnCode rc;
  NSObjectFileImage image;

  rc = NSCreateObjectFileImageFromFile(libname, &image);
  lh = NSLinkModule(image, libname, TRUE);
  cmDynamicLoaderCache::GetInstance()->CacheFile(libname, lh);
  return lh; 
}

int cmDynamicLoader::CloseLibrary(cmLibHandle lib)
{
  return 0;
}

cmDynamicLoaderFunction
cmDynamicLoader::GetSymbolAddress(cmLibHandle lib, const char* sym)
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
  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<cmDynamicLoaderFunction*>(&result);
}

const char* cmDynamicLoader::LibPrefix()
{
  return "lib";
}

const char* cmDynamicLoader::LibExtension()
{
  return ".so";
}

const char* cmDynamicLoader::LastError()
{
  return 0;
}

#endif




// ---------------------------------------------------------------
// 3. Implementation for Windows win32 code
#ifdef _WIN32
#include <windows.h>
#define CMDYNAMICLOADER_DEFINED 1

cmLibHandle cmDynamicLoader::OpenLibrary(const char* libname )
{
  cmLibHandle lh;
  if ( cmDynamicLoaderCache::GetInstance()->GetCacheFile(libname, lh) )
    {
    return lh;
    }
#ifdef UNICODE
  wchar_t *libn = new wchar_t [mbstowcs(NULL, libname, 32000)];
  mbstowcs(libn, libname, 32000);
  cmLibHandle ret = LoadLibrary(libn);
  delete [] libn;
  lh = ret;
#else
  lh = LoadLibrary(libname);
#endif
  
  cmDynamicLoaderCache::GetInstance()->CacheFile(libname, lh);
  return lh;
}

int cmDynamicLoader::CloseLibrary(cmLibHandle lib)
{
  return (int)FreeLibrary(lib);
}

cmDynamicLoaderFunction
cmDynamicLoader::GetSymbolAddress(cmLibHandle lib, const char* sym)
{ 
  void* result = 0;
#ifdef UNICODE
        wchar_t *wsym = new wchar_t [mbstowcs(NULL, sym, 32000)];
        mbstowcs(wsym, sym, 32000);
        void *ret = GetProcAddress(lib, wsym);
        delete [] wsym;
        result = ret;
#else
  result = GetProcAddress(lib, sym);
#endif
  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<cmDynamicLoaderFunction*>(&result);
}

const char* cmDynamicLoader::LibPrefix()
{ 
  return "";
}

const char* cmDynamicLoader::LibExtension()
{
  return ".dll";
}

const char* cmDynamicLoader::LastError()
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
  
  // Free the buffer.
  LocalFree( lpMsgBuf );
  static char* str = 0;
  delete [] str;
  str = strcpy(new char[strlen((char*)lpMsgBuf)+1], (char*)lpMsgBuf);
  return str;
}

#endif

// ---------------------------------------------------------------
// 4. Implementation for default UNIX machines.
// if nothing has been defined then use this
#ifndef CMDYNAMICLOADER_DEFINED
#define CMDYNAMICLOADER_DEFINED
// Setup for most unix machines
#include <dlfcn.h>

cmLibHandle cmDynamicLoader::OpenLibrary(const char* libname )
{
  cmLibHandle lh;
  if ( cmDynamicLoaderCache::GetInstance()->GetCacheFile(libname, lh) )
    {
    return lh;
    }
  
  lh = dlopen(libname, RTLD_LAZY);
  cmDynamicLoaderCache::GetInstance()->CacheFile(libname, lh);
  return lh;
}

int cmDynamicLoader::CloseLibrary(cmLibHandle lib)
{
  return (int)dlclose(lib);
}

cmDynamicLoaderFunction
cmDynamicLoader::GetSymbolAddress(cmLibHandle lib, const char* sym)
{ 
  void* result = dlsym(lib, sym);
  
  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<cmDynamicLoaderFunction*>(&result);
}

const char* cmDynamicLoader::LibPrefix()
{ 
  return "lib";
}

const char* cmDynamicLoader::LibExtension()
{
  return ".so";
}

const char* cmDynamicLoader::LastError()
{
  return dlerror(); 
}
#endif
