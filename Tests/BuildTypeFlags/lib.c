#include <stdio.h>
#include <stdlib.h>

#include "lib.h"

#ifndef FLAG
#define FLAG "not given"
#endif

void lib()
{
  printf("Lib: %s\n", FLAG);
}
