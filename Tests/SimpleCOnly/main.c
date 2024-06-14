#include <stdio.h>

extern int foo();
extern int bar();

int main(void)
{
  int i = foo();
  int k = bar();
  i = i * k;
  return i;
}
