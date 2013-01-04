

#include "testSharedLibDepends.h"

int main(int,char **)
{
  TestSharedLibDepends dep;
  TestSharedLibRequired req;

  return dep.foo() + req.foo();
}
