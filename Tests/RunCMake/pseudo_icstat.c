#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
  int i;
  int result = 0;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-bad") == 0) {
      fprintf(stdout, "stdout from bad command line arg '-bad'\n");
      fprintf(stderr, "stderr from bad command line arg '-bad'\n");
      return 1;
    }
  }
  fprintf(stderr,
          "\"foo/bar.c\",2 Severity-High[SPC-uninit-var-some]:"
          "Variable `i' may be uninitialized.\n\n");
  fprintf(stderr,
          "\"foo/bar.c\",2 Severity-Medium[MISRAC2012-Rule-8.2_a]:"
          "`main' does not have a valid prototype.\n\n");
  fprintf(stderr,
          "\"foo/bar.c\",2 Severity-Low[MISRAC2012-Rule-21.6]:"
          "Use of `stdio.h' is not compliant.\n\n");
  fprintf(stderr,
          "\"foo/bar.c\",2 Severity-Low[MISRAC2012-Rule-17.7]:"
          "The return value of this call to `printf()' is discarded.\n\n");
  return result;
}
