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
#include <stdlib.h> // required for atof

// cmAbstractFilesCommand
bool cmAbstractFilesCommand::InitialPass(std::vector<std::string> const& args)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (atof(versionValue) > 1.4)
    {
    this->SetError("The ABSTRACT_FILES command was deprecated in CMake version 1.4 and will be removed in later versions of CMake. You should modify your CMakeLists.txt files to use the SET command instead, or set the cache value of CMAKE_BACKWARDS_COMPATIBILITY to 1.2 or less.\n");
    return false;
    }
  if (atof(versionValue) > 1.2)
    {
    cmSystemTools::Message("The ABSTRACT_FILES command was deprecated in CMake version 1.4 and will be removed in later versions. You should modify your CMakeLists.txt files to use the SET command instead, or set the cache value of CMAKE_BACKWARDS_COMPATIBILITY to 1.2 or less.\n","Warning");
    }

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

