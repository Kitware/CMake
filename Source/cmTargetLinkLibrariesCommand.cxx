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
      if( dir && *dir )
        {
        m_Makefile->AddLinkDirectoryForTarget(args[0].c_str(), dir );
        }
      else
        {
        m_HasLocation.push_back(*i);
        }
      }
    else
      {
      m_Makefile->AddLinkDirectoryForTarget(args[0].c_str(), ldir );
      }
    } 
  return true;
}

void cmTargetLinkLibrariesCommand::FinalPass()
{
  std::vector<std::string>::size_type cc;
  std::string libPath;
  if ( !m_Makefile->GetDefinition("CMAKE_IGNORE_DEPENDENCIES_ORDERING") )
    {
    for ( cc = 0; cc < m_HasLocation.size(); cc ++ )
      {
      libPath = m_HasLocation[cc] + "_CMAKE_PATH";
      const char* dir = m_Makefile->GetDefinition(libPath.c_str());
      if ( dir && *dir )
        {
        std::string str = "Library " + m_HasLocation[cc] + 
          " is defined using ADD_LIBRARY after the library is used "
          "using TARGET_LINK_LIBRARIES for the target " + m_TargetName +
          ". This breaks CMake's dependency "
          "handling. Please fix the CMakeLists.txt file.";
        this->SetError(str.c_str());
        cmSystemTools::Message(str.c_str(), "CMake Error");
        }
      }
    }
}
