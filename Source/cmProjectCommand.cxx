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
#include "cmProjectCommand.h"

// cmProjectCommand
bool cmProjectCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 || args.size() > 1)
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    }
  m_Makefile->SetProjectName(args[0].c_str());

  std::string bindir = args[0];
  bindir += "_BINARY_DIR";
  std::string srcdir = args[0];
  srcdir += "_SOURCE_DIR";
  
  m_Makefile->AddDefinition(bindir.c_str(),
	  m_Makefile->GetCurrentOutputDirectory());
  m_Makefile->AddDefinition(srcdir.c_str(),
	  m_Makefile->GetCurrentDirectory());
  
  return true;
}

