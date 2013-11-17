
#include "zlib.h"

#ifndef FAKE_ZLIB
#error Expected FAKE_ZLIB
#endif

int main(int ,char **)
{
  return fakezlib();
}
