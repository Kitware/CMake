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
  
  // Cache entries to be excluded from the import list.
  // If this set is empty, all cache entries are brought in
  // and they can not be overridden.
  bool excludeFiles=false;
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
    if (excludeFiles && (args[i] == "INCLUDE_INTERNALS"))
      {
      break;
      }
    }

  // Internal cache entries to be imported.
  // If this set is empty, no internal cache entries are
  // brought in.
  bool includeFiles=false;
  std::set<std::string> includes;

  for(i=0; i<args.size(); i++)
    {
    if (includeFiles)
      {
      m_Makefile->ExpandVariablesInString(args[i]);
      includes.insert(args[i]);
      }
    if (args[i] == "INCLUDE_INTERNALS")
      {
      includeFiles=true;
      }
    if (includeFiles && (args[i] == "EXCLUDE"))
      {
      break;
      }
    }

  for(i=0; i<args.size(); i++)
    {
    if ((args[i] == "EXCLUDE") || (args[i] == "INCLUDE_INTERNALS"))
      {
      break;
      }
    m_Makefile->ExpandVariablesInString(args[i]);
    cmCacheManager::GetInstance()->LoadCache(args[i].c_str(), false,
					     excludes, includes);
    cmCacheManager::GetInstance()->DefineCache(m_Makefile);
    }


  return true;
}


