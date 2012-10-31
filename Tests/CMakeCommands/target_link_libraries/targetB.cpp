
#include "depD.h"
#include "depE.h"

int main(int argc, char **argv)
{
  DepD d;
  DepA a = d.getA();

  DepE e;
  DepB b = e.getB();

  return d.foo() + a.foo() + e.foo() + b.foo();
}
