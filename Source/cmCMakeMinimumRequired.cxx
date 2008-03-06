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
#include "cmCMakeMinimumRequired.h"

#include "cmVersion.h"

// cmCMakeMinimumRequired
bool cmCMakeMinimumRequired
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // Process arguments.
  std::string version_string;
  bool doing_version = false;
  for(unsigned int i=0; i < args.size(); ++i)
    {
    if(args[i] == "VERSION")
      {
      doing_version = true;
      }
    else if(args[i] == "FATAL_ERROR")
      {
      if(doing_version)
        {
        this->SetError("called with no value for VERSION.");
        return false;
        }
      doing_version = false;
      }
    else if(doing_version)
      {
      doing_version = false;
      version_string = args[i];
      }
    else
      {
      cmOStringStream e;
      e << "called with unknown argument \"" << args[i].c_str() << "\".";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  if(doing_version)
    {
    this->SetError("called with no value for VERSION.");
    return false;
    }

  // Make sure there was a version to check.
  if(version_string.empty())
    {
    return true;
    }

  // Save the required version string.
  this->Makefile->AddDefinition("CMAKE_MINIMUM_REQUIRED_VERSION",
                                version_string.c_str());


  // Get the current version number.
  int current_major = cmVersion::GetMajorVersion();
  int current_minor = cmVersion::GetMinorVersion();
  int current_patch = cmVersion::GetPatchVersion();

  // Parse the required version number.  If no patch-level is given
  // use zero.
  int required_major = 0;
  int required_minor = 0;
  int required_patch = 0;
  if(sscanf(version_string.c_str(), "%d.%d.%d",
            &required_major, &required_minor, &required_patch) < 2)
    {
    cmOStringStream e;
    e << "could not parse VERSION \"" << version_string.c_str() << "\".";
    this->SetError(e.str().c_str());
    return false;
    }

  // Compare the version numbers.
  if(current_major < required_major ||
     current_major == required_major &&
     current_minor < required_minor ||
     current_major == required_major &&
     current_minor == required_minor &&
     current_patch < required_patch)
    {
    // The current version is too low.
    cmOStringStream e;
    e << "This project requires version " << version_string.c_str()
      << " of CMake.  "
      << "You are running version "
      << current_major << "." << current_minor << "." << current_patch
      << ".\n";
    cmSystemTools::Error(e.str().c_str());
    cmSystemTools::SetFatalErrorOccured();
    }

  return true;
}

