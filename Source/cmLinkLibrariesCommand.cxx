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
#include "cmLinkLibrariesCommand.h"

// cmLinkLibrariesCommand
bool cmLinkLibrariesCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    return true;
    }
  // add libraries, nothe that there is an optional prefix 
  // of debug and optimized than can be used
  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    if (*i == "debug")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("The \"debug\" argument must be followed by a library");
        return false;
        }
      m_Makefile->AddLinkLibrary(i->c_str(),
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
      m_Makefile->AddLinkLibrary(i->c_str(),
                                 cmTarget::OPTIMIZED);
      }
    else
      {
      m_Makefile->AddLinkLibrary(i->c_str());  
      }

    const char* ldir = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    if (cmSystemTools::IsOff(ldir))
      {
      std::string libPath = *i + "_CMAKE_PATH";
      const char* dir = m_Makefile->GetDefinition(libPath.c_str());
      if( dir && *dir )
        {
        m_Makefile->AddLinkDirectory( dir );
        }
      }
    else
      {
      m_Makefile->AddLinkDirectory( ldir );
      }
    }
  
  return true;
}

