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
#include "cmElseCommand.h"
#include "cmCacheManager.h"

bool cmElseCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // check to see if the argument is defined first
  const char *def = m_Makefile->GetDefinition(args[0].c_str());
  if(def && strcmp(def,"0") && strcmp(def,"false") && strcmp(def,"") && 
     strcmp(def,"NOTFOUND"))
    {
    // add block
    cmIfFunctionBlocker *f = new cmIfFunctionBlocker();
    f->m_Define = args[0];
    m_Makefile->AddFunctionBlocker(f);
    }
  else
    {
    // remove any function blockers for this define
    m_Makefile->RemoveFunctionBlocker("ENDIF",args);
    }
  
  return true;
}

