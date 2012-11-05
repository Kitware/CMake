
#include "depF.h"

#include "foo.h"
#include "bar.h"

#ifndef TEST_DEF
#error Expected TEST_DEF definition
#endif

int main(int argc, char **argv)
{
  DepF f;

  return f.foo();
}
