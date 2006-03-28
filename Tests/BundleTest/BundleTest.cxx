#include <stdio.h>

extern int foo(char* exec);

int main(int argc, char* argv[])
{
  printf("Started with: %d arguments\n", argc);
  return foo(argv[0]);
}
