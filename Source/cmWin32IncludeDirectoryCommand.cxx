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
#include "cmWin32IncludeDirectoryCommand.h"
#include "cmCacheManager.h"

// cmWin32IncludeDirectoryCommand
cmWin32IncludeDirectoryCommand::cmWin32IncludeDirectoryCommand()
{
#ifndef _WIN32
  this->EnabledOff();
#endif
}


bool cmWin32IncludeDirectoryCommand::Invoke(std::vector<std::string>& args)
{
  this->SetError(" deprecated - use INCLUDE_DIRECTORIES command inside an if block ");
  return false;

  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddIncludeDirectory((*i).c_str());
    }
  return true;
}

