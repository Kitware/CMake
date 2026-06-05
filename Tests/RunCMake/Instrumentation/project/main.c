#include <stdio.h>

#include "lib.h"

int main(void)
{
  fprintf(stdout, "test stdout\n");
  fprintf(stderr, "test stderr\n");
  return lib();
}
