/*=========================================================================

Program:   KWSys - Kitware System Library
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
See http://www.cmake.org/HTML/Copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#define KWSYS_IN_PROCESS_C
#include <Process.h>

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
#ifdef _MSC_VER
#pragma warning (pop)
#pragma warning (disable: 4514)
#pragma warning (disable: 4706)
#endif

/* The number of pipes for the child's output.  The standard stdout
   and stderr pipes are the first two.  One more pipe is used on Win9x
   for the forwarding executable to use in reporting problems.  */
#define CMPE_PIPE_COUNT 3
#define CMPE_PIPE_STDOUT 0
#define CMPE_PIPE_STDERR 1
#define CMPE_PIPE_ERROR 2

/* The maximum amount to read from a pipe at a time.  */
#define CMPE_PIPE_BUFFER_SIZE 1024

#define kwsysEncodedWriteArrayProcessFwd9x kwsys(EncodedWriteArrayProcessFwd9x)

typedef LARGE_INTEGER kwsysProcessTime;

/*--------------------------------------------------------------------------*/
typedef struct kwsysProcessPipeData_s kwsysProcessPipeData;
static DWORD WINAPI kwsysProcessPipeThread(LPVOID ptd);
static void kwsysProcessPipeThreadReadPipe(kwsysProcess* cp, kwsysProcessPipeData* td);
static void kwsysProcessCleanupHandle(PHANDLE h);
static void kwsysProcessCleanup(kwsysProcess* cp, int error);
static int kwsysProcessGetTimeoutTime(kwsysProcess* cp, double* userTimeout,
                                      kwsysProcessTime* timeoutTime);
static int kwsysProcessGetTimeoutLeft(kwsysProcessTime* timeoutTime,
                                      kwsysProcessTime* timeoutLength);
static kwsysProcessTime kwsysProcessTimeGetCurrent();
static DWORD kwsysProcessTimeToDWORD(kwsysProcessTime t);
static double kwsysProcessTimeToDouble(kwsysProcessTime t);
static kwsysProcessTime kwsysProcessTimeFromDouble(double d);
static int kwsysProcessTimeLess(kwsysProcessTime in1, kwsysProcessTime in2);
static kwsysProcessTime kwsysProcessTimeAdd(kwsysProcessTime in1, kwsysProcessTime in2);
static kwsysProcessTime kwsysProcessTimeSubtract(kwsysProcessTime in1, kwsysProcessTime in2);
extern kwsysEXPORT int kwsysEncodedWriteArrayProcessFwd9x(const char* fname);

/*--------------------------------------------------------------------------*/
/* A structure containing data for each pipe's thread.  */
struct kwsysProcessPipeData_s
{
  /* ------------- Data managed per instance of kwsysProcess ------------- */
  
  /* Handle for the thread for this pipe.  */
  HANDLE Thread;
    
  /* Semaphore indicating a process and pipe are available.  */
  HANDLE Ready;
    
  /* Semaphore indicating when this thread's buffer is empty.  */
  HANDLE Empty;
    
  /* Semaphore indicating a pipe thread has reset for another process.  */
  HANDLE Reset;
    
  /* Index of this pipe.  */
  int Index;

  /* The kwsysProcess instance owning this pipe.  */
  kwsysProcess* Process;
  
  /* ------------- Data managed per call to Execute ------------- */
  
  /* Buffer for data read in this pipe's thread.  */
  char DataBuffer[CMPE_PIPE_BUFFER_SIZE];
    
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
  
  /* The status of the process.  */
  int State;
  
  /* The command line to execute. */
  char* Command;
  
  /* On Win9x platforms, the path to the forwarding executable.  */
  char* Win9x;
  
  /* On Win9x platforms, the kill event for the forwarding executable.  */
  HANDLE Win9xKillEvent;
  
  /* Mutex to protect the shared index used by threads to report data.  */
  HANDLE SharedIndexMutex;
  
  /* Semaphore used by threads to signal data ready.  */
  HANDLE Full;
  
  /* The number of pipes needed to implement the child's execution.
     This is 3 on Win9x and 2 otherwise.  */
  int PipeCount;
  
