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
#include "cmCableCommand.h"
#include "cmCacheManager.h"

// cmCableCommand


/**
 * Constructor initializes to empty m_CableData.
 */
cmCableCommand::cmCableCommand(): m_CableData(0)
{
}


/**
 * Destructor frees the cmCableData only if this command is its owner.
 */
cmCableCommand::~cmCableCommand()
{
  if(m_CableData && m_CableData->OwnerIs(this))
    {
    delete m_CableData;
    }
}


/**
 * Ensure that this cmCableCommand has a valid m_CableData pointer.
 */
void cmCableCommand::SetupCableData()
{
  // Only do something if the pointer is invalid.
  if(m_CableData)
    { return; }
  
  // Look through the vector of commands from the makefile.
  const std::vector<cmCommand*>& usedCommands =
    m_Makefile->GetUsedCommands();  
  for(std::vector<cmCommand*>::const_iterator commandIter =
        usedCommands.begin(); commandIter != usedCommands.end(); ++commandIter)
    {
    // If this command is a cmCableCommand, see if it has a cmCableData
    // instance.
    cmCableCommand* command = cmCableCommand::SafeDownCast(*commandIter);
    if(command)
      { m_CableData = command->m_CableData; }
    
    // If we found an instance of cmCableData, then we are done.
    if(m_CableData)
      { return; }
    }
  
  // We didn't find another cmCableCommand with a valid cmCableData.
  // We must allocate the new cmCableData ourselves, and with this
  // command as its owner.
  std::string pathName = m_Makefile->GetStartOutputDirectory();
  pathName += "/cable_config.xml";
  m_CableData = new cmCableData(this, pathName);
  
//  std::vector<std::string> depends;
//  depends.push_back("cable_config.xml");
//  m_Makefile->AddCustomCommand("source_cable_config.xml",
//                               "result_file",
//                               "cable cable_config.xml",
//                               depends);
}
