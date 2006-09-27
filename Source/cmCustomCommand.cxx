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
#include "cmCustomCommand.h"

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand()
{
  this->EscapeOldStyle = true;
  this->EscapeAllowMakeVars = false;
  this->Used = false;
}

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand(const cmCustomCommand& r):
  Outputs(r.Outputs),
  Depends(r.Depends),
  CommandLines(r.CommandLines),
  Comment(r.Comment),
  WorkingDirectory(r.WorkingDirectory),
  EscapeAllowMakeVars(r.EscapeAllowMakeVars),
  EscapeOldStyle(r.EscapeOldStyle)
{
  this->Used = false;
}

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand(const std::vector<std::string>& outputs,
                                 const std::vector<std::string>& depends,
                                 const cmCustomCommandLines& commandLines,
                                 const char* comment, 
                                 const char* workingDirectory):
  Outputs(outputs),
  Depends(depends),
  CommandLines(commandLines),
  Comment(comment?comment:""),
  WorkingDirectory(workingDirectory?workingDirectory:"")
{
  this->EscapeOldStyle = true;
  this->EscapeAllowMakeVars = false;
  this->Used = false;
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmCustomCommand::GetOutputs() const
{
  return this->Outputs;
}

//----------------------------------------------------------------------------
const char* cmCustomCommand::GetWorkingDirectory() const
{
  if(this->WorkingDirectory.size() == 0)
    {
    return 0;
    }
  return this->WorkingDirectory.c_str();
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmCustomCommand::GetDepends() const
{
  return this->Depends;
}

//----------------------------------------------------------------------------
const cmCustomCommandLines& cmCustomCommand::GetCommandLines() const
{
  return this->CommandLines;
}

//----------------------------------------------------------------------------
const char* cmCustomCommand::GetComment() const
{
  return this->Comment.c_str();
}

//----------------------------------------------------------------------------
bool cmCustomCommand::GetEscapeOldStyle() const
{
  return this->EscapeOldStyle;
}

//----------------------------------------------------------------------------
void cmCustomCommand::SetEscapeOldStyle(bool b)
{
  this->EscapeOldStyle = b;
}

//----------------------------------------------------------------------------
bool cmCustomCommand::GetEscapeAllowMakeVars() const
{
  return this->EscapeAllowMakeVars;
}

//----------------------------------------------------------------------------
void cmCustomCommand::SetEscapeAllowMakeVars(bool b)
{
  this->EscapeAllowMakeVars = b;
}
