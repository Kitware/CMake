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
#include "cmSourceFilesRequireCommand.h"

// cmSourceFilesRequireCommand
bool cmSourceFilesRequireCommand::Invoke(std::vector<std::string>& args)
{
  this->SetError(" deprecated - use SourceFiles command inside an If block ");
  return false;

  std::vector<std::string>::iterator i = args.begin();
  // Search to the key word SOURCES_BEGIN is found
  // if one of the required defines is not there, then
  // return as none of the source files will be added
  // if the required definition is not there.
  while(i != args.end() && (*i) != "SOURCES_BEGIN" )
    {
    if(!m_Makefile->GetDefinition((*i).c_str()))
      {
      return true;
      }
    i++;
    }
  if(i != args.end())
    {
    i++;
    }
  
  // Add the rest of the arguments as source files
  const char *sname = (*i).c_str();
  ++i;
  for(; i != args.end(); ++i)
    {
    cmSourceFile file;
    file.SetIsAnAbstractClass(false);
    file.SetName((*i).c_str(), m_Makefile->GetCurrentDirectory());
    m_Makefile->AddSource(file, sname);
    }
  return true;
}

