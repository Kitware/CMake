/* Distributed under the OSI-approved BSD 3-Clause License.
   See https://cmake.org/licensing#kwsys for details.  */
#if defined(_WIN32)
#  define NOMINMAX // hide min,max to not conflict with <limits>
#endif

#include <DynamicLoader.hxx>

#if defined(__hpux)
#  include <dl.h>

DynamicLoader::LibraryHandle DynamicLoader::OpenLibrary(
  const std::string& libname)
{
  return shl_load(libname.c_str(), BIND_DEFERRED | DYNAMIC_PATH, 0L);
}

int DynamicLoader::CloseLibrary(DynamicLoader::LibraryHandle lib)
{
  if (!lib) {
    return 0;
  }
  return !shl_unload(lib);
}

DynamicLoader::SymbolPointer DynamicLoader::GetSymbolAddress(
  DynamicLoader::LibraryHandle lib, const std::string& sym)
{
  void* addr;
  int status;

  /* TYPE_PROCEDURE Look for a function or procedure. (This used to be default)
   * TYPE_DATA      Look for a symbol in the data segment (for example,
   * variables).
   * TYPE_UNDEFINED Look for any symbol.
   */
  status = shl_findsym(&lib, sym.c_str(), TYPE_UNDEFINED, &addr);
  void* result = (status < 0) ? (void*)0 : addr;

  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<DynamicLoader::SymbolPointer*>(&result);
}

#elif defined(__APPLE__) && (MAC_OS_X_VERSION_MAX_ALLOWED < 1030)
#  include <mach-o/dyld.h>

DynamicLoader::LibraryHandle DynamicLoader::OpenLibrary(
  const std::string& libname)
{
  NSObjectFileImageReturnCode rc;
  NSObjectFileImage image = 0;

  rc = NSCreateObjectFileImageFromFile(libname.c_str(), &image);
  // rc == NSObjectFileImageInappropriateFile when trying to load a dylib file
  if (rc != NSObjectFileImageSuccess) {
    return 0;
  }
  NSModule handle = NSLinkModule(image, libname.c_str(),
                                 NSLINKMODULE_OPTION_BINDNOW |
                                   NSLINKMODULE_OPTION_RETURN_ON_ERROR);
  NSDestroyObjectFileImage(image);
  return handle;
}

int DynamicLoader::CloseLibrary(DynamicLoader::LibraryHandle lib)
{
  bool success = NSUnLinkModule(lib, NSUNLINKMODULE_OPTION_NONE);
  return success;
}

DynamicLoader::SymbolPointer DynamicLoader::GetSymbolAddress(
  DynamicLoader::LibraryHandle lib, const std::string& sym)
{
  void* result = 0;
  // Need to prepend symbols with '_' on Apple-gcc compilers
  std::string rsym = '_' + sym;

  NSSymbol symbol = NSLookupSymbolInModule(lib, rsym.c_str());
  if (symbol) {
    result = NSAddressOfSymbol(symbol);
  }

  // Hack to cast pointer-to-data to pointer-to-function.
  return *reinterpret_cast<DynamicLoader::SymbolPointer*>(&result);
}

#elif defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>

#  include <stdio.h>

DynamicLoader::LibraryHandle DynamicLoader::OpenLibrary(
  const std::string& libname)
{
  DynamicLoader::LibraryHandle lh;
  int length = MultiByteToWideChar(CP_UTF8, 0, libname.c_str(), -1, NULL, 0);
  wchar_t* wchars = new wchar_t[length + 1];
  wchars[0] = '\0';
  MultiByteToWideChar(CP_UTF8, 0, libname.c_str(), -1, wchars, length);
  lh = LoadLibraryW(wchars);
  delete[] wchars;
  return lh;
}

int DynamicLoader::CloseLibrary(DynamicLoader::LibraryHandle lib)
{
  return (int)FreeLibrary(lib);
}

DynamicLoader::SymbolPointer DynamicLoader::GetSymbolAddress(
  DynamicLoader::LibraryHandle lib, const std::string& sym)
{
  void* result;
#  if defined(__BORLANDC__) || defined(__WATCOMC__)
  // Need to prepend symbols with '_'
  std::string ssym = '_' + sym;
  const char* rsym = ssym.c_str();
#  else
  const char* rsym = sym.c_str();
#  endif
  result = (void*)GetProcAddress(lib, rsym);
// Hack to cast pointer-to-data to pointer-to-function.
#  ifdef __WATCOMC__
  return *(DynamicLoader::SymbolPointer*)(&result);
#  else
  return *reinterpret_cast<DynamicLoader::SymbolPointer*>(&result);
#  endif
}

