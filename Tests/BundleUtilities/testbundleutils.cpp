
#include "framework.h"
#include "shared.h"
#include "stdio.h"

#if defined(WIN32)
#include <windows.h>
#else
#include "dlfcn.h"
#endif

int main(int, char**)
{
  framework();
  shared();

#if defined(WIN32)
  HANDLE lib = LoadLibraryA("module.dll");
  if(!lib)
  {
    printf("Failed to open module\n");
  }
#else
  void* lib = dlopen("module.so", RTLD_LAZY);
  if(!lib)
  {
    printf("Failed to open module\n%s\n", dlerror());
  }
#endif


  return lib == 0 ? 1 : 0;
}
