#include <stdio.h>
#include "hello.h"
int Hello::Data = 0;
void Hello::real()
{
  return;
}
void hello()
{
  printf("hello");
}
void Hello::operator delete[](void*) {};
void Hello::operator delete(void*) {};
