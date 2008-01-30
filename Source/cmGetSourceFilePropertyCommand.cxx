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
#include "cmGetSourceFilePropertyCommand.h"

#include "cmSourceFile.h"

// cmSetSourceFilePropertyCommand
bool cmGetSourceFilePropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() != 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* var = args[0].c_str();
  const char* file = args[1].c_str();
  cmSourceFile* sf = this->Makefile->GetSource(file);

  // for the location we must create a source file first
  if (!sf && args[2] == "LOCATION")
    {
    sf = this->Makefile->GetOrCreateSource(file);
    }
  if(sf)
    {
    if(args[2] == "LANGUAGE")
      {
      this->Makefile->AddDefinition(var, sf->GetLanguage());
      return true;
      }
    const char *prop = sf->GetPropertyForUser(args[2].c_str());
    if (prop)
      {
      this->Makefile->AddDefinition(var, prop);
      return true;
      }
    }

  this->Makefile->AddDefinition(var, "NOTFOUND");
  return true;
}

