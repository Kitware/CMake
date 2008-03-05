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
  if(args.size() < 1)
    {
    this->SetError("requires at least one argument.");
    return false;
    }

  if(args[0] == "SET")
    {
    return this->HandleSetMode(args);
    }
  else if(args[0] == "PUSH")
    {
    if(args.size() > 1)
      {
      this->SetError("PUSH may not be given additional arguments.");
      return false;
      }
    return this->Makefile->PushPolicy();
    }
  else if(args[0] == "POP")
    {
    if(args.size() > 1)
      {
      this->SetError("POP may not be given additional arguments.");
      return false;
      }
    if(this->Makefile->PopPolicy(false))
      {
      return true;
      }
    else
      {
      this->SetError("POP without matching PUSH");
      return false;
      }
    }
  else if(args[0] == "VERSION")
    {
    return this->HandleVersionMode(args);
    }

  cmOStringStream e;
  e << "given unknown first argument \"" << args[0] << "\"";
  this->SetError(e.str().c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmCMakePolicyCommand::HandleSetMode(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("SET must be given exactly 2 additional arguments.");
    return false;
    }

  cmPolicies::PolicyStatus status;
  if(args[2] == "OLD")
    {
    status = cmPolicies::OLD;
    }
  else if(args[2] == "NEW")
    {
    status = cmPolicies::NEW;
    }
  else
    {
    cmOStringStream e;
    e << "SET given unrecognized policy status \"" << args[2] << "\"";
    this->SetError(e.str().c_str());
    return false;
    }

  if(!this->Makefile->SetPolicy(args[1].c_str(), status))
    {
    this->SetError("SET failed to set policy.");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool
cmCMakePolicyCommand::HandleVersionMode(std::vector<std::string> const& args)
{
  if(args.size() <= 1)
    {
    this->SetError("VERSION not given an argument");
    return false;
    }
  else if(args.size() >= 3)
    {
    this->SetError("VERSION given too many arguments");
    return false;
    }
  if(!this->Makefile->SetPolicyVersion(args[1].c_str()))
    {
    cmOStringStream e;
    e << "VERSION given invalid value \"" << args[1] << "\".  "
      << "A numeric major.minor[.patch] must be given.";
    this->SetError(e.str().c_str());
    return false;
    }
  return true;
}
