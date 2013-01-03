
#include "depB.h"

#include "depA.h"

int DepB::foo()
{
  DepA a;

  return a.foo();
}
