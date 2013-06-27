
#ifdef DO_GNU_TESTS
#  ifndef TEST_DEFINE
#    error Expected TEST_DEFINE
#  endif
#endif

#include <cstring>

int main(int argc, char **argv)
{
#ifdef DO_GNU_TESTS
  return strcmp(NEEDS_ESCAPE, "E$CAPE") == 0 ? 0 : 1;
#endif
  return 0;
}
