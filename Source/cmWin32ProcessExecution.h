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
 */

#include "cmStandardIncludes.h"
#include "windows.h"
#include "stdio.h"

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
  void Initialize() 
    {
    this->m_ProcessHandle = 0;
    this->m_ExitValue    = -1;
    this->m_StdIn  =  0;
    this->m_StdOut =  0;
    this->m_StdErr =  0;
    }
  bool StartProcess(const char*, const char* path, bool verbose);
  bool Wait(int timeout);
  const std::string GetOutput() const { return this->m_Output; }
  int GetExitValue() const { return this->m_ExitValue; }

  void SetConsoleSpawn(const char* prog) { this->m_ConsoleSpawn = prog; }

  static int Windows9xHack(const char* command);

private:
  bool PrivateOpen(const char*, const char*, int, int);
  bool PrivateClose(int timeout);

  HANDLE m_ProcessHandle;
  FILE* m_StdIn;
  FILE* m_StdOut;
  FILE* m_StdErr;
  
  int m_pStdIn;
  int m_pStdOut;
  int m_pStdErr;

  int m_ExitValue;

  std::string m_Output;
  std::string m_ConsoleSpawn;
  bool m_Verbose;
};


#endif
