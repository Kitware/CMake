/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCMakeMinimumRequired.h"
#include "stdio.h"

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
  float reqVersion = 0;
  sscanf(args[1].c_str(), "%f", &reqVersion);
  if(reqVersion > version)
    {
    cmOStringStream str;
    str << "WARNING: This project requires version: " << args[1].c_str() << " of cmake.\n"
        << "You are running version: " << version;
    cmSystemTools::Message(str.str().c_str());
    }
  return true;
}

