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
  
  for( unsigned int i=0; i< args.size(); i++)
    {
      m_Makefile->ExpandVariablesInString( args[i]);
      cmCacheManager::GetInstance()->LoadCache(args[i].c_str(),false);
      cmCacheManager::GetInstance()->DefineCache(m_Makefile);
    }

  return true;
}


