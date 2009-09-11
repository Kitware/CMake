/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmProcess_h
#define cmProcess_h


#include "cmStandardIncludes.h"
#include <cmsys/Process.h>


/** \class cmProcess
 * \brief run a process with c++
 *
 * cmProcess wraps the kwsys process stuff in a c++ class.
 */
class cmProcess 
{
public:
  cmProcess();
  ~cmProcess();
  const char* GetCommand() { return this->Command.c_str();}
  void SetCommand(const char* command);
  void SetCommandArguments(std::vector<std::string> const& arg);
  void SetWorkingDirectory(const char* dir) { this->WorkingDirectory = dir;}
  void SetTimeout(double t) { this->Timeout = t;}
  // Return true if the process starts
  bool StartProcess();

  // return the process status
  int GetProcessStatus();
  // Report the status of the program 
  int ReportStatus();
  int GetId() { return this->Id; }
  void SetId(int id) { this->Id = id;}
  int GetExitValue() { return this->ExitValue;}
  double GetTotalTime() { return this->TotalTime;}

  /**
   * Read one line of output but block for no more than timeout.
   * Returns:
   *   cmsysProcess_Pipe_None    = Process terminated and all output read
   *   cmsysProcess_Pipe_STDOUT  = Line came from stdout
   *   cmsysProcess_Pipe_STDOUT  = Line came from stderr
   *   cmsysProcess_Pipe_Timeout = Timeout expired while waiting
   */
  int GetNextOutputLine(std::string& line, double timeout);
private:
  double Timeout;
  double StartTime;
  double TotalTime;
  cmsysProcess* Process;
  class Buffer: public std::vector<char>
  {
    // Half-open index range of partial line already scanned.
    size_type First;
    size_type Last;
  public:
    Buffer(): First(0), Last(0) {}
    bool GetLine(std::string& line);
    bool GetLast(std::string& line);
  };
  Buffer StdErr;
  Buffer StdOut;
  std::string Command;
  std::string WorkingDirectory;
  std::vector<std::string> Arguments;
  std::vector<const char*> ProcessArgs;
  std::string Output;
  int Id;
  int ExitValue;
};

#endif
