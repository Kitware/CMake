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
  if (args.size()< 1 || args.size() > 2)
    {
      this->SetError("called with wrong number of arguments.  "
                     "Include only takes one file.");
    }

  m_Makefile->ExpandVariablesInString( args[0]);
  bool exists = cmSystemTools::FileExists(args[0].c_str());
  if(args.size() == 2 && args[1] == "OPTIONAL" && !exists)
    {
    return true;
    }
  if(!exists)
    {
    std::string error = "Include file not found: " + args[0];
    this->SetError(error.c_str());
    return false;
    }
  m_Makefile->ReadListFile( m_Makefile->GetCurrentListFile(), 
                            args[0].c_str());
  return true;
}


