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
#include "cmLinkLibrariesCommand.h"

// cmLinkLibrariesCommand
bool cmLinkLibrariesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    return true;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  // add libraries, nothe that there is an optional prefix 
  // of debug and optimized than can be used
  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    if (*i == "debug")
      {
      ++i;
      m_Makefile->AddLinkLibrary(i->c_str(),
                                 cmTarget::DEBUG);
      }
    else if (*i == "optimized")
      {
      ++i;
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
      if( dir )
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

