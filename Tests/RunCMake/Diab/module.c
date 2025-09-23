#include "module.h"
#if defined(__USE_LIBFUN)
extern int diab_libfun();
#endif

int i;
int main()
{
#if defined(__USE_LIBFUN)
  i = diab_libfun();
#else
  i = INTERNAL;
#endif
  return i;
}
