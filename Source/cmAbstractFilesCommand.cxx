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
#include "cmAbstractFilesCommand.h"

// cmAbstractFilesCommand
bool cmAbstractFilesCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  cmMakefile::ClassMap &Classes = m_Makefile->GetClasses();
  for(std::vector<std::string>::iterator j = args.begin();
      j != args.end(); ++j)
    {   
    for(cmMakefile::ClassMap::iterator l = Classes.begin(); 
        l != Classes.end(); l++)
      {
      for(std::vector<cmClassFile>::iterator i = l->second.begin(); 
          i != l->second.end(); i++)
        {
        if(i->m_ClassName == (*j))
          {
          i->m_AbstractClass = true;
          }
        }
      }
    }
  return true;
}

