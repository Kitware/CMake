#include <stdio.h>
#include <stdlib.h>

#include "lib.h"

#ifndef FLAG
#define FLAG "not given"
#endif

int main(int argc, char* argv[])
{
  printf("Flag: %s -> %s\n", argv[0], FLAG);
  lib();
  return EXIT_SUCCESS;
}
