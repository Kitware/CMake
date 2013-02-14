
#include "depB.h"
#include "depC.h"
#include "depIfaceOnly.h"

int main(int argc, char **argv)
{
  DepA a;
  DepB b;
  DepC c;

  DepIfaceOnly iface_only;

  return a.foo() + b.foo() + c.foo() + iface_only.foo();
}
