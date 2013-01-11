
#include "depD.h"

int main(int argc, char **argv)
{
  DepD d;
  DepA a = d.getA();

  return d.foo() + a.foo();
}
