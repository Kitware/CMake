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
#include "cmTargetLinkLibrariesCommand.h"

// cmTargetLinkLibrariesCommand
bool cmTargetLinkLibrariesCommand::InitialPass(std::vector<std::string> const& args)
{
  // must have one argument
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  m_TargetName = args[0];

  // but we might not have any libs after variable expansion
  if(args.size() < 2)
    {
    return true;
    }
  // add libraries, nothe that there is an optional prefix 
  // of debug and optimized than can be used
  std::vector<std::string>::const_iterator i = args.begin();
  
  for(++i; i != args.end(); ++i)
    {
    if (*i == "debug")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("The \"debug\" argument must be followed by a library");
        return false;
        }
      m_Makefile->AddLinkLibraryForTarget(args[0].c_str(),i->c_str(),
                                          cmTarget::DEBUG);
      }
    else if (*i == "optimized")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("The \"optimized\" argument must be followed by a library");
        return false;
        }
      m_Makefile->AddLinkLibraryForTarget(args[0].c_str(),i->c_str(),
                                 cmTarget::OPTIMIZED);
      }
    else
      {
      m_Makefile->AddLinkLibraryForTarget(args[0].c_str(),i->c_str(),
                                          cmTarget::GENERAL);  
      }
    } 
  return true;
}
