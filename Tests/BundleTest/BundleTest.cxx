#include <stdio.h>

#include <Carbon/Carbon.h>

extern int foo(char* exec);

int main(int argc, char* argv[])
{
  printf("Started with: %d arguments\n", argc);

  // Call a "Carbon" function... but pull in the link dependency on "-framework
  // Carbon" via CMake's dependency chaining mechanism. This code exists to
  // verify that the chaining mechanism works with "-framework blah" style
  // link dependencies.
  //
  CFBundleRef br = CFBundleGetMainBundle();
  (void) br;

  return foo(argv[0]);
}
