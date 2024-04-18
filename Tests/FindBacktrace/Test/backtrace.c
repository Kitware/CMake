/* This is the code from `man backtrace_symbols`, reformatted, and without
 * requiring a command-line argument */

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BT_BUF_SIZE 100

void myfunc3(void)
{
  int nptrs;
  void* buffer[BT_BUF_SIZE];
  char** strings;
  size_t j;

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  printf("backtrace() returned %d addresses\n", nptrs);

  /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
     would produce similar output to the following: */

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  for (j = 0; j < nptrs; j++) {
    printf("%s\n", strings[j]);
  }

  free(strings);
}

static void /* "static" means don't export the symbol... */
myfunc2(void)
{
  myfunc3();
}

void myfunc(int ncalls)
{
  if (ncalls > 1) {
    myfunc(ncalls - 1);
  } else {
    myfunc2();
  }
}

int main(int argc, char* argv[])
{
  myfunc(5);
  exit(EXIT_SUCCESS);
}
