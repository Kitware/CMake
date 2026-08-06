#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  Usage: pseudo_asm [--version] [-?]

  --version: print a banner that identifies this program as Clang.
  -?: return 0 for MSVC-like command-line, 1 for GNU-like command-line,
      depending on the presence of the PSEUDO_ASM_GNU environment variable.
*/
int main(int argc, char* argv[])
{
  int i;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--version") == 0) {
      printf("clang version 99.0.0 (pseudo_asm)\n");
      return 0;
    }
    if (strcmp(argv[i], "-?") == 0) {
      char const* gnu = getenv("PSEUDO_ASM_GNU");
      return (gnu && *gnu) ? 1 : 0;
    }
  }
  return 0;
}
