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

/*
  On Windows9x platforms, this executable is spawned between a parent
  process and the child it is invoking to work around a bug.  See the
  Win32 implementation file for details.
*/

#ifdef _MSC_VER
#pragma warning (push, 1)
#endif
#include <windows.h>
#include <stdio.h>

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

  /* Handle to the event the parent uses to tell us to kill the child.
     This is parsed off the command line.  */
  HANDLE killEvent = 0;
  
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
  sscanf(cmdLine, "%d", &errorPipe);

  /* Parse the kill event handle.  */
  while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }
  sscanf(cmdLine, "%d", &killEvent);
  
  /* Skip to the beginning of the command line of the real child.  */
  while(*cmdLine && *cmdLine != ' ') { ++cmdLine; }
  while(*cmdLine && *cmdLine == ' ') { ++cmdLine; }

  /* Create the subprocess.  */
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_SHOWDEFAULT;
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  if(!CreateProcess(0, cmdLine, 0, 0, TRUE, 0, 0, 0, &si, &pi))
    {
    /* Error creating the process.  Report the error to the parent
       process through the special error reporting pipe.  */
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
    return 1;
    }
  CloseHandle(pi.hThread);  

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
  else if(GetExitCodeProcess(pi.hProcess, &retVal))
    {
    /* The child exited and we could get the return code.  */
    CloseHandle(pi.hProcess);
    return retVal;
    }
  else
    {
    /* The child exited and we could not get the return code.  Report
       the problem to the parent process.  */
    DWORD n;
    const char* msg = "Failed to get process return code.";
    WriteFile(errorPipe, msg, strlen(msg)+1, &n, 0);
    CloseHandle(pi.hProcess);
    return -1;
    }
}
