/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/* Stand-in for "make" used by the RunCMake.Make output-sync tests so they can
   assert what "cmake --build" passes to make without a real make tool.

   On "--version" (the output-sync probe): touch $FAKE_MAKE_PROBE_MARKER, print
   $FAKE_MAKE_VERSION (default a GNU Make 4.x banner), and exit with
   $FAKE_MAKE_VERSION_RESULT (default 0).
   Otherwise (the build): append the argument list to $FAKE_MAKE_RECORD and
   exit 0.  */

#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
  int i;
  int isVersion = 0;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--version") == 0) {
      isVersion = 1;
      break;
    }
  }

  if (isVersion) {
    char const* marker = getenv("FAKE_MAKE_PROBE_MARKER");
    char const* banner = getenv("FAKE_MAKE_VERSION");
    char const* result = getenv("FAKE_MAKE_VERSION_RESULT");
    if (marker) {
      FILE* f = fopen(marker, "a");
      if (f) {
        fprintf(f, "probe\n");
        fclose(f);
      }
    }
    if (!banner) {
      banner = "GNU Make 4.4.1";
    }
    if (banner[0] != '\0') {
      printf("%s\n", banner);
    }
    return result ? atoi(result) : 0;
  }

  {
    char const* record = getenv("FAKE_MAKE_RECORD");
    if (record) {
      FILE* f = fopen(record, "a");
      if (f) {
        for (i = 1; i < argc; ++i) {
          fprintf(f, "%s%s", i > 1 ? " " : "", argv[i]);
        }
        fprintf(f, "\n");
        fclose(f);
      }
    }
  }
  return 0;
}
