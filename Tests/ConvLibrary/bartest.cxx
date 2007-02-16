extern "C" int bar();
int foo();
int car();

#include <stdio.h>
int main()
{
  if(foo() == 10)
    {
    printf("foo is 10!\n");
    }
  else
    {
    printf("foo is not 10 error!\n");
    return -1;
    }
  if(bar() == 20)
    {
    printf("bar is 20!\n");
    }
  else
    {
    printf("bar is not 20 error!\n");
    return -1;
    }
  if(car() == 30)
    {
    printf("car is 30!\n");
    }
  else
    {
    printf("car is not 30 error!\n");
    return -1;
    }
  printf("Test past\n");
  return 0;
}
