/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddDependenciesCommand.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

// cmDependenciesCommand
bool cmAddDependenciesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string target_name = args[0];
  if(cmTarget* target = this->Makefile->FindTargetToUse(target_name.c_str()))
    {
    std::vector<std::string>::const_iterator s = args.begin();
    ++s; // skip over target_name
    for (; s != args.end(); ++s)
      {
      target->AddUtility(s->c_str());
      }
    }
  else
    {
    std::string error = "Adding dependency to non-existent target: ";
    error += target_name;
    this->SetError(error.c_str());
    return false;
    }

  return true;
}

