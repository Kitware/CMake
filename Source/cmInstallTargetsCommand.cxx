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
#include "cmInstallTargetsCommand.h"

// cmExecutableCommand
bool cmInstallTargetsCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  cmTargets &tgts = m_Makefile->GetTargets();
  std::vector<std::string>::const_iterator s = args.begin();
  ++s;
  std::string runtime_dir = "/bin";
  for (;s != args.end(); ++s)
    {
    if (*s == "RUNTIME_DIRECTORY")
      {
      ++s;
      if ( s == args.end() )
        {
        this->SetError("called with RUNTIME_DIRECTORY but no actual directory");
        return false;
        }

      runtime_dir = *s;
      }
    else if (tgts.find(*s) != tgts.end())
      {
      tgts[*s].SetInstallPath(args[0].c_str());
      tgts[*s].SetRuntimeInstallPath(runtime_dir.c_str());
      }
    }
  
  return true;
}

