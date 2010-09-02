#include "libcxx1.h"
#include "libcxx2.h"
extern int testCPP;
extern int testCXX;
extern int testCplusplus;

#include <stdio.h>

int main ()
{
  testCPP = testCXX= testCplusplus = 1;
  if ( LibCxx1Class::Method() != 2.0 )
    {
    printf("Problem with libcxx1\n");
    return 1;
    }
  if ( LibCxx2Class::Method() != 1.0 )
    {
    printf("Problem with libcxx2\n");
    return 1;
    }
  return 0;
}
