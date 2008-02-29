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
#include "cmCMakePolicyCommand.h"

#include "cmVersion.h"

// cmCMakePolicyCommand
bool cmCMakePolicyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if (args.size() < 1)
  {
    this->SetError("cmake_policy requires at least one argument.");
    return false;
  }

  if (args[0] == "OLD" && args.size() == 2)
  {
    return this->Makefile->SetPolicy(args[1].c_str(),cmPolicies::OLD);
  }
  
  if (args[0] == "NEW" && args.size() == 2)
  {
    return this->Makefile->SetPolicy(args[1].c_str(),cmPolicies::NEW);
  }
  
  if (args[0] == "VERSION" && args.size() == 2)
  {
    return this->Makefile->SetPolicyVersion(args[1].c_str());
  }

  if (args[0] == "PUSH" && args.size() == 1)
  {
    return this->Makefile->PushPolicy();
  }
  
  if (args[0] == "POP" && args.size() == 1)
  {
    return this->Makefile->PopPolicy();
  }

  this->SetError("incorrect arguments for cmake_policy.");  
  return false;
}

