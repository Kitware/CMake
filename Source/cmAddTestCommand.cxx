/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddTestCommand.h"

#include "cmTestGenerator.h"

#include "cmTest.h"


// cmExecutableCommand
bool cmAddTestCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(!args.empty() && args[0] == "NAME")
    {
    return this->HandleNameMode(args);
    }

  // First argument is the name of the test Second argument is the name of
  // the executable to run (a target or external program) Remaining arguments
  // are the arguments to pass to the executable
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Collect the command with arguments.
  std::vector<std::string> command;
  for(std::vector<std::string>::const_iterator it = args.begin() + 1;
      it != args.end(); ++it)
    {
    command.push_back(*it);
    }

  // Create the test but add a generator only the first time it is
  // seen.  This preserves behavior from before test generators.
  cmTest* test = this->Makefile->GetTest(args[0].c_str());
  if(test)
    {
    // If the test was already added by a new-style signature do not
    // allow it to be duplicated.
    if(!test->GetOldStyle())
      {
      cmOStringStream e;
      e << " given test name \"" << args[0]
        << "\" which already exists in this directory.";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  else
    {
    test = this->Makefile->CreateTest(args[0].c_str());
    test->SetOldStyle(true);
    this->Makefile->AddTestGenerator(new cmTestGenerator(test));
    }
  test->SetCommand(command);

  return true;
}

//----------------------------------------------------------------------------
bool cmAddTestCommand::HandleNameMode(std::vector<std::string> const& args)
{
  std::string name;
  std::vector<std::string> configurations;
  std::vector<std::string> command;

  // Read the arguments.
  enum Doing {
    DoingName,
    DoingCommand,
    DoingConfigs,
    DoingNone
  };
  Doing doing = DoingName;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "COMMAND")
      {
      if(!command.empty())
        {
        this->SetError(" may be given at most one COMMAND.");
        return false;
        }
      doing = DoingCommand;
      }
    else if(args[i] == "CONFIGURATIONS")
      {
      if(!configurations.empty())
        {
        this->SetError(" may be given at most one set of CONFIGURATIONS.");
        return false;
        }
      doing = DoingConfigs;
      }
    else if(doing == DoingName)
      {
      name = args[i];
      doing = DoingNone;
      }
    else if(doing == DoingCommand)
      {
      command.push_back(args[i]);
      }
    else if(doing == DoingConfigs)
      {
      configurations.push_back(args[i]);
      }
    else
      {
      cmOStringStream e;
      e << " given unknown argument:\n  " << args[i] << "\n";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Require a test name.
  if(name.empty())
    {
    this->SetError(" must be given non-empty NAME.");
    return false;
    }

  // Require a command.
  if(command.empty())
    {
    this->SetError(" must be given non-empty COMMAND.");
    return false;
    }

  // Require a unique test name within the directory.
  if(this->Makefile->GetTest(name.c_str()))
    {
    cmOStringStream e;
    e << " given test NAME \"" << name
      << "\" which already exists in this directory.";
    this->SetError(e.str().c_str());
    return false;
    }

  // Add the test.
  cmTest* test = this->Makefile->CreateTest(name.c_str());
  test->SetOldStyle(false);
  test->SetCommand(command);
  this->Makefile->AddTestGenerator(new cmTestGenerator(test, configurations));

  return true;
}
