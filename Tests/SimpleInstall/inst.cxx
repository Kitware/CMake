#include "foo.h"

#include "lib1.h"
#include "lib2.h"

#include <stdio.h>

int main ()
{
  if ( Lib1Func() != 2.0 )
    {
    printf("Problem with lib1\n");
    return 1;
    }
  if ( Lib2Func() != 1.0 )
    {
    printf("Problem with lib2\n");
    return 1;
    }
  printf("Foo: %s\n", foo);
  return SomeFunctionInFoo(-5);
}