#elif defined(__BEOS__)
#  include <be/kernel/image.h>
#  include <be/support/Errors.h>

static image_id last_dynamic_err = B_OK;

DynamicLoader::LibraryHandle DynamicLoader::OpenLibrary(
  const std::string& libname)
{
  // image_id's are integers, errors are negative. Add one just in case we
  //  get a valid image_id of zero (is that even possible?).
  image_id rc = load_add_on(libname.c_str());
  if (rc < 0) {
    last_dynamic_err = rc;
    return 0;
  }

  return rc + 1;
}

int DynamicLoader::CloseLibrary(DynamicLoader::LibraryHandle lib)
{
  if (!lib) {
    last_dynamic_err = B_BAD_VALUE;
    return 0;
  } else {
    // The function dlclose() returns 0 on success, and non-zero on error.
    status_t rc = unload_add_on(lib - 1);
    if (rc != B_OK) {
      last_dynamic_err = rc;
      return 0;
    }
  }

  return 1;
}

DynamicLoader::SymbolPointer DynamicLoader::GetSymbolAddress(
  DynamicLoader::LibraryHandle lib, const std::string& sym)
{
  // Hack to cast pointer-to-data to pointer-to-function.
  union
  {
    void* pvoid;
    DynamicLoader::SymbolPointer psym;
  } result;

  result.psym = NULL;

  if (!lib) {
    last_dynamic_err = B_BAD_VALUE;
  } else {
    // !!! FIXME: BeOS can do function-only lookups...does this ever
    // !!! FIXME:  actually _want_ a data symbol lookup, or was this union
    // !!! FIXME:  a leftover of dlsym()? (s/ANY/TEXT for functions only).
    status_t rc =
      get_image_symbol(lib - 1, sym.c_str(), B_SYMBOL_TYPE_ANY, &result.pvoid);
    if (rc != B_OK) {
      last_dynamic_err = rc;
      result.psym = NULL;
    }
  }
  return result.psym;
}

#elif defined(__MINT__)
#  define _GNU_SOURCE /* for program_invocation_name */
#  include <dld.h>
#  include <errno.h>
#  include <malloc.h>

DynamicLoader::LibraryHandle DynamicLoader::OpenLibrary(
  const std::string& libname)
{
  char* name = (char*)calloc(1, libname.size() + 1);
  dld_init(program_invocation_name);
  strncpy(name, libname.c_str(), libname.size());
  dld_link(libname.c_str());
  return (void*)name;
}

int DynamicLoader::CloseLibrary(DynamicLoader::LibraryHandle lib)
{
  dld_unlink_by_file((char*)lib, 0);
  free(lib);
  return 0;
}

DynamicLoader::SymbolPointer DynamicLoader::GetSymbolAddress(
  DynamicLoader::LibraryHandle lib, const std::string& sym)
{
  // Hack to cast pointer-to-data to pointer-to-function.
  union
  {
    void* pvoid;
    DynamicLoader::SymbolPointer psym;
  } result;
  result.pvoid = dld_get_symbol(sym.c_str());
  return result.psym;
}

#else
#  include <dlfcn.h>

DynamicLoader::LibraryHandle DynamicLoader::OpenLibrary(
  const std::string& libname)
{
  return dlopen(libname.c_str(), RTLD_LAZY);
}

int DynamicLoader::CloseLibrary(DynamicLoader::LibraryHandle lib)
{
  if (lib) {
    // The function dlclose() returns 0 on success, and non-zero on error.
    return !dlclose(lib);
  }
  // else
  return 0;
}

DynamicLoader::SymbolPointer DynamicLoader::GetSymbolAddress(
  DynamicLoader::LibraryHandle lib, const std::string& sym)
{
  // Hack to cast pointer-to-data to pointer-to-function.
  union
  {
    void* pvoid;
    DynamicLoader::SymbolPointer psym;
  } result;
  result.pvoid = dlsym(lib, sym.c_str());
  return result.psym;
}

#endif
