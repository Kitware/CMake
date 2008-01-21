#ifdef __cplusplus
# error "A C++ compiler has been selected for C."
#endif

#ifdef __CLASSIC_C__
# define const
#endif

/*--------------------------------------------------------------------------*/

#include "CMakeCompilerABI.h"

/*--------------------------------------------------------------------------*/

/* Make sure the information strings are referenced.  */
#define REQUIRE(x) (&x[0] != &require)

int main()
{
  const char require = 0;
  return
    (
      REQUIRE(info_sizeof_dptr)
#if defined(ABI_ID)
      && REQUIRE(info_abi)
#endif
      );
}
