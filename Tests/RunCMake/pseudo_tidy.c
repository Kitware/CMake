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
    if (strcmp(argv[i], "-p") == 0) {
      // Ensure compile commands were not appended after the source file
      for (++i; i < argc; ++i) {
        if (strcmp(argv[i], "--") == 0) {
          fprintf(stderr, "Command line arguments unexpectedly appended\n");
          return 1;
        }
      }
      return 0;
    }
    if (strcmp(argv[i], "-bad") == 0) {
      fprintf(stdout, "stdout from bad command line arg '-bad'\n");
      fprintf(stderr, "stderr from bad command line arg '-bad'\n");
      return 1;
    }
    if (strncmp(argv[i], "--export-fixes=", 15) == 0) {
      f = fopen(argv[i] + 15, "w");
      if (!f) {
        fprintf(stderr, "Error opening %s for writing\n", argv[i] + 15);
        return 1;
      }
      fclose(f);
    }
    if (argv[i][0] != '-') {
      fprintf(stdout, "%s:0:0: warning: message [checker]\n", argv[i]);
      break;
    }
  }
  fprintf(stderr, "1 warning generated.\n");
  return 0;
}
