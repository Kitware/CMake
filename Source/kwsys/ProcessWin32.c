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
#include KWSYS_HEADER(System.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "Process.h.in"
# include "System.h.in"
#endif

/*

Implementation for Windows

On windows, a thread is created to wait for data on each pipe.  The
threads are synchronized with the main thread to simulate the use of
a UNIX-style select system call.

On Windows9x platforms, a small WIN32 console application is spawned
in-between the calling process and the actual child to be executed.
This is to work-around a problem with connecting pipes from WIN16
console applications to WIN32 applications.

For more information, please check Microsoft Knowledge Base Articles
Q190351 and Q150956.

*/

#ifdef _MSC_VER
#pragma warning (push, 1)
#endif
#include <windows.h> /* Windows API */
#include <string.h>  /* strlen, strdup */
#include <stdio.h>   /* sprintf */
#include <io.h>      /* _unlink */
#ifdef __WATCOMC__
#define _unlink unlink
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME 4096
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 4096
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#pragma warning (disable: 4514)
#pragma warning (disable: 4706)
#endif

#if defined(__BORLANDC__)
# pragma warn -8060 /* Assignment inside if() condition.  */
#endif

/* There are pipes for the process pipeline's stdout and stderr.  */
#define KWSYSPE_PIPE_COUNT 2
#define KWSYSPE_PIPE_STDOUT 0
#define KWSYSPE_PIPE_STDERR 1

/* The maximum amount to read from a pipe at a time.  */
#define KWSYSPE_PIPE_BUFFER_SIZE 1024

#define kwsysEncodedWriteArrayProcessFwd9x kwsys_ns(EncodedWriteArrayProcessFwd9x)

typedef LARGE_INTEGER kwsysProcessTime;

typedef struct kwsysProcessCreateInformation_s
{
  /* Windows child startup control data.  */
  STARTUPINFO StartupInfo;

  /* Special error reporting pipe for Win9x forwarding executable.  */
  HANDLE ErrorPipeRead;
  HANDLE ErrorPipeWrite;
} kwsysProcessCreateInformation;

/*--------------------------------------------------------------------------*/
typedef struct kwsysProcessPipeData_s kwsysProcessPipeData;
static DWORD WINAPI kwsysProcessPipeThreadRead(LPVOID ptd);
static void kwsysProcessPipeThreadReadPipe(kwsysProcess* cp,
                                           kwsysProcessPipeData* td);
static DWORD WINAPI kwsysProcessPipeThreadWake(LPVOID ptd);
static void kwsysProcessPipeThreadWakePipe(kwsysProcess* cp,
                                           kwsysProcessPipeData* td);
static int kwsysProcessInitialize(kwsysProcess* cp);
static int kwsysProcessCreate(kwsysProcess* cp, int index,
                              kwsysProcessCreateInformation* si,
                              PHANDLE readEnd);
static void kwsysProcessDestroy(kwsysProcess* cp, int event);
static int kwsysProcessSetupOutputPipeFile(PHANDLE handle, const char* name);
static int kwsysProcessSetupSharedPipe(DWORD nStdHandle, PHANDLE handle);
static void kwsysProcessCleanupHandle(PHANDLE h);
static void kwsysProcessCleanupHandleSafe(PHANDLE h, DWORD nStdHandle);
static void kwsysProcessCleanup(kwsysProcess* cp, int error);
static void kwsysProcessCleanErrorMessage(kwsysProcess* cp);
static int kwsysProcessComputeCommandLength(kwsysProcess* cp,
                                            char const* const* command);
static void kwsysProcessComputeCommandLine(kwsysProcess* cp,
                                           char const* const* command,
                                           char* cmd);
static int kwsysProcessGetTimeoutTime(kwsysProcess* cp, double* userTimeout,
                                      kwsysProcessTime* timeoutTime);
static int kwsysProcessGetTimeoutLeft(kwsysProcessTime* timeoutTime,
                                      double* userTimeout,
                                      kwsysProcessTime* timeoutLength);
static kwsysProcessTime kwsysProcessTimeGetCurrent(void);
static DWORD kwsysProcessTimeToDWORD(kwsysProcessTime t);
static double kwsysProcessTimeToDouble(kwsysProcessTime t);
static kwsysProcessTime kwsysProcessTimeFromDouble(double d);
static int kwsysProcessTimeLess(kwsysProcessTime in1, kwsysProcessTime in2);
static kwsysProcessTime kwsysProcessTimeAdd(kwsysProcessTime in1, kwsysProcessTime in2);
static kwsysProcessTime kwsysProcessTimeSubtract(kwsysProcessTime in1, kwsysProcessTime in2);
static void kwsysProcessSetExitException(kwsysProcess* cp, int code);
static void kwsysProcessKillTree(int pid);
static void kwsysProcessDisablePipeThreads(kwsysProcess* cp);
extern kwsysEXPORT int kwsysEncodedWriteArrayProcessFwd9x(const char* fname);

/*--------------------------------------------------------------------------*/
/* A structure containing synchronization data for each thread.  */
typedef struct kwsysProcessPipeSync_s kwsysProcessPipeSync;
struct kwsysProcessPipeSync_s
{
  /* Handle to the thread.  */
  HANDLE Thread;

  /* Semaphore indicating to the thread that a process has started.  */
  HANDLE Ready;

  /* Semaphore indicating to the thread that it should begin work.  */
  HANDLE Go;

  /* Semaphore indicating thread has reset for another process.  */
  HANDLE Reset;
};

/*--------------------------------------------------------------------------*/
/* A structure containing data for each pipe's threads.  */
struct kwsysProcessPipeData_s
{
  /* ------------- Data managed per instance of kwsysProcess ------------- */

  /* Synchronization data for reading thread.  */
  kwsysProcessPipeSync Reader;

  /* Synchronization data for waking thread.  */
  kwsysProcessPipeSync Waker;

  /* Index of this pipe.  */
  int Index;

  /* The kwsysProcess instance owning this pipe.  */
  kwsysProcess* Process;

  /* ------------- Data managed per call to Execute ------------- */

  /* Buffer for data read in this pipe's thread.  */
  char DataBuffer[KWSYSPE_PIPE_BUFFER_SIZE];

  /* The length of the data stored in the buffer.  */
  DWORD DataLength;

  /* Whether the pipe has been closed.  */
  int Closed;

  /* Handle for the read end of this pipe. */
  HANDLE Read;

  /* Handle for the write end of this pipe. */
  HANDLE Write;
};

/*--------------------------------------------------------------------------*/
/* Structure containing data used to implement the child's execution.  */
struct kwsysProcess_s
{
  /* ------------- Data managed per instance of kwsysProcess ------------- */

  /* The status of the process structure.  */
  int State;

  /* The command lines to execute.  */
  char** Commands;
  int NumberOfCommands;

  /* The exit code of each command.  */
  DWORD* CommandExitCodes;

  /* The working directory for the child process.  */
  char* WorkingDirectory;

  /* Whether to create the child as a detached process.  */
  int OptionDetach;

  /* Whether the child was created as a detached process.  */
  int Detached;

  /* Whether to hide the child process's window.  */
  int HideWindow;

  /* Whether to treat command lines as verbatim.  */
  int Verbatim;

  /* On Win9x platforms, the path to the forwarding executable.  */
  char* Win9x;

  /* On Win9x platforms, the resume event for the forwarding executable.  */
  HANDLE Win9xResumeEvent;

  /* On Win9x platforms, the kill event for the forwarding executable.  */
  HANDLE Win9xKillEvent;

  /* Mutex to protect the shared index used by threads to report data.  */
  HANDLE SharedIndexMutex;

  /* Semaphore used by threads to signal data ready.  */
  HANDLE Full;

  /* Whether we are currently deleting this kwsysProcess instance.  */
  int Deleting;

  /* Data specific to each pipe and its thread.  */
  kwsysProcessPipeData Pipe[KWSYSPE_PIPE_COUNT];

  /* Name of files to which stdin and stdout pipes are attached.  */
  char* PipeFileSTDIN;
  char* PipeFileSTDOUT;
  char* PipeFileSTDERR;

  /* Whether each pipe is shared with the parent process.  */
  int PipeSharedSTDIN;
  int PipeSharedSTDOUT;
  int PipeSharedSTDERR;

  /* Handle to automatically delete the Win9x forwarding executable.  */
  HANDLE Win9xHandle;

  /* ------------- Data managed per call to Execute ------------- */

  /* The exceptional behavior that terminated the process, if any.  */
  int ExitException;

  /* The process exit code.  */
  DWORD ExitCode;

  /* The process return code, if any.  */
  int ExitValue;

  /* Index of last pipe to report data, if any.  */
  int CurrentIndex;

  /* Index shared by threads to report data.  */
  int SharedIndex;

  /* The timeout length.  */
  double Timeout;

  /* Time at which the child started.  */
  kwsysProcessTime StartTime;

  /* Time at which the child will timeout.  Negative for no timeout.  */
  kwsysProcessTime TimeoutTime;

  /* Flag for whether the process was killed.  */
  int Killed;

  /* Flag for whether the timeout expired.  */
  int TimeoutExpired;

  /* Flag for whether the process has terminated.  */
  int Terminated;

  /* The number of pipes still open during execution and while waiting
     for pipes to close after process termination.  */
  int PipesLeft;

  /* Buffer for error messages (possibly from Win9x child).  */
  char ErrorMessage[KWSYSPE_PIPE_BUFFER_SIZE+1];

  /* Description for the ExitException.  */
  char ExitExceptionString[KWSYSPE_PIPE_BUFFER_SIZE+1];

  /* Windows process information data.  */
  PROCESS_INFORMATION* ProcessInformation;

  /* Data and process termination events for which to wait.  */
  PHANDLE ProcessEvents;
  int ProcessEventsLength;

  /* Real working directory of our own process.  */
  DWORD RealWorkingDirectoryLength;
  char* RealWorkingDirectory;
};

