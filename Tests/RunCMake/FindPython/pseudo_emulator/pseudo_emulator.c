
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

static int execute(char* argv[])
{
  pid_t my_pid;
  int status, timeout;
  struct timespec duration;
  duration.tv_sec = 0;
  duration.tv_nsec = 100000000;

  if (0 == (my_pid = fork())) {
    if (-1 == execve(argv[0], (char**)argv, NULL)) {
      perror("child process execve failed");
      return -1;
    }
  }

  timeout = 100;
  while (0 == waitpid(my_pid, &status, WNOHANG)) {
    if (--timeout < 0) {
      perror(argv[0]);
      return -1;
    }
    nanosleep(&duration, NULL);
  }

  if (1 != WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
    return -1;
  }

  return 0;
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    fprintf(stderr, "%s: require at least one argument.\n", argv[0]);
    return -1;
  }

  return execute(&argv[1]);
}
