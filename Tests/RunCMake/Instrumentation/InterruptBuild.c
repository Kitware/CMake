/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/* Test helper to exercise instrumentation handling of an interrupted build.

   Usage: InterruptBuild <delay-seconds> <command> [args...]

   It runs <command> in its own process group, waits <delay-seconds>, then
   delivers a user interrupt to that group only -- mimicking a user pressing
   Ctrl+C on the build's foreground process group, while leaving the calling
   test driver untouched.  It then reports a fixed exit code so the test can
   assert deterministically:
     42  - the build was interrupted and then stopped (expected)
     2   - usage error
     3   - failed to start the build
     99  - the build did not stop after the interrupt
     100 + N - the build exited normally with code N (unexpected, POSIX only)

   On POSIX the interrupt is SIGINT sent to the child's process group; the
   instrumented `cmake` re-raises it, so the child terminates via a signal.
   On Windows the interrupt is a CTRL_BREAK_EVENT sent to the child's process
   group (created with CREATE_NEW_PROCESS_GROUP); `cmake`'s console handler
   treats CTRL_BREAK identically to CTRL_C.  Because Windows `cmake` returns
   the build's (failed) exit code rather than a signal status, success is
   detected as "the child exited promptly after the interrupt".  The real
   assertion -- that an interrupted snippet was written -- is done by the
   accompanying check script. */

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)

#  include <windows.h>

#  include <string.h>

int main(int argc, char** argv)
{
  int delay;
  char cmdline[32768];
  size_t len = 0;
  int i;
  size_t n;
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  DWORD waitRc;

  if (argc < 3) {
    fprintf(stderr,
            "Usage: InterruptBuild <delay-seconds> <command> [args...]\n");
    return 2;
  }
  delay = atoi(argv[1]);
  if (delay <= 0) {
    delay = 1;
  }

  /* Rebuild a command line from argv[2..].  The inputs are controlled by the
     test (a cmake path plus simple flags) and never contain embedded quotes,
     so wrapping each argument in quotes is sufficient and correct.  The buffer
     is sized to the CreateProcess command-line limit of 32768 characters,
     including the terminating null. */
  for (i = 2; i < argc; ++i) {
    n = strlen(argv[i]);
    if (len + n + 4 >= sizeof(cmdline)) {
      return 2;
    }
    if (i > 2) {
      cmdline[len++] = ' ';
    }
    cmdline[len++] = '"';
    memcpy(cmdline + len, argv[i], n);
    len += n;
    cmdline[len++] = '"';
  }
  cmdline[len] = '\0';

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  /* New process group so the control event reaches only the build (and its
     children), not this helper or the test driver. */
  if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
                      CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi)) {
    return 3;
  }

  Sleep((DWORD)delay * 1000);
  GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pi.dwProcessId);

  waitRc = WaitForSingleObject(pi.hProcess, 30000);
  if (waitRc != WAIT_OBJECT_0) {
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 99;
  }
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 42;
}

#else

#  include <signal.h>
#  include <unistd.h>

#  include <sys/types.h>
#  include <sys/wait.h>

int main(int argc, char** argv)
{
  int delay;
  pid_t build;
  pid_t killer;
  int status = 0;

  if (argc < 3) {
    fprintf(stderr,
            "Usage: InterruptBuild <delay-seconds> <command> [args...]\n");
    return 2;
  }
  delay = atoi(argv[1]);
  if (delay <= 0) {
    delay = 1;
  }

  build = fork();
  if (build < 0) {
    return 3;
  }
  if (build == 0) {
    /* Become the leader of a new process group, then run the build.  The
       native build tool inherits this group, so a later group signal reaches
       it too. */
    setpgid(0, 0);
    execvp(argv[2], &argv[2]);
    _exit(127);
  }

  /* Best-effort from the parent side to avoid a setpgid race; ignore errors.
   */
  setpgid(build, build);

  killer = fork();
  if (killer == 0) {
    sleep((unsigned int)delay);
    killpg(build, SIGINT);
    _exit(0);
  }

  waitpid(build, &status, 0);
  if (killer > 0) {
    kill(killer, SIGKILL);
    waitpid(killer, NULL, 0);
  }

  if (WIFSIGNALED(status)) {
    return 42;
  }
  if (WIFEXITED(status)) {
    return 100 + WEXITSTATUS(status);
  }
  return 99;
}

#endif
