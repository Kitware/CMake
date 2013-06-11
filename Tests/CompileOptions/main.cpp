
#ifdef DO_GNU_TESTS
#  ifndef TEST_DEFINE
#    error Expected TEST_DEFINE
#  endif
#endif

#include <cstring>

int main(int argc, char **argv)
{
#ifdef DO_GNU_TESTS
  return (strcmp(NEEDS_ESCAPE, "E$CAPE") == 0
      && strcmp(EXPECTED_C_COMPILER_VERSION, TEST_C_COMPILER_VERSION) == 0
      && strcmp(EXPECTED_CXX_COMPILER_VERSION, TEST_CXX_COMPILER_VERSION) == 0
      && TEST_C_COMPILER_VERSION_EQUALITY == 1
      && TEST_CXX_COMPILER_VERSION_EQUALITY == 1) ? 0 : 1;
#endif
  return 0;
}
