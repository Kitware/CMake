/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Process.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "Process.h.in"
#endif

/*

Implementation for UNIX

On UNIX, a child process is forked to exec the program.  Three
output pipes from the child are read by the parent process using a
select call to block until data are ready.  Two of the pipes are
stdout and stderr for the child.  The third is a special error pipe
that has two purposes.  First, if the child cannot exec the program,
the error is reported through the error pipe.  Second, the error
pipe is left open until the child exits.  This is used in
conjunction with the timeout on the select call to implement a
timeout for program even when it closes stdout and stderr.
*/

/*

TODO:

We cannot create the pipeline of processes in suspended states.  How
do we cleanup processes already started when one fails to load?  Right
now we are just killing them, which is probably not the right thing to
do.

*/

#include <stdio.h>     /* snprintf */
#include <stdlib.h>    /* malloc, free */
#include <string.h>    /* strdup, strerror, memset */
#include <sys/time.h>  /* struct timeval */
#include <sys/types.h> /* pid_t, fd_set */
#include <sys/wait.h>  /* waitpid */
#include <sys/stat.h>  /* open mode */
#include <unistd.h>    /* pipe, close, fork, execvp, select, _exit */
#include <fcntl.h>     /* fcntl */
#include <errno.h>     /* errno */
#include <time.h>      /* gettimeofday */
#include <signal.h>    /* sigaction */
#include <dirent.h>    /* DIR, dirent */

/* The number of pipes for the child's output.  The standard stdout
   and stderr pipes are the first two.  One more pipe is used to
   detect when the child process has terminated.  The third pipe is
   not given to the child process, so it cannot close it until it
   terminates.  */
#define KWSYSPE_PIPE_COUNT 3
#define KWSYSPE_PIPE_STDOUT 0
#define KWSYSPE_PIPE_STDERR 1
#define KWSYSPE_PIPE_TERM 2

/* The maximum amount to read from a pipe at a time.  */
#define KWSYSPE_PIPE_BUFFER_SIZE 1024

/* Keep track of times using a signed representation.  Switch to the
   native (possibly unsigned) representation only when calling native
   functions.  */
typedef struct timeval kwsysProcessTimeNative;
typedef struct kwsysProcessTime_s kwsysProcessTime;
struct kwsysProcessTime_s
{
  long tv_sec;
  long tv_usec;
};

typedef struct kwsysProcessCreateInformation_s
{
  int StdIn;
  int StdOut;
  int StdErr;
  int TermPipe;
  int ErrorPipe[2];
} kwsysProcessCreateInformation;

/*--------------------------------------------------------------------------*/
static int kwsysProcessInitialize(kwsysProcess* cp);
static void kwsysProcessCleanup(kwsysProcess* cp, int error);
static void kwsysProcessCleanupDescriptor(int* pfd);
static int kwsysProcessCreate(kwsysProcess* cp, int prIndex,
                              kwsysProcessCreateInformation* si, int* readEnd);
static int kwsysProcessSetupOutputPipeFile(int* p, const char* name);
static int kwsysProcessGetTimeoutTime(kwsysProcess* cp, double* userTimeout,
                                      kwsysProcessTime* timeoutTime);
static int kwsysProcessGetTimeoutLeft(kwsysProcessTime* timeoutTime,
                                      double* userTimeout,
                                      kwsysProcessTimeNative* timeoutLength);
static kwsysProcessTime kwsysProcessTimeGetCurrent(void);
static double kwsysProcessTimeToDouble(kwsysProcessTime t);
static kwsysProcessTime kwsysProcessTimeFromDouble(double d);
static int kwsysProcessTimeLess(kwsysProcessTime in1, kwsysProcessTime in2);
static kwsysProcessTime kwsysProcessTimeAdd(kwsysProcessTime in1, kwsysProcessTime in2);
static kwsysProcessTime kwsysProcessTimeSubtract(kwsysProcessTime in1, kwsysProcessTime in2);
static void kwsysProcessSetExitException(kwsysProcess* cp, int sig);
static void kwsysProcessChildErrorExit(int errorPipe);
static void kwsysProcessRestoreDefaultSignalHandlers(void);
static pid_t kwsysProcessFork(kwsysProcess* cp,
                              kwsysProcessCreateInformation* si);
static void kwsysProcessKill(pid_t process_id);

/*--------------------------------------------------------------------------*/
/* Structure containing data used to implement the child's execution.  */
struct kwsysProcess_s
{
  /* The command lines to execute.  */
  char*** Commands;
  int NumberOfCommands;

  /* Descriptors for the read ends of the child's output pipes. */
  int PipeReadEnds[KWSYSPE_PIPE_COUNT];

  /* Buffer for pipe data.  */
  char PipeBuffer[KWSYSPE_PIPE_BUFFER_SIZE];

  /* Process IDs returned by the calls to fork.  */
  pid_t* ForkPIDs;

  /* Flag for whether the children were terminated by a faild select.  */
  int SelectError;

  /* The timeout length.  */
  double Timeout;

  /* The working directory for the process. */
  char* WorkingDirectory;

  /* Whether to create the child as a detached process.  */
  int OptionDetach;

  /* Whether the child was created as a detached process.  */
  int Detached;

  /* Time at which the child started.  Negative for no timeout.  */
  kwsysProcessTime StartTime;

  /* Time at which the child will timeout.  Negative for no timeout.  */
  kwsysProcessTime TimeoutTime;

  /* Flag for whether the timeout expired.  */
  int TimeoutExpired;

  /* The old SIGCHLD handler.  */
  struct sigaction OldSigChldAction;

  /* The number of pipes left open during execution.  */
  int PipesLeft;

  /* File descriptor set for call to select.  */
  fd_set PipeSet;

  /* The current status of the child process. */
  int State;

  /* The exceptional behavior that terminated the child process, if
   * any.  */
  int ExitException;

  /* The exit code of the child process.  */
  int ExitCode;

  /* The exit value of the child process, if any.  */
  int ExitValue;

  /* Whether the process was killed.  */
  int Killed;

  /* Buffer for error message in case of failure.  */
  char ErrorMessage[KWSYSPE_PIPE_BUFFER_SIZE+1];

  /* Description for the ExitException.  */
  char ExitExceptionString[KWSYSPE_PIPE_BUFFER_SIZE+1];

  /* The exit codes of each child process in the pipeline.  */
  int* CommandExitCodes;

  /* Name of files to which stdin and stdout pipes are attached.  */
  char* PipeFileSTDIN;
  char* PipeFileSTDOUT;
  char* PipeFileSTDERR;

  /* Whether each pipe is shared with the parent process.  */
  int PipeSharedSTDIN;
  int PipeSharedSTDOUT;
  int PipeSharedSTDERR;

  /* The real working directory of this process.  */
  int RealWorkingDirectoryLength;
  char* RealWorkingDirectory;
};

