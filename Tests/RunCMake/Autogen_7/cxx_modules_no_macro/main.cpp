#include "object.h"

import Mod;

int main()
{
  Object object;
  return object.metaObject()->methodCount() > 0 && modValue() == 42 ? 0 : 1;
}
