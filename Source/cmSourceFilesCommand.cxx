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
#include "cmSourceFilesCommand.h"

// cmSourceFilesCommand
bool cmSourceFilesCommand::Invoke(std::vector<std::string>& args)
{
 if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  for(std::vector<std::string>::iterator i = (args.begin() + 1);
      i != args.end(); ++i)
    {
    cmSourceFile file;
    file.SetIsAnAbstractClass(false);
    file.SetName((*i).c_str(), m_Makefile->GetCurrentDirectory());
    m_Makefile->AddSource(file, args[0].c_str());
    }
  return true;
}