/*--------------------------------------------------------------------------*/
kwsysProcess* kwsysProcess_New(void)
{
  int i;

  /* Process control structure.  */
  kwsysProcess* cp;

  /* Path to Win9x forwarding executable.  */
  char* win9x = 0;

  /* Windows version number data.  */
  OSVERSIONINFO osv;

  /* Allocate a process control structure.  */
  cp = (kwsysProcess*)malloc(sizeof(kwsysProcess));
  if(!cp)
    {
    /* Could not allocate memory for the control structure.  */
    return 0;
    }
  ZeroMemory(cp, sizeof(*cp));

  /* Share stdin with the parent process by default.  */
  cp->PipeSharedSTDIN = 1;

  /* Set initial status.  */
  cp->State = kwsysProcess_State_Starting;

  /* Choose a method of running the child based on version of
     windows.  */
  ZeroMemory(&osv, sizeof(osv));
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);
  if(osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
    /* This is Win9x.  We need the console forwarding executable to
       work-around a Windows 9x bug.  */
    char fwdName[_MAX_FNAME+1] = "";
    char tempDir[_MAX_PATH+1] = "";

    /* We will try putting the executable in the system temp
       directory.  Note that the returned path already has a trailing
       slash.  */
    DWORD length = GetTempPath(_MAX_PATH+1, tempDir);

    /* Construct the executable name from the process id and kwsysProcess
       instance.  This should be unique.  */
    sprintf(fwdName, KWSYS_NAMESPACE_STRING "pew9xfwd_%u_%p.exe",
            GetCurrentProcessId(), cp);

    /* If we have a temp directory, use it.  */
    if(length > 0 && length <= _MAX_PATH)
      {
      /* Allocate a buffer to hold the forwarding executable path.  */
      size_t tdlen = strlen(tempDir);
      win9x = (char*)malloc(tdlen + strlen(fwdName) + 2);
      if(!win9x)
        {
        kwsysProcess_Delete(cp);
        return 0;
        }

      /* Construct the full path to the forwarding executable.  */
      sprintf(win9x, "%s%s", tempDir, fwdName);
      }

    /* If we found a place to put the forwarding executable, try to
       write it. */
    if(win9x)
      {
      if(!kwsysEncodedWriteArrayProcessFwd9x(win9x))
        {
        /* Failed to create forwarding executable.  Give up.  */
        free(win9x);
        kwsysProcess_Delete(cp);
        return 0;
        }

      /* Get a handle to the file that will delete it when closed.  */
      cp->Win9xHandle = CreateFile(win9x, GENERIC_READ, FILE_SHARE_READ, 0,
                                   OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0);
      if(cp->Win9xHandle == INVALID_HANDLE_VALUE)
        {
        /* We were not able to get a read handle for the forwarding
           executable.  It will not be deleted properly.  Give up.  */
        _unlink(win9x);
        free(win9x);
        kwsysProcess_Delete(cp);
        return 0;
        }
      }
    else
      {
      /* Failed to find a place to put forwarding executable.  */
      kwsysProcess_Delete(cp);
      return 0;
      }
    }

  /* Save the path to the forwarding executable.  */
  cp->Win9x = win9x;

  /* Initially no thread owns the mutex.  Initialize semaphore to 1.  */
  if(!(cp->SharedIndexMutex = CreateSemaphore(0, 1, 1, 0)))
    {
    kwsysProcess_Delete(cp);
    return 0;
    }

  /* Initially no data are available.  Initialize semaphore to 0.  */
  if(!(cp->Full = CreateSemaphore(0, 0, 1, 0)))
    {
    kwsysProcess_Delete(cp);
    return 0;
    }

  if(cp->Win9x)
    {
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    /* Create an event to tell the forwarding executable to resume the
       child.  */
    if(!(cp->Win9xResumeEvent = CreateEvent(&sa, TRUE, 0, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* Create an event to tell the forwarding executable to kill the
       child.  */
    if(!(cp->Win9xKillEvent = CreateEvent(&sa, TRUE, 0, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }
    }

  /* Create the thread to read each pipe.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    DWORD dummy=0;

    /* Assign the thread its index.  */
    cp->Pipe[i].Index = i;

    /* Give the thread a pointer back to the kwsysProcess instance.  */
    cp->Pipe[i].Process = cp;

    /* No process is yet running.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Reader.Ready = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* The pipe is not yet reset.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Reader.Reset = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* The thread's buffer is initially empty.  Initialize semaphore to 1.  */
    if(!(cp->Pipe[i].Reader.Go = CreateSemaphore(0, 1, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* Create the reading thread.  It will block immediately.  The
       thread will not make deeply nested calls, so we need only a
       small stack.  */
    if(!(cp->Pipe[i].Reader.Thread = CreateThread(0, 1024,
                                                  kwsysProcessPipeThreadRead,
                                                  &cp->Pipe[i], 0, &dummy)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* No process is yet running.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Waker.Ready = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* The pipe is not yet reset.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Waker.Reset = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* The waker should not wake immediately.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Waker.Go = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }

    /* Create the waking thread.  It will block immediately.  The
       thread will not make deeply nested calls, so we need only a
       small stack.  */
    if(!(cp->Pipe[i].Waker.Thread = CreateThread(0, 1024,
                                                 kwsysProcessPipeThreadWake,
                                                 &cp->Pipe[i], 0, &dummy)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }
    }

  return cp;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Delete(kwsysProcess* cp)
{
  int i;

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

  /* We are deleting the kwsysProcess instance.  */
  cp->Deleting = 1;

  /* Terminate each of the threads.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    /* Terminate this reading thread.  */
    if(cp->Pipe[i].Reader.Thread)
      {
      /* Signal the thread we are ready for it.  It will terminate
         immediately since Deleting is set.  */
      ReleaseSemaphore(cp->Pipe[i].Reader.Ready, 1, 0);

      /* Wait for the thread to exit.  */
      WaitForSingleObject(cp->Pipe[i].Reader.Thread, INFINITE);

      /* Close the handle to the thread. */
      kwsysProcessCleanupHandle(&cp->Pipe[i].Reader.Thread);
      }

    /* Terminate this waking thread.  */
    if(cp->Pipe[i].Waker.Thread)
      {
      /* Signal the thread we are ready for it.  It will terminate
         immediately since Deleting is set.  */
      ReleaseSemaphore(cp->Pipe[i].Waker.Ready, 1, 0);

      /* Wait for the thread to exit.  */
      WaitForSingleObject(cp->Pipe[i].Waker.Thread, INFINITE);

      /* Close the handle to the thread. */
      kwsysProcessCleanupHandle(&cp->Pipe[i].Waker.Thread);
      }

    /* Cleanup the pipe's semaphores.  */
    kwsysProcessCleanupHandle(&cp->Pipe[i].Reader.Ready);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Reader.Go);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Reader.Reset);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Waker.Ready);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Waker.Go);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Waker.Reset);
    }

  /* Close the shared semaphores.  */
  kwsysProcessCleanupHandle(&cp->SharedIndexMutex);
  kwsysProcessCleanupHandle(&cp->Full);

  /* Close the Win9x resume and kill event handles.  */
  if(cp->Win9x)
    {
    kwsysProcessCleanupHandle(&cp->Win9xResumeEvent);
    kwsysProcessCleanupHandle(&cp->Win9xKillEvent);
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
  if(cp->Win9x)
    {
    /* Close our handle to the forwarding executable file.  This will
       cause it to be deleted.  */
    kwsysProcessCleanupHandle(&cp->Win9xHandle);
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
  char** newCommands;

  /* Make sure we have a command to add.  */
  if(!cp || !command || !*command)
    {
    return 0;
    }

  /* Allocate a new array for command pointers.  */
  newNumberOfCommands = cp->NumberOfCommands + 1;
  if(!(newCommands = (char**)malloc(sizeof(char*) * newNumberOfCommands)))
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

  /* We need to construct a single string representing the command
     and its arguments.  We will surround each argument containing
     spaces with double-quotes.  Inside a double-quoted argument, we
     need to escape double-quotes and all backslashes before them.
     We also need to escape backslashes at the end of an argument
     because they come before the closing double-quote for the
     argument.  */
  {
  /* First determine the length of the final string.  */
  int length = kwsysProcessComputeCommandLength(cp, command);

  /* Allocate enough space for the command.  We do not need an extra
     byte for the terminating null because we allocated a space for
     the first argument that we will not use.  */
  newCommands[cp->NumberOfCommands] = (char*)malloc(length);
  if(!newCommands[cp->NumberOfCommands])
    {
    /* Out of memory.  */
    free(newCommands);
    return 0;
    }

  /* Construct the command line in the allocated buffer.  */
  kwsysProcessComputeCommandLine(cp, command,
                                 newCommands[cp->NumberOfCommands]);
  }

  /* Save the new array of commands.  */
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
  if(cp->WorkingDirectory)
    {
    free(cp->WorkingDirectory);
    cp->WorkingDirectory = 0;
    }
  if(dir && dir[0])
    {
    /* We must convert the working directory to a full path.  */
    DWORD length = GetFullPathName(dir, 0, 0, 0);
    if(length > 0)
      {
      cp->WorkingDirectory = (char*)malloc(length);
      if(!cp->WorkingDirectory)
        {
        return 0;
        }
      if(!GetFullPathName(dir, length, cp->WorkingDirectory, 0))
        {
        free(cp->WorkingDirectory);
        cp->WorkingDirectory = 0;
        return 0;
        }
      }
    }
  return 1;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_SetPipeFile(kwsysProcess* cp, int pipe, const char* file)
{
  char** pfile;
  if(!cp)
    {
    return 0;
    }
  switch(pipe)
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
    kwsysProcess_SetPipeShared(cp, pipe, 0);
    }

  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_SetPipeShared(kwsysProcess* cp, int pipe, int shared)
{
  if(!cp)
    {
    return;
    }

  switch(pipe)
    {
    case kwsysProcess_Pipe_STDIN: cp->PipeSharedSTDIN = shared?1:0; break;
    case kwsysProcess_Pipe_STDOUT: cp->PipeSharedSTDOUT = shared?1:0; break;
    case kwsysProcess_Pipe_STDERR: cp->PipeSharedSTDERR = shared?1:0; break;
    default: return;
    }

  /* If we are sharing the pipe, do not redirect it to a file.  */
  if(shared)
    {
    kwsysProcess_SetPipeFile(cp, pipe, 0);
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
    case kwsysProcess_Option_HideWindow: return cp->HideWindow;
    case kwsysProcess_Option_Verbatim: return cp->Verbatim;
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
    case kwsysProcess_Option_HideWindow: cp->HideWindow = value; break;
    case kwsysProcess_Option_Verbatim: cp->Verbatim = value; break;
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
int kwsysProcess_GetExitValue(kwsysProcess* cp)
{
  return cp? cp->ExitValue : -1;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitCode(kwsysProcess* cp)
{
  return cp? cp->ExitCode : 0;
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

  /* Child startup control data.  */
  kwsysProcessCreateInformation si;

  /* Do not execute a second time.  */
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
    if(!GetCurrentDirectory(cp->RealWorkingDirectoryLength,
                            cp->RealWorkingDirectory))
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }
    SetCurrentDirectory(cp->WorkingDirectory);
    }

  /* Reset the Win9x resume and kill events.  */
  if(cp->Win9x)
    {
    if(!ResetEvent(cp->Win9xResumeEvent))
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }
    if(!ResetEvent(cp->Win9xKillEvent))
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }
    }

  /* Initialize startup info data.  */
  ZeroMemory(&si, sizeof(si));
  si.StartupInfo.cb = sizeof(si.StartupInfo);

  /* Decide whether a child window should be shown.  */
  si.StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
  si.StartupInfo.wShowWindow =
    (unsigned short)(cp->HideWindow?SW_HIDE:SW_SHOWDEFAULT);

  /* Connect the child's output pipes to the threads.  */
  si.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

  /* Create stderr pipe to be shared by all processes in the pipeline.
     Neither end is directly inherited.  */
  if(!CreatePipe(&cp->Pipe[KWSYSPE_PIPE_STDERR].Read,
                 &cp->Pipe[KWSYSPE_PIPE_STDERR].Write, 0, 0))
    {
    kwsysProcessCleanup(cp, 1);
    return;
    }

  /* Create an inherited duplicate of the write end, but do not
     close the non-inherited version.  We need to keep it open
     to use in waking up the pipe threads.  */
  if(!DuplicateHandle(GetCurrentProcess(), cp->Pipe[KWSYSPE_PIPE_STDERR].Write,
                      GetCurrentProcess(), &si.StartupInfo.hStdError,
                      0, TRUE, DUPLICATE_SAME_ACCESS))
    {
    kwsysProcessCleanup(cp, 1);
    kwsysProcessCleanupHandle(&si.StartupInfo.hStdError);
    return;
    }

  /* Replace the stderr pipe with a file if requested.  In this case
     the pipe thread will still run but never report data.  */
  if(cp->PipeFileSTDERR)
    {
    if(!kwsysProcessSetupOutputPipeFile(&si.StartupInfo.hStdError,
                                        cp->PipeFileSTDERR))
      {
      kwsysProcessCleanup(cp, 1);
      kwsysProcessCleanupHandle(&si.StartupInfo.hStdError);
      return;
      }
    }

  /* Replace the stderr pipe with the parent process's if requested.
     In this case the pipe thread will still run but never report
     data.  */
  if(cp->PipeSharedSTDERR)
    {
    if(!kwsysProcessSetupSharedPipe(STD_ERROR_HANDLE,
                                    &si.StartupInfo.hStdError))
      {
      kwsysProcessCleanup(cp, 1);
      kwsysProcessCleanupHandleSafe(&si.StartupInfo.hStdError,
                                    STD_ERROR_HANDLE);
      return;
      }
    }

  /* Create the pipeline of processes.  */
  {
  HANDLE readEnd = 0;
  for(i=0; i < cp->NumberOfCommands; ++i)
    {
    if(kwsysProcessCreate(cp, i, &si, &readEnd))
      {
      cp->ProcessEvents[i+1] = cp->ProcessInformation[i].hProcess;
      }
    else
      {
      kwsysProcessCleanup(cp, 1);

      /* Release resources that may have been allocated for this
         process before an error occurred.  */
      kwsysProcessCleanupHandle(&readEnd);
      kwsysProcessCleanupHandleSafe(&si.StartupInfo.hStdInput,
                                    STD_INPUT_HANDLE);
      kwsysProcessCleanupHandleSafe(&si.StartupInfo.hStdOutput,
                                    STD_OUTPUT_HANDLE);
      kwsysProcessCleanupHandleSafe(&si.StartupInfo.hStdError,
                                    STD_ERROR_HANDLE);
      kwsysProcessCleanupHandle(&si.ErrorPipeRead);
      kwsysProcessCleanupHandle(&si.ErrorPipeWrite);
      return;
      }
    }

  /* Save a handle to the output pipe for the last process.  */
  cp->Pipe[KWSYSPE_PIPE_STDOUT].Read = readEnd;
  }

  /* Close the inherited handles to the stderr pipe shared by all
     processes in the pipeline.  The stdout and stdin pipes are not
     shared among all children and are therefore closed by
     kwsysProcessCreate after each child is created.  */
  kwsysProcessCleanupHandleSafe(&si.StartupInfo.hStdError, STD_ERROR_HANDLE);

  /* Restore the working directory.  */
  if(cp->RealWorkingDirectory)
    {
    SetCurrentDirectory(cp->RealWorkingDirectory);
    free(cp->RealWorkingDirectory);
    cp->RealWorkingDirectory = 0;
    }

  /* The timeout period starts now.  */
  cp->StartTime = kwsysProcessTimeGetCurrent();
  cp->TimeoutTime = kwsysProcessTimeFromDouble(-1);

  /* All processes in the pipeline have been started in suspended
     mode.  Resume them all now.  */
  if(cp->Win9x)
    {
    SetEvent(cp->Win9xResumeEvent);
    }
  else
    {
    for(i=0; i < cp->NumberOfCommands; ++i)
      {
      ResumeThread(cp->ProcessInformation[i].hThread);
      }
    }

  /* ---- It is no longer safe to call kwsysProcessCleanup. ----- */
  /* Tell the pipe threads that a process has started.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    ReleaseSemaphore(cp->Pipe[i].Reader.Ready, 1, 0);
    ReleaseSemaphore(cp->Pipe[i].Waker.Ready, 1, 0);
    }

  /* We don't care about the children's main threads.  */
  for(i=0; i < cp->NumberOfCommands; ++i)
    {
    kwsysProcessCleanupHandle(&cp->ProcessInformation[i].hThread);
    }

  /* No pipe has reported data.  */
  cp->CurrentIndex = KWSYSPE_PIPE_COUNT;
  cp->PipesLeft = KWSYSPE_PIPE_COUNT;

  /* The process has now started.  */
  cp->State = kwsysProcess_State_Executing;
  cp->Detached = cp->OptionDetach;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Disown(kwsysProcess* cp)
{
  int i;

  /* Make sure we are executing a detached process.  */
  if(!cp || !cp->Detached || cp->State != kwsysProcess_State_Executing ||
     cp->TimeoutExpired || cp->Killed || cp->Terminated)
    {
    return;
    }

  /* Disable the reading threads.  */
  kwsysProcessDisablePipeThreads(cp);

  /* Wait for all pipe threads to reset.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    WaitForSingleObject(cp->Pipe[i].Reader.Reset, INFINITE);
    WaitForSingleObject(cp->Pipe[i].Waker.Reset, INFINITE);
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
  kwsysProcessTime userStartTime;
  kwsysProcessTime timeoutLength;
  kwsysProcessTime timeoutTime;
  DWORD timeout;
  int user;
  int done = 0;
  int expired = 0;
  int pipeId = kwsysProcess_Pipe_None;
  DWORD w;

  /* Make sure we are executing a process.  */
  if(!cp || cp->State != kwsysProcess_State_Executing || cp->Killed ||
     cp->TimeoutExpired)
    {
    return kwsysProcess_Pipe_None;
    }

  /* Record the time at which user timeout period starts.  */
  userStartTime = kwsysProcessTimeGetCurrent();

  /* Calculate the time at which a timeout will expire, and whether it
     is the user or process timeout.  */
  user = kwsysProcessGetTimeoutTime(cp, userTimeout, &timeoutTime);

  /* Loop until we have a reason to return.  */
  while(!done && cp->PipesLeft > 0)
    {
    /* If we previously got data from a thread, let it know we are
       done with the data.  */
    if(cp->CurrentIndex < KWSYSPE_PIPE_COUNT)
      {
      ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Reader.Go, 1, 0);
      cp->CurrentIndex = KWSYSPE_PIPE_COUNT;
      }

    /* Setup a timeout if required.  */
    if(kwsysProcessGetTimeoutLeft(&timeoutTime, user?userTimeout:0,
                                  &timeoutLength))
      {
      /* Timeout has already expired.  */
      expired = 1;
      break;
      }
    if(timeoutTime.QuadPart < 0)
      {
      timeout = INFINITE;
      }
    else
      {
      timeout = kwsysProcessTimeToDWORD(timeoutLength);
      }

    /* Wait for a pipe's thread to signal or a process to terminate.  */
    w = WaitForMultipleObjects(cp->ProcessEventsLength, cp->ProcessEvents,
                               0, timeout);
    if(w == WAIT_TIMEOUT)
      {
      /* Timeout has expired.  */
      expired = 1;
      done = 1;
      }
    else if(w == WAIT_OBJECT_0)
      {
      /* Save the index of the reporting thread and release the mutex.
         The thread will block until we signal its Empty mutex.  */
      cp->CurrentIndex = cp->SharedIndex;
      ReleaseSemaphore(cp->SharedIndexMutex, 1, 0);

      /* Data are available or a pipe closed.  */
      if(cp->Pipe[cp->CurrentIndex].Closed)
        {
        /* The pipe closed at the write end.  Close the read end and
           inform the wakeup thread it is done with this process.  */
        kwsysProcessCleanupHandle(&cp->Pipe[cp->CurrentIndex].Read);
        ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Waker.Go, 1, 0);
        --cp->PipesLeft;
        }
      else if(data && length)
        {
        /* Report this data.  */
        *data = cp->Pipe[cp->CurrentIndex].DataBuffer;
        *length = cp->Pipe[cp->CurrentIndex].DataLength;
        switch(cp->CurrentIndex)
          {
          case KWSYSPE_PIPE_STDOUT:
            pipeId = kwsysProcess_Pipe_STDOUT; break;
          case KWSYSPE_PIPE_STDERR:
            pipeId = kwsysProcess_Pipe_STDERR; break;
          }
        done = 1;
        }
      }
    else
      {
      /* A process has terminated.  */
      kwsysProcessDestroy(cp, w-WAIT_OBJECT_0);
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
      /* The process timeout has expired.  Kill the child now.  */
      kwsysProcess_Kill(cp);
      cp->TimeoutExpired = 1;
      cp->Killed = 0;
      return kwsysProcess_Pipe_None;
      }
    }
  else
    {
    /* The children have terminated and no more data are available.  */
    return kwsysProcess_Pipe_None;
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_WaitForExit(kwsysProcess* cp, double* userTimeout)
{
  int i;
  int pipe;

  /* Make sure we are executing a process.  */
  if(!cp || cp->State != kwsysProcess_State_Executing)
    {
    return 1;
    }

  /* Wait for the process to terminate.  Ignore all data.  */
  while((pipe = kwsysProcess_WaitForData(cp, 0, 0, userTimeout)) > 0)
    {
    if(pipe == kwsysProcess_Pipe_Timeout)
      {
      /* The user timeout has expired.  */
      return 0;
      }
    }

  /* When the last pipe closes in WaitForData, the loop terminates
     without releasing the pipe's thread.  Release it now.  */
  if(cp->CurrentIndex < KWSYSPE_PIPE_COUNT)
    {
    ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Reader.Go, 1, 0);
    cp->CurrentIndex = KWSYSPE_PIPE_COUNT;
    }

  /* Wait for all pipe threads to reset.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    WaitForSingleObject(cp->Pipe[i].Reader.Reset, INFINITE);
    WaitForSingleObject(cp->Pipe[i].Waker.Reset, INFINITE);
    }

  /* ---- It is now safe again to call kwsysProcessCleanup. ----- */
  /* Close all the pipes.  */
  kwsysProcessCleanup(cp, 0);

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
  else
    {
    /* The children exited.  Report the outcome of the last process.  */
    cp->ExitCode = cp->CommandExitCodes[cp->NumberOfCommands-1];
    if((cp->ExitCode & 0xF0000000) == 0xC0000000)
      {
      /* Child terminated due to exceptional behavior.  */
      cp->State = kwsysProcess_State_Exception;
      cp->ExitValue = 1;
      kwsysProcessSetExitException(cp, cp->ExitCode);
      }
    else
      {
      /* Child exited without exception.  */
      cp->State = kwsysProcess_State_Exited;
      cp->ExitException = kwsysProcess_Exception_None;
      cp->ExitValue = cp->ExitCode;
      }
    }

  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Kill(kwsysProcess* cp)
{
  int i;

  /* Make sure we are executing a process.  */
  if(!cp || cp->State != kwsysProcess_State_Executing || cp->TimeoutExpired ||
     cp->Killed)
    {
    return;
    }

  /* Disable the reading threads.  */
  kwsysProcessDisablePipeThreads(cp);

  /* Skip actually killing the child if it has already terminated.  */
  if(cp->Terminated)
    {
    return;
    }

  /* Kill the children.  */
  cp->Killed = 1;
  if(cp->Win9x)
    {
    /* Windows 9x.  Tell the forwarding executable to kill the child.  */
    SetEvent(cp->Win9xKillEvent);
    }
  else
    {
    /* Not Windows 9x.  Just terminate the children.  */
    for(i=0; i < cp->NumberOfCommands; ++i)
      {
      kwsysProcessKillTree(cp->ProcessInformation[i].dwProcessId);
      }
    }

  /* We are killing the children and ignoring all data.  Do not wait
     for them to exit.  */
}

/*--------------------------------------------------------------------------*/

/*
  Function executed for each pipe's thread.  Argument is a pointer to
  the kwsysProcessPipeData instance for this thread.
*/
DWORD WINAPI kwsysProcessPipeThreadRead(LPVOID ptd)
{
  kwsysProcessPipeData* td = (kwsysProcessPipeData*)ptd;
  kwsysProcess* cp = td->Process;

  /* Wait for a process to be ready.  */
  while((WaitForSingleObject(td->Reader.Ready, INFINITE), !cp->Deleting))
    {
    /* Read output from the process for this thread's pipe.  */
    kwsysProcessPipeThreadReadPipe(cp, td);

    /* Signal the main thread we have reset for a new process.  */
    ReleaseSemaphore(td->Reader.Reset, 1, 0);
    }
  return 0;
}

/*--------------------------------------------------------------------------*/

/*
  Function called in each pipe's thread to handle data for one
  execution of a subprocess.
*/
void kwsysProcessPipeThreadReadPipe(kwsysProcess* cp, kwsysProcessPipeData* td)
{
  /* Wait for space in the thread's buffer. */
  while((WaitForSingleObject(td->Reader.Go, INFINITE), !td->Closed))
    {
    /* Read data from the pipe.  This may block until data are available.  */
    if(!ReadFile(td->Read, td->DataBuffer, KWSYSPE_PIPE_BUFFER_SIZE,
                 &td->DataLength, 0))
      {
      if(GetLastError() != ERROR_BROKEN_PIPE)
        {
        /* UNEXPECTED failure to read the pipe.  */
        }

      /* The pipe closed.  There are no more data to read.  */
      td->Closed = 1;
      }

    /* Wait for our turn to be handled by the main thread.  */
    WaitForSingleObject(cp->SharedIndexMutex, INFINITE);

    /* Tell the main thread we have something to report.  */
    cp->SharedIndex = td->Index;
    ReleaseSemaphore(cp->Full, 1, 0);
    }

  /* We were signalled to exit with our buffer empty.  Reset the
     mutex for a new process.  */
  ReleaseSemaphore(td->Reader.Go, 1, 0);
}

/*--------------------------------------------------------------------------*/

/*
  Function executed for each pipe's thread.  Argument is a pointer to
  the kwsysProcessPipeData instance for this thread.
*/
DWORD WINAPI kwsysProcessPipeThreadWake(LPVOID ptd)
{
  kwsysProcessPipeData* td = (kwsysProcessPipeData*)ptd;
  kwsysProcess* cp = td->Process;

  /* Wait for a process to be ready.  */
  while((WaitForSingleObject(td->Waker.Ready, INFINITE), !cp->Deleting))
    {
    /* Wait for a possible wakeup.  */
    kwsysProcessPipeThreadWakePipe(cp, td);

    /* Signal the main thread we have reset for a new process.  */
    ReleaseSemaphore(td->Waker.Reset, 1, 0);
    }
  return 0;
}

/*--------------------------------------------------------------------------*/

/*
  Function called in each pipe's thread to handle reading thread
  wakeup for one execution of a subprocess.
*/
void kwsysProcessPipeThreadWakePipe(kwsysProcess* cp, kwsysProcessPipeData* td)
{
  (void)cp;

  /* Wait for a possible wake command. */
  WaitForSingleObject(td->Waker.Go, INFINITE);

  /* If the pipe is not closed, we need to wake up the reading thread.  */
  if(!td->Closed)
    {
    DWORD dummy;
    WriteFile(td->Write, "", 1, &dummy, 0);
    }
}

/*--------------------------------------------------------------------------*/
/* Initialize a process control structure for kwsysProcess_Execute.  */
int kwsysProcessInitialize(kwsysProcess* cp)
{
  /* Reset internal status flags.  */
  cp->TimeoutExpired = 0;
  cp->Terminated = 0;
  cp->Killed = 0;
  cp->ExitException = kwsysProcess_Exception_None;
  cp->ExitCode = 1;
  cp->ExitValue = 1;

  /* Reset error data.  */
  cp->ErrorMessage[0] = 0;
  strcpy(cp->ExitExceptionString, "No exception");

  /* Allocate process information for each process.  */
  cp->ProcessInformation =
    (PROCESS_INFORMATION*)malloc(sizeof(PROCESS_INFORMATION) *
                                 cp->NumberOfCommands);
  if(!cp->ProcessInformation)
    {
    return 0;
    }
  ZeroMemory(cp->ProcessInformation,
             sizeof(PROCESS_INFORMATION) * cp->NumberOfCommands);
  if(cp->CommandExitCodes)
    {
    free(cp->CommandExitCodes);
    }
  cp->CommandExitCodes = (DWORD*)malloc(sizeof(DWORD)*cp->NumberOfCommands);
  if(!cp->CommandExitCodes)
    {
    return 0;
    }
  ZeroMemory(cp->CommandExitCodes, sizeof(DWORD)*cp->NumberOfCommands);

  /* Allocate event wait array.  The first event is cp->Full, the rest
     are the process termination events.  */
  cp->ProcessEvents = (PHANDLE)malloc(sizeof(HANDLE)*(cp->NumberOfCommands+1));
  if(!cp->ProcessEvents)
    {
    return 0;
    }
  ZeroMemory(cp->ProcessEvents, sizeof(HANDLE) * (cp->NumberOfCommands+1));
  cp->ProcessEvents[0] = cp->Full;
  cp->ProcessEventsLength = cp->NumberOfCommands+1;

  /* Allocate space to save the real working directory of this process.  */
  if(cp->WorkingDirectory)
    {
    cp->RealWorkingDirectoryLength = GetCurrentDirectory(0, 0);
    if(cp->RealWorkingDirectoryLength > 0)
      {
      cp->RealWorkingDirectory = malloc(cp->RealWorkingDirectoryLength);
      if(!cp->RealWorkingDirectory)
        {
        return 0;
        }
      }
    }

  return 1;
}

/*--------------------------------------------------------------------------*/
int kwsysProcessCreate(kwsysProcess* cp, int index,
                       kwsysProcessCreateInformation* si,
                       PHANDLE readEnd)
{
  /* Setup the process's stdin.  */
  if(*readEnd)
    {
    /* Create an inherited duplicate of the read end from the output
       pipe of the previous process.  This also closes the
       non-inherited version. */
    if(!DuplicateHandle(GetCurrentProcess(), *readEnd,
                        GetCurrentProcess(), readEnd,
                        0, TRUE, (DUPLICATE_CLOSE_SOURCE |
                                  DUPLICATE_SAME_ACCESS)))
      {
      return 0;
      }
    si->StartupInfo.hStdInput = *readEnd;

    /* This function is done with this handle.  */
    *readEnd = 0;
    }
  else if(cp->PipeFileSTDIN)
    {
    /* Create a handle to read a file for stdin.  */
    HANDLE fin = CreateFile(cp->PipeFileSTDIN, GENERIC_READ|GENERIC_WRITE,
                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                            0, OPEN_EXISTING, 0, 0);
    if(fin == INVALID_HANDLE_VALUE)
      {
      return 0;
      }
    /* Create an inherited duplicate of the handle.  This also closes
       the non-inherited version.  */
    if(!DuplicateHandle(GetCurrentProcess(), fin,
                        GetCurrentProcess(), &fin,
                        0, TRUE, (DUPLICATE_CLOSE_SOURCE |
                                  DUPLICATE_SAME_ACCESS)))
      {
      return 0;
      }
    si->StartupInfo.hStdInput = fin;
    }
  else if(cp->PipeSharedSTDIN)
    {
    /* Share this process's stdin with the child.  */
    if(!kwsysProcessSetupSharedPipe(STD_INPUT_HANDLE,
                                    &si->StartupInfo.hStdInput))
      {
      return 0;
      }
    }
  else
    {
    /* Explicitly give the child no stdin.  */
    si->StartupInfo.hStdInput = INVALID_HANDLE_VALUE;
    }

  /* Setup the process's stdout.  */
  {
  DWORD maybeClose = DUPLICATE_CLOSE_SOURCE;
  HANDLE writeEnd;

  /* Create the output pipe for this process.  Neither end is directly
     inherited.  */
  if(!CreatePipe(readEnd, &writeEnd, 0, 0))
    {
    return 0;
    }

  /* Create an inherited duplicate of the write end.  Close the
     non-inherited version unless this is the last process.  Save the
     non-inherited write end of the last process.  */
  if(index == cp->NumberOfCommands-1)
    {
    cp->Pipe[KWSYSPE_PIPE_STDOUT].Write = writeEnd;
    maybeClose = 0;
    }
  if(!DuplicateHandle(GetCurrentProcess(), writeEnd,
                      GetCurrentProcess(), &writeEnd,
                      0, TRUE, (maybeClose | DUPLICATE_SAME_ACCESS)))
    {
    return 0;
    }
  si->StartupInfo.hStdOutput = writeEnd;
  }

  /* Replace the stdout pipe with a file if requested.  In this case
     the pipe thread will still run but never report data.  */
  if(index == cp->NumberOfCommands-1 && cp->PipeFileSTDOUT)
    {
    if(!kwsysProcessSetupOutputPipeFile(&si->StartupInfo.hStdOutput,
                                        cp->PipeFileSTDOUT))
      {
      return 0;
      }
    }

  /* Replace the stdout pipe of the last child with the parent
     process's if requested.  In this case the pipe thread will still
     run but never report data.  */
  if(index == cp->NumberOfCommands-1 && cp->PipeSharedSTDOUT)
    {
    if(!kwsysProcessSetupSharedPipe(STD_OUTPUT_HANDLE,
                                    &si->StartupInfo.hStdOutput))
      {
      return 0;
      }
    }

  /* Create the child process.  */
  {
  BOOL r;
  char* realCommand;
  if(cp->Win9x)
    {
    /* Create an error reporting pipe for the forwarding executable.
       Neither end is directly inherited.  */
    if(!CreatePipe(&si->ErrorPipeRead, &si->ErrorPipeWrite, 0, 0))
      {
      return 0;
      }

    /* Create an inherited duplicate of the write end.  This also closes
       the non-inherited version. */
    if(!DuplicateHandle(GetCurrentProcess(), si->ErrorPipeWrite,
                        GetCurrentProcess(), &si->ErrorPipeWrite,
                        0, TRUE, (DUPLICATE_CLOSE_SOURCE |
                                  DUPLICATE_SAME_ACCESS)))
      {
      return 0;
      }

    /* The forwarding executable is given a handle to the error pipe
       and resume and kill events.  */
    realCommand = malloc(strlen(cp->Win9x)+strlen(cp->Commands[index])+100);
    if(!realCommand)
      {
      return 0;
      }
    sprintf(realCommand, "%s %p %p %p %d %s", cp->Win9x,
            si->ErrorPipeWrite, cp->Win9xResumeEvent, cp->Win9xKillEvent,
            cp->HideWindow, cp->Commands[index]);
    }
  else
    {
    realCommand = cp->Commands[index];
    }

  /* Create the child in a suspended state so we can wait until all
     children have been created before running any one.  */
  r = CreateProcess(0, realCommand, 0, 0, TRUE,
                    cp->Win9x? 0 : CREATE_SUSPENDED, 0, 0,
                    &si->StartupInfo, &cp->ProcessInformation[index]);

  if(cp->Win9x)
    {
    /* Free memory.  */
    free(realCommand);

    /* Close the error pipe write end so we can detect when the
       forwarding executable closes it.  */
    kwsysProcessCleanupHandle(&si->ErrorPipeWrite);
    if(r)
      {
      /* Wait for the forwarding executable to report an error or
         close the error pipe to report success.  */
      DWORD total = 0;
      DWORD n = 1;
      while(total < KWSYSPE_PIPE_BUFFER_SIZE && n > 0)
        {
        if(ReadFile(si->ErrorPipeRead, cp->ErrorMessage+total,
                    KWSYSPE_PIPE_BUFFER_SIZE-total, &n, 0))
          {
          total += n;
          }
        else
          {
          n = 0;
          }
        }
      if(total > 0 || GetLastError() != ERROR_BROKEN_PIPE)
        {
        /* The forwarding executable could not run the process, or
           there was an error reading from its error pipe.  Preserve
           the last error while cleaning up the forwarding executable
           so the cleanup our caller does reports the proper error.  */
        DWORD error = GetLastError();
        kwsysProcessCleanupHandle(&cp->ProcessInformation[index].hThread);
        kwsysProcessCleanupHandle(&cp->ProcessInformation[index].hProcess);
        SetLastError(error);
        return 0;
        }
      }
    kwsysProcessCleanupHandle(&si->ErrorPipeRead);
    }

  if(!r)
    {
    return 0;
    }
  }

  /* Successfully created this child process.  Close the current
     process's copies of the inherited stdout and stdin handles.  The
     stderr handle is shared among all children and is closed by
     kwsysProcess_Execute after all children have been created.  */
  kwsysProcessCleanupHandleSafe(&si->StartupInfo.hStdInput,
                                STD_INPUT_HANDLE);
  kwsysProcessCleanupHandleSafe(&si->StartupInfo.hStdOutput,
                                STD_OUTPUT_HANDLE);

  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcessDestroy(kwsysProcess* cp, int event)
{
  int i;
  int index;

  /* Find the process index for the termination event.  */
  for(index=0; index < cp->NumberOfCommands; ++index)
    {
    if(cp->ProcessInformation[index].hProcess == cp->ProcessEvents[event])
      {
      break;
      }
    }

  /* Check the exit code of the process.  */
  GetExitCodeProcess(cp->ProcessInformation[index].hProcess,
                     &cp->CommandExitCodes[index]);

  /* Close the process handle for the terminated process.  */
  kwsysProcessCleanupHandle(&cp->ProcessInformation[index].hProcess);

  /* Remove the process from the available events.  */
  cp->ProcessEventsLength -= 1;
  for(i=event; i < cp->ProcessEventsLength; ++i)
    {
    cp->ProcessEvents[i] = cp->ProcessEvents[i+1];
    }

  /* Check if all processes have terminated.  */
  if(cp->ProcessEventsLength == 1)
    {
    cp->Terminated = 1;

    /* Close our copies of the pipe write handles so the pipe threads
       can detect end-of-data.  */
    for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
      {
      kwsysProcessCleanupHandle(&cp->Pipe[i].Write);
      }
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcessSetupOutputPipeFile(PHANDLE phandle, const char* name)
{
  HANDLE fout;
  if(!name)
    {
    return 1;
    }

  /* Close the existing inherited handle.  */
  kwsysProcessCleanupHandle(phandle);

  /* Create a handle to write a file for the pipe.  */
  fout = CreateFile(name, GENERIC_WRITE, FILE_SHARE_READ, 0,
                    CREATE_ALWAYS, 0, 0);
  if(fout == INVALID_HANDLE_VALUE)
    {
    return 0;
    }

  /* Create an inherited duplicate of the handle.  This also closes
     the non-inherited version.  */
  if(!DuplicateHandle(GetCurrentProcess(), fout,
                      GetCurrentProcess(), &fout,
                      0, TRUE, (DUPLICATE_CLOSE_SOURCE |
                                DUPLICATE_SAME_ACCESS)))
    {
    return 0;
    }

  /* Assign the replacement handle.  */
  *phandle = fout;
  return 1;
}

/*--------------------------------------------------------------------------*/
int kwsysProcessSetupSharedPipe(DWORD nStdHandle, PHANDLE handle)
{
  /* Check whether the handle to be shared is already inherited.  */
  DWORD flags;
  int inherited = 0;
  if(GetHandleInformation(GetStdHandle(nStdHandle), &flags) &&
     (flags & HANDLE_FLAG_INHERIT))
    {
    inherited = 1;
    }

  /* Cleanup the previous handle.  */
  kwsysProcessCleanupHandle(handle);

  /* If the standard handle is not inherited then duplicate it to
     create an inherited copy.  Do not close the original handle when
     duplicating!  */
  if(inherited)
    {
    *handle = GetStdHandle(nStdHandle);
    return 1;
    }
  else if(DuplicateHandle(GetCurrentProcess(), GetStdHandle(nStdHandle),
                          GetCurrentProcess(), handle,
                          0, TRUE, DUPLICATE_SAME_ACCESS))
    {
    return 1;
    }
  else
    {
    /* The given standard handle is not valid for this process.  Some
       child processes may break if they do not have a valid standard
       pipe, so give the child an empty pipe.  For the stdin pipe we
       want to close the write end and give the read end to the child.
       For stdout and stderr we want to close the read end and give
       the write end to the child.  */
    int child_end = (nStdHandle == STD_INPUT_HANDLE)? 0:1;
    int parent_end = (nStdHandle == STD_INPUT_HANDLE)? 1:0;
    HANDLE emptyPipe[2];
    if(!CreatePipe(&emptyPipe[0], &emptyPipe[1], 0, 0))
      {
      return 0;
      }

    /* Close the non-inherited end so the pipe will be broken
       immediately.  */
    CloseHandle(emptyPipe[parent_end]);

    /* Create an inherited duplicate of the handle.  This also
       closes the non-inherited version.  */
    if(!DuplicateHandle(GetCurrentProcess(), emptyPipe[child_end],
                        GetCurrentProcess(), &emptyPipe[child_end],
                        0, TRUE, (DUPLICATE_CLOSE_SOURCE |
                                  DUPLICATE_SAME_ACCESS)))
      {
      return 0;
      }

    /* Give the inherited handle to the child.  */
    *handle = emptyPipe[child_end];
    return 1;
    }
}

/*--------------------------------------------------------------------------*/

/* Close the given handle if it is open.  Reset its value to 0.  */
void kwsysProcessCleanupHandle(PHANDLE h)
{
  if(h && *h)
    {
    CloseHandle(*h);
    *h = 0;
    }
}

/*--------------------------------------------------------------------------*/

/* Close the given handle if it is open and not a standard handle.
   Reset its value to 0.  */
void kwsysProcessCleanupHandleSafe(PHANDLE h, DWORD nStdHandle)
{
  if(h && *h && (*h != GetStdHandle(nStdHandle)))
    {
    CloseHandle(*h);
    *h = 0;
    }
}

/*--------------------------------------------------------------------------*/

/* Close all handles created by kwsysProcess_Execute.  */
void kwsysProcessCleanup(kwsysProcess* cp, int error)
{
  int i;

  /* If this is an error case, report the error.  */
  if(error)
    {
    /* Construct an error message if one has not been provided already.  */
    if(cp->ErrorMessage[0] == 0)
      {
      /* Format the error message.  */
      DWORD original = GetLastError();
      DWORD length = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS, 0, original,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   cp->ErrorMessage, KWSYSPE_PIPE_BUFFER_SIZE, 0);
      if(length < 1)
        {
        /* FormatMessage failed.  Use a default message.  */
        _snprintf(cp->ErrorMessage, KWSYSPE_PIPE_BUFFER_SIZE,
                  "Process execution failed with error 0x%X.  "
                  "FormatMessage failed with error 0x%X",
                  original, GetLastError());
        }
      }

    /* Remove trailing period and newline, if any.  */
    kwsysProcessCleanErrorMessage(cp);

    /* Set the error state.  */
    cp->State = kwsysProcess_State_Error;

    /* Cleanup any processes already started in a suspended state.  */
    if(cp->ProcessInformation)
      {
      if(cp->Win9x)
        {
        SetEvent(cp->Win9xKillEvent);
        }
      else
        {
        for(i=0; i < cp->NumberOfCommands; ++i)
          {
          if(cp->ProcessInformation[i].hProcess)
            {
            TerminateProcess(cp->ProcessInformation[i].hProcess, 255);
            WaitForSingleObject(cp->ProcessInformation[i].hProcess, INFINITE);
            }
          }
        }
      for(i=0; i < cp->NumberOfCommands; ++i)
        {
        kwsysProcessCleanupHandle(&cp->ProcessInformation[i].hThread);
        kwsysProcessCleanupHandle(&cp->ProcessInformation[i].hProcess);
        }
      }

    /* Restore the working directory.  */
    if(cp->RealWorkingDirectory)
      {
      SetCurrentDirectory(cp->RealWorkingDirectory);
      }
    }

  /* Free memory.  */
  if(cp->ProcessInformation)
    {
    free(cp->ProcessInformation);
    cp->ProcessInformation = 0;
    }
  if(cp->ProcessEvents)
    {
    free(cp->ProcessEvents);
    cp->ProcessEvents = 0;
    }
  if(cp->RealWorkingDirectory)
    {
    free(cp->RealWorkingDirectory);
    cp->RealWorkingDirectory = 0;
    }

  /* Close each pipe.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    kwsysProcessCleanupHandle(&cp->Pipe[i].Write);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Read);
    cp->Pipe[i].Closed = 0;
    }
}

/*--------------------------------------------------------------------------*/
void kwsysProcessCleanErrorMessage(kwsysProcess* cp)
{
  /* Remove trailing period and newline, if any.  */
  size_t length = strlen(cp->ErrorMessage);
  if(cp->ErrorMessage[length-1] == '\n')
    {
    cp->ErrorMessage[length-1] = 0;
    --length;
    if(length > 0 && cp->ErrorMessage[length-1] == '\r')
      {
      cp->ErrorMessage[length-1] = 0;
      --length;
      }
    }
  if(length > 0 && cp->ErrorMessage[length-1] == '.')
    {
    cp->ErrorMessage[length-1] = 0;
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcessComputeCommandLength(kwsysProcess* cp,
                                     char const* const* command)
{
  int length = 0;
  if(cp->Verbatim)
    {
    /* Treat the first argument as a verbatim command line.  Use its
       length directly and add space for the null-terminator.  */
    length = (int)strlen(*command)+1;
    }
  else
    {
    /* Compute the length of the command line when it is converted to
       a single string.  Space for the null-terminator is allocated by
       the whitespace character allocated for the first argument that
       will not be used.  */
    char const* const* arg;
    for(arg = command; *arg; ++arg)
      {
      /* Add the length of this argument.  It already includes room
         for a separating space or terminating null.  */
      length += kwsysSystem_Windows_ShellArgumentSize(*arg);
      }
    }

  return length;
}

/*--------------------------------------------------------------------------*/
void kwsysProcessComputeCommandLine(kwsysProcess* cp,
                                    char const* const* command,
                                    char* cmd)
{
  if(cp->Verbatim)
    {
    /* Copy the verbatim command line into the buffer.  */
    strcpy(cmd, *command);
    }
  else
    {
    /* Construct the command line in the allocated buffer.  */
    char const* const* arg;
    for(arg = command; *arg; ++arg)
      {
      /* Add the separating space if this is not the first argument.  */
      if(arg != command)
        {
        *cmd++ = ' ';
        }

      /* Add the current argument.  */
      cmd = kwsysSystem_Windows_ShellArgument(*arg, cmd);
      }

    /* Add the terminating null character to the command line.  */
    *cmd = 0;
    }
}

/*--------------------------------------------------------------------------*/
/* Get the time at which either the process or user timeout will
   expire.  Returns 1 if the user timeout is first, and 0 otherwise.  */
int kwsysProcessGetTimeoutTime(kwsysProcess* cp, double* userTimeout,
                               kwsysProcessTime* timeoutTime)
{
  /* The first time this is called, we need to calculate the time at
     which the child will timeout.  */
  if(cp->Timeout && cp->TimeoutTime.QuadPart < 0)
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
    if(timeoutTime->QuadPart < 0 ||
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
int kwsysProcessGetTimeoutLeft(kwsysProcessTime* timeoutTime,
                               double* userTimeout,
                               kwsysProcessTime* timeoutLength)
{
  if(timeoutTime->QuadPart < 0)
    {
    /* No timeout time has been requested.  */
    return 0;
    }
  else
    {
    /* Calculate the remaining time.  */
    kwsysProcessTime currentTime = kwsysProcessTimeGetCurrent();
    *timeoutLength = kwsysProcessTimeSubtract(*timeoutTime, currentTime);

    if(timeoutLength->QuadPart < 0 && userTimeout && *userTimeout <= 0)
      {
      /* Caller has explicitly requested a zero timeout.  */
      timeoutLength->QuadPart = 0;
      }

    if(timeoutLength->QuadPart < 0)
      {
      /* Timeout has already expired.  */
      return 1;
      }
    else
      {
      /* There is some time left.  */
      return 0;
      }
    }
}

/*--------------------------------------------------------------------------*/
kwsysProcessTime kwsysProcessTimeGetCurrent()
{
  kwsysProcessTime current;
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  current.LowPart = ft.dwLowDateTime;
  current.HighPart = ft.dwHighDateTime;
  return current;
}

/*--------------------------------------------------------------------------*/
DWORD kwsysProcessTimeToDWORD(kwsysProcessTime t)
{
  return (DWORD)(t.QuadPart * 0.0001);
}

/*--------------------------------------------------------------------------*/
double kwsysProcessTimeToDouble(kwsysProcessTime t)
{
  return t.QuadPart * 0.0000001;
}

/*--------------------------------------------------------------------------*/
kwsysProcessTime kwsysProcessTimeFromDouble(double d)
{
  kwsysProcessTime t;
  t.QuadPart = (LONGLONG)(d*10000000);
  return t;
}

/*--------------------------------------------------------------------------*/
int kwsysProcessTimeLess(kwsysProcessTime in1, kwsysProcessTime in2)
{
  return in1.QuadPart < in2.QuadPart;
}

/*--------------------------------------------------------------------------*/
kwsysProcessTime kwsysProcessTimeAdd(kwsysProcessTime in1, kwsysProcessTime in2)
{
  kwsysProcessTime out;
  out.QuadPart = in1.QuadPart + in2.QuadPart;
  return out;
}

/*--------------------------------------------------------------------------*/
kwsysProcessTime kwsysProcessTimeSubtract(kwsysProcessTime in1, kwsysProcessTime in2)
{
  kwsysProcessTime out;
  out.QuadPart = in1.QuadPart - in2.QuadPart;
  return out;
}

/*--------------------------------------------------------------------------*/
#define KWSYSPE_CASE(type, str) \
  cp->ExitException = kwsysProcess_Exception_##type; \
  strcpy(cp->ExitExceptionString, str)
static void kwsysProcessSetExitException(kwsysProcess* cp, int code)
{
  switch (code)
    {
    case STATUS_CONTROL_C_EXIT:
      KWSYSPE_CASE(Interrupt, "User interrupt"); break;

    case STATUS_FLOAT_DENORMAL_OPERAND:
      KWSYSPE_CASE(Numerical, "Floating-point exception (denormal operand)"); break;
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
      KWSYSPE_CASE(Numerical, "Divide-by-zero"); break;
    case STATUS_FLOAT_INEXACT_RESULT:
      KWSYSPE_CASE(Numerical, "Floating-point exception (inexact result)"); break;
    case STATUS_FLOAT_INVALID_OPERATION:
      KWSYSPE_CASE(Numerical, "Invalid floating-point operation"); break;
    case STATUS_FLOAT_OVERFLOW:
      KWSYSPE_CASE(Numerical, "Floating-point overflow"); break;
    case STATUS_FLOAT_STACK_CHECK:
      KWSYSPE_CASE(Numerical, "Floating-point stack check failed"); break;
    case STATUS_FLOAT_UNDERFLOW:
      KWSYSPE_CASE(Numerical, "Floating-point underflow"); break;
#ifdef STATUS_FLOAT_MULTIPLE_FAULTS
    case STATUS_FLOAT_MULTIPLE_FAULTS:
      KWSYSPE_CASE(Numerical, "Floating-point exception (multiple faults)"); break;
#endif
#ifdef STATUS_FLOAT_MULTIPLE_TRAPS
    case STATUS_FLOAT_MULTIPLE_TRAPS:
      KWSYSPE_CASE(Numerical, "Floating-point exception (multiple traps)"); break;
#endif
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
      KWSYSPE_CASE(Numerical, "Integer divide-by-zero"); break;
    case STATUS_INTEGER_OVERFLOW:
      KWSYSPE_CASE(Numerical, "Integer overflow"); break;

    case STATUS_DATATYPE_MISALIGNMENT:
      KWSYSPE_CASE(Fault, "Datatype misalignment"); break;
    case STATUS_ACCESS_VIOLATION:
      KWSYSPE_CASE(Fault, "Access violation"); break;
    case STATUS_IN_PAGE_ERROR:
      KWSYSPE_CASE(Fault, "In-page error"); break;
    case STATUS_INVALID_HANDLE:
      KWSYSPE_CASE(Fault, "Invalid hanlde"); break;
    case STATUS_NONCONTINUABLE_EXCEPTION:
      KWSYSPE_CASE(Fault, "Noncontinuable exception"); break;
    case STATUS_INVALID_DISPOSITION:
      KWSYSPE_CASE(Fault, "Invalid disposition"); break;
    case STATUS_ARRAY_BOUNDS_EXCEEDED:
      KWSYSPE_CASE(Fault, "Array bounds exceeded"); break;
    case STATUS_STACK_OVERFLOW:
      KWSYSPE_CASE(Fault, "Stack overflow"); break;

    case STATUS_ILLEGAL_INSTRUCTION:
      KWSYSPE_CASE(Illegal, "Illegal instruction"); break;
    case STATUS_PRIVILEGED_INSTRUCTION:
      KWSYSPE_CASE(Illegal, "Privileged instruction"); break;

    case STATUS_NO_MEMORY:
    default:
      cp->ExitException = kwsysProcess_Exception_Other;
      sprintf(cp->ExitExceptionString, "Exit code 0x%x\n", code);
      break;
    }
}
#undef KWSYSPE_CASE

typedef struct kwsysProcess_List_s kwsysProcess_List;
static kwsysProcess_List* kwsysProcess_List_New(void);
static void kwsysProcess_List_Delete(kwsysProcess_List* self);
static int kwsysProcess_List_Update(kwsysProcess_List* self);
static int kwsysProcess_List_NextProcess(kwsysProcess_List* self);
static int kwsysProcess_List_GetCurrentProcessId(kwsysProcess_List* self);
static int kwsysProcess_List_GetCurrentParentId(kwsysProcess_List* self);

/*--------------------------------------------------------------------------*/
/* Windows NT 4 API definitions.  */
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
typedef LONG NTSTATUS;
typedef LONG KPRIORITY;
typedef struct _UNICODE_STRING UNICODE_STRING;
struct _UNICODE_STRING
{
  USHORT Length;
  USHORT MaximumLength;
  PWSTR Buffer;
};

/* The process information structure.  Declare only enough to get
   process identifiers.  The rest may be ignored because we use the
   NextEntryDelta to move through an array of instances.  */
typedef struct _SYSTEM_PROCESS_INFORMATION SYSTEM_PROCESS_INFORMATION;
typedef SYSTEM_PROCESS_INFORMATION* PSYSTEM_PROCESS_INFORMATION;
struct _SYSTEM_PROCESS_INFORMATION
{
  ULONG          NextEntryDelta;
  ULONG          ThreadCount;
  ULONG          Reserved1[6];
  LARGE_INTEGER  CreateTime;
  LARGE_INTEGER  UserTime;
  LARGE_INTEGER  KernelTime;
  UNICODE_STRING ProcessName;
  KPRIORITY      BasePriority;
  ULONG          ProcessId;
  ULONG          InheritedFromProcessId;
};

/*--------------------------------------------------------------------------*/
/* Toolhelp32 API definitions.  */
#define TH32CS_SNAPPROCESS  0x00000002
typedef struct tagPROCESSENTRY32 PROCESSENTRY32;
typedef PROCESSENTRY32* LPPROCESSENTRY32;
struct tagPROCESSENTRY32
{
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ProcessID;
  DWORD th32DefaultHeapID;
  DWORD th32ModuleID;
  DWORD cntThreads;
  DWORD th32ParentProcessID;
  LONG  pcPriClassBase;
  DWORD dwFlags;
  char szExeFile[MAX_PATH];
};

/*--------------------------------------------------------------------------*/
/* Windows API function types.  */
typedef HANDLE (WINAPI* CreateToolhelp32SnapshotType)(DWORD, DWORD);
typedef BOOL (WINAPI* Process32FirstType)(HANDLE, LPPROCESSENTRY32);
typedef BOOL (WINAPI* Process32NextType)(HANDLE, LPPROCESSENTRY32);
typedef NTSTATUS (WINAPI* ZwQuerySystemInformationType)(ULONG, PVOID,
                                                        ULONG, PULONG);


/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__New_NT4(kwsysProcess_List* self);
static int kwsysProcess_List__New_Snapshot(kwsysProcess_List* self);
static void kwsysProcess_List__Delete_NT4(kwsysProcess_List* self);
static void kwsysProcess_List__Delete_Snapshot(kwsysProcess_List* self);
static int kwsysProcess_List__Update_NT4(kwsysProcess_List* self);
static int kwsysProcess_List__Update_Snapshot(kwsysProcess_List* self);
static int kwsysProcess_List__Next_NT4(kwsysProcess_List* self);
static int kwsysProcess_List__Next_Snapshot(kwsysProcess_List* self);
static int kwsysProcess_List__GetProcessId_NT4(kwsysProcess_List* self);
static int kwsysProcess_List__GetProcessId_Snapshot(kwsysProcess_List* self);
static int kwsysProcess_List__GetParentId_NT4(kwsysProcess_List* self);
static int kwsysProcess_List__GetParentId_Snapshot(kwsysProcess_List* self);

struct kwsysProcess_List_s
{
  /* Implementation switches at runtime based on version of Windows.  */
  int NT4;

  /* Implementation functions and data for NT 4.  */
  ZwQuerySystemInformationType P_ZwQuerySystemInformation;
  char* Buffer;
  int BufferSize;
  PSYSTEM_PROCESS_INFORMATION CurrentInfo;

  /* Implementation functions and data for other Windows versions.  */
  CreateToolhelp32SnapshotType P_CreateToolhelp32Snapshot;
  Process32FirstType P_Process32First;
  Process32NextType P_Process32Next;
  HANDLE Snapshot;
  PROCESSENTRY32 CurrentEntry;
};

/*--------------------------------------------------------------------------*/
static kwsysProcess_List* kwsysProcess_List_New(void)
{
  OSVERSIONINFO osv;
  kwsysProcess_List* self;

  /* Allocate and initialize the list object.  */
  if(!(self = (kwsysProcess_List*)malloc(sizeof(kwsysProcess_List))))
    {
    return 0;
    }
  memset(self, 0, sizeof(*self));

  /* Select an implementation.  */
  ZeroMemory(&osv, sizeof(osv));
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);
  self->NT4 = (osv.dwPlatformId == VER_PLATFORM_WIN32_NT &&
               osv.dwMajorVersion < 5)? 1:0;

  /* Initialize the selected implementation.  */
  if(!(self->NT4?
       kwsysProcess_List__New_NT4(self) :
       kwsysProcess_List__New_Snapshot(self)))
    {
    kwsysProcess_List_Delete(self);
    return 0;
    }

  /* Update to the current set of processes.  */
  if(!kwsysProcess_List_Update(self))
    {
    kwsysProcess_List_Delete(self);
    return 0;
    }
  return self;
}

/*--------------------------------------------------------------------------*/
static void kwsysProcess_List_Delete(kwsysProcess_List* self)
{
  if(self)
    {
    if(self->NT4)
      {
      kwsysProcess_List__Delete_NT4(self);
      }
    else
      {
      kwsysProcess_List__Delete_Snapshot(self);
      }
    free(self);
    }
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List_Update(kwsysProcess_List* self)
{
  return self? (self->NT4?
                kwsysProcess_List__Update_NT4(self) :
                kwsysProcess_List__Update_Snapshot(self)) : 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List_GetCurrentProcessId(kwsysProcess_List* self)
{
  return self? (self->NT4?
                kwsysProcess_List__GetProcessId_NT4(self) :
                kwsysProcess_List__GetProcessId_Snapshot(self)) : -1;

}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List_GetCurrentParentId(kwsysProcess_List* self)
{
  return self? (self->NT4?
                kwsysProcess_List__GetParentId_NT4(self) :
                kwsysProcess_List__GetParentId_Snapshot(self)) : -1;

}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List_NextProcess(kwsysProcess_List* self)
{
  return (self? (self->NT4?
                 kwsysProcess_List__Next_NT4(self) :
                 kwsysProcess_List__Next_Snapshot(self)) : 0);
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__New_NT4(kwsysProcess_List* self)
{
  /* Get a handle to the NT runtime module that should already be
     loaded in this program.  This does not actually increment the
     reference count to the module so we do not need to close the
     handle.  */
  HANDLE hNT = GetModuleHandle("ntdll.dll");
  if(hNT)
    {
    /* Get pointers to the needed API functions.  */
    self->P_ZwQuerySystemInformation =
      ((ZwQuerySystemInformationType)
       GetProcAddress(hNT, "ZwQuerySystemInformation"));
    }
  if(!self->P_ZwQuerySystemInformation)
    {
    return 0;
    }

  /* Allocate an initial process information buffer.  */
  self->BufferSize = 32768;
  self->Buffer = (char*)malloc(self->BufferSize);
  return self->Buffer? 1:0;
}

/*--------------------------------------------------------------------------*/
static void kwsysProcess_List__Delete_NT4(kwsysProcess_List* self)
{
  /* Free the process information buffer.  */
  if(self->Buffer)
    {
    free(self->Buffer);
    }
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__Update_NT4(kwsysProcess_List* self)
{
  self->CurrentInfo = 0;
  for(;;)
    {
    /* Query number 5 is for system process list.  */
    NTSTATUS status =
      self->P_ZwQuerySystemInformation(5, self->Buffer, self->BufferSize, 0);
    if(status == STATUS_INFO_LENGTH_MISMATCH)
      {
      /* The query requires a bigger buffer.  */
      int newBufferSize = self->BufferSize * 2;
      char* newBuffer = (char*)malloc(newBufferSize);
      if(newBuffer)
        {
        free(self->Buffer);
        self->Buffer = newBuffer;
        self->BufferSize = newBufferSize;
        }
      else
        {
        return 0;
        }
      }
    else if(status >= 0)
      {
      /* The query succeeded.  Initialize traversal of the process list.  */
      self->CurrentInfo = (PSYSTEM_PROCESS_INFORMATION)self->Buffer;
      return 1;
      }
    else
      {
      /* The query failed.  */
      return 0;
      }
    }
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__Next_NT4(kwsysProcess_List* self)
{
  if(self->CurrentInfo)
    {
    if(self->CurrentInfo->NextEntryDelta > 0)
      {
      self->CurrentInfo = ((PSYSTEM_PROCESS_INFORMATION)
                              ((char*)self->CurrentInfo +
                               self->CurrentInfo->NextEntryDelta));
      return 1;
      }
    self->CurrentInfo = 0;
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__GetProcessId_NT4(kwsysProcess_List* self)
{
  return self->CurrentInfo? self->CurrentInfo->ProcessId : -1;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__GetParentId_NT4(kwsysProcess_List* self)
{
  return self->CurrentInfo? self->CurrentInfo->InheritedFromProcessId : -1;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__New_Snapshot(kwsysProcess_List* self)
{
  /* Get a handle to the Windows runtime module that should already be
     loaded in this program.  This does not actually increment the
     reference count to the module so we do not need to close the
     handle.  */
  HANDLE hKernel = GetModuleHandle("kernel32.dll");
  if(hKernel)
    {
    self->P_CreateToolhelp32Snapshot =
      ((CreateToolhelp32SnapshotType)
       GetProcAddress(hKernel, "CreateToolhelp32Snapshot"));
    self->P_Process32First =
      ((Process32FirstType)
       GetProcAddress(hKernel, "Process32First"));
    self->P_Process32Next =
      ((Process32NextType)
       GetProcAddress(hKernel, "Process32Next"));
    }
  return (self->P_CreateToolhelp32Snapshot &&
          self->P_Process32First &&
          self->P_Process32Next)? 1:0;
}

/*--------------------------------------------------------------------------*/
static void kwsysProcess_List__Delete_Snapshot(kwsysProcess_List* self)
{
  if(self->Snapshot)
    {
    CloseHandle(self->Snapshot);
    }
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__Update_Snapshot(kwsysProcess_List* self)
{
  if(self->Snapshot)
    {
    CloseHandle(self->Snapshot);
    }
  if(!(self->Snapshot =
       self->P_CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)))
    {
    return 0;
    }
  ZeroMemory(&self->CurrentEntry, sizeof(self->CurrentEntry));
  self->CurrentEntry.dwSize = sizeof(self->CurrentEntry);
  if(!self->P_Process32First(self->Snapshot, &self->CurrentEntry))
    {
    CloseHandle(self->Snapshot);
    self->Snapshot = 0;
    return 0;
    }
  return 1;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__Next_Snapshot(kwsysProcess_List* self)
{
  if(self->Snapshot)
    {
    if(self->P_Process32Next(self->Snapshot, &self->CurrentEntry))
      {
      return 1;
      }
    CloseHandle(self->Snapshot);
    self->Snapshot = 0;
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__GetProcessId_Snapshot(kwsysProcess_List* self)
{
  return self->Snapshot? self->CurrentEntry.th32ProcessID : -1;
}

/*--------------------------------------------------------------------------*/
static int kwsysProcess_List__GetParentId_Snapshot(kwsysProcess_List* self)
{
  return self->Snapshot? self->CurrentEntry.th32ParentProcessID : -1;
}

/*--------------------------------------------------------------------------*/
static void kwsysProcessKill(DWORD pid)
{
  HANDLE h = OpenProcess(PROCESS_TERMINATE, 0, pid);
  if(h)
    {
    TerminateProcess(h, 255);
    WaitForSingleObject(h, INFINITE);
    }
}

/*--------------------------------------------------------------------------*/
static void kwsysProcessKillTree(int pid)
{
  kwsysProcess_List* plist = kwsysProcess_List_New();
  kwsysProcessKill(pid);
  if(plist)
    {
    do
      {
      if(kwsysProcess_List_GetCurrentParentId(plist) == pid)
        {
        int ppid = kwsysProcess_List_GetCurrentProcessId(plist);
        kwsysProcessKillTree(ppid);
        }
      } while(kwsysProcess_List_NextProcess(plist));
    kwsysProcess_List_Delete(plist);
    }
}

/*--------------------------------------------------------------------------*/
static void kwsysProcessDisablePipeThreads(kwsysProcess* cp)
{
  int i;

  /* If data were just reported data, release the pipe's thread.  */
  if(cp->CurrentIndex < KWSYSPE_PIPE_COUNT)
    {
    ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Reader.Go, 1, 0);
    cp->CurrentIndex = KWSYSPE_PIPE_COUNT;
    }

  /* Wakeup all reading threads that are not on closed pipes.  */
  for(i=0; i < KWSYSPE_PIPE_COUNT; ++i)
    {
    /* The wakeup threads will write one byte to the pipe write ends.
       If there are no data in the pipe then this is enough to wakeup
       the reading threads.  If there are already data in the pipe
       this may block.  We cannot use PeekNamedPipe to check whether
       there are data because an outside process might still be
       writing data if we are disowning it.  Also, PeekNamedPipe will
       block if checking a pipe on which the reading thread is
       currently calling ReadPipe.  Therefore we need a separate
       thread to call WriteFile.  If it blocks, that is okay because
       it will unblock when we close the read end and break the pipe
       below.  */
    if(cp->Pipe[i].Read)
      {
      ReleaseSemaphore(cp->Pipe[i].Waker.Go, 1, 0);
      }
    }

  /* Tell pipe threads to reset until we run another process.  */
  while(cp->PipesLeft > 0)
    {
    /* The waking threads will cause all reading threads to report.
       Wait for the next one and save its index.  */
    WaitForSingleObject(cp->Full, INFINITE);
    cp->CurrentIndex = cp->SharedIndex;
    ReleaseSemaphore(cp->SharedIndexMutex, 1, 0);

    /* We are done reading this pipe.  Close its read handle.  */
    cp->Pipe[cp->CurrentIndex].Closed = 1;
    kwsysProcessCleanupHandle(&cp->Pipe[cp->CurrentIndex].Read);
    --cp->PipesLeft;

    /* Tell the reading thread we are done with the data.  It will
       reset immediately because the pipe is closed.  */
    ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Reader.Go, 1, 0);
    }
}
