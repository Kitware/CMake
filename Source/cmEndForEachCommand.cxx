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
#include "cmEndForEachCommand.h"

bool cmEndForEachCommand::InvokeInitialPass(std::vector<cmListFileArgument> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // remove any function blockers for this define
  cmListFileFunction lff;
  lff.m_Name = "ENDFOREACH";
  lff.m_Arguments = args;
  m_Makefile->RemoveFunctionBlocker(lff);
  
  return true;
}

