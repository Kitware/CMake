module;

#include "unity.h"

export module importable;

export int from_import()
{
  return unity1() + unity2();
}
