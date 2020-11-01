#include <iostream>
#include <string>

#include <example.h>
#include <example_exe.h>
#include <stdio.h>

#include "DynamicLoader.hxx"

// Implement the ABI used by plugins.
extern "C" int example_exe_function()
{
  std::cout << "hello" << std::endl;
  return 123;
}

#ifdef CMAKE_INTDIR
#  define CONFIG_DIR "/" CMAKE_INTDIR
#else
#  define CONFIG_DIR ""
#endif

int main()
{
  std::string const libName = EXAMPLE_EXE_PLUGIN_DIR CONFIG_DIR
    "/" EXAMPLE_EXE_MOD_PREFIX "example_mod_1" EXAMPLE_EXE_MOD_SUFFIX;
  DynamicLoader::LibraryHandle handle = DynamicLoader::OpenLibrary(libName);
  if (!handle) {
    // Leave the .c_str() on this one.  It is needed on OpenWatcom.
    std::cerr << "Could not open plugin \"" << libName.c_str() << "\"!"
              << std::endl;
    return 1;
  }
  DynamicLoader::SymbolPointer sym =
    DynamicLoader::GetSymbolAddress(handle, "example_mod_1_function");
  if (!sym) {
    std::cerr << "Could not get plugin symbol \"example_mod_1_function\"!"
              << std::endl;
    return 1;
  }
#ifdef __WATCOMC__
  int(__cdecl * f)(int) = (int(__cdecl*)(int))(sym);
#else
  int (*f)(int) = reinterpret_cast<int (*)(int)>(sym);
#endif
  if (f(456) != (123 + 456)) {
    std::cerr << "Incorrect return value from plugin!" << std::endl;
    return 1;
  }
  DynamicLoader::CloseLibrary(handle);
  return 0;
}
