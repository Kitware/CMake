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
  for(std::vector<std::string>::iterator j = args.begin();
      j != args.end(); ++j)
    {   
    std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
    for(int i = 0; i < Classes.size(); i++)
      {
      if(Classes[i].m_ClassName == (*j))
        {
        Classes[i].m_AbstractClass = true;
        break;
        }
      }
    }
  return true;
}

