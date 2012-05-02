/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

/*
  On Windows9x platforms, this executable is spawned between a parent
  process and the child it is invoking to work around a bug.  See the
  Win32 implementation file for details.

  Future Work: This executable must be linked statically against the C
  runtime library before being encoded into the library.  Building it
  in this way may be hard because CMake has limited abilities to build
  different targets with different configurations in the same
  directory.  We may just have to create and encode the executable
  once instead of generating it during the build.  This would be an
  acceptable solution because the forwarding executable should not
  change very often and is pretty simple.
*/

#ifdef _MSC_VER
#pragma warning (push, 1)
#endif
#include <windows.h>
#include <stdio.h>

void ReportLastError(HANDLE errorPipe);

int main()
{
  /* Process startup information for the real child.  */
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  /* The result of waiting for the child to exit.  */
  DWORD waitResult;

  /* The child's process return code.  */
  DWORD retVal;

  /* The command line used to invoke this process.  */
  LPSTR commandLine = GetCommandLine();

  /* Pointer that will be advanced to the beginning of the command
     line of the real child process.  */
  LPSTR cmdLine = commandLine;

  /* Handle to the error reporting pipe provided by the parent.  This
     is parsed off the command line.  */
  HANDLE errorPipe = 0;
  HANDLE errorPipeOrig = 0;

  /* Handle to the event the parent uses to tell us to resume the child.
     This is parsed off the command line.  */
  HANDLE resumeEvent = 0;

  /* Handle to the event the parent uses to tell us to kill the child.
     This is parsed off the command line.  */
  HANDLE killEvent = 0;

  /* Flag for whether to hide window of child process.  */
  int hideWindow = 0;

  /* An array of the handles on which we wait when the child is
     running.  */
  HANDLE waitHandles[2] = {0, 0};

  /* Move the pointer past the name of this executable.  */
  if(*cmdLine == '"')
    {
    ++cmdLine;
    while(*cmdLine && *cmdLine != '"') { ++cmdLine; }
    if(*cmdLine) { ++cmdLine; }
    }
  else
    {
    while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
    }

  /* Parse the error pipe handle.  */
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }
  sscanf(cmdLine, "%p", &errorPipeOrig);

  /* Parse the resume event handle.  */
  while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }
  sscanf(cmdLine, "%p", &resumeEvent);

  /* Parse the kill event handle.  */
  while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }
  sscanf(cmdLine, "%p", &killEvent);

  /* Parse the hide window flag.  */
  while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }
  sscanf(cmdLine, "%d", &hideWindow);

  /* Skip to the beginning of the command line of the real child.  */
  while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }

  /* Create a non-inherited copy of the error pipe.  We do not want
     the child to get it.  */
  if(DuplicateHandle(GetCurrentProcess(), errorPipeOrig,
                     GetCurrentProcess(), &errorPipe,
                     0, FALSE, DUPLICATE_SAME_ACCESS))
    {
    /* Have a non-inherited duplicate.  Close the inherited one.  */
    CloseHandle(errorPipeOrig);
    }
  else
    {
    /* Could not duplicate handle.  Report the error.  */
    ReportLastError(errorPipeOrig);
    return 1;
    }

  /* Create the subprocess.  */
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  si.wShowWindow = hideWindow?SW_HIDE:SW_SHOWDEFAULT;
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  if(CreateProcess(0, cmdLine, 0, 0, TRUE, CREATE_SUSPENDED, 0, 0, &si, &pi))
    {
    /* Process created successfully.  Close the error reporting pipe
       to notify the parent of success.  */
    CloseHandle(errorPipe);
    }
  else
    {
    /* Error creating the process.  Report the error to the parent
       process through the special error reporting pipe.  */
    ReportLastError(errorPipe);
    return 1;
    }

  /* Wait for resume or kill event from parent.  */
  waitHandles[0] = killEvent;
  waitHandles[1] = resumeEvent;
  waitResult = WaitForMultipleObjects(2, waitHandles, 0, INFINITE);

  /* Check what happened.  */
  if(waitResult == WAIT_OBJECT_0)
    {
    /* We were asked to kill the child.  */
    TerminateProcess(pi.hProcess, 255);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
    }
  else
    {
    /* We were asked to resume the child.  */
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    }

  /* Wait for subprocess to exit or for kill event from parent.  */
  waitHandles[0] = killEvent;
  waitHandles[1] = pi.hProcess;
  waitResult = WaitForMultipleObjects(2, waitHandles, 0, INFINITE);

  /* Check what happened.  */
  if(waitResult == WAIT_OBJECT_0)
    {
    /* We were asked to kill the child.  */
    TerminateProcess(pi.hProcess, 255);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    return 1;
    }
  else
    {
    /* The child exited.  Get the return code.  */
    GetExitCodeProcess(pi.hProcess, &retVal);
    CloseHandle(pi.hProcess);
    return retVal;
    }
}

void ReportLastError(HANDLE errorPipe)
{
  LPVOID lpMsgBuf;
  DWORD n;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
    );
  WriteFile(errorPipe, lpMsgBuf, strlen(lpMsgBuf)+1, &n, 0);
  LocalFree( lpMsgBuf );
}
