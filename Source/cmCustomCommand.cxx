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

#include "cmMakefile.h"

#include <cmsys/auto_ptr.hxx>

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand()
  : Backtrace()
{
  this->HaveComment = false;
  this->EscapeOldStyle = true;
  this->EscapeAllowMakeVars = false;
  this->UsesTerminal = false;
}

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand(cmMakefile const* mf,
                                 const std::vector<std::string>& outputs,
                                 const std::vector<std::string>& byproducts,
                                 const std::vector<std::string>& depends,
                                 const cmCustomCommandLines& commandLines,
                                 const char* comment,
                                 const char* workingDirectory):
  Outputs(outputs),
  Byproducts(byproducts),
  Depends(depends),
  CommandLines(commandLines),
  Backtrace(),
  Comment(comment?comment:""),
  WorkingDirectory(workingDirectory?workingDirectory:""),
  HaveComment(comment?true:false),
  EscapeAllowMakeVars(false),
  EscapeOldStyle(true)
{
  if(mf)
    {
    this->Backtrace = mf->GetBacktrace();
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmCustomCommand::GetOutputs() const
{
  return this->Outputs;
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmCustomCommand::GetByproducts() const
{
  return this->Byproducts;
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
  this->CommandLines.insert(this->CommandLines.end(),
                            commandLines.begin(), commandLines.end());
}

//----------------------------------------------------------------------------
void cmCustomCommand::AppendDepends(const std::vector<std::string>& depends)
{
  this->Depends.insert(this->Depends.end(), depends.begin(), depends.end());
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
cmListFileBacktrace const& cmCustomCommand::GetBacktrace() const
{
  return this->Backtrace;
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

//----------------------------------------------------------------------------
bool cmCustomCommand::GetUsesTerminal() const
{
  return this->UsesTerminal;
}

//----------------------------------------------------------------------------
void cmCustomCommand::SetUsesTerminal(bool b)
{
  this->UsesTerminal = b;
}
