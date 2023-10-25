#include <stdio.h>

extern int qax(void);
extern int qux(void);

int main(void)
{
  printf("qux: %d qax: %d\n", qux(), qax());

  return 0;
}
