#ifndef TEST_DEFINE
# error Expected definition TEST_DEFINE
#endif

#ifndef NEEDS_ESCAPE
# error Expected definition NEEDS_ESCAPE
#endif

#ifdef DO_GNU_TESTS
# ifndef TEST_DEFINE_GNU
#  error Expected definition TEST_DEFINE_GNU
# endif
#endif

#include <string.h>

int main()
{
  return strcmp(NEEDS_ESCAPE, "E$CAPE") == 0 ? 0 : 1;
}
