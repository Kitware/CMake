
#include "testSharedLibDepends.h"

int TestSharedLibDepends::foo()
{
  TestSharedLibRequired req;
  return req.foo();
}
