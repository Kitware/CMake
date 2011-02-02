/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGetCMakePropertyCommand.h"

#include "cmake.h"

// cmGetCMakePropertyCommand
bool cmGetCMakePropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string>::size_type cc;
  std::string variable = args[0];
  std::string output = "NOTFOUND";

  if ( args[1] == "VARIABLES" )
    {
    int cacheonly = 0;
    std::vector<std::string> vars = this->Makefile->GetDefinitions(cacheonly);
    if (vars.size()>0)
      {
      output = vars[0];
      }
    for ( cc = 1; cc < vars.size(); ++cc )
      {
      output += ";";
      output += vars[cc];
      }
    }
  else if ( args[1] == "MACROS" )
    {
    this->Makefile->GetListOfMacros(output);
    }
  else if ( args[1] == "COMPONENTS" )
    {
    const std::set<cmStdString>* components
      = this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
        ->GetInstallComponents();
    std::set<cmStdString>::const_iterator compIt;
    output = "";
    for (compIt = components->begin(); compIt != components->end(); ++compIt)
      {
      if (compIt != components->begin())
        {
        output += ";";
        }
      output += *compIt;
      }
    }
  else
    {
    const char *prop =
      this->Makefile->GetCMakeInstance()->GetProperty(args[1].c_str());
    if (prop)
      {
      output = prop;
      }
    }

  this->Makefile->AddDefinition(variable.c_str(), output.c_str());

  return true;
}
