#include <stdio.h>

int main(int argc, char* argv[])
{
  int i;
  for (i = 1; i < argc; ++i) {
    if (argv[i][0] != '-') {
      fprintf(stdout, "%s:0:0: warning: message [checker]\n", argv[i]);
      break;
    }
  }
  fprintf(stderr, "1 warning generated.\n");
  return 0;
}
