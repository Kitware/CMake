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
#include "cmUnixLibrariesCommand.h"

cmUnixLibrariesCommand::cmUnixLibrariesCommand()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->EnabledOff();
#endif
}

// cmUnixLibrariesCommand
bool cmUnixLibrariesCommand::Invoke(std::vector<std::string>& args)
{
  this->SetError(" deprecated - use LIBRARY command inside an IF block ");
  return false;

  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddLinkLibrary((*i).c_str());
    }
  return true;
}

