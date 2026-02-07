#include <stdio.h>

extern void liba_greet();
extern void libb_greet();
extern void libc_greet();

int main()
{
  printf("Hello from C main\n");
  liba_greet();
  libb_greet();
  libc_greet();

  return 0;
}
