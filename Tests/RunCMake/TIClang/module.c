#include "module.h"
#if defined(__USE_LIBFUN)
extern int ticlang_libfun();
#endif
int i;
int main()
{
#if defined(__USE_LIBFUN)
  i = ticlang_libfun();
#else
  i = INTERNAL;
#endif
  return i;
}
