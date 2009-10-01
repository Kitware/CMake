/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCustomCommand.h"

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand()
{
  this->HaveComment = false;
  this->EscapeOldStyle = true;
  this->EscapeAllowMakeVars = false;
}

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand(const cmCustomCommand& r):
  Outputs(r.Outputs),
  Depends(r.Depends),
  CommandLines(r.CommandLines),
  HaveComment(r.HaveComment),
  Comment(r.Comment),
  WorkingDirectory(r.WorkingDirectory),
  EscapeAllowMakeVars(r.EscapeAllowMakeVars),
  EscapeOldStyle(r.EscapeOldStyle)
{
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
  HaveComment(comment?true:false),
  Comment(comment?comment:""),
  WorkingDirectory(workingDirectory?workingDirectory:""),
  EscapeAllowMakeVars(false),
  EscapeOldStyle(true)
{
  this->EscapeOldStyle = true;
  this->EscapeAllowMakeVars = false;
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
  const char* no_comment = 0;
  return this->HaveComment? this->Comment.c_str() : no_comment;
}

//----------------------------------------------------------------------------
void cmCustomCommand::AppendCommands(const cmCustomCommandLines& commandLines)
{
  for(cmCustomCommandLines::const_iterator i=commandLines.begin();
      i != commandLines.end(); ++i)
    {
    this->CommandLines.push_back(*i);
    }
}

//----------------------------------------------------------------------------
void cmCustomCommand::AppendDepends(const std::vector<std::string>& depends)
{
  for(std::vector<std::string>::const_iterator i=depends.begin();
      i != depends.end(); ++i)
    {
    this->Depends.push_back(*i);
    }
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

//----------------------------------------------------------------------------
cmCustomCommand::ImplicitDependsList const&
cmCustomCommand::GetImplicitDepends() const
{
  return this->ImplicitDepends;
}

//----------------------------------------------------------------------------
void cmCustomCommand::SetImplicitDepends(ImplicitDependsList const& l)
{
  this->ImplicitDepends = l;
}

//----------------------------------------------------------------------------
void cmCustomCommand::AppendImplicitDepends(ImplicitDependsList const& l)
{
  this->ImplicitDepends.insert(this->ImplicitDepends.end(),
                               l.begin(), l.end());
}
