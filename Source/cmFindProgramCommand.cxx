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
#include "cmFindProgramCommand.h"
#include <stdlib.h>
#include <stdio.h>
  

// cmFindProgramCommand
bool cmFindProgramCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string> path;
  cmSystemTools::GetPath(path);

  std::vector<std::string>::iterator i = args.begin();
  const char* define = (*i).c_str();
  i++;
  for(; i != args.end(); ++i)
    {
    for(int k=0; k < path.size(); k++)
      {
      std::string tryPath = path[k];
      tryPath += "/";
      tryPath += *i;
#ifdef _WIN32
      tryPath += ".exe";
#endif
      if(cmSystemTools::FileExists(tryPath.c_str()))
        {
        m_Makefile->AddDefinition(define, tryPath.c_str());
        return true;
        }
      }
    }
  return false;
}

