#include <kwsys/Process.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
# include <windows.h>
#else
# include <unistd.h>
#endif

int test1()
{
  fprintf(stdout, "Output on stdout from test returning 0.\n");
  fprintf(stderr, "Output on stderr from test returning 0.\n");
  return 0;
}

int test2()
{
  fprintf(stdout, "Output on stdout from test returning 123.\n");
  fprintf(stderr, "Output on stderr from test returning 123.\n");
  return 123;
}

int test3()
{
  fprintf(stdout, "Output before sleep on stdout from timeout test.\n");
  fprintf(stderr, "Output before sleep on stderr from timeout test.\n");
  fflush(stdout);
  fflush(stderr);
#if defined(_WIN32)
  Sleep(5000);
#else
  sleep(5);
#endif
  fprintf(stdout, "Output after sleep on stdout from timeout test.\n");
  fprintf(stderr, "Output after sleep on stderr from timeout test.\n");
  return 0;
}

int test4()
{
  fprintf(stdout, "Output before crash on stdout from crash test.\n");
  fprintf(stderr, "Output before crash on stderr from crash test.\n");  
  fflush(stdout);
  fflush(stderr);
  *(int*)0 = 0;
  fprintf(stdout, "Output after crash on stdout from crash test.\n");
  fprintf(stderr, "Output after crash on stderr from crash test.\n");
  return 0;
}

int runChild(const char* cmd[], int state, int exception, int value)
{
  int result = 0;
  char* data = 0;
  int length = 0;
  kwsysProcess* kp = kwsysProcess_New();
  if(!kp)
    {
    fprintf(stderr, "kwsysProcess_New returned NULL!\n");
    return 1;
    }
  
  kwsysProcess_SetCommand(kp, cmd);
  kwsysProcess_SetTimeout(kp, 3);
  kwsysProcess_Execute(kp);
  
  while(kwsysProcess_WaitForData(kp, (kwsysProcess_Pipe_STDOUT |
                                      kwsysProcess_Pipe_STDERR),
                                 &data, &length, 0))
    {
    fwrite(data, 1, length, stdout);
    fflush(stdout);
    }
  
  kwsysProcess_WaitForExit(kp, 0);
  
  switch (kwsysProcess_GetState(kp))
    {
    case kwsysProcess_State_Starting:
      printf("No process has been executed.\n"); break;
    case kwsysProcess_State_Executing:
      printf("The process is still executing.\n"); break;
    case kwsysProcess_State_Expired:
      printf("Child was killed when timeout expired.\n"); break;
    case kwsysProcess_State_Exited:
      printf("Child exited with value = %d\n",
             kwsysProcess_GetExitValue(kp));
      result = ((exception != kwsysProcess_GetExitException(kp)) ||
                (value != kwsysProcess_GetExitValue(kp))); break;
    case kwsysProcess_State_Killed:
      printf("Child was killed by parent.\n"); break;
    case kwsysProcess_State_Exception:
      printf("Child terminated abnormally.\n");
      result = ((exception != kwsysProcess_GetExitException(kp)) ||
                (value != kwsysProcess_GetExitValue(kp))); break;
    case kwsysProcess_State_Error:
      printf("Error in administrating child process: [%s]\n",
             kwsysProcess_GetErrorString(kp)); break;
    };
  
  if(result)
    {
    if(exception != kwsysProcess_GetExitException(kp))
      {
      fprintf(stderr, "Mismatch in exit exception.  Should have been %d.\n",
              exception);
      }
    if(value != kwsysProcess_GetExitValue(kp))
      {
      fprintf(stderr, "Mismatch in exit value.  Should have been %d.\n",
              value);
      }
    }
  
  if(kwsysProcess_GetState(kp) != state)
    {
    fprintf(stderr, "Mismatch in state.  Should have been %d.\n", state);
    result = 1;
    }
  
  kwsysProcess_Delete(kp);
  return result;
}

int main(int argc, const char* argv[])
{
  int n = 0;
  if(argc == 2)
    {
    n = atoi(argv[1]);
    }
  else if(argc == 3)
    {
    n = atoi(argv[2]);
    }
  /* Check arguments.  */
  if(n < 1 || n > 5 || (argc == 3 && strcmp(argv[1], "run") != 0))
    {
    fprintf(stdout, "Usage: %s <test number>\n", argv[0]);
    return 1;
    }
  if(argc == 3)
    {
    switch (n)
      {
      case 1: return test1();
      case 2: return test2();
      case 3: return test3();
      case 4: return test4();
      }
    fprintf(stderr, "Invalid test number %d.\n", n);
    return 1;
    }
  
  if(n <= 4)
    {
    int states[4] =
      {
        kwsysProcess_State_Exited,
        kwsysProcess_State_Exited,
        kwsysProcess_State_Expired,
        kwsysProcess_State_Exception
      };
    int exceptions[4] = {kwsysProcess_Exception_None, kwsysProcess_Exception_None,
                         kwsysProcess_Exception_None, kwsysProcess_Exception_Fault};
    int values[4] = {0, 123, 1, 1};
    const char* cmd[] = {argv[0], "run", argv[1], 0};
    return runChild(cmd, states[n-1], exceptions[n-1], values[n-1]);
    }
  else
    {
    return 0;
    }  
}
