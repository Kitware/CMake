#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char const* get_filename(char const* path)
{
  // Find the last occurrence of '/'
  char const* last_slash = strrchr(path, '/');

  // If not found, try finding the last occurrence of '\'
  if (!last_slash) {
    last_slash = strrchr(path, '\\');
  }

  // If a separator was found, return the character after it
  // Otherwise, return the original path (no directory component)
  return last_slash ? last_slash + 1 : path;
}

int main(int argc, char* argv[])
{
  int i;
  int result = 0;
  char const* source = "";
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-bad") == 0) {
      fprintf(
        stdout,
        "C-STAT Command line error: Unexpected command line arguments:\n");
      fprintf(stdout, "            -bad\n");
      fprintf(stderr, "            -bad\n");
      return 1;
    }
    if (strcmp(argv[i], "--silent") == 0 && (i + 1) < argc) {
      source = get_filename(argv[++i]);
    }
  }
  fprintf(stderr, "C-STAT: Analyzing %s\n", source);
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
