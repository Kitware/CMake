
#ifdef DO_GNU_TESTS
#  ifndef TEST_DEFINE
#    error Expected TEST_DEFINE
#  endif
#endif

#include <cstring>

int main(int argc, char **argv)
{
#ifdef DO_GNU_TESTS
  return strcmp(EXPECTED_C_COMPILER_VERSION, TEST_C_COMPILER_VERSION)
      && strcmp(EXPECTED_CXX_COMPILER_VERSION, TEST_CXX_COMPILER_VERSION)
      && TEST_C_COMPILER_VERSION_EQUALITY
      && TEST_CXX_COMPILER_VERSION_EQUALITY;
#endif
  return 0;
}
