/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmLinkLibrariesCommand.h"

// cmLinkLibrariesCommand
bool cmLinkLibrariesCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // add libraries, nothe that there is an optional prefix 
  // of debug and optimized than can be used
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    if (*i == "debug")
      {
      ++i;
      m_Makefile->AddLinkLibrary(i->c_str(),
                                 cmMakefile::DEBUG);
      }
    else if (*i == "optimized")
      {
      ++i;
      m_Makefile->AddLinkLibrary(i->c_str(),
                                 cmMakefile::OPTIMIZED);
      }
    else
      {
      m_Makefile->AddLinkLibrary(i->c_str());  
      }
    }
  return true;
}

