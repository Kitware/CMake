#include <stdio.h>

int foo();

#ifdef WITH_ZOOM
int zoom();
#endif

class A
{
public:
  A()
  {
    this->i = foo();
#ifdef WITH_ZOOM
    i += zoom();
    i -= zoom();
#endif
  }
  int i;
};

int main()
{
  A a;
  if (a.i == 21) {
    printf("passed foo is 21\n");
    return 0;
  }
  printf("Failed foo is not 21\n");
  return -1;
}
