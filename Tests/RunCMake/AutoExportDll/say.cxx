#include <stdio.h>

#include "hello.h"
#ifdef _MSC_VER
#  include "windows.h"
#else
#  define WINAPI
#endif

extern "C" {
// test __cdecl stuff
int WINAPI foo();
// test regular C
int bar();
int objlib();
void justnop();
}

// test c++ functions
// forward declare hello, world, cliFunction and nonCliFunction
void hello();
void world();
void cliFunction();
void nonCliFunction();

// test exports for executable target
extern "C" {
int own_auto_export_function(int i)
{
  return i + 1;
}
}

int main()
{
  // test static data (needs declspec to work)
  Hello::Data = 120;
  Hello h;
  h.real();
  hello();
  printf(" ");
  world();
  printf("\n");
  foo();
  printf("\n");
  bar();
  objlib();
  printf("\n");
  cliFunction();
  printf("\n");
  nonCliFunction();
  printf("\n");
#ifdef HAS_JUSTNOP
  justnop();
#endif
  return 0;
}
