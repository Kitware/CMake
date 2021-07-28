#ifndef TEST_DEFINE
#  error Expected definition TEST_DEFINE
#endif

#ifndef NEEDS_ESCAPE
#  error Expected definition NEEDS_ESCAPE
#endif

#ifdef DO_GNU_TESTS
#  ifndef TEST_DEFINE_GNU
#    error Expected definition TEST_DEFINE_GNU
#  endif
#  ifndef TEST_DEFINE_CXX_AND_GNU
#    error Expected definition TEST_DEFINE_CXX_AND_GNU
#  endif
#endif

#ifndef NO_DEF_TESTS
#  ifndef DEF_A
#    error Expected definition DEF_A
#  endif

#  ifndef DEF_B
#    error Expected definition DEF_B
#  endif

#  ifndef DEF_C
#    error Expected definition DEF_C
#  endif

#  ifndef DEF_D
#    error Expected definition DEF_D
#  endif

#  ifndef DEF_STR
#    error Expected definition DEF_STR
#  endif
#endif

#ifdef DO_FLAG_TESTS
#  if FLAG_A != 2
#    error "FLAG_A is not 2"
#  endif
#  if FLAG_B != 2
#    error "FLAG_B is not 2"
#  endif
#  if FLAG_C != 2
#    error "FLAG_C is not 2"
#  endif
#  if FLAG_D != 2
#    error "FLAG_D is not 2"
#  endif
#  if defined(FLAG_E) && FLAG_E != 2
#    error "FLAG_E is not 2"
#  endif
#endif

#include <string.h>

int main()
{
  return (strcmp(NEEDS_ESCAPE, "E$CAPE") == 0
#ifdef TEST_OCTOTHORPE
          && strcmp(TEST_OCTOTHORPE, "#") == 0
#endif
#ifndef NO_DEF_TESTS
          && strcmp(DEF_STR, "string with spaces") == 0
#endif
          &&
          strcmp(EXPECTED_C_COMPILER_VERSION, TEST_C_COMPILER_VERSION) == 0 &&
          strcmp(EXPECTED_CXX_COMPILER_VERSION, TEST_CXX_COMPILER_VERSION) == 0
#ifdef TEST_FORTRAN
          && strcmp(EXPECTED_Fortran_COMPILER_VERSION,
                    TEST_Fortran_COMPILER_VERSION) == 0
#endif
          && TEST_C_COMPILER_VERSION_EQUALITY == 1 &&
          TEST_CXX_COMPILER_VERSION_EQUALITY == 1
#ifdef TEST_FORTRAN
          && TEST_Fortran_COMPILER_VERSION_EQUALITY == 1
#endif
          )
    ? 0
    : 1;
}
