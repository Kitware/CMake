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
#include "cmLoadCacheCommand.h"


// cmLoadcacheCommand
bool cmLoadCacheCommand::InitialPass(std::vector<std::string>& args)
{
  if (args.size()< 1)
    {
    this->SetError("called with wrong number of arguments.");
    }
  
  bool excludeFiles=false;
  unsigned int excludeIndex=0;
  unsigned int i;
  std::set<std::string> excludes;

  for(i=0; i<args.size(); i++)
    {
    if (excludeFiles)
      {
      m_Makefile->ExpandVariablesInString(args[i]);
      excludes.insert(args[i]);
      }
    if (args[i] == "EXCLUDE")
      {
      excludeFiles=true;
      }
    }

  for(i=0; i<args.size(); i++)
    {
    if (args[i] == "EXCLUDE")
      {
      break;
      }
    m_Makefile->ExpandVariablesInString(args[i]);
    cmCacheManager::GetInstance()->LoadCache(args[i].c_str(),false,excludes);
    cmCacheManager::GetInstance()->DefineCache(m_Makefile);
    }


  return true;
}


