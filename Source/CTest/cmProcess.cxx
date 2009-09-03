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

#include <cmProcess.h>
#include <cmSystemTools.h>

cmProcess::cmProcess()
{
  this->Process = 0;
  this->Timeout = 0;
}

cmProcess::~cmProcess()
{
  cmsysProcess_Delete(this->Process);
}
void cmProcess::SetCommand(const char* command)
{
  this->Command = command;
}

void cmProcess::SetCommandArguments(std::vector<std::string> const& args)
{
  this->Arguments = args;
}

bool cmProcess::StartProcess()
{
  if(this->Command.size() == 0)
    {
    return false;
    }
  this->StartTime = cmSystemTools::GetTime();
  this->ProcessArgs.clear();
  // put the command as arg0
  this->ProcessArgs.push_back(this->Command.c_str());
  // now put the command arguments in
  for(std::vector<std::string>::iterator i = this->Arguments.begin();
      i != this->Arguments.end(); ++i)
    {
    this->ProcessArgs.push_back(i->c_str());
    }
  this->ProcessArgs.push_back(0); // null terminate the list
  this->Process = cmsysProcess_New();
  cmsysProcess_SetCommand(this->Process, &*this->ProcessArgs.begin());
  if(this->WorkingDirectory.size())
    {
    cmsysProcess_SetWorkingDirectory(this->Process,
                                     this->WorkingDirectory.c_str());
    }
  cmsysProcess_SetOption(this->Process, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(this->Process, this->Timeout);
  cmsysProcess_Execute(this->Process);
  return (cmsysProcess_GetState(this->Process)
          == cmsysProcess_State_Executing);
}

bool cmProcess::GetNextOutputLine(std::string& stdOutLine,
                            std::string& stdErrLine)
{
  if(this->StdErrorBuffer.empty() && this->StdOutBuffer.empty())
    {
    return false;
    }
  stdOutLine = "";
  stdErrLine = "";
  std::vector<char>::iterator outiter = 
    this->StdOutBuffer.begin();
  std::vector<char>::iterator erriter = 
    this->StdErrorBuffer.begin();
  // Check for a newline in stdout.
  for(;outiter != this->StdOutBuffer.end(); ++outiter)
    {
    if((*outiter == '\r') && ((outiter+1) == this->StdOutBuffer.end()))
      {
      break;
      }
    else if(*outiter == '\n' || *outiter == '\0')
      {
      int length = outiter-this->StdOutBuffer.begin();
      if(length > 1 && *(outiter-1) == '\r')
        {
        --length;
        }
      if(length > 0)
        {
        stdOutLine.append(&this->StdOutBuffer[0], length);
        }
      this->StdOutBuffer.erase(this->StdOutBuffer.begin(), outiter+1);
      this->LastOutputPipe = cmsysProcess_Pipe_STDOUT;
      return true;
      }
    }

  // Check for a newline in stderr.
  for(;erriter != this->StdErrorBuffer.end(); ++erriter)
    {
    if((*erriter == '\r') && ((erriter+1) == this->StdErrorBuffer.end()))
      {
      break;
      }
    else if(*erriter == '\n' || *erriter == '\0')
      {
      int length = erriter-this->StdErrorBuffer.begin();
      if(length > 1 && *(erriter-1) == '\r')
        {
        --length;
        }
      if(length > 0)
        {
        stdErrLine.append(&this->StdErrorBuffer[0], length);
        }
      this->StdErrorBuffer.erase(this->StdErrorBuffer.begin(), erriter+1);
      this->LastOutputPipe = cmsysProcess_Pipe_STDERR;
      return true;
      }
    }
  //If we get here, we have stuff waiting in the buffers, but no newline
  return false;
}
// return true if there is a new line of data
// return false if there is no new data
int cmProcess::CheckOutput(double timeout)
{
  // Wait for data from the process.
  int length;
  char* data;

  while(1)
    {
    int pipe = cmsysProcess_WaitForData(this->Process, &data, 
                                        &length, &timeout);
    if(pipe == cmsysProcess_Pipe_Timeout)
      {
      // Timeout has been exceeded.
      this->LastOutputPipe = pipe;
      return pipe;
      }
    else if(pipe == cmsysProcess_Pipe_STDOUT)
      {
        // Append to the stdout buffer.
      this->StdOutBuffer.insert(this->StdOutBuffer.end(), data, data+length);
      }
    else if(pipe == cmsysProcess_Pipe_STDERR)
      {
      // Append to the stderr buffer.
      this->StdErrorBuffer.insert(this->StdErrorBuffer.end(),
                                  data, data+length);
      }
    else if(pipe == cmsysProcess_Pipe_None)
      {
      // Both stdout and stderr pipes have broken.  Return leftover data.
      if(!this->StdOutBuffer.empty())
        {
        this->LastOutputPipe = cmsysProcess_Pipe_STDOUT;
        return this->LastOutputPipe;
        }
      else if(!this->StdErrorBuffer.empty())
        {
        this->LastOutputPipe = cmsysProcess_Pipe_STDERR;
        return this->LastOutputPipe;
        }
      else
        {
        this->LastOutputPipe = cmsysProcess_Pipe_None;
        return this->LastOutputPipe;
        }
      }
    }
}

// return the process status
int cmProcess::GetProcessStatus()
{
  if(!this->Process)
    {
    return cmsysProcess_State_Exited;
    }
  return cmsysProcess_GetState(this->Process);
}

// return true if the process is running
bool cmProcess::IsRunning()
{
  int status = this->GetProcessStatus();
  if(status == cmsysProcess_State_Executing )
    {
    if(this->LastOutputPipe != 0)
      {
      return true;
      }
    }
  // if the process is done, then wait for it to exit
  cmsysProcess_WaitForExit(this->Process, 0);
  this->ExitValue = cmsysProcess_GetExitValue(this->Process);
  this->TotalTime = cmSystemTools::GetTime() - this->StartTime;
//  std::cerr << "Time to run: " << this->TotalTime << "\n";
  return false;
}


int cmProcess::ReportStatus()
{
  int result = 1;
  switch(cmsysProcess_GetState(this->Process))
    {
    case cmsysProcess_State_Starting:
      {
      std::cerr << "cmProcess: Never started " 
           << this->Command << " process.\n";
      } break;
    case cmsysProcess_State_Error:
      {
      std::cerr << "cmProcess: Error executing " << this->Command 
                << " process: "
                << cmsysProcess_GetErrorString(this->Process)
                << "\n";
      } break;
    case cmsysProcess_State_Exception:
      {
      std::cerr << "cmProcess: " << this->Command
                      << " process exited with an exception: ";
      switch(cmsysProcess_GetExitException(this->Process))
        {
        case cmsysProcess_Exception_None:
          {
          std::cerr << "None";
          } break;
        case cmsysProcess_Exception_Fault:
          {
          std::cerr << "Segmentation fault";
          } break;
        case cmsysProcess_Exception_Illegal:
          {
          std::cerr << "Illegal instruction";
          } break;
        case cmsysProcess_Exception_Interrupt:
          {
          std::cerr << "Interrupted by user";
          } break;
        case cmsysProcess_Exception_Numerical:
          {
          std::cerr << "Numerical exception";
          } break;
        case cmsysProcess_Exception_Other:
          {
          std::cerr << "Unknown";
          } break;
        }
      std::cerr << "\n";
      } break;
    case cmsysProcess_State_Executing:
      {
      std::cerr << "cmProcess: Never terminated " << 
        this->Command << " process.\n";
      } break;
    case cmsysProcess_State_Exited:
      {
      result = cmsysProcess_GetExitValue(this->Process);
      std::cerr << "cmProcess: " << this->Command 
                << " process exited with code "
                << result << "\n";
      } break;
    case cmsysProcess_State_Expired:
      {
      std::cerr << "cmProcess: killed " << this->Command 
                << " process due to timeout.\n";
      } break;
    case cmsysProcess_State_Killed:
      {
      std::cerr << "cmProcess: killed " << this->Command << " process.\n";
      } break;
    }
  return result;

}