  /* Whether we are currently deleting this kwsysProcess instance.  */
  int Deleting;
  
  /* Data specific to each pipe and its thread.  */
  kwsysProcessPipeData Pipe[CMPE_PIPE_COUNT];  
  
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
  char ErrorMessage[CMPE_PIPE_BUFFER_SIZE+1];
  int ErrorMessageLength;
  
  /* The actual command line that will be used to create the process.  */
  char* RealCommand;

  /* Windows process information data.  */
  PROCESS_INFORMATION ProcessInformation;
};

/*--------------------------------------------------------------------------*/
kwsysProcess* kwsysProcess_New()
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
       directory.  */
    DWORD length = GetEnvironmentVariable("TEMP", tempDir, _MAX_PATH);
    
    /* Construct the executable name from the process id and kwsysProcess
       instance.  This should be unique.  */
    sprintf(fwdName, "cmw9xfwd_%u_%p.exe", GetCurrentProcessId(), cp);
    
    /* If the environment variable "TEMP" gave us a directory, use it.  */
    if(length > 0 && length <= _MAX_PATH)
      {
      /* Make sure there is no trailing slash.  */
      size_t tdlen = strlen(tempDir);
      if(tempDir[tdlen-1] == '/' || tempDir[tdlen-1] == '\\')
        {
        tempDir[tdlen-1] = 0;
        --tdlen;
        }
      
      /* Allocate a buffer to hold the forwarding executable path.  */
      win9x = (char*)malloc(tdlen + strlen(fwdName) + 2);
      if(!win9x)
        {
        kwsysProcess_Delete(cp);
        return 0;
        }
      
      /* Construct the full path to the forwarding executable.  */
      sprintf(win9x, "%s/%s", tempDir, fwdName);
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
      }
    else
      {
      /* Failed to find a place to put forwarding executable.  */
      kwsysProcess_Delete(cp);
      return 0;
      }
    }
  
  /* We need the extra error pipe on Win9x.  */
  cp->Win9x = win9x;
  cp->PipeCount = cp->Win9x? 3:2;
  
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
    /* Create an event to tell the forwarding executable to kill the
       child.  */
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    if(!(cp->Win9xKillEvent = CreateEvent(&sa, TRUE, 0, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }
    }
    
  /* Create the thread to read each pipe.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    DWORD dummy=0;
    
    /* Assign the thread its index.  */
    cp->Pipe[i].Index = i;
    
    /* Give the thread a pointer back to the kwsysProcess instance.  */
    cp->Pipe[i].Process = cp;
    
    /* The pipe is not yet ready to read.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Ready = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }
    
    /* The pipe is not yet reset.  Initialize semaphore to 0.  */
    if(!(cp->Pipe[i].Reset = CreateSemaphore(0, 0, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }
    
    /* The thread's buffer is initially empty.  Initialize semaphore to 1.  */
    if(!(cp->Pipe[i].Empty = CreateSemaphore(0, 1, 1, 0)))
      {
      kwsysProcess_Delete(cp);
      return 0;
      }
    
    /* Create the thread.  It will block immediately.  The thread will
       not make deeply nested calls, so we need only a small
       stack.  */
    if(!(cp->Pipe[i].Thread = CreateThread(0, 1024, kwsysProcessPipeThread,
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

  /* If the process is executing, wait for it to finish.  */
  if(cp->State == kwsysProcess_State_Executing)
    {
    kwsysProcess_WaitForExit(cp, 0);
    }
  
  /* We are deleting the kwsysProcess instance.  */
  cp->Deleting = 1;
  
  /* Terminate each of the threads.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    if(cp->Pipe[i].Thread)
      {
      /* Signal the thread we are ready for it.  It will terminate
         immediately since Deleting is set.  */
      ReleaseSemaphore(cp->Pipe[i].Ready, 1, 0);
      
      /* Wait for the thread to exit.  */
      WaitForSingleObject(cp->Pipe[i].Thread, INFINITE);
      
      /* Close the handle to the thread. */
      kwsysProcessCleanupHandle(&cp->Pipe[i].Thread);
      }
    
    /* Cleanup the pipe's semaphores.  */
    kwsysProcessCleanupHandle(&cp->Pipe[i].Ready);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Empty);
    }  
  
  /* Close the shared semaphores.  */
  kwsysProcessCleanupHandle(&cp->SharedIndexMutex);
  kwsysProcessCleanupHandle(&cp->Full);
  
  /* Close the Win9x kill event handle.  */
  if(cp->Win9x)
    {
    kwsysProcessCleanupHandle(&cp->Win9xKillEvent);
    }
  
  /* Free memory.  */
  kwsysProcess_SetCommand(cp, 0);
  if(cp->Win9x)
    {
    _unlink(cp->Win9x);
    free(cp->Win9x);
    }
  free(cp);
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_SetCommand(kwsysProcess* cp, char const* const* command)
{
  if(cp->Command)
    {
    free(cp->Command);
    cp->Command = 0;
    }
  if(command)
    {
    /* We need to construct a single string representing the command
       and its arguments.  We will surround each argument containing
       spaces with double-quotes.  Inside a double-quoted argument, we
       need to escape double-quotes and all backslashes before them.
       We also need to escape backslashes at the end of an argument
       because they come before the closing double-quote for the
       argument.  */
    char* cmd;
    char const* const* arg;
    int length = 0;
    /* First determine the length of the final string.  */
    for(arg = command; *arg; ++arg)
      {
      /* Keep track of how many backslashes have been encountered in a
         row in this argument.  */
      int backslashes = 0;
      int spaces = 0;
      const char* c;

      /* Scan the string for spaces.  If there are no spaces, we can
         pass the argument verbatim.  */
      for(c=*arg; *c; ++c)
        {
        if(*c == ' ' || *c == '\t')
          {
          spaces = 1;
          break;
          }
        }
      
      /* Add the length of the argument, plus 1 for the space
         separating the arguments.  */
      length += (int)strlen(*arg) + 1;
      
      if(spaces)
        {
        /* Add 2 for double quotes since spaces are present.  */
        length += 2;

        /* Scan the string to find characters that need escaping.  */
        for(c=*arg; *c; ++c)
          {
          if(*c == '\\')
            {
            /* Found a backslash.  It may need to be escaped later.  */
            ++backslashes;
            }
          else if(*c == '"')
            {
            /* Found a double-quote.  We need to escape it and all
               immediately preceding backslashes.  */
            length += backslashes + 1;
            backslashes = 0;
            }
          else
            {
            /* Found another character.  This eliminates the possibility
               that any immediately preceding backslashes will be
               escaped.  */
            backslashes = 0;
            }
          }
      
        /* We need to escape all ending backslashes. */
        length += backslashes;
        }
      }
    
    /* Allocate enough space for the command.  We do not need an extra
       byte for the terminating null because we allocated a space for
       the first argument that we will not use.  */
    cp->Command = (char*)malloc(length);
    
    /* Construct the command line in the allocated buffer.  */
    cmd = cp->Command;
    for(arg = command; *arg; ++arg)
      {
      /* Keep track of how many backslashes have been encountered in a
         row in an argument.  */
      int backslashes = 0;
      int spaces = 0;
      const char* c;

      /* Scan the string for spaces.  If there are no spaces, we can
         pass the argument verbatim.  */
      for(c=*arg; *c; ++c)
        {
        if(*c == ' ' || *c == '\t')
          {
          spaces = 1;
          break;
          }
        }      
      
      /* Add the separating space if this is not the first argument.  */
      if(arg != command)
        {
        *cmd++ = ' ';
        }
      
      if(spaces)
        {
        /* Add the opening double-quote for this argument.  */
        *cmd++ = '"';
        
        /* Add the characters of the argument, possibly escaping them.  */
        for(c=*arg; *c; ++c)
          {
          if(*c == '\\')
            {
            /* Found a backslash.  It may need to be escaped later.  */
            ++backslashes;
            *cmd++ = '\\';
            }
          else if(*c == '"')
            {
            /* Add enough backslashes to escape any that preceded the
               double-quote.  */
            while(backslashes > 0)
              {
              --backslashes;
              *cmd++ = '\\';
              }
            
            /* Add the backslash to escape the double-quote.  */
            *cmd++ = '\\';
            
            /* Add the double-quote itself.  */
            *cmd++ = '"';
            }
          else
            {
            /* We encountered a normal character.  This eliminates any
               escaping needed for preceding backslashes.  Add the
               character.  */
            backslashes = 0;
            *cmd++ = *c;
            }
          }
        
        /* Add enough backslashes to escape any trailing ones.  */
        while(backslashes > 0)
          {
          --backslashes;
          *cmd++ = '\\';
          }

        /* Add the closing double-quote for this argument.  */
        *cmd++ = '"';
        }
      else
        {
        /* No spaces.  Add the argument verbatim.  */
        for(c=*arg; *c; ++c)
          {
          *cmd++ = *c;
          }
        }
      }
    
    /* Add the terminating null character to the command line.  */
    *cmd = 0;
    }
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_SetTimeout(kwsysProcess* cp, double timeout)
{
  cp->Timeout = timeout;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetState(kwsysProcess* cp)
{
  return cp->State;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitException(kwsysProcess* cp)
{
  return cp->ExitException;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitValue(kwsysProcess* cp)
{
  return cp->ExitValue;
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_GetExitCode(kwsysProcess* cp)
{
  return cp->ExitCode;
}

/*--------------------------------------------------------------------------*/
const char* kwsysProcess_GetErrorString(kwsysProcess* cp)
{
  if(cp->State == kwsysProcess_State_Error)
    {
    return cp->ErrorMessage;
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Execute(kwsysProcess* cp)
{
  int i=0;

  /* Windows child startup control data.  */
  STARTUPINFO si;
  
  /* Do not execute a second time.  */
  if(cp->State == kwsysProcess_State_Executing)
    {
    return;
    }
  
  /* Initialize startup info data.  */
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  
  /* Reset internal status flags.  */
  cp->TimeoutExpired = 0;
  cp->Terminated = 0;
  cp->Killed = 0;
  cp->ExitException = kwsysProcess_Exception_None;
  cp->ExitCode = 1;
  cp->ExitValue = 1;
  
  /* Reset error data.  */
  cp->ErrorMessage[0] = 0;
  cp->ErrorMessageLength = 0;
  
  /* Reset the Win9x kill event.  */
  if(cp->Win9x)
    {
    if(!ResetEvent(cp->Win9xKillEvent))
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }
    }
  
  /* Create a pipe for each child output.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    HANDLE writeEnd;
    
    /* The pipe is not closed.  */
    cp->Pipe[i].Closed = 0;
    
    /* Create the pipe.  Neither end is directly inherited.  */
    if(!CreatePipe(&cp->Pipe[i].Read, &writeEnd, 0, 0))
      {
      kwsysProcessCleanup(cp, 1);
      return;
      }
    
    /* Create an inherited duplicate of the write end.  This also closes
       the non-inherited version. */
    if(!DuplicateHandle(GetCurrentProcess(), writeEnd,
                        GetCurrentProcess(), &cp->Pipe[i].Write,
                        0, TRUE, (DUPLICATE_CLOSE_SOURCE |
                                  DUPLICATE_SAME_ACCESS)))
      {
      kwsysProcessCleanup(cp, 1);
      CloseHandle(writeEnd);
      return;
      }
    }
  
  /* Construct the real command line.  */
  if(cp->Win9x)
    {
    /* Windows 9x */
    
    /* The forwarding executable is given a handle to the error pipe
       and a handle to the kill event.  */
    cp->RealCommand = malloc(strlen(cp->Win9x)+strlen(cp->Command)+100);
    sprintf(cp->RealCommand, "%s %p %p %s", cp->Win9x,
            cp->Pipe[CMPE_PIPE_ERROR].Write,
            cp->Win9xKillEvent, cp->Command);
    }
  else
    {
    /* Not Windows 9x */    
    cp->RealCommand = strdup(cp->Command);
    }

  /* Connect the child's output pipes to the threads.  */
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = cp->Pipe[CMPE_PIPE_STDOUT].Write;
  si.hStdError = cp->Pipe[CMPE_PIPE_STDERR].Write;
  
  /* Hide the forwarding executable console on Windows 9x.  */
  si.dwFlags |= STARTF_USESHOWWINDOW;  
  if(cp->Win9x)
    {
    si.wShowWindow = SW_HIDE;
    }
  else
    {
    si.wShowWindow = SW_SHOWDEFAULT;
    }
  
  /* The timeout period starts now.  */
  cp->StartTime = kwsysProcessTimeGetCurrent();
  cp->TimeoutTime = kwsysProcessTimeFromDouble(-1);
  
  /* CREATE THE CHILD PROCESS */
  if(!CreateProcess(0, cp->RealCommand, 0, 0, TRUE,
                    cp->Win9x? CREATE_NEW_CONSOLE:DETACHED_PROCESS, 0,
                    0, &si, &cp->ProcessInformation))
    {
    kwsysProcessCleanup(cp, 1);
    return;
    }
  
  /* ---- It is no longer safe to call kwsysProcessCleanup. ----- */
  /* Tell the pipe threads that a process has started.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    ReleaseSemaphore(cp->Pipe[i].Ready, 1, 0);
    }
  
  /* We don't care about the child's main thread.  */
  kwsysProcessCleanupHandle(&cp->ProcessInformation.hThread);
  
  /* No pipe has reported data.  */
  cp->CurrentIndex = CMPE_PIPE_COUNT;
  cp->PipesLeft = cp->PipeCount;
  
  /* The process has now started.  */
  cp->State = kwsysProcess_State_Executing;
}

/*--------------------------------------------------------------------------*/

int kwsysProcess_WaitForData(kwsysProcess* cp, int pipes, char** data, int* length,
                             double* userTimeout)
{
  kwsysProcessTime userStartTime;
  kwsysProcessTime timeoutLength;
  kwsysProcessTime timeoutTime;
  DWORD timeout;
  int user;
  int done = 0;
  int expired = 0;
  int pipeId = 0;
  DWORD w;
  HANDLE events[2];

  /* Make sure we are executing a process.  */
  if(cp->State != kwsysProcess_State_Executing || cp->Killed ||
     cp->TimeoutExpired)
    {
    return 0;
    }
  
  /* We will wait for data until the process termiantes or data are
     available. */
  events[0] = cp->Full;
  events[1] = cp->ProcessInformation.hProcess;
  
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
    if(cp->CurrentIndex < CMPE_PIPE_COUNT)
      {
      ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Empty, 1, 0);
      cp->CurrentIndex = CMPE_PIPE_COUNT;
      }
    
    /* Setup a timeout if required.  */
    if(kwsysProcessGetTimeoutLeft(&timeoutTime, &timeoutLength))
      {
      /* Timeout has already expired.  */
      expired = 1;
      done = 1;
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
    
    /* Wait for a pipe's thread to signal or the application to
       terminate.  */
    w = WaitForMultipleObjects(cp->Terminated?1:2, events, 0, timeout);
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
        /* The pipe closed.  */
        --cp->PipesLeft;
        }
      else if(cp->CurrentIndex == CMPE_PIPE_ERROR)
        {
        /* This is data on the special error reporting pipe for Win9x.
           Append it to the error buffer.  */
        int length = cp->Pipe[cp->CurrentIndex].DataLength;
        if(length > CMPE_PIPE_BUFFER_SIZE - cp->ErrorMessageLength)
          {
          length = CMPE_PIPE_BUFFER_SIZE - cp->ErrorMessageLength;
          }
        if(length > 0)
          {
          memcpy(cp->ErrorMessage+cp->ErrorMessageLength,
                 cp->Pipe[cp->CurrentIndex].DataBuffer, length);
          cp->ErrorMessageLength += length;
          }
        else
          {
          cp->ErrorMessage[cp->ErrorMessageLength] = 0;
          }
        }
      else if(pipes & (1 << cp->CurrentIndex))
        {
        /* Caller wants this data.  Report it.  */
        *data = cp->Pipe[cp->CurrentIndex].DataBuffer;
        *length = cp->Pipe[cp->CurrentIndex].DataLength;
        pipeId = (1 << cp->CurrentIndex);
        done = 1;
        }
      else
        {
        /* Caller does not care about this pipe.  Ignore the data.  */
        }
      }
    else
      {
      int i;

      /* Process has terminated.  */
      cp->Terminated = 1;
      
      /* Close our copies of the pipe write handles so the pipe
         threads can detect end-of-data.  */
      for(i=0; i < cp->PipeCount; ++i)
        {
        kwsysProcessCleanupHandle(&cp->Pipe[i].Write);
        }
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
      return 0;
      }
    }
  else
    {
    /* The process has terminated and no more data are available.  */
    return 0;
    }
}

/*--------------------------------------------------------------------------*/
int kwsysProcess_WaitForExit(kwsysProcess* cp, double* userTimeout)
{
  int i;
  int pipe = 0;
  
  /* Make sure we are executing a process.  */
  if(cp->State != kwsysProcess_State_Executing)
    {
    return 1;
    }
  
  /* Wait for the process to terminate.  Ignore all data.  */
  while((pipe = kwsysProcess_WaitForData(cp, 0, 0, 0, userTimeout)) > 0)
    {
    if(pipe == kwsysProcess_Pipe_Timeout)
      {
      /* The user timeout has expired.  */
      return 0;
      }
    }

  /* When the last pipe closes in WaitForData, the loop terminates
     without releaseing the pipe's thread.  Release it now.  */
  if(cp->CurrentIndex < CMPE_PIPE_COUNT)
    {
    ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Empty, 1, 0);
    cp->CurrentIndex = CMPE_PIPE_COUNT;
    }

  /* Wait for all pipe threads to reset.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    WaitForSingleObject(cp->Pipe[i].Reset, INFINITE);
    }
  
  /* ---- It is now safe again to call kwsysProcessCleanup. ----- */
  /* Close all the pipes.  */
  kwsysProcessCleanup(cp, 0);
  
  /* We are done reading all data.  Wait for the child to terminate.
     This will only block if we killed the child and are waiting for
     it to cleanup.  */
  WaitForSingleObject(cp->ProcessInformation.hProcess, INFINITE);
  
  /* Determine the outcome.  */
  if(cp->Killed)
    {
    /* We killed the child.  */
    cp->State = kwsysProcess_State_Killed;
    }
  else if(cp->ErrorMessageLength)
    {
    /* Failed to run the process.  */
    cp->State = kwsysProcess_State_Error;
    }
  else if(cp->TimeoutExpired)
    {
    /* The timeout expired.  */
    cp->State = kwsysProcess_State_Expired;
    }
  else if(GetExitCodeProcess(cp->ProcessInformation.hProcess,
                             &cp->ExitCode))
    {
    /* The child exited.  */
    if(cp->ExitCode & 0xC0000000)
      {
      /* Child terminated due to exceptional behavior.  */
      cp->State = kwsysProcess_State_Exception;
      switch (cp->ExitCode)
        {
        case CONTROL_C_EXIT:          
          cp->ExitException = kwsysProcess_Exception_Interrupt; break;

        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_FLT_UNDERFLOW:
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
        case EXCEPTION_INT_OVERFLOW:
          cp->ExitException = kwsysProcess_Exception_Numerical; break;

        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        case EXCEPTION_DATATYPE_MISALIGNMENT:
        case EXCEPTION_INVALID_DISPOSITION:
        case EXCEPTION_IN_PAGE_ERROR:
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        case EXCEPTION_STACK_OVERFLOW:
          cp->ExitException = kwsysProcess_Exception_Fault; break;

        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_PRIV_INSTRUCTION:
          cp->ExitException = kwsysProcess_Exception_Illegal; break;

        default:
          cp->ExitException = kwsysProcess_Exception_Other; break;
        }
      cp->ExitValue = 1;
      }
    else
      {
      /* Child exited normally.  */
      cp->State = kwsysProcess_State_Exited;
      cp->ExitException = kwsysProcess_Exception_None;
      cp->ExitValue = cp->ExitCode & 0x000000FF;
      }
    }
  else
    {
    /* Error getting the child return code.  */
    strcpy(cp->ErrorMessage, "Error getting child return code.");
    cp->State = kwsysProcess_State_Error;
    }
  
  /* The child process is terminated.  */
  CloseHandle(cp->ProcessInformation.hProcess);  
  
  return 1;
}

/*--------------------------------------------------------------------------*/
void kwsysProcess_Kill(kwsysProcess* cp)
{
  int i;
  
  /* Make sure we are executing a process.  */
  if(cp->State != kwsysProcess_State_Executing || cp->TimeoutExpired ||
     cp->Killed || cp->Terminated)
    {
    return;
    }
  
  /* If we are killing a process that just reported data, release
     the pipe's thread.  */
  if(cp->CurrentIndex < CMPE_PIPE_COUNT)
    {
    ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Empty, 1, 0);
    cp->CurrentIndex = CMPE_PIPE_COUNT;
    }
  
  /* Wake up all the pipe threads with dummy data.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    DWORD dummy;
    WriteFile(cp->Pipe[i].Write, "", 1, &dummy, 0);
    }
  
  /* Tell pipe threads to reset until we run another process.  */
  while(cp->PipesLeft > 0)
    {
    WaitForSingleObject(cp->Full, INFINITE);
    cp->CurrentIndex = cp->SharedIndex;
    ReleaseSemaphore(cp->SharedIndexMutex, 1, 0);
    cp->Pipe[cp->CurrentIndex].Closed = 1;
    ReleaseSemaphore(cp->Pipe[cp->CurrentIndex].Empty, 1, 0);
    --cp->PipesLeft;
    }
  
  /* Kill the child.  */
  cp->Killed = 1;
  if(cp->Win9x)
    {
    /* Windows 9x.  Tell the forwarding executable to kill the child.  */
    SetEvent(cp->Win9xKillEvent);
    }
  else
    {
    /* Not Windows 9x.  Just terminate the child.  */
    TerminateProcess(cp->ProcessInformation.hProcess, 255);
    }
}

/*--------------------------------------------------------------------------*/

/*
  Function executed for each pipe's thread.  Argument is a pointer to
  the kwsysProcessPipeData instance for this thread.
*/
DWORD WINAPI kwsysProcessPipeThread(LPVOID ptd)
{
  kwsysProcessPipeData* td = (kwsysProcessPipeData*)ptd;
  kwsysProcess* cp = td->Process;
  
  /* Wait for a process to be ready.  */
  while((WaitForSingleObject(td->Ready, INFINITE), !cp->Deleting))
    {
    /* Read output from the process for this thread's pipe.  */
    kwsysProcessPipeThreadReadPipe(cp, td);
    
    /* We were signalled to exit with our buffer empty.  Reset the
       mutex for a new process.  */
    ReleaseSemaphore(td->Empty, 1, 0);
    
    /* Signal the main thread we have reset for a new process.  */
    ReleaseSemaphore(td->Reset, 1, 0);
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
  while((WaitForSingleObject(td->Empty, INFINITE), !td->Closed))
    {
    /* Read data from the pipe.  This may block until data are available.  */
    if(!ReadFile(td->Read, td->DataBuffer, CMPE_PIPE_BUFFER_SIZE,
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

/* Close all handles created by kwsysProcess_Execute.  */
void kwsysProcessCleanup(kwsysProcess* cp, int error)
{
  int i;
  
  /* If this is an error case, report the error.  */
  if(error)
    {
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  cp->ErrorMessage, CMPE_PIPE_BUFFER_SIZE, 0); 
    cp->State = kwsysProcess_State_Error;
    }
  
  /* Free memory.  */
  if(cp->RealCommand)
    {
    free(cp->RealCommand);
    cp->RealCommand = 0;
    }

  /* Close each pipe.  */
  for(i=0; i < cp->PipeCount; ++i)
    {
    kwsysProcessCleanupHandle(&cp->Pipe[i].Write);
    kwsysProcessCleanupHandle(&cp->Pipe[i].Read);
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
    if(kwsysProcessTimeLess(userTimeoutTime, *timeoutTime))
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
