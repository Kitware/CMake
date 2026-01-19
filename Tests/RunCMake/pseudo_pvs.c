#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
  FILE* f;
  int i;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-bad") == 0) {
      fprintf(stdout, "stdout from bad command line arg '-bad'\n");
      fprintf(stderr, "stderr from bad command line arg '-bad'\n");
      return 1;
    }
    if (strcmp(argv[i], "--output-file") == 0) {
      i++;
      f = fopen(argv[i], "w");
      if (!f) {
        fprintf(stderr, "Error opening %s for writing\n", argv[i]);
        return 1;
      }
      fprintf(f, "discard this line\nexample warning\n");
      fclose(f);
    }
  }
  return 0;
}
