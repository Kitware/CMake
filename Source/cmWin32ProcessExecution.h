/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmWin32ProcessExecution_h
#define cmWin32ProcessExecution_h

/*
 * Portable 'popen' replacement for Win32.
 *
 * Written by Bill Tutt <billtut@microsoft.com>.  Minor tweaks
 * and 2.0 integration by Fredrik Lundh <fredrik@pythonware.com>
 * Return code handling by David Bolen <db3l@fitlinxx.com>.
 *
 * Modified for CMake.
 *
 * For more information, please check Microsoft Knowledge Base
 * Articles Q190351 and Q150956.
 */

#include "cmStandardIncludes.h"
#include "windows.h"

class cmMakefile;

/** \class cmWin32ProcessExecution
 * \brief A process executor for windows
 *
 * cmWin32ProcessExecution is a class that provides a "clean" way of
 * executing processes on Windows.
 */
class cmWin32ProcessExecution
{
public:
  cmWin32ProcessExecution()
    {
    this->SetConsoleSpawn("w9xpopen.exe");
    this->Initialize();
    }

  /**
   * Initialize the process execution datastructure. Do not call while
   * running the process.
   */
  void Initialize() 
    {
    this->m_ProcessHandle = 0;
    this->m_ExitValue    = -1;
    // Comment this out. Maybe we will need it in the future.
    // file IO access to the process might be cool.
    //this->m_StdIn  =  0;
    //this->m_StdOut =  0;
    //this->m_StdErr =  0;
    this->m_pStdIn  =  -1;
    this->m_pStdOut =  -1;
    this->m_pStdErr =  -1;
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
  const std::string GetOutput() const { return this->m_Output; }

  /**
   * Get the return value of the process. If the process is still
   * running, the return value is -1.
   */
  int GetExitValue() const { return this->m_ExitValue; }

  /**
   * On Windows 9x there is a bug in the process execution code which
   * may result in blocking. That is why this workaround is
   * used. Specify the console spawn, which should run the
   * Windows9xHack code.
   */
  void SetConsoleSpawn(const char* prog) { this->m_ConsoleSpawn = prog; }
  static int Windows9xHack(const char* command);

private:
  bool PrivateOpen(const char*, const char*, int, int);
  bool PrivateClose(int timeout);

  HANDLE m_ProcessHandle;

  // Comment this out. Maybe we will need it in the future.
  // file IO access to the process might be cool.
  // FILE* m_StdIn;
  // FILE* m_StdOut;
  // FILE* m_StdErr;

  int m_pStdIn;
  int m_pStdOut;
  int m_pStdErr;

  int m_ExitValue;

  std::string m_Output;
  std::string m_ConsoleSpawn;
  bool m_Verbose;
};


#endif
