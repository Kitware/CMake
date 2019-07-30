#include "lib1.h"
#include "p.h"

int main(void)
{
  return func1() + func2() + func3() + func1p() + func2p() + func3p() +
    PROTO1 + PROTO2 + PROTO3;
}
