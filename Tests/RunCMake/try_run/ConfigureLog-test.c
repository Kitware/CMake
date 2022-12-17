#include <stdio.h>

int main()
{
  fprintf(stderr, "Output, with backslash '\\', on stderr!\n");
  fflush(stderr); /* make output deterministic even if stderr is buffered */
  fprintf(stdout, "Output on stdout!\n");
  return 12;
}
