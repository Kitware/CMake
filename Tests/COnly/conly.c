#include "foo.h"

#include "libc1.h"
#include "libc2.h"

#include <stdio.h>

int main ()
{
  if ( LibC1Func() != 2.0 )
    {
    printf("Problem with libc1\n");
    return 1;
    }
  if ( LibC2Func() != 1.0 )
    {
    printf("Problem with libc2\n");
    return 1;
    }
  printf("Foo: %s\n", foo);
  return 0;
}
