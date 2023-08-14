#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__OpenBSD__)
/* NOLINTNEXTLINE(bugprone-reserved-identifier) */
#  define _XOPEN_SOURCE 600
#endif

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <sched.h>
#  include <unistd.h>
#endif

#include <stdio.h>

#ifdef SIGNAL
#  include <errno.h>
#  include <signal.h>
#  include <string.h>

static unsigned int signal_count;
static void signal_handler(int signum)
{
  (void)signum;
  ++signal_count;
}
#endif

int main(void)
{
#ifdef FORK
  pid_t pid = fork();
  if (pid != 0) {
    return 0;
  }
#endif

#ifdef SIGNAL
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_handler;
  while ((sigaction(SIGUSR1, &sa, NULL) < 0) && (errno == EINTR))
    ;
#endif

#if defined(_WIN32)
  Sleep((TIMEOUT + 4) * 1000);
#elif defined(SIGNAL_IGNORE)
#  if defined(__CYGWIN__) || defined(__sun__)
#    define ERRNO_IS_EINTR (errno == EINTR || errno == 0)
#  else
#    define ERRNO_IS_EINTR (errno == EINTR)
#  endif
  {
    unsigned int timeLeft = (TIMEOUT + 4 + SIGNAL_IGNORE);
    while ((timeLeft = sleep(timeLeft), timeLeft > 0 && ERRNO_IS_EINTR)) {
      printf("EINTR: timeLeft=%u\n", timeLeft);
      fflush(stdout);
    }
  }
#else
  sleep((TIMEOUT + 4));
#endif

#ifdef SIGNAL
  if (signal_count > 0) {
    printf("SIGUSR1: count=%u\n", signal_count);
    fflush(stdout);
  }
#endif

  return 0;
}
