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
bool cmTargetLinkLibrariesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  // must have one argument
  if(argsIn.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // but we might not have any libs after variable expansion
  if(argsIn.size() < 2)
    {
    return true;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  // add libraries, nothe that there is an optional prefix 
  // of debug and optimized than can be used
  std::vector<std::string>::const_iterator i = args.begin();
  
  for(++i; i != args.end(); ++i)
    {
    if (*i == "debug")
      {
      ++i;
      m_Makefile->AddLinkLibraryForTarget(args[0].c_str(),i->c_str(),
                                          cmTarget::DEBUG);
      }
    else if (*i == "optimized")
      {
      ++i;
      m_Makefile->AddLinkLibraryForTarget(args[0].c_str(),i->c_str(),
                                 cmTarget::OPTIMIZED);
      }
    else
      {
      m_Makefile->AddLinkLibraryForTarget(args[0].c_str(),i->c_str(),
                                          cmTarget::GENERAL);  
      }
    // if this is a library that cmake knows about, and LIBRARY_OUTPUT_PATH 
    // is not set, then add the link directory
    const char* ldir = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    if (cmSystemTools::IsOff(ldir))
      {
      std::string libPath = *i + "_CMAKE_PATH";
      const char* dir = m_Makefile->GetDefinition(libPath.c_str());
      if( dir )
        {
        m_Makefile->AddLinkDirectoryForTarget(args[0].c_str(), dir );
        }
      }
    else
      {
      m_Makefile->AddLinkDirectoryForTarget(args[0].c_str(), ldir );
      }
    } 
  return true;
}

