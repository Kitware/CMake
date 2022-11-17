#if defined(_WIN32)
#  include <windows.h>
#elif _XOPEN_SOURCE >= 500 || defined(_ALL_SOURCE)
#  include <unistd.h>
#else
#  include <time.h>

#  include <sys/select.h>
#endif

/* sleeps for 0.1 second */
int main(int argc, char** argv)
{
#if defined(_WIN32)
  Sleep(100);
#elif _XOPEN_SOURCE >= 500 || defined(_ALL_SOURCE)
  usleep(100 * 1000);
#else
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100 * 1000;

  select(0, NULL, NULL, NULL, &tv);
#endif
  return 0;
}
