#include "module.h"
#if defined(__USE_LIBFUN)
extern int iar_libfun();
#endif
__root int i;
__root int main()
{
#if defined(__USE_LIBFUN)
  i = iar_libfun();
#else
  i = INTERNAL;
#endif
  return i;
}
