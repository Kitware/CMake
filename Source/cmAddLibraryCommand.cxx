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
#include "cmAddLibraryCommand.h"
#include "cmCacheManager.h"

// cmLibraryCommand
bool cmAddLibraryCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::iterator s = args.begin();
  std::vector<std::string> srclists(++s, args.end());
  
  m_Makefile->AddLibrary(args[0].c_str(),srclists);

  // Add an entry into the cache 
  cmCacheManager::GetInstance()->
    AddCacheEntry(args[0].c_str(),
                  m_Makefile->GetCurrentOutputDirectory(),
                  "Path to a library", cmCacheManager::INTERNAL);
  return true;
}

