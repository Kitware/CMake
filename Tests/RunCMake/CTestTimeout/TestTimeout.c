#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#include <stdio.h>

int main(void)
{
#ifdef FORK
  pid_t pid = fork();
  if (pid != 0) {
    return 0;
  }
#endif

#if defined(_WIN32)
  Sleep((TIMEOUT + 4) * 1000);
#else
  sleep((TIMEOUT + 4));
#endif
  return 0;
}
