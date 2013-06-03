/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVariableWatchCommand.h"

#include "cmVariableWatch.h"

//----------------------------------------------------------------------------
static void cmVariableWatchCommandVariableAccessed(
  const std::string& variable, int access_type, void* client_data,
  const char* newValue, const cmMakefile* mf)
{
  cmVariableWatchCommand* command
    = static_cast<cmVariableWatchCommand*>(client_data);
  command->VariableAccessed(variable, access_type, newValue, mf);
}

//----------------------------------------------------------------------------
cmVariableWatchCommand::cmVariableWatchCommand()
{
  this->InCallback = false;
}

//----------------------------------------------------------------------------
bool cmVariableWatchCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if ( args.size() < 1 )
    {
    this->SetError("must be called with at least one argument.");
    return false;
    }
  std::string variable = args[0];
  if ( args.size() > 1 )
    {
    std::string command = args[1];
    this->Handlers[variable].Commands.push_back(args[1]);
    }
  if ( variable == "CMAKE_CURRENT_LIST_FILE" )
    {
    cmOStringStream ostr;
    ostr << "cannot be set on the variable: " << variable.c_str();
    this->SetError(ostr.str().c_str());
    return false;
    }

  this->Makefile->GetCMakeInstance()->GetVariableWatch()->AddWatch(
    variable, cmVariableWatchCommandVariableAccessed, this);

  return true;
}

//----------------------------------------------------------------------------
void cmVariableWatchCommand::VariableAccessed(const std::string& variable,
  int access_type, const char* newValue, const cmMakefile* mf)
{
  if ( this->InCallback )
    {
    return;
    }
  this->InCallback = true;

  cmListFileFunction newLFF;
  cmVariableWatchCommandHandler *handler = &this->Handlers[variable];
  cmVariableWatchCommandHandler::VectorOfCommands::iterator it;
  cmListFileArgument arg;
  bool processed = false;
  const char* accessString = cmVariableWatch::GetAccessAsString(access_type);
  const char* currentListFile = mf->GetDefinition("CMAKE_CURRENT_LIST_FILE");

  /// Ultra bad!!
  cmMakefile* makefile = const_cast<cmMakefile*>(mf);

  std::string stack = makefile->GetProperty("LISTFILE_STACK");
  for ( it = handler->Commands.begin(); it != handler->Commands.end();
    ++ it )
    {
    std::string command = *it;
    newLFF.Arguments.clear();
    newLFF.Arguments.push_back(
      cmListFileArgument(variable, true, "unknown", 9999));
    newLFF.Arguments.push_back(
      cmListFileArgument(accessString, true, "unknown", 9999));
    newLFF.Arguments.push_back(
      cmListFileArgument(newValue?newValue:"", true, "unknown", 9999));
    newLFF.Arguments.push_back(
      cmListFileArgument(currentListFile, true, "unknown", 9999));
    newLFF.Arguments.push_back(
      cmListFileArgument(stack, true, "unknown", 9999));
    newLFF.Name = command;
    newLFF.FilePath = "Some weird path";
    newLFF.Line = 9999;
    cmExecutionStatus status;
    if(!makefile->ExecuteCommand(newLFF,status))
      {
      arg.FilePath =  "Unknown";
      arg.Line = 0;
      cmOStringStream error;
      error << "Error in cmake code at\n"
        << arg.FilePath << ":" << arg.Line << ":\n"
        << "A command failed during the invocation of callback\""
        << command << "\".";
      cmSystemTools::Error(error.str().c_str());
      this->InCallback = false;
      return;
      }
    processed = true;
    }
  if ( !processed )
    {
    cmOStringStream msg;
    msg << "Variable \"" << variable.c_str() << "\" was accessed using "
        << accessString << " with value \"" << newValue << "\".";
    makefile->IssueMessage(cmake::LOG, msg.str());
    }
  this->InCallback = false;
}
