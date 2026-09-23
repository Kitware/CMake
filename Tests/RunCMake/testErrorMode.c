#if defined(_MSC_VER) && _MSC_VER >= 1928
#  pragma warning(disable : 5105) /* macro expansion warning in windows.h */
#endif

#include <windows.h>

#include <stdio.h>
#include <string.h>

/* Verify that a test process inherited the error mode of its CTest mode.  */
int main(int argc, char** argv)
{
  UINT mode;
  int expectSuppressed;
  int suppressed;

  if (argc != 2) {
    fprintf(stderr, "usage: testErrorMode interactive|non-interactive\n");
    return 2;
  }
  if (strcmp(argv[1], "interactive") == 0) {
    expectSuppressed = 0;
  } else if (strcmp(argv[1], "non-interactive") == 0) {
    expectSuppressed = 1;
  } else {
    fprintf(stderr, "unknown mode: %s\n", argv[1]);
    return 2;
  }

  /* Older MinGW-w64 headers declare GetErrorMode only for Vista+.  */
  mode = SetErrorMode(0);
  SetErrorMode(mode);

  suppressed = (mode & SEM_NOGPFAULTERRORBOX) != 0;
  if (suppressed != expectSuppressed) {
    fprintf(stderr, "SEM_NOGPFAULTERRORBOX is %s, expected %s\n",
            suppressed ? "set" : "clear", expectSuppressed ? "set" : "clear");
    return 1;
  }
  return 0;
}
