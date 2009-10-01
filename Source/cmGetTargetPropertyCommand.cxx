/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGetTargetPropertyCommand.h"

// cmSetTargetPropertyCommand
bool cmGetTargetPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() != 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string var = args[0].c_str();
  const char* targetName = args[1].c_str();

  if(cmTarget* tgt = this->Makefile->FindTargetToUse(targetName))
    {
    cmTarget& target = *tgt;
    const char *prop = target.GetProperty(args[2].c_str());
    if (prop)
      {
      this->Makefile->AddDefinition(var.c_str(), prop);
      return true;
      }
    }
  this->Makefile->AddDefinition(var.c_str(), (var+"-NOTFOUND").c_str());
  return true;
}

