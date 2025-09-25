#include <stdio.h>

int main(int argc, char const* argv[])
{
  int i;
  for (i = 1; i < argc; ++i) {
    fprintf(stderr, "unexpected argument: '%s'\n", argv[i]);
  }
  return argc == 1 ? 0 : 1;
}
