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
#include "cmTargetLinkLibrariesCommand.h"

// cmTargetLinkLibrariesCommand
bool cmTargetLinkLibrariesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
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
      const char* dir = m_Makefile->GetDefinition(i->c_str());
      if( dir )
        {
        m_Makefile->AddLinkDirectory( dir );
        }
      }
    } 
  return true;
}

