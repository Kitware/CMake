/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCTestVC.h"

#include "cmCTest.h"

#include <cmsys/Process.h>

//----------------------------------------------------------------------------
cmCTestVC::cmCTestVC(cmCTest* ct, std::ostream& log): CTest(ct), Log(log)
{
}

//----------------------------------------------------------------------------
cmCTestVC::~cmCTestVC()
{
}

//----------------------------------------------------------------------------
void cmCTestVC::SetCommandLineTool(std::string const& tool)
{
  this->CommandLineTool = tool;
}

//----------------------------------------------------------------------------
void cmCTestVC::SetSourceDirectory(std::string const& dir)
{
  this->SourceDirectory = dir;
}

//----------------------------------------------------------------------------
bool cmCTestVC::RunChild(char const* const* cmd, OutputParser* out,
                         OutputParser* err, const char* workDir)
{
  this->Log << this->ComputeCommandLine(cmd) << "\n";

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, cmd);
  workDir = workDir? workDir : this->SourceDirectory.c_str();
  cmsysProcess_SetWorkingDirectory(cp, workDir);
  this->RunProcess(cp, out, err);
  int result = cmsysProcess_GetExitValue(cp);
  cmsysProcess_Delete(cp);
  return result == 0;
}

//----------------------------------------------------------------------------
std::string cmCTestVC::ComputeCommandLine(char const* const* cmd)
{
  cmOStringStream line;
  const char* sep = "";
  for(const char* const* arg = cmd; *arg; ++arg)
    {
    line << sep << "\"" << *arg << "\"";
    sep = " ";
    }
  return line.str();
}

//----------------------------------------------------------------------------
std::string cmCTestVC::GetNightlyTime()
{
  // Get the nightly start time corresponding to the current dau.
  struct tm* t = this->CTest->GetNightlyTime(
    this->CTest->GetCTestConfiguration("NightlyStartTime"),
    this->CTest->GetTomorrowTag());
  char current_time[1024];
  sprintf(current_time, "%04d-%02d-%02d %02d:%02d:%02d",
          t->tm_year + 1900,
          t->tm_mon + 1,
          t->tm_mday,
          t->tm_hour,
          t->tm_min,
          t->tm_sec);
  return std::string(current_time);
}

//----------------------------------------------------------------------------
void cmCTestVC::Cleanup()
{
  this->Log << "--- Begin Cleanup ---\n";
  this->CleanupImpl();
  this->Log << "--- End Cleanup ---\n";
}

//----------------------------------------------------------------------------
void cmCTestVC::CleanupImpl()
{
  // We do no cleanup by default.
}

//----------------------------------------------------------------------------
void cmCTestVC::NoteOldRevision()
{
  // We do nothing by default.
}

//----------------------------------------------------------------------------
void cmCTestVC::NoteNewRevision()
{
  // We do nothing by default.
}
