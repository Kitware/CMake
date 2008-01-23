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

  if ( args[1] == "VARIABLES")
    {
    int cacheonly = 0;
    std::vector<std::string> vars = this->Makefile->GetDefinitions(cacheonly);
    for ( cc = 0; cc < vars.size(); cc ++ )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += vars[cc];
      }
    }
  else if ( args[1] == "MACROS" )
    {
    this->Makefile->GetListOfMacros(output);
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

