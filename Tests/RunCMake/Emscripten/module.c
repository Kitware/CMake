#include "module.h"
#if defined(__USE_LIBFUN)
extern int emscripten_libfun();
#endif

int i;
int main()
{
#if defined(__USE_LIBFUN)
  i = emscripten_libfun();
#else
  i = INTERNAL;
#endif
  return i;
}
