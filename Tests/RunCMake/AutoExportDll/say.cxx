#include <iostream>
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
  std::cout << " ";
  world();
  std::cout << "\n" << foo() << "\n";
  std::cout << "\n" << bar() << "\n";
  std::cout << "\n";
  return 0;
}
