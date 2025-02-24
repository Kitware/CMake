#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
  int result = 0;
  int i;
  for (i = 2; i < argc; i += 2) {
    if (strcmp(argv[i - 1], argv[i]) != 0) {
      fprintf(stderr, "Argument %d expected '%s' but got '%s'.\n", i, argv[i],
              argv[i - 1]);
      result = 1;
    }
  }
  return result;
}
