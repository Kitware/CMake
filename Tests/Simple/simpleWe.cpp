#include <stdio.h>

class Foo 
{
public:
  Foo()
    {
      printf("This one has nonstandard extension\n");
    }
};

int bar()
{
  Foo f;
  return 0;
}
