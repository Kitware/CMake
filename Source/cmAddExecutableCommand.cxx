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
#include "cmAddExecutableCommand.h"

// cmExecutableCommand
bool cmAddExecutableCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::iterator s = args.begin();
  std::vector<std::string> srclists(++s, args.end());
  
  m_Makefile->AddExecutable(args[0].c_str(),srclists);
  return true;
}

