#include "module.h"
#if defined(__USE_LIBFUN)
extern int renesas_libfun();
#endif

int i;
int main()
{
#if defined(__USE_LIBFUN)
  i = renesas_libfun();
#else
  i = INTERNAL;
#endif
  return i;
}
