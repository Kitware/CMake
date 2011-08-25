/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddExecutableCommand.h"

// cmExecutableCommand
bool cmAddExecutableCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>::const_iterator s = args.begin();

  std::string exename = *s;

  ++s;
  bool use_win32 = false;
  bool use_macbundle = false;
  bool excludeFromAll = false;
  bool importTarget = false;
  while ( s != args.end() )
    {
    if (*s == "WIN32")
      {
      ++s;
      use_win32 = true;
      }
    else if ( *s == "MACOSX_BUNDLE" )
      {
      ++s;
      use_macbundle = true;
      }
    else if(*s == "EXCLUDE_FROM_ALL")
      {
      ++s;
      excludeFromAll = true;
      }
    else if(*s == "IMPORTED")
     {
     ++s;
     importTarget = true;
     }
    else
      {
      break;
      }
    }

  // Special modifiers are not allowed with IMPORTED signature.
  if(importTarget
      && (use_win32 || use_macbundle || excludeFromAll))
    {
    if(use_win32)
      {
      this->SetError("may not be given WIN32 for an IMPORTED target.");
      }
    else if(use_macbundle)
      {
      this->SetError(
        "may not be given MACOSX_BUNDLE for an IMPORTED target.");
      }
    else // if(excludeFromAll)
      {
      this->SetError(
        "may not be given EXCLUDE_FROM_ALL for an IMPORTED target.");
      }
    return false;
    }

  // Handle imported target creation.
  if(importTarget)
    {
    // Make sure the target does not already exist.
    if(this->Makefile->FindTargetToUse(exename.c_str()))
      {
      cmOStringStream e;
      e << "cannot create imported target \"" << exename
        << "\" because another target with the same name already exists.";
      this->SetError(e.str().c_str());
      return false;
      }

    // Create the imported target.
    this->Makefile->AddImportedTarget(exename.c_str(), cmTarget::EXECUTABLE);
    return true;
    }

  // Enforce name uniqueness.
  {
  std::string msg;
  if(!this->Makefile->EnforceUniqueName(exename, msg))
    {
    this->SetError(msg.c_str());
    return false;
    }
  }

  if (s == args.end())
    {
    this->SetError
      ("called with incorrect number of arguments, no sources provided");
    return false;
    }

  std::vector<std::string> srclists(s, args.end());
  cmTarget* tgt = this->Makefile->AddExecutable(exename.c_str(), srclists,
                                                excludeFromAll);
  if ( use_win32 )
    {
    tgt->SetProperty("WIN32_EXECUTABLE", "ON");
    }
  if ( use_macbundle)
    {
    tgt->SetProperty("MACOSX_BUNDLE", "ON");
    }

  return true;
}
