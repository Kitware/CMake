#include <stdio.h>

#include "extra_ispc.h"

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

EXPORT int extra()
{
  float vin[16], vout[16];
  for (int i = 0; i < 16; ++i)
    vin[i] = i;

  ispc::extra(vin, vout, 16);

  for (int i = 0; i < 16; ++i)
    printf("%d: extra(%f) = %f\n", i, vin[i], vout[i]);

  return 0;
}
