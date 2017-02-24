#include <stdlib.h>
int myobj_foo();

void mylib_foo()
{
  exit(myobj_foo());
}
