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
#include "cmCablePackageEntryCommand.h"
#include "cmCacheManager.h"

// cmCablePackageEntryCommand
bool cmCablePackageEntryCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // This command instance needs to use the cmCableData instance.
  this->SetupCableData();
  
  // The arguments are the entries to the Pacakge.
  for(std::vector<std::string>::const_iterator arg = args.begin();
      arg != args.end(); ++arg)
    {
    m_Entries.push_back(*arg);
    }  
  
  // Write this command's configuration.
  return this->WriteConfiguration();
}
