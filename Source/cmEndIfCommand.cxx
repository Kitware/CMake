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
#include "cmEndIfCommand.h"

bool cmEndIfCommand::InitialPass(std::vector<std::string> const&)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (!versionValue || (atof(versionValue) <= 1.4))
    {
    return true;
    }
  
  this->SetError("An ENDIF command was found outside of a proper IF ENDIF structure. Or its arguments did not match the opening IF command.");
  return false;
}

