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
#include "cmIncludeCommand.h"

#include <iostream.h>

// cmIncludeCommand
bool cmIncludeCommand::InitialPass(std::vector<std::string>& args)
{
  if (args.size()< 1)
    {
      this->SetError("called with wrong number of arguments.");
    }

  for( unsigned int i=0; i< args.size(); i++)
    {
      m_Makefile->ExpandVariablesInString( args[i]);
      m_Makefile->ReadListFile( m_Makefile->GetCurrentListFile(), 
				args[i].c_str());
    }

  return true;
}


