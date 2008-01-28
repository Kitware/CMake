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

  cmTarget* target = 
    this->GetMakefile()->GetLocalGenerator()->
    GetGlobalGenerator()->FindTarget(0, target_name.c_str());
  if(target)
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

