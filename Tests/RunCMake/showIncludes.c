#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1928
#  pragma warning(disable : 5105) /* macro expansion warning in windows.h */
#endif
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

int main(void)
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

  // clang-cl <= 17 (special case for test, not a real VS value).
  if (strcmp(vslang, "clang-cl-17") == 0) {
    if (cp == 437 || cp == 65001) {
      printf("Note: including file: ./foo.h\n");
      return 0;
    }
  }

  // clang-cl >= 18 (special case for test, not a real VS value).
  if (strcmp(vslang, "clang-cl-18") == 0) {
    if (cp == 437 || cp == 65001) {
      printf("Note: including file: .\\\\foo.h\n");
      return 0;
    }
  }

  // msvc-wine (special case for test, not a real VS value).
  if (strcmp(vslang, "msvc-wine") == 0) {
    if (cp == 437 || cp == 65001) {
      printf("Note: including file: /c/foo.h\n");
      return 0;
    }
  }

  // German.
  if (strcmp(vslang, "1031") == 0) {
    if (cp == 437 || cp == 65001) {
      printf("Hinweis: Einlesen der Datei: C:\\foo.h\n");
      return 0;
    }
  }

  // English.
  if (strcmp(vslang, "1033") == 0) {
    if (cp == 437 || cp == 65001) {
      printf("Note: including file: C:\\foo.h\n");
      return 0;
    }
  }

  // French.
  if (strcmp(vslang, "1036") == 0) {
    if (cp == 437 || cp == 863) {
      printf("Remarque\xff: inclusion du fichier\xff:  C:\\foo.h\n");
      return 0;
    }
    if (cp == 65001) {
      printf("Remarque\xc2\xa0: inclusion du fichier\xc2\xa0:  C:\\foo.h\n");
      return 0;
    }
  }

  // Italian.
  if (strcmp(vslang, "1040") == 0) {
    if (cp == 437 || cp == 65001) {
      printf("Nota: file incluso  C:\\foo.h\n");
      return 0;
    }
  }

  // Japanese.
  if (strcmp(vslang, "1041") == 0) {
    if (cp == 932) {
      printf("\x83\x81\x83\x82: "
             "\x83\x43\x83\x93\x83\x4e\x83\x8b\x81\x5b\x83\x68 "
             "\x83\x74\x83\x40\x83\x43\x83\x8b:  C:\\foo.h\n");
      return 0;
    }
    if (cp == 65001) {
      printf("\xe3\x83\xa1\xe3\x83\xa2: \xe3\x82\xa4\xe3\x83\xb3"
             "\xe3\x82\xaf\xe3\x83\xab\xe3\x83\xbc\xe3\x83\x89 "
             "\xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab:  C:\\foo.h\n");
      return 0;
    }
  }

  // Chinese.
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
