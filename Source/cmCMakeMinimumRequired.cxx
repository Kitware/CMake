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

// cmCMakeMinimumRequired
bool cmCMakeMinimumRequired::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  if(args[0] == "VERSION")
    {
    m_Makefile->AddDefinition("CMAKE_MINIMUM_REQUIRED_VERSION", args[1].c_str());
    }
  float version = float(m_Makefile->GetMajorVersion());
  version += (float(m_Makefile->GetMinorVersion()) * (float).1);
  version += (float(m_Makefile->GetPatchVersion()) * (float).01);
  float reqVersion = 0;
  int major=0;
  int minor=0;
  int patch=0;

  int res=sscanf(args[1].c_str(), "%d.%d.%d", &major, &minor, &patch);
  if (res==3)
     reqVersion=float(major)+0.1*float(minor)+0.01*float(patch);
  else if (res==2)
     reqVersion=float(major)+0.1*float(minor);

  if(reqVersion > version)
    {
    cmOStringStream str;
    str << "WARNING: This project requires version: " << args[1].c_str() << " of cmake.\n"
        << "You are running version: " << version;
    cmSystemTools::Message(str.str().c_str());
    }
  return true;
}

