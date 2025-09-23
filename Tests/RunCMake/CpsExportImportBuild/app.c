#include <stdio.h>

extern
#ifdef _WIN32
__declspec(dllimport)
#endif
int ask(void);

int main(void)
{
  printf("%i\n", ask());
  return 0;
}
