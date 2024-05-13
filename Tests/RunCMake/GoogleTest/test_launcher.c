#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER) && _MSC_VER < 1900
#  include <stdarg.h>
static int snprintf(char* buffer, size_t count, const char* format, ...)
{
  int n;
  va_list argptr;
  va_start(argptr, format);
  n = _vscprintf(format, argptr);
  vsnprintf_s(buffer, count, _TRUNCATE, format, argptr);
  va_end(argptr);
  return n;
}
#endif

static int launch(int argc, const char* argv[])
{
  char cmd[4096];
  size_t len = 0;
  const char* sep = "";
  int i;
  int n;
#ifdef _WIN32
  n = snprintf(cmd + len, sizeof(cmd) - len, "cmd /C \"");
  if (n < 0) {
    return 1;
  }
  len += n;
#endif
  for (i = 0; i < argc; ++i) {
    n = snprintf(cmd + len, sizeof(cmd) - len, "%s\"%s\"", sep, argv[i]);
    if (n < 0) {
      return 1;
    }
    len += n;
    if (len >= sizeof(cmd)) {
      fprintf(stderr, "error: command too long\n");
      return 1;
    }
    sep = " ";
  }
#ifdef _WIN32
  printf("launching: %s\n", cmd + 8);
  n = snprintf(cmd + len, sizeof(cmd) - len, "\"");
  if (n < 1) {
    return 1;
  }
#else
  printf("launching: %s\n", cmd);
#endif
  fflush(stdout);
  return system(cmd);
}

int main(int argc, const char* argv[])
{
  int ownArgs = 1;
  int i;
  for (i = 0; i < argc; ++i) {
    printf("test_launcher: got arg %d '%s'\n", i, argv[i]);
    if (ownArgs && strcmp(argv[i], "--") == 0) {
      ownArgs = 0;
    } else if (!ownArgs) {
      return launch(argc - i, argv + i);
    }
  }
  return 1;
}
