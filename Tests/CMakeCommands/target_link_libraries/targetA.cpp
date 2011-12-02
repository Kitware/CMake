
#include "depB.h"
#include "depC.h"

int main(int argc, char **argv)
{
  DepA a;
  DepB b;
  DepC c;

  return a.foo() + b.foo() + c.foo();
}
