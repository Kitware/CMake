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
#include "cmAbstractFilesCommand.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"

// cmAbstractFilesCommand
bool cmAbstractFilesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (versionValue && atof(versionValue) > 1.2)
    {
    this->SetError("The ABSTRACT_FILES command has been deprecated in CMake version 1.4. You should use the SET_SOURCE_FILES_PROPERTIES command instead.\n");
    return false;
    }
  
  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  bool ret = true;
  std::string m = "could not find source file(s):\n";

  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {  
    cmSourceFile* sf = m_Makefile->GetSource(j->c_str());
    if(sf)
      {
      sf->SetProperty("ABSTRACT","1");
      }
    else
      {
      // for VTK 4.0 we have to support missing abstract sources
      if(m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION"))
        {
        m += *j;
        m += "\n";
        ret = false;
        } 
      }
    }
  if(!ret)
    {
    this->SetError(m.c_str());
    }
  return ret;
}

