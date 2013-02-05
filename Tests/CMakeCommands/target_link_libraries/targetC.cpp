
#include "depG.h"

#include "foo.h"
#include "bar.h"

#ifndef TEST_DEF
#error Expected TEST_DEF definition
#endif

int main(int argc, char **argv)
{
  DepG g;

  return g.foo();
}
