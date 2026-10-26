#include <FortranBuildType.h>
#include <HelloWorldFCMangle.h> /* created by FortranCInterface */
#include <stdio.h>
#include <string.h>
extern void FC_hello(void);
int main(void)
{
#ifdef FORTRAN_BUILD_TYPE
  if (strcmp(FORTRAN_BUILD_TYPE, EXPECTED_BUILD_TYPE) != 0) {
    fprintf(stderr, "Fortran built as '%s', expected '%s'\n",
            FORTRAN_BUILD_TYPE, EXPECTED_BUILD_TYPE);
    return 1;
  }
#endif
  FC_hello();
  return 0;
}
