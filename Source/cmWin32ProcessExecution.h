/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmWin32ProcessExecution_h
#define cmWin32ProcessExecution_h

#include "cmStandardIncludes.h"
#include "windows.h"

class cmMakefile;

/** \class cmWin32ProcessExecution
 * \brief A process executor for windows
 *
 * cmWin32ProcessExecution is a class that provides a "clean" way of
 * executing processes on Windows. It is modified code from Python 2.1
 * distribution.
 *
 * Portable 'popen' replacement for Win32.
 *
 * Written by Bill Tutt <billtut@microsoft.com>.  Minor tweaks and 2.0
 * integration by Fredrik Lundh <fredrik@pythonware.com> Return code
 * handling by David Bolen <db3l@fitlinxx.com>.
 *
 * Modified for CMake.
 *
 * For more information, please check Microsoft Knowledge Base
 * Articles Q190351 and Q150956.
 */
class cmWin32ProcessExecution
{
public:
  cmWin32ProcessExecution()
    {
    this->HideWindows = false;
    this->SetConsoleSpawn("w9xpopen.exe");
    this->Initialize();
    }
  ~cmWin32ProcessExecution();
  ///! If true windows will be created hidden.
  void SetHideWindows(bool v) { this->HideWindows = v;  }
  
  /**
   * Initialize the process execution datastructure. Do not call while
   * running the process.
   */
  void Initialize() 
    {
    this->ProcessHandle = 0;
    this->ExitValue    = -1;
    // Comment this out. Maybe we will need it in the future.
    // file IO access to the process might be cool.
    //this->StdIn  =  0;
    //this->StdOut =  0;
    //this->StdErr =  0;
    this->pStdIn  =  -1;
    this->pStdOut =  -1;
    this->pStdErr =  -1;
    }
  
  /**
   * Start the process in the directory path. Make sure that the
   * executable is either in the path or specify the full path. The
   * argument verbose specifies wether or not to display output while
   * it is being generated.
   */
  bool StartProcess(const char*, const char* path, bool verbose);

  /**
   * Wait for the process to finish. If timeout is specified, it will
   * break the process after timeout expires. (Timeout code is not yet
   * implemented.
   */
  bool Wait(int timeout);

  /**
   * Get the output of the process (mixed stdout and stderr) as
   * std::string.
   */
  const std::string GetOutput() const { return this->Output; }

  /**
   * Get the return value of the process. If the process is still
   * running, the return value is -1.
   */
  int GetExitValue() const { return this->ExitValue; }

  /**
   * On Windows 9x there is a bug in the process execution code which
   * may result in blocking. That is why this workaround is
   * used. Specify the console spawn, which should run the
   * Windows9xHack code.
   */
  void SetConsoleSpawn(const char* prog) { this->ConsoleSpawn = prog; }
  static int Windows9xHack(const char* command);

  /** Code from a Borland web site with the following explaination :
   * In this article, I will explain how to spawn a console
   * application and redirect its standard input/output using
   * anonymous pipes. An anonymous pipe is a pipe that goes only in
   * one direction (read pipe, write pipe, etc.). Maybe you are
   * asking, "why would I ever need to do this sort of thing?" One
   * example would be a Windows telnet server, where you spawn a shell
   * and listen on a port and send and receive data between the shell
   * and the socket client. (Windows does not really have a built-in
   * remote shell). First, we should talk about pipes. A pipe in
   * Windows is simply a method of communication, often between
   * process. The SDK defines a pipe as "a communication conduit with
   * two ends; a process with a handle to one end can communicate with
   * a process having a handle to the other end." In our case, we are
   * using "anonymous" pipes, one-way pipes that "transfer data
   * between a parent process and a child process or between two child
   * processes of the same parent process." It's easiest to imagine a
   * pipe as its namesake. An actual pipe running between processes
   * that can carry data. We are using anonymous pipes because the
   * console app we are spawning is a child process. We use the
   * CreatePipe function which will create an anonymous pipe and
   * return a read handle and a write handle. We will create two
   * pipes, on for stdin and one for stdout. We will then monitor the
   * read end of the stdout pipe to check for display on our child
   * process. Every time there is something availabe for reading, we
   * will display it in our app. Consequently, we check for input in
   * our app and send it off to the write end of the stdin pipe.
   */ 
  static bool BorlandRunCommand(const char* command, 
                                const char* dir, 
                                std::string& output, int& retVal, 
                                bool verbose,
                                int timeout, bool hideWindows);

private:
  bool CloseHandles();
  bool PrivateOpen(const char*, const char*, int, int);
  bool PrivateClose(int timeout);

  HANDLE ProcessHandle;
  HANDLE hChildStdinRd;
  HANDLE hChildStdinWr;
  HANDLE hChildStdoutRd;
  HANDLE hChildStdoutWr;
  HANDLE hChildStderrRd;
  HANDLE hChildStderrWr;
  HANDLE hChildStdinWrDup;
  HANDLE hChildStdoutRdDup;
  HANDLE hChildStderrRdDup;
  
  
  int pStdIn;
  int pStdOut;
  int pStdErr;

  int ExitValue;

  std::string Output;
  std::string ConsoleSpawn;
  bool Verbose;
  bool HideWindows;
};


#endif
