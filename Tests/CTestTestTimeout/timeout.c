#if defined(_WIN32)
# include <windows.h>
#else
# include <unistd.h>
#endif

#include <stdio.h>

int main(void)
{
#if defined(_WIN32)
  Sleep((TIMEOUT+4)*1000);
#else
  sleep((TIMEOUT+4));
#endif
  printf("timeout process finished sleeping!\n");
  return -1;
}
