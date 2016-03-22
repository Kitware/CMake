#ifdef __cplusplus
# error "A C++ compiler has been selected for C."
#endif

#if !defined(__STDC__) || __STDC__ == 0
# define const
#endif

/*--------------------------------------------------------------------------*/

#include "CMakeCompilerABI.h"

/*--------------------------------------------------------------------------*/

#if !defined(__STDC__) || __STDC__ == 0
int main(argc, argv) int argc; char *argv[];
#else
int main(int argc, char *argv[])
#endif
{
  int require = 0;
  require += info_sizeof_dptr[argc];
#if defined(ABI_ID)
  require += info_abi[argc];
#endif
  (void)argv;
  return require;
}
