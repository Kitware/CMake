#include <stdio.h>
#include "hello.h"
#ifdef _MSC_VER
#include "windows.h"
#else
#define WINAPI
#endif

extern "C"
{
// test __cdecl stuff
  int WINAPI foo();
// test regular C
  int bar();
}

// test c++ functions
// forward declare hello and world
void hello();
void world();

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
  printf("\n");
  return 0;
}
