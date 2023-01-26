#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1928
#  pragma warning(disable : 5105) /* macro expansion warning in windows.h */
#endif
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

int main()
{
  /* 'cl /showIncludes' encodes output in the console output code page.  */
  unsigned int cp = GetConsoleOutputCP();

  /* 'cl /showIncludes' prints output in the VS language.  */
  const char* vslang = getenv("VSLANG");
  if (!vslang) {
    fprintf(stderr, "VSLANG is not set.\n");
    return 1;
  }

  printf("Console output code page: %u\n", cp);
  printf("Console input code page: %u\n", GetConsoleCP());
  printf("ANSI code page: %u\n", GetACP());
  printf("OEM code page: %u\n", GetOEMCP());
  printf("VSLANG: %s\n", vslang);

  if (strcmp(vslang, "2052") == 0) {
    if (cp == 54936 || cp == 936) {
      printf("\xd7\xa2\xd2\xe2: "
             "\xb0\xfc\xba\xac\xce\xc4\xbc\xfe:  C:\\foo.h\n");
      return 0;
    }

    if (cp == 65001) {
      printf("\xe6\xb3\xa8\xe6\x84\x8f: "
             "\xe5\x8c\x85\xe5\x90\xab\xe6\x96\x87\xe4\xbb\xb6:  C:\\foo.h\n");
      return 0;
    }
  }

  fprintf(stderr, "No example showIncludes for this code page and VSLANG.\n");
  return 1;
}
