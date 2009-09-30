#if defined(_WIN32)
# include <windows.h>
#else
# include <unistd.h>
#endif

int main(void)
{
#if defined(_WIN32)
  Sleep(5000);
#else
  sleep(5);
#endif
  return -1;
}
