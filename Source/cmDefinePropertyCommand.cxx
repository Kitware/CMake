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
#include "cmDefinePropertyCommand.h"
#include "cmake.h"

// cmDefinePropertiesCommand
bool cmDefinePropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 5 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // determine the scope
  cmProperty::ScopeType scope;
  if (args[1] == "GLOBAL")
    {
    scope = cmProperty::GLOBAL;
    }
  else if (args[1] == "DIRECTORY")
    {
    scope = cmProperty::DIRECTORY;
    }
  else if (args[1] == "TARGET")
    {
    scope = cmProperty::TARGET;
    }
  else if (args[1] == "SOURCE_FILE")
    {
    scope = cmProperty::SOURCE_FILE;
    }
  else if (args[1] == "TEST")
    {
    scope = cmProperty::TEST;
    }
  else if (args[1] == "VARIABLE")
    {
    scope = cmProperty::VARIABLE;
    }
  else if (args[1] == "CACHED_VARIABLE")
    {
    scope = cmProperty::CACHED_VARIABLE;
    }
  else
    {
    this->SetError("called with illegal arguments.");
    return false;
    }

  this->Makefile->GetCMakeInstance()->DefineProperty
    (args[0].c_str(), scope,args[2].c_str(), args[3].c_str(),
     cmSystemTools::IsOn(args[4].c_str()));
  
  return true;
}

