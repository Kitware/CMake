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
#include "cmIfCommand.h"
#include "cmCacheManager.h"

bool cmIfFunctionBlocker::
IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                  const cmMakefile &mf) const
{
  if (strcmp(name,"ELSE") && strcmp(name,"ENDIF"))
    {
    return true;
    }
  if (strcmp(args[0].c_str(),m_Define.c_str()))
    {
    return true;
    }
  return false;
}

bool cmIfFunctionBlocker::
ShouldRemove(const char *name, const std::vector<std::string> &args, 
             const cmMakefile &mf) const
{
  return !this->IsFunctionBlocked(name,args,mf);
}

bool cmIfCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // check to see if the argument is defined first
  const char *def = m_Makefile->GetDefinition(args[0].c_str());
  if(cmSystemTools::IsOn(def))
    {
    // do nothing
    }
  else
    {
    // create a function blocker
    cmIfFunctionBlocker *f = new cmIfFunctionBlocker();
    f->m_Define = args[0];
    m_Makefile->AddFunctionBlocker(f);
    }
  
  return true;
}

