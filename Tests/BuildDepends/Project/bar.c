#include "stdio.h"

const char* foo();
int main()
{
  printf("%s", foo());
  return 0;
}