/*--------------------------------------------------------------------------*/
kwsysProcess* kwsysProcess_New(void)
{
  /* Allocate a process control structure.  */
  kwsysProcess* cp = (kwsysProcess*)malloc(sizeof(kwsysProcess));
  if(!cp)
    {
    return 0;
    }
  memset(cp, 0, sizeof(kwsysProcess));

  /* Share stdin with the parent process by default.  */
  cp->PipeSharedSTDIN = 1;

  /* Set initial status.  */
  cp->State = kwsysProcess_State_Starting;

  return cp;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Delete(kwsysProcess* cp)
{
  /* Make sure we have an instance.  */
  if(!cp)
    {
    return;
    }

  /* If the process is executing, wait for it to finish.  */
  if(cp->State == kwsysProcess_State_Executing)
    {
    if(cp->Detached)
      {
      kwsysProcess_Disown(cp);
      }
    else
      {
      kwsysProcess_WaitForExit(cp, 0);
      }
    }

  /* Free memory.  */
  kwsysProcess_SetCommand(cp, 0);
  kwsysProcess_SetWorkingDirectory(cp, 0);
  kwsysProcess_SetPipeFile(cp, kwsysProcess_Pipe_STDIN, 0);
  kwsysProcess_SetPipeFile(cp, kwsysProcess_Pipe_STDOUT, 0);
  kwsysProcess_SetPipeFile(cp, kwsysProcess_Pipe_STDERR, 0);
  if(cp->CommandExitCodes)
    {
    free(cp->CommandExitCodes);
    }
  free(cp);
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_SetCommand(kwsysProcess* cp, char const* const* command)
{
  int i;
  if(!cp)
    {
    return 0;
    }
  for(i=0; i < cp->NumberOfCommands; ++i)
    {
    char** c = cp->Commands[i];
    while(*c)
      {
      free(*c++);
      }
    free(cp->Commands[i]);
    }
  cp->NumberOfCommands = 0;
  if(cp->Commands)
    {
    free(cp->Commands);
    cp->Commands = 0;
    }
  if(command)
    {
    return kwsysProcess_AddCommand(cp, command);
    }
  return 1;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_AddCommand(kwsysProcess* cp, char const* const* command)
{
  int newNumberOfCommands;
  char*** newCommands;

  /* Make sure we have a command to add.  */
  if(!cp || !command)
    {
    return 0;
    }

  /* Allocate a new array for command pointers.  */
  newNumberOfCommands = cp->NumberOfCommands + 1;
  if(!(newCommands = (char***)malloc(sizeof(char**) * newNumberOfCommands)))
    {
    /* Out of memory.  */
    return 0;
    }

  /* Copy any existing commands into the new array.  */
  {
  int i;
  for(i=0; i < cp->NumberOfCommands; ++i)
    {
    newCommands[i] = cp->Commands[i];
    }
  }

  /* Add the new command.  */
  {
  char const* const* c = command;
  int n = 0;
  int i = 0;
  while(*c++);
  n = c - command - 1;
  newCommands[cp->NumberOfCommands] = (char**)malloc((n+1)*sizeof(char*));
  if(!newCommands[cp->NumberOfCommands])
    {
    /* Out of memory.  */
    free(newCommands);
    return 0;
    }
  for(i=0; i < n; ++i)
    {
    newCommands[cp->NumberOfCommands][i] = strdup(command[i]);
    if(!newCommands[cp->NumberOfCommands][i])
      {
      break;
      }
    }
  if(i < n)
    {
    /* Out of memory.  */
    for(;i > 0; --i)
      {
      free(newCommands[cp->NumberOfCommands][i-1]);
      }
    free(newCommands);
    return 0;
    }
  newCommands[cp->NumberOfCommands][n] = 0;
  }

  /* Successfully allocated new command array.  Free the old array. */
  free(cp->Commands);
  cp->Commands = newCommands;
  cp->NumberOfCommands = newNumberOfCommands;

  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_SetTimeout(kwsysProcess* cp, double timeout)
{
  if(!cp)
    {
    return;
    }
  cp->Timeout = timeout;
  if(cp->Timeout < 0)
    {
    cp->Timeout = 0;
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_SetWorkingDirectory(kwsysProcess* cp, const char* dir)
{
  if(!cp)
    {
    return 0;
    }
  if(cp->WorkingDirectory == dir)
    {
    return 1;
    }
  if(cp->WorkingDirectory && dir && strcmp(cp->WorkingDirectory, dir) == 0)
    {
    return 1;
    }
  if(cp->WorkingDirectory)
    {
    free(cp->WorkingDirectory);
    cp->WorkingDirectory = 0;
    }
  if(dir)
    {
    cp->WorkingDirectory = (char*)malloc(strlen(dir) + 1);
    if(!cp->WorkingDirectory)
      {
      return 0;
      }
    strcpy(cp->WorkingDirectory, dir);
    }
  return 1;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_SetPipeFile(kwsysProcess* cp, int prPipe, const char* file)
{
  char** pfile;
  if(!cp)
    {
    return 0;
    }
  switch(prPipe)
    {
    case kwsysProcess_Pipe_STDIN: pfile = &cp->PipeFileSTDIN; break;
    case kwsysProcess_Pipe_STDOUT: pfile = &cp->PipeFileSTDOUT; break;
    case kwsysProcess_Pipe_STDERR: pfile = &cp->PipeFileSTDERR; break;
    default: return 0;
    }
  if(*pfile)
    {
    free(*pfile);
    *pfile = 0;
    }
  if(file)
    {
    *pfile = malloc(strlen(file)+1);
    if(!*pfile)
      {
      return 0;
      }
    strcpy(*pfile, file);
    }

  /* If we are redirecting the pipe, do not share it.  */
  if(*pfile)
    {
    kwsysProcess_SetPipeShared(cp, prPipe, 0);
    }
  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_SetPipeShared(kwsysProcess* cp, int prPipe, int shared)
{
  if(!cp)
    {
    return;
    }

  switch(prPipe)
    {
    case kwsysProcess_Pipe_STDIN: cp->PipeSharedSTDIN = shared?1:0; break;
    case kwsysProcess_Pipe_STDOUT: cp->PipeSharedSTDOUT = shared?1:0; break;
    case kwsysProcess_Pipe_STDERR: cp->PipeSharedSTDERR = shared?1:0; break;
    default: return;
    }

  /* If we are sharing the pipe, do not redirect it to a file.  */
  if(shared)
    {
    kwsysProcess_SetPipeFile(cp, prPipe, 0);
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetOption(kwsysProcess* cp, int optionId)
{
  if(!cp)
    {
    return 0;
    }

  switch(optionId)
    {
    case kwsysProcess_Option_Detach: return cp->OptionDetach;
    default: return 0;
    }
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_SetOption(kwsysProcess* cp, int optionId, int value)
{
  if(!cp)
    {
    return;
    }

  switch(optionId)
    {
    case kwsysProcess_Option_Detach: cp->OptionDetach = value; break;
    default: break;
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetState(kwsysProcess* cp)
{
  return cp? cp->State : kwsysProcess_State_Error;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitException(kwsysProcess* cp)
{
  return cp? cp->ExitException : kwsysProcess_Exception_Other;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitCode(kwsysProcess* cp)
{
  return cp? cp->ExitCode : 0;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitValue(kwsysProcess* cp)
{
  return cp? cp->ExitValue : -1;
}

/*--------------------------------------------------------------------------*/
const char* kwsysProcess_GetErrorString(kwsysProcess* cp)
{
  if(!cp)
    {
    return "Process management structure could not be allocated";
    }
  else if(cp->State == kwsysProcess_State_Error)
    {
    return cp->ErrorMessage;
    }
  return "Success";
}

/*--------------------------------------------------------------------------*/
const char* kwsysProcess_GetExceptionString(kwsysProcess* cp)
{
  if(!cp)
    {
    return "GetExceptionString called with NULL process management structure";
    }
  else if(cp->State == kwsysProcess_State_Exception)
    {
    return cp->ExitExceptionString;
    }
  return "No exception";
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Execute(kwsysProcess* cp)
{
  int i;
  struct sigaction newSigChldAction;
  kwsysProcessCreateInformation si = {-1, -1, -1, -1, {-1, -1}};

  /* Do not execute a second copy simultaneously.  */
  if(!cp || cp->State == kwsysProcess_State_Executing)
    {
    return;
    }

  /* Initialize the control structure for a new process.  */
  if(!kwsysProcessInitialize(cp))
    {
    strcpy(cp->ErrorMessage, "Out of memory");
    cp->State = kwsysProcess_State_Error;
    return;
    }

  /* Save the real working directory of this process and change to
     the working directory for the child processes.  This is needed
     to make pipe file paths evaluate correctly.  */
  if(cp->WorkingDirectory)
    {
    int r;
    if(!getcwd(cp->RealWorkingDirectory, cp->RealWorkingDirectoryLength))
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }

    /* Some platforms specify that the chdir call may be
       interrupted.  Repeat the call until it finishes.  */
    while(((r = chdir(cp->WorkingDirectory)) < 0) && (errno == EINTR));
    if(r < 0)
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }
    }

  /* We want no special handling of SIGCHLD.  Repeat call until it is
     not interrupted.  */
  memset(&newSigChldAction, 0, sizeof(struct sigaction));
  newSigChldAction.sa_handler = SIG_DFL;
  while((sigaction(SIGCHLD, &newSigChldAction, &cp->OldSigChldAction) < 0) &&
        (errno == EINTR));

  /* Setup the stderr and termination pipes to be shared by all processes.  */
  for(i=KWSYSPE_PIPE_STDERR; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    /* Create the pipe.  */
    int p[2];
    if(pipe(p) < 0)
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }

    /* Store the pipe.  */
    cp->PipeReadEnds[i] = p[0];
    if(i == KWSYSPE_PIPE_STDERR)
      {
      si.StdErr = p[1];
      }
    else
      {
      si.TermPipe = p[1];
      }

    /* Set close-on-exec flag on the pipe's ends.  */
    if((fcntl(p[0], F_SETFD, FD_CLOEXEC) < 0) ||
       (fcntl(p[1], F_SETFD, FD_CLOEXEC) < 0))
      {
      kwsysProcessCleanup(cp, 1);
      kwsysProcessCleanupDescriptor(&si.StdErr);
      kwsysProcessCleanupDescriptor(&si.TermPipe);
      return;
      }
    }

  /* Replace the stderr pipe with a file if requested.  In this case
     the select call will report that stderr is closed immediately.  */
  if(cp->PipeFileSTDERR)
    {
    if(!kwsysProcessSetupOutputPipeFile(&si.StdErr, cp->PipeFileSTDERR))
      {
      kwsysProcessCleanup(cp, 1);
      kwsysProcessCleanupDescriptor(&si.StdErr);
      kwsysProcessCleanupDescriptor(&si.TermPipe);
      return;
      }
    }

  /* Replace the stderr pipe with the parent's if requested.  In this
     case the select call will report that stderr is closed
     immediately.  */
  if(cp->PipeSharedSTDERR)
    {
    kwsysProcessCleanupDescriptor(&si.StdErr);
    si.StdErr = 2;
    }

  /* The timeout period starts now.  */
  cp->StartTime = kwsysProcessTimeGetCurrent();
  cp->TimeoutTime.tv_sec = -1;
  cp->TimeoutTime.tv_usec = -1;

  /* Create the pipeline of processes.  */
  {
  int readEnd = -1;
  for(i=0; i < cp->NumberOfCommands; ++i)
    {
    if(!kwsysProcessCreate(cp, i, &si, &readEnd))
      {
      kwsysProcessCleanup(cp, 1);

      /* Release resources that may have been allocated for this
         process before an error occurred.  */
      kwsysProcessCleanupDescriptor(&readEnd);
      if(si.StdIn != 0)
        {
        kwsysProcessCleanupDescriptor(&si.StdIn);
        }
      if(si.StdOut != 1)
        {
        kwsysProcessCleanupDescriptor(&si.StdOut);
        }
      if(si.StdErr != 2)
        {
        kwsysProcessCleanupDescriptor(&si.StdErr);
        }
      kwsysProcessCleanupDescriptor(&si.TermPipe);
      kwsysProcessCleanupDescriptor(&si.ErrorPipe[0]);
      kwsysProcessCleanupDescriptor(&si.ErrorPipe[1]);
      return;
      }
    }
  /* Save a handle to the output pipe for the last process.  */
  cp->PipeReadEnds[KWSYSPE_PIPE_STDOUT] = readEnd;
  }

  /* The parent process does not need the output pipe write ends.  */
  if(si.StdErr != 2)
    {
    kwsysProcessCleanupDescriptor(&si.StdErr);
    }
  kwsysProcessCleanupDescriptor(&si.TermPipe);

  /* Restore the working directory. */
  if(cp->RealWorkingDirectory)
    {
    /* Some platforms specify that the chdir call may be
       interrupted.  Repeat the call until it finishes.  */
    while((chdir(cp->RealWorkingDirectory) < 0) && (errno == EINTR));
    free(cp->RealWorkingDirectory);
    cp->RealWorkingDirectory = 0;
    }

  /* All the pipes are now open.  */
  cp->PipesLeft = KWSYSPE_PIPE_COUNT;

  /* The process has now started.  */
  cp->State = kwsysProcess_State_Executing;
  cp->Detached = cp->OptionDetach;
}

/*--------------------------------------------------------------------------*/
kwsysEXPORT void kwsysProcess_Disown(kwsysProcess* cp)
{
  int i;

  /* Make sure a detached child process is running.  */
  if(!cp || !cp->Detached || cp->State != kwsysProcess_State_Executing ||
     cp->TimeoutExpired || cp->Killed)
    {
    return;
    }

  /* Close any pipes that are still open.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    if(cp->PipeReadEnds[i] >= 0)
      {
      /* If the pipe was reported by the last call to select, we must
         read from it.  Ignore the data.  */
      if(FD_ISSET(cp->PipeReadEnds[i], &cp->PipeSet))
        {
        /* We are handling this pipe now.  Remove it from the set.  */
        FD_CLR(cp->PipeReadEnds[i], &cp->PipeSet);

        /* The pipe is ready to read without blocking.  Keep trying to
           read until the operation is not interrupted.  */
        while((read(cp->PipeReadEnds[i], cp->PipeBuffer,
                    KWSYSPE_PIPE_BUFFER_SIZE) < 0) && (errno == EINTR));
        }

      /* We are done reading from this pipe.  */
      kwsysProcessCleanupDescriptor(&cp->PipeReadEnds[i]);
      --cp->PipesLeft;
      }
    }

  /* We will not wait for exit, so cleanup now.  */
  kwsysProcessCleanup(cp, 0);

  /* The process has been disowned.  */
  cp->State = kwsysProcess_State_Disowned;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_WaitForData(kwsysProcess* cp, char** data, int* length,
                             double* userTimeout)
{
  int i;
  int max = -1;
  kwsysProcessTimeNative* timeout = 0;
  kwsysProcessTimeNative timeoutLength;
  kwsysProcessTime timeoutTime;
  kwsysProcessTime userStartTime = {0, 0};
  int user = 0;
  int expired = 0;
  int pipeId = kwsysProcess_Pipe_None;
  int numReady = 0;

  /* Make sure we are executing a process.  */
  if(!cp || cp->State != kwsysProcess_State_Executing || cp->Killed ||
     cp->TimeoutExpired)
    {
    return kwsysProcess_Pipe_None;
    }

  /* Record the time at which user timeout period starts.  */
  if(userTimeout)
    {
    userStartTime = kwsysProcessTimeGetCurrent();
    }

  /* Calculate the time at which a timeout will expire, and whether it
     is the user or process timeout.  */
  user = kwsysProcessGetTimeoutTime(cp, userTimeout, &timeoutTime);

  /* Data can only be available when pipes are open.  If the process
     is not running, cp->PipesLeft will be 0.  */
  while(cp->PipesLeft > 0)
    {
    /* Check for any open pipes with data reported ready by the last
       call to select.  According to "man select_tut" we must deal
       with all descriptors reported by a call to select before
       passing them to another select call.  */
    for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
      {
      if(cp->PipeReadEnds[i] >= 0 &&
         FD_ISSET(cp->PipeReadEnds[i], &cp->PipeSet))
        {
        int n;

        /* We are handling this pipe now.  Remove it from the set.  */
        FD_CLR(cp->PipeReadEnds[i], &cp->PipeSet);

        /* The pipe is ready to read without blocking.  Keep trying to
           read until the operation is not interrupted.  */
        while(((n = read(cp->PipeReadEnds[i], cp->PipeBuffer,
                         KWSYSPE_PIPE_BUFFER_SIZE)) < 0) && (errno == EINTR));
        if(n > 0)
          {
          /* We have data on this pipe.  */
          if(i == KWSYSPE_PIPE_TERM)
            {
            /* This is data on the special termination pipe.  Ignore it.  */
            }
          else if(data && length)
            {
            /* Report this data.  */
            *data = cp->PipeBuffer;
            *length = n;
            switch(i)
              {
              case KWSYSPE_PIPE_STDOUT:
                pipeId = kwsysProcess_Pipe_STDOUT; break;
              case KWSYSPE_PIPE_STDERR:
                pipeId = kwsysProcess_Pipe_STDERR; break;
              };
            break;
            }
          }
        else
          {
          /* We are done reading from this pipe.  */
          kwsysProcessCleanupDescriptor(&cp->PipeReadEnds[i]);
          --cp->PipesLeft;
          }
        }
      }

    /* If we have data, break early.  */
    if(pipeId)
      {
      break;
      }

    /* Make sure the set is empty (it should always be empty here
       anyway).  */
    FD_ZERO(&cp->PipeSet);

    /* Setup a timeout if required.  */
    if(timeoutTime.tv_sec < 0)
      {
      timeout = 0;
      }
    else
      {
      timeout = &timeoutLength;
      }
    if(kwsysProcessGetTimeoutLeft(&timeoutTime, user?userTimeout:0, &timeoutLength))
      {
      /* Timeout has already expired.  */
      expired = 1;
      break;
      }

    /* Add the pipe reading ends that are still open.  */
    max = -1;
    for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
      {
      if(cp->PipeReadEnds[i] >= 0)
        {
        FD_SET(cp->PipeReadEnds[i], &cp->PipeSet);
        if(cp->PipeReadEnds[i] > max)
          {
          max = cp->PipeReadEnds[i];
          }
        }
      }

    /* Make sure we have a non-empty set.  */
    if(max < 0)
      {
      /* All pipes have closed.  Child has terminated.  */
      break;
      }

    /* Run select to block until data are available.  Repeat call
       until it is not interrupted.  */
    while(((numReady = select(max+1, &cp->PipeSet, 0, 0, timeout)) < 0) &&
          (errno == EINTR));

    /* Check result of select.  */
    if(numReady == 0)
      {
      /* Select's timeout expired.  */
      expired = 1;
      break;
      }
    else if(numReady < 0)
      {
      /* Select returned an error.  Leave the error description in the
         pipe buffer.  */
      strncpy(cp->ErrorMessage, strerror(errno), KWSYSPE_PIPE_BUFFER_SIZE);

      /* Kill the children now.  */
      kwsysProcess_Kill(cp);
      cp->Killed = 0;
      cp->SelectError = 1;
      }
    }

  /* Update the user timeout.  */
  if(userTimeout)
    {
    kwsysProcessTime userEndTime = kwsysProcessTimeGetCurrent();
    kwsysProcessTime difference = kwsysProcessTimeSubtract(userEndTime,
                                                     userStartTime);
    double d = kwsysProcessTimeToDouble(difference);
    *userTimeout -= d;
    if(*userTimeout < 0)
      {
      *userTimeout = 0;
      }
    }

  /* Check what happened.  */
  if(pipeId)
    {
    /* Data are ready on a pipe.  */
    return pipeId;
    }
  else if(expired)
    {
    /* A timeout has expired.  */
    if(user)
      {
      /* The user timeout has expired.  It has no time left.  */
      return kwsysProcess_Pipe_Timeout;
      }
    else
      {
      /* The process timeout has expired.  Kill the children now.  */
      kwsysProcess_Kill(cp);
      cp->Killed = 0;
      cp->TimeoutExpired = 1;
      return kwsysProcess_Pipe_None;
      }
    }
  else
    {
    /* No pipes are left open.  */
    return kwsysProcess_Pipe_None;
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_WaitForExit(kwsysProcess* cp, double* userTimeout)
{
  int result = 0;
  int status = 0;
  int prPipe = 0;

  /* Make sure we are executing a process.  */
  if(!cp || cp->State != kwsysProcess_State_Executing)
    {
    return 1;
    }

  /* Wait for all the pipes to close.  Ignore all data.  */
  while((prPipe = kwsysProcess_WaitForData(cp, 0, 0, userTimeout)) > 0)
    {
    if(prPipe == kwsysProcess_Pipe_Timeout)
      {
      return 0;
      }
    }

  /* Wait for each child to terminate.  The process should have
     already exited because KWSYSPE_PIPE_TERM has been closed by this
     point.  Repeat the call until it is not interrupted.  */
  if(!cp->Detached)
    {
    int i;
    for(i=0; i < cp->NumberOfCommands; ++i)
      {
      while(((result = waitpid(cp->ForkPIDs[i],
                               &cp->CommandExitCodes[i], 0)) < 0) &&
            (errno == EINTR));
      if(result <= 0 && cp->State != kwsysProcess_State_Error)
        {
        /* Unexpected error.  Report the first time this happens.  */
        strncpy(cp->ErrorMessage, strerror(errno), KWSYSPE_PIPE_BUFFER_SIZE);
        cp->State = kwsysProcess_State_Error;
        }
      }
    }

  /* Check if there was an error in one of the waitpid calls.  */
  if(cp->State == kwsysProcess_State_Error)
    {
    /* The error message is already in its buffer.  Tell
       kwsysProcessCleanup to not create it.  */
    kwsysProcessCleanup(cp, 0);
    return 1;
    }

  /* Check whether the child reported an error invoking the process.  */
  if(cp->SelectError)
    {
    /* The error message is already in its buffer.  Tell
       kwsysProcessCleanup to not create it.  */
    kwsysProcessCleanup(cp, 0);
    cp->State = kwsysProcess_State_Error;
    return 1;
    }

  /* Use the status of the last process in the pipeline.  */
  status = cp->CommandExitCodes[cp->NumberOfCommands-1];

  /* Determine the outcome.  */
  if(cp->Killed)
    {
    /* We killed the child.  */
    cp->State = kwsysProcess_State_Killed;
    }
  else if(cp->TimeoutExpired)
    {
    /* The timeout expired.  */
    cp->State = kwsysProcess_State_Expired;
    }
  else if(WIFEXITED(status))
    {
    /* The child exited normally.  */
    cp->State = kwsysProcess_State_Exited;
    cp->ExitException = kwsysProcess_Exception_None;
    cp->ExitCode = status;
    cp->ExitValue = (int)WEXITSTATUS(status);
    }
  else if(WIFSIGNALED(status))
    {
    /* The child received an unhandled signal.  */
    cp->State = kwsysProcess_State_Exception;
    cp->ExitCode = status;
    kwsysProcessSetExitException(cp, (int)WTERMSIG(status));
    }
  else
    {
    /* Error getting the child return code.  */
    strcpy(cp->ErrorMessage, "Error getting child return code.");
    cp->State = kwsysProcess_State_Error;
    }

  /* Normal cleanup.  */
  kwsysProcessCleanup(cp, 0);
  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Kill(kwsysProcess* cp)
{
  int i;

  /* Make sure we are executing a process.  */
  if(!cp || cp->State != kwsysProcess_State_Executing)
    {
    return;
    }

  /* Kill the children.  */
  cp->Killed = 1;
  for(i=0; i < cp->NumberOfCommands; ++i)
    {
    if(cp->ForkPIDs[i])
      {
      kwsysProcessKill(cp->ForkPIDs[i]);
      }
    }

  /* Close all the pipe read ends.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    kwsysProcessCleanupDescriptor(&cp->PipeReadEnds[i]);
    }
  cp->PipesLeft = 0;
}

/*--------------------------------------------------------------------------*/
/* Initialize a process control structure for kwsysProcess_Execute.  */
static int kwsysProcessInitialize(kwsysProcess* cp)
{
  int i;
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    cp->PipeReadEnds[i] = -1;
    }
  cp->SelectError = 0;
  cp->StartTime.tv_sec = -1;
  cp->StartTime.tv_usec = -1;
  cp->TimeoutTime.tv_sec = -1;
  cp->TimeoutTime.tv_usec = -1;
  cp->TimeoutExpired = 0;
  cp->PipesLeft = 0;
  FD_ZERO(&cp->PipeSet);
  cp->State = kwsysProcess_State_Starting;
  cp->Killed = 0;
  cp->ExitException = kwsysProcess_Exception_None;
  cp->ExitCode = 1;
  cp->ExitValue = 1;
  cp->ErrorMessage[0] = 0;
  strcpy(cp->ExitExceptionString, "No exception");

  if(cp->ForkPIDs)
    {
    free(cp->ForkPIDs);
    }
  cp->ForkPIDs = (pid_t*)malloc(sizeof(pid_t)*cp->NumberOfCommands);
  if(!cp->ForkPIDs)
    {
    return 0;
    }
  memset(cp->ForkPIDs, 0, sizeof(pid_t)*cp->NumberOfCommands);

  if(cp->CommandExitCodes)
    {
    free(cp->CommandExitCodes);
    }
  cp->CommandExitCodes = (int*)malloc(sizeof(int)*cp->NumberOfCommands);
  if(!cp->CommandExitCodes)
    {
    return 0;
    }
  memset(cp->CommandExitCodes, 0, sizeof(int)*cp->NumberOfCommands);

  /* Allocate memory to save the real working directory.  */
  if ( cp->WorkingDirectory )
    {
#if defined(MAXPATHLEN)
    cp->RealWorkingDirectoryLength = MAXPATHLEN;
#elif defined(PATH_MAX)
    cp->RealWorkingDirectoryLength = PATH_MAX;
#else
    cp->RealWorkingDirectoryLength = 4096;
#endif
    cp->RealWorkingDirectory = malloc(cp->RealWorkingDirectoryLength);
    if(!cp->RealWorkingDirectory)
      {
      return 0;
      }
    }

  return 1;
}

/*--------------------------------------------------------------------------*/
/* Free all resources used by the given kwsysProcess instance that were
   allocated by kwsysProcess_Execute.  */
static void kwsysProcessCleanup(kwsysProcess* cp, int error)
{
  int i;

  if(error)
    {
    /* We are cleaning up due to an error.  Report the error message
       if one has not been provided already.  */
    if(cp->ErrorMessage[0] == 0)
      {
      strncpy(cp->ErrorMessage, strerror(errno), KWSYSPE_PIPE_BUFFER_SIZE);
      }

    /* Set the error state.  */
    cp->State = kwsysProcess_State_Error;

    /* Kill any children already started.  */
    if(cp->ForkPIDs)
      {
      int status;
      for(i=0; i < cp->NumberOfCommands; ++i)
        {
        if(cp->ForkPIDs[i])
          {
          /* Kill the child.  */
          kwsysProcessKill(cp->ForkPIDs[i]);
          /* Reap the child.  Keep trying until the call is not
             interrupted.  */
          while((waitpid(cp->ForkPIDs[i], &status, 0) < 0) &&
                (errno == EINTR));
          }
        }
      }

    /* Restore the working directory.  */
    if(cp->RealWorkingDirectory)
      {
      while((chdir(cp->RealWorkingDirectory) < 0) && (errno == EINTR));
      }
    }

  /* Restore the SIGCHLD handler.  */
  while((sigaction(SIGCHLD, &cp->OldSigChldAction, 0) < 0) &&
        (errno == EINTR));

  /* Free memory.  */
  if(cp->ForkPIDs)
    {
    free(cp->ForkPIDs);
    cp->ForkPIDs = 0;
    }
  if(cp->RealWorkingDirectory)
    {
    free(cp->RealWorkingDirectory);
    cp->RealWorkingDirectory = 0;
    }

  /* Close pipe handles.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    kwsysProcessCleanupDescriptor(&cp->PipeReadEnds[i]);
    }
}

/*--------------------------------------------------------------------------*/
/* Close the given file descriptor if it is open.  Reset its value to -1.  */
static void kwsysProcessCleanupDescriptor(int* pfd)
{
  if(pfd && *pfd >= 0)
    {
    /* Keep trying to close until it is not interrupted by a
     * signal.  */
    while((close(*pfd) < 0) && (errno == EINTR));
    *pfd = -1;
    }
}

/*--------------------------------------------------------------------------*/
static int kwsysProcessCreate(kwsysProcess* cp, int prIndex,
                              kwsysProcessCreateInformation* si, int* readEnd)
{
  /* Setup the process's stdin.  */
  if(prIndex > 0)
    {
    si->StdIn = *readEnd;
    *readEnd = 0;
    }
  else if(cp->PipeFileSTDIN)
    {
    /* Open a file for the child's stdin to read.  */
    si->StdIn = open(cp->PipeFileSTDIN, O_RDONLY);
    if(si->StdIn < 0)
      {
      return 0;
      }

    /* Set close-on-exec flag on the pipe's end.  */
    if(fcntl(si->StdIn, F_SETFD, FD_CLOEXEC) < 0)
      {
      return 0;
      }
    }
  else if(cp->PipeSharedSTDIN)
    {
    si->StdIn = 0;
    }
  else
    {
    si->StdIn = -1;
    }

  /* Setup the process's stdout.  */
  {
  /* Create the pipe.  */
  int p[2];
  if(pipe(p) < 0)
    {
    return 0;
    }
  *readEnd = p[0];
  si->StdOut = p[1];

  /* Set close-on-exec flag on the pipe's ends.  */
  if((fcntl(p[0], F_SETFD, FD_CLOEXEC) < 0) ||
     (fcntl(p[1], F_SETFD, FD_CLOEXEC) < 0))
    {
    return 0;
    }
  }

  /* Replace the stdout pipe with a file if requested.  In this case
     the select call will report that stdout is closed immediately.  */
  if(prIndex == cp->NumberOfCommands-1 && cp->PipeFileSTDOUT)
    {
    if(!kwsysProcessSetupOutputPipeFile(&si->StdOut, cp->PipeFileSTDOUT))
      {
      return 0;
      }
    }

  /* Replace the stdout pipe with the parent's if requested.  In this
     case the select call will report that stderr is closed
     immediately.  */
  if(prIndex == cp->NumberOfCommands-1 && cp->PipeSharedSTDOUT)
    {
    kwsysProcessCleanupDescriptor(&si->StdOut);
    si->StdOut = 1;
    }

  /* Create the error reporting pipe.  */
  if(pipe(si->ErrorPipe) < 0)
    {
    return 0;
    }

  /* Set close-on-exec flag on the error pipe's write end.  */
  if(fcntl(si->ErrorPipe[1], F_SETFD, FD_CLOEXEC) < 0)
    {
    return 0;
    }

  /* Fork off a child process.  */
  cp->ForkPIDs[prIndex] = kwsysProcessFork(cp, si);
  if(cp->ForkPIDs[prIndex] < 0)
    {
    return 0;
    }

  if(cp->ForkPIDs[prIndex] == 0)
    {
    /* Close the read end of the error reporting pipe.  */
    close(si->ErrorPipe[0]);

    /* Setup the stdin, stdout, and stderr pipes.  */
    if(si->StdIn > 0)
      {
      dup2(si->StdIn, 0);
      }
    else if(si->StdIn < 0)
      {
      close(0);
      }
    if(si->StdOut != 1)
      {
      dup2(si->StdOut, 1);
      }
    if(si->StdErr != 2)
      {
      dup2(si->StdErr, 2);
      }

    /* Clear the close-on-exec flag for stdin, stdout, and stderr.
       Also clear it for the termination pipe.  All other pipe handles
       will be closed when exec succeeds.  */
    fcntl(0, F_SETFD, 0);
    fcntl(1, F_SETFD, 0);
    fcntl(2, F_SETFD, 0);
    fcntl(si->TermPipe, F_SETFD, 0);

    /* Restore all default signal handlers. */
    kwsysProcessRestoreDefaultSignalHandlers();

    /* Execute the real process.  If successful, this does not return.  */
    execvp(cp->Commands[prIndex][0], cp->Commands[prIndex]);

    /* Failure.  Report error to parent and terminate.  */
    kwsysProcessChildErrorExit(si->ErrorPipe[1]);
    }

  /* We are done with the error reporting pipe write end.  */
  kwsysProcessCleanupDescriptor(&si->ErrorPipe[1]);

  /* Block until the child's exec call succeeds and closes the error
     pipe or writes data to the pipe to report an error.  */
  {
  int total = 0;
  int n = 1;
  /* Read the entire error message up to the length of our buffer.  */
  while(total < KWSYSPE_PIPE_BUFFER_SIZE && n > 0)
    {
    /* Keep trying to read until the operation is not interrupted.  */
    while(((n = read(si->ErrorPipe[0], cp->ErrorMessage+total,
                     KWSYSPE_PIPE_BUFFER_SIZE-total)) < 0) &&
          (errno == EINTR));
    if(n > 0)
      {
      total += n;
      }
    }

  /* We are done with the error reporting pipe read end.  */
  kwsysProcessCleanupDescriptor(&si->ErrorPipe[0]);

  if(total > 0)
    {
    /* The child failed to execute the process.  */
    return 0;
    }
  }

  /* Successfully created this child process.  */
  if(prIndex > 0 || si->StdIn > 0)
    {
    /* The parent process does not need the input pipe read end.  */
    kwsysProcessCleanupDescriptor(&si->StdIn);
    }

  /* The parent process does not need the output pipe write ends.  */
  if(si->StdOut != 1)
    {
    kwsysProcessCleanupDescriptor(&si->StdOut);
    }

  return 1;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcessSetupOutputPipeFile(int* p, const char* name)
{
  int fout;
  if(!name)
    {
    return 1;
    }

  /* Close the existing descriptor.  */
  kwsysProcessCleanupDescriptor(p);

  /* Open a file for the pipe to write (permissions 644).  */
  if((fout = open(name, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
    return 0;
    }

  /* Set close-on-exec flag on the pipe's end.  */
  if(fcntl(fout, F_SETFD, FD_CLOEXEC) < 0)
    {
    return 0;
    }

  /* Assign the replacement descriptor.  */
  *p = fout;
  return 1;  
}

/*--------------------------------------------------------------------------*/
/* Get the time at which either the process or user timeout will
   expire.  Returns 1 if the user timeout is first, and 0 otherwise.  */
static int kwsysProcessGetTimeoutTime(kwsysProcess* cp, double* userTimeout,
                                      kwsysProcessTime* timeoutTime)
{
  /* The first time this is called, we need to calculate the time at
     which the child will timeout.  */
  if(cp->Timeout && cp->TimeoutTime.tv_sec < 0)
    {
    kwsysProcessTime length = kwsysProcessTimeFromDouble(cp->Timeout);
    cp->TimeoutTime = kwsysProcessTimeAdd(cp->StartTime, length);
    }

  /* Start with process timeout.  */
  *timeoutTime = cp->TimeoutTime;

  /* Check if the user timeout is earlier.  */
  if(userTimeout)
    {
    kwsysProcessTime currentTime = kwsysProcessTimeGetCurrent();
    kwsysProcessTime userTimeoutLength = kwsysProcessTimeFromDouble(*userTimeout);
    kwsysProcessTime userTimeoutTime = kwsysProcessTimeAdd(currentTime,
                                                           userTimeoutLength);
    if(timeoutTime->tv_sec < 0 ||
       kwsysProcessTimeLess(userTimeoutTime, *timeoutTime))
      {
      *timeoutTime = userTimeoutTime;
      return 1;
      }
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
/* Get the length of time before the given timeout time arrives.
   Returns 1 if the time has already arrived, and 0 otherwise.  */
static int kwsysProcessGetTimeoutLeft(kwsysProcessTime* timeoutTime,
                                      double* userTimeout,
                                      kwsysProcessTimeNative* timeoutLength)
{
  if(timeoutTime->tv_sec < 0)
    {
    /* No timeout time has been requested.  */
    return 0;
    }
  else
    {
    /* Calculate the remaining time.  */
    kwsysProcessTime currentTime = kwsysProcessTimeGetCurrent();
    kwsysProcessTime timeLeft = kwsysProcessTimeSubtract(*timeoutTime,
                                                         currentTime);
    if(timeLeft.tv_sec < 0 && userTimeout && *userTimeout <= 0)
      {
      /* Caller has explicitly requested a zero timeout.  */
      timeLeft.tv_sec = 0;
      timeLeft.tv_usec = 0;
      }

    if(timeLeft.tv_sec < 0)
      {
      /* Timeout has already expired.  */
      return 1;
      }
    else
      {
      /* There is some time left.  */
      timeoutLength->tv_sec = timeLeft.tv_sec;
      timeoutLength->tv_usec = timeLeft.tv_usec;
      return 0;
      }
    }
}

/*--------------------------------------------------------------------------*/
static kwsysProcessTime kwsysProcessTimeGetCurrent(void)
{
  kwsysProcessTime current;
  kwsysProcessTimeNative current_native;
  gettimeofday(&current_native, 0);
  current.tv_sec = (long)current_native.tv_sec;
  current.tv_usec = (long)current_native.tv_usec;
  return current;
}

/*--------------------------------------------------------------------------*/
static double kwsysProcessTimeToDouble(kwsysProcessTime t)
{
  return (double)t.tv_sec + t.tv_usec*0.000001;
}

/*--------------------------------------------------------------------------*/
static kwsysProcessTime kwsysProcessTimeFromDouble(double d)
{
  kwsysProcessTime t;
  t.tv_sec = (long)d;
  t.tv_usec = (long)((d-t.tv_sec)*1000000);
  return t;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcessTimeLess(kwsysProcessTime in1, kwsysProcessTime in2)
{
  return ((in1.tv_sec < in2.tv_sec) ||
          ((in1.tv_sec == in2.tv_sec) && (in1.tv_usec < in2.tv_usec)));
}

/*--------------------------------------------------------------------------*/
static kwsysProcessTime kwsysProcessTimeAdd(kwsysProcessTime in1, kwsysProcessTime in2)
{
  kwsysProcessTime out;
  out.tv_sec = in1.tv_sec + in2.tv_sec;
  out.tv_usec = in1.tv_usec + in2.tv_usec;
  if(out.tv_usec > 1000000)
    {
    out.tv_usec -= 1000000;
    out.tv_sec += 1;
    }
  return out;
}

/*--------------------------------------------------------------------------*/
static kwsysProcessTime kwsysProcessTimeSubtract(kwsysProcessTime in1, kwsysProcessTime in2)
{
  kwsysProcessTime out;
  out.tv_sec = in1.tv_sec - in2.tv_sec;
  out.tv_usec = in1.tv_usec - in2.tv_usec;
  if(out.tv_usec < 0)
    {
    out.tv_usec += 1000000;
    out.tv_sec -= 1;
    }
  return out;
}

/*--------------------------------------------------------------------------*/
#define KWSYSPE_CASE(type, str) \
  cp->ExitException = kwsysProcess_Exception_##type; \
  strcpy(cp->ExitExceptionString, str)
static void kwsysProcessSetExitException(kwsysProcess* cp, int sig)
{
  switch (sig)
    {
#ifdef SIGSEGV
    case SIGSEGV: KWSYSPE_CASE(Fault, "Segmentation fault"); break;
#endif
#ifdef SIGBUS
# if !defined(SIGSEGV) || SIGBUS != SIGSEGV
    case SIGBUS: KWSYSPE_CASE(Fault, "Bus error"); break;
# endif
#endif
#ifdef SIGFPE
    case SIGFPE: KWSYSPE_CASE(Numerical, "Floating-point exception"); break;
#endif
#ifdef SIGILL
    case SIGILL: KWSYSPE_CASE(Illegal, "Illegal instruction"); break;
#endif
#ifdef SIGINT
    case SIGINT: KWSYSPE_CASE(Interrupt, "User interrupt"); break;
#endif
#ifdef SIGABRT
    case SIGABRT: KWSYSPE_CASE(Other, "Child aborted"); break;
#endif
#ifdef SIGKILL
    case SIGKILL: KWSYSPE_CASE(Other, "Child killed"); break;
#endif
#ifdef SIGTERM
    case SIGTERM: KWSYSPE_CASE(Other, "Child terminated"); break;
#endif
#ifdef SIGHUP
    case SIGHUP: KWSYSPE_CASE(Other, "SIGHUP"); break;
#endif
#ifdef SIGQUIT
    case SIGQUIT: KWSYSPE_CASE(Other, "SIGQUIT"); break;
#endif
#ifdef SIGTRAP
    case SIGTRAP: KWSYSPE_CASE(Other, "SIGTRAP"); break;
#endif
#ifdef SIGIOT
# if !defined(SIGABRT) || SIGIOT != SIGABRT
    case SIGIOT: KWSYSPE_CASE(Other, "SIGIOT"); break;
# endif
#endif
#ifdef SIGUSR1
    case SIGUSR1: KWSYSPE_CASE(Other, "SIGUSR1"); break;
#endif
#ifdef SIGUSR2
    case SIGUSR2: KWSYSPE_CASE(Other, "SIGUSR2"); break;
#endif
#ifdef SIGPIPE
    case SIGPIPE: KWSYSPE_CASE(Other, "SIGPIPE"); break;
#endif
#ifdef SIGALRM
    case SIGALRM: KWSYSPE_CASE(Other, "SIGALRM"); break;
#endif
#ifdef SIGSTKFLT
    case SIGSTKFLT: KWSYSPE_CASE(Other, "SIGSTKFLT"); break;
#endif
#ifdef SIGCHLD
    case SIGCHLD: KWSYSPE_CASE(Other, "SIGCHLD"); break;
#elif defined(SIGCLD)
    case SIGCLD: KWSYSPE_CASE(Other, "SIGCLD"); break;
#endif
#ifdef SIGCONT
    case SIGCONT: KWSYSPE_CASE(Other, "SIGCONT"); break;
#endif
#ifdef SIGSTOP
    case SIGSTOP: KWSYSPE_CASE(Other, "SIGSTOP"); break;
#endif
#ifdef SIGTSTP
    case SIGTSTP: KWSYSPE_CASE(Other, "SIGTSTP"); break;
#endif
#ifdef SIGTTIN
    case SIGTTIN: KWSYSPE_CASE(Other, "SIGTTIN"); break;
#endif
#ifdef SIGTTOU
    case SIGTTOU: KWSYSPE_CASE(Other, "SIGTTOU"); break;
#endif
#ifdef SIGURG
    case SIGURG: KWSYSPE_CASE(Other, "SIGURG"); break;
#endif
#ifdef SIGXCPU
    case SIGXCPU: KWSYSPE_CASE(Other, "SIGXCPU"); break;
#endif
#ifdef SIGXFSZ
    case SIGXFSZ: KWSYSPE_CASE(Other, "SIGXFSZ"); break;
#endif
#ifdef SIGVTALRM
    case SIGVTALRM: KWSYSPE_CASE(Other, "SIGVTALRM"); break;
#endif
#ifdef SIGPROF
    case SIGPROF: KWSYSPE_CASE(Other, "SIGPROF"); break;
#endif
#ifdef SIGWINCH
    case SIGWINCH: KWSYSPE_CASE(Other, "SIGWINCH"); break;
#endif
#ifdef SIGPOLL
    case SIGPOLL: KWSYSPE_CASE(Other, "SIGPOLL"); break;
#endif
#ifdef SIGIO
# if !defined(SIGPOLL) || SIGIO != SIGPOLL
    case SIGIO: KWSYSPE_CASE(Other, "SIGIO"); break;
# endif
#endif
#ifdef SIGPWR
    case SIGPWR: KWSYSPE_CASE(Other, "SIGPWR"); break;
#endif
#ifdef SIGSYS
    case SIGSYS: KWSYSPE_CASE(Other, "SIGSYS"); break;
#endif
#ifdef SIGUNUSED
# if !defined(SIGSYS) || SIGUNUSED != SIGSYS
    case SIGUNUSED: KWSYSPE_CASE(Other, "SIGUNUSED"); break;
# endif
#endif
    default:
      cp->ExitException = kwsysProcess_Exception_Other;
      sprintf(cp->ExitExceptionString, "Signal %d", sig);
      break;
    }
}
#undef KWSYSPE_CASE

/*--------------------------------------------------------------------------*/
/* When the child process encounters an error before its program is
   invoked, this is called to report the error to the parent and
   exit.  */
static void kwsysProcessChildErrorExit(int errorPipe)
{
  /* Construct the error message.  */
  char buffer[KWSYSPE_PIPE_BUFFER_SIZE];
  strncpy(buffer, strerror(errno), KWSYSPE_PIPE_BUFFER_SIZE);

  /* Report the error to the parent through the special pipe.  */
  write(errorPipe, buffer, strlen(buffer));

  /* Terminate without cleanup.  */
  _exit(1);
}

/*--------------------------------------------------------------------------*/
/* Restores all signal handlers to their default values.  */
static void kwsysProcessRestoreDefaultSignalHandlers(void)
{
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_DFL;
#ifdef SIGHUP
  sigaction(SIGHUP, &act, 0);
#endif
#ifdef SIGINT
  sigaction(SIGINT, &act, 0);
#endif
#ifdef SIGQUIT
  sigaction(SIGQUIT, &act, 0);
#endif
#ifdef SIGILL
  sigaction(SIGILL, &act, 0);
#endif
#ifdef SIGTRAP
  sigaction(SIGTRAP, &act, 0);
#endif
#ifdef SIGABRT
  sigaction(SIGABRT, &act, 0);
#endif
#ifdef SIGIOT
  sigaction(SIGIOT, &act, 0);
#endif
#ifdef SIGBUS
  sigaction(SIGBUS, &act, 0);
#endif
#ifdef SIGFPE
  sigaction(SIGFPE, &act, 0);
#endif
#ifdef SIGUSR1
  sigaction(SIGUSR1, &act, 0);
#endif
#ifdef SIGSEGV
  sigaction(SIGSEGV, &act, 0);
#endif
#ifdef SIGUSR2
  sigaction(SIGUSR2, &act, 0);
#endif
#ifdef SIGPIPE
  sigaction(SIGPIPE, &act, 0);
#endif
#ifdef SIGALRM
  sigaction(SIGALRM, &act, 0);
#endif
#ifdef SIGTERM
  sigaction(SIGTERM, &act, 0);
#endif
#ifdef SIGSTKFLT
  sigaction(SIGSTKFLT, &act, 0);
#endif
#ifdef SIGCLD
  sigaction(SIGCLD, &act, 0);
#endif
#ifdef SIGCHLD
  sigaction(SIGCHLD, &act, 0);
#endif
#ifdef SIGCONT
  sigaction(SIGCONT, &act, 0);
#endif
#ifdef SIGTSTP
  sigaction(SIGTSTP, &act, 0);
#endif
#ifdef SIGTTIN
  sigaction(SIGTTIN, &act, 0);
#endif
#ifdef SIGTTOU
  sigaction(SIGTTOU, &act, 0);
#endif
#ifdef SIGURG
  sigaction(SIGURG, &act, 0);
#endif
#ifdef SIGXCPU
  sigaction(SIGXCPU, &act, 0);
#endif
#ifdef SIGXFSZ
  sigaction(SIGXFSZ, &act, 0);
#endif
#ifdef SIGVTALRM
  sigaction(SIGVTALRM, &act, 0);
#endif
#ifdef SIGPROF
  sigaction(SIGPROF, &act, 0);
#endif
#ifdef SIGWINCH
  sigaction(SIGWINCH, &act, 0);
#endif
#ifdef SIGPOLL
  sigaction(SIGPOLL, &act, 0);
#endif
#ifdef SIGIO
  sigaction(SIGIO, &act, 0);
#endif
#ifdef SIGPWR
  sigaction(SIGPWR, &act, 0);
#endif
#ifdef SIGSYS
  sigaction(SIGSYS, &act, 0);
#endif
#ifdef SIGUNUSED
  sigaction(SIGUNUSED, &act, 0);
#endif
}

/*--------------------------------------------------------------------------*/
static pid_t kwsysProcessFork(kwsysProcess* cp,
                              kwsysProcessCreateInformation* si)
{
  /* Create a detached process if requested.  */
  if(cp->OptionDetach)
    {
    /* Create an intermediate process.  */
    pid_t middle_pid = fork();
    if(middle_pid < 0)
      {
      /* Fork failed.  Return as if we were not detaching.  */
      return middle_pid;
      }
    else if(middle_pid == 0)
      {
      /* This is the intermediate process.  Create the real child.  */
      pid_t child_pid = fork();
      if(child_pid == 0)
        {
        /* This is the real child process.  There is nothing to do here.  */
        return 0;
        }
      else
        {
        /* Use the error pipe to report the pid to the real parent.  */
        while((write(si->ErrorPipe[1], &child_pid, sizeof(child_pid)) < 0) &&
              (errno == EINTR));

        /* Exit without cleanup.  The parent holds all resources.  */
        _exit(0);
        return 0; /* Never reached, but avoids SunCC warning.  */
        }
      }
    else
      {
      /* This is the original parent process.  The intermediate
         process will use the error pipe to report the pid of the
         detached child.  */
      pid_t child_pid;
      int status;
      while((read(si->ErrorPipe[0], &child_pid, sizeof(child_pid)) < 0) &&
            (errno == EINTR));

      /* Wait for the intermediate process to exit and clean it up.  */
      while((waitpid(middle_pid, &status, 0) < 0) && (errno == EINTR));
      return child_pid;
      }
    }
  else
    {
    /* Not creating a detached process.  Use normal fork.  */
    return fork();
    }
}

/*--------------------------------------------------------------------------*/
/* We try to obtain process information by invoking the ps command.
   Here we define the command to call on each platform and the
   corresponding parsing format string.  The parsing format should
   have two integers to store: the pid and then the ppid.  */
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
# define KWSYSPE_PS_COMMAND "ps axo pid,ppid"
# define KWSYSPE_PS_FORMAT  "%d %d\n"
#elif defined(__hpux) || defined(__sparc) || defined(__sgi) || defined(_AIX)
# define KWSYSPE_PS_COMMAND "ps -ef"
# define KWSYSPE_PS_FORMAT  "%*s %d %d %*[^\n]\n"
#endif

/*--------------------------------------------------------------------------*/
static void kwsysProcessKill(pid_t process_id)
{
#if defined(__linux__)
  DIR* procdir;
#endif

  /* Suspend the process to be sure it will not create more children.  */
  kill(process_id, SIGSTOP);

  /* Kill all children if we can find them.  */
#if defined(__linux__)
  /* First try using the /proc filesystem.  */
  if((procdir = opendir("/proc")) != NULL)
    {
#if defined(MAXPATHLEN)
    char fname[MAXPATHLEN];
#elif defined(PATH_MAX)
    char fname[PATH_MAX];
#else
    char fname[4096];
#endif
    char buffer[KWSYSPE_PIPE_BUFFER_SIZE+1];
    struct dirent* d;

    /* Each process has a directory in /proc whose name is the pid.
       Within this directory is a file called stat that has the
       following format:

         pid (command line) status ppid ...

       We want to get the ppid for all processes.  Those that have
       process_id as their parent should be recursively killed.  */
    for(d = readdir(procdir); d; d = readdir(procdir))
      {
      int pid;
      if(sscanf(d->d_name, "%d", &pid) == 1 && pid != 0)
        {
        struct stat finfo;
        sprintf(fname, "/proc/%d/stat", pid);
        if(stat(fname, &finfo) == 0)
          {
          FILE* f = fopen(fname, "r");
          if(f)
            {
            int nread = fread(buffer, 1, KWSYSPE_PIPE_BUFFER_SIZE, f);
            buffer[nread] = '\0';
            if(nread > 0)
              {
              const char* rparen = strrchr(buffer, ')');
              int ppid;
              if(rparen && (sscanf(rparen+1, "%*s %d", &ppid) == 1))
                {
                if(ppid == process_id)
                  {
                  /* Recursively kill this child and its children.  */
                  kwsysProcessKill(pid);
                  }
                }
              }
            fclose(f);
            }
          }
        }
      }
    closedir(procdir);
    }
  else
#endif
#if defined(KWSYSPE_PS_COMMAND)
    {
    /* Try running "ps" to get the process information.  */
    FILE* ps = popen(KWSYSPE_PS_COMMAND, "r");

    /* Make sure the process started and provided a valid header.  */
    if(ps && fscanf(ps, "%*[^\n]\n") != EOF)
      {
      /* Look for processes whose parent is the process being killed.  */
      int pid, ppid;
      while(fscanf(ps, KWSYSPE_PS_FORMAT, &pid, &ppid) == 2)
        {
        if(ppid == process_id)
          {
          /* Recursively kill this child aned its children.  */
          kwsysProcessKill(pid);
          }
        }
      }

    /* We are done with the ps process.  */
    if(ps)
      {
      pclose(ps);
      }
    }
#endif

  /* Kill the process.  */
  kill(process_id, SIGKILL);
}
