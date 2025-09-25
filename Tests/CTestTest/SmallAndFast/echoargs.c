#include <stdio.h>

int main(int argc, char const* argv[])
{
  int i = 0;
  for (; i < argc; ++i) {
    fprintf(stdout, "%s\n", argv[i]);
  }
  return 0;
}
