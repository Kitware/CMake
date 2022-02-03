#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
  FILE* source;
  FILE* target;
  int i;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-bad") == 0) {
      fprintf(stdout, "stdout from bad command line arg '-bad'\n");
      fprintf(stderr, "stderr from bad command line arg '-bad'\n");
      return 1;
    }
  }
  source = fopen(argv[argc - 1], "rb");
  if (source == NULL) {
    return 1;
  }
  target = fopen(argv[argc - 2], "wb");
  if (target != NULL) {
    char buffer[500];
    size_t n = fread(buffer, 1, sizeof(buffer), source);
    fwrite(buffer, 1, n, target);
    fclose(source);
    fclose(target);
    return 0;
  }
  return 1;
}
