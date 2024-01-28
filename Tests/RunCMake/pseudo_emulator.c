#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  int ii;
  const char* fail = getenv("PSEUDO_EMULATOR_FAIL");

  printf("Command:");
  for (ii = 1; ii < argc; ++ii) {
    printf(" \"%s\"", argv[ii]);
  }
  printf("\n");

  if (fail && *fail) {
    return 42;
  }

  return 0;
}
