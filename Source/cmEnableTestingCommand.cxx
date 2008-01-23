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
#include "cmEnableTestingCommand.h"
#include "cmLocalGenerator.h"

// we do this in the final pass so that we now the subdirs have all 
// been defined
bool cmEnableTestingCommand::InitialPass(std::vector<std::string> const&,
                                         cmExecutionStatus &)
{
  this->Makefile->AddDefinition("CMAKE_TESTING_ENABLED","1");
  return true;
}
