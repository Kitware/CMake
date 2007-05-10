#include "stdio.h"

const char* foo();
int main()
{
  int i;
  printf("%s\n", foo());
  fflush(stdout);
  for(;;);
  return 0;
}
