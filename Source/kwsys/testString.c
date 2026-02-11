/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"
#include KWSYS_HEADER(String.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
#  include "String.h.in"
#endif

#include <ctype.h>
#include <locale.h>
#include <stdio.h>

#ifdef _WIN32
static int isprint_posix(int c)
{
  return isprint(c) && c != '\t';
}
#  define isprint isprint_posix
#endif

#define MAKE_testString_(isXXXXX)                                             \
  static int testString_##isXXXXX(void)                                       \
  {                                                                           \
    int i = 0;                                                                \
    for (; i < 128; ++i) {                                                    \
      if (!!isXXXXX(i) != !!kwsysString_##isXXXXX((char)i)) {                 \
        printf(#isXXXXX "(%d) failed: %d\n", i,                               \
               kwsysString_##isXXXXX((char)i));                               \
        return 1;                                                             \
      }                                                                       \
    }                                                                         \
    for (; i < 256; ++i) {                                                    \
      if (kwsysString_##isXXXXX((char)i)) {                                   \
        printf(#isXXXXX "(%d) failed: %d\n", i,                               \
               kwsysString_##isXXXXX((char)i));                               \
        return 1;                                                             \
      }                                                                       \
    }                                                                         \
    return 0;                                                                 \
  }                                                                           \
  static int testString_##isXXXXX(void)

MAKE_testString_(isalnum);
MAKE_testString_(isalpha);
MAKE_testString_(isascii);
MAKE_testString_(isblank);
MAKE_testString_(iscntrl);
MAKE_testString_(isdigit);
MAKE_testString_(isgraph);
MAKE_testString_(islower);
MAKE_testString_(isprint);
MAKE_testString_(ispunct);
MAKE_testString_(isspace);
MAKE_testString_(isupper);
MAKE_testString_(isxdigit);

static int testString_tolower(void)
{
  int i = 0;
  int j;
  for (; i < 'A'; ++i) {
    if ((j = kwsysString_tolower((char)i)) != i) {
      printf("tolower(%d) failed: %d\n", i, j);
      return 1;
    }
  }
  for (; i <= 'Z'; ++i) {
    if ((j = kwsysString_tolower((char)i)) != i + 32) {
      printf("tolower(%d) failed: %d\n", i, j);
      return 1;
    }
  }
  for (; i < 256; ++i) {
    if ((j = kwsysString_tolower((char)i)) != i) {
      printf("tolower(%d) failed: %d\n", i, j);
      return 1;
    }
  }
  return 0;
}

static int testString_toupper(void)
{
  int i = 0;
  int j;
  for (; i < 'a'; ++i) {
    if ((j = kwsysString_toupper((char)i)) != i) {
      printf("toupper(%d) failed: %d\n", i, j);
      return 1;
    }
  }
  for (; i <= 'z'; ++i) {
    if ((j = kwsysString_toupper((char)i)) != i - 32) {
      printf("toupper(%d) failed: %d\n", i, j);
      return 1;
    }
  }
  for (; i < 256; ++i) {
    if ((j = kwsysString_toupper((char)i)) != i) {
      printf("toupper(%d) failed: %d\n", i, j);
      return 1;
    }
  }
  return 0;
}

int testString(int argc, char* argv[])
{
  int result = 0;
  (void)argc;
  (void)argv;
  setlocale(LC_CTYPE, "");

  result |= testString_isalnum();
  result |= testString_isalpha();
  result |= testString_isascii();
  result |= testString_isblank();
  result |= testString_iscntrl();
  result |= testString_isdigit();
  result |= testString_isgraph();
  result |= testString_islower();
  result |= testString_isprint();
  result |= testString_ispunct();
  result |= testString_isspace();
  result |= testString_isupper();
  result |= testString_isxdigit();
  result |= testString_tolower();
  result |= testString_toupper();

  return result;
}
