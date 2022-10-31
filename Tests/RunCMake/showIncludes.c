#if defined(_MSC_VER) && _MSC_VER >= 1928
#  pragma warning(disable : 5105) /* macro expansion warning in windows.h */
#endif
#include <windows.h>

#include <stdio.h>

int main()
{
  /* 'cl /showIncludes' encodes output in the console output code page.  */
  unsigned int cp = GetConsoleOutputCP();
  printf("Console output code page: %u\n", cp);
  printf("Console input code page: %u\n", GetConsoleCP());
  printf("ANSI code page: %u\n", GetACP());
  printf("OEM code page: %u\n", GetOEMCP());

  if (cp == 54936 || cp == 936) {
    /* VSLANG=2052 */
    printf("\xd7\xa2\xd2\xe2: "
           "\xb0\xfc\xba\xac\xce\xc4\xbc\xfe:\n");
    return 0;
  }

  if (cp == 65001) {
    /* VSLANG=2052  */
    printf("\xe6\xb3\xa8\xe6\x84\x8f: "
           "\xe5\x8c\x85\xe5\x90\xab\xe6\x96\x87\xe4\xbb\xb6:\n");
    return 0;
  }

  fprintf(stderr, "No example showIncludes for console's output code page.\n");
  return 1;
}
