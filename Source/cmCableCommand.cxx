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
#include "cmCabilCommand.h"
#include "cmCacheManager.h"

// cmCabilCommand


/**
 * Constructor initializes to empty m_CabilData.
 */
cmCabilCommand::cmCabilCommand(): m_CabilData(0)
{
}


/**
 * Destructor frees the cmCabilData only if this command is its owner.
 */
cmCabilCommand::~cmCabilCommand()
{
  if(m_CabilData && m_CabilData->OwnerIs(this))
    {
    delete m_CabilData;
    }
}


/**
 * Write a CABIL configuration file header.
 */
void cmCabilCommand::WriteConfigurationHeader(std::ostream& os) const
{
  os << "<?xml version=\"1.0\"?>" << std::endl
     << "<CabilConfiguration>" << std::endl;
}


/**
 * Write a CABIL configuration file footer.
 */
void cmCabilCommand::WriteConfigurationFooter(std::ostream& os) const
{
  os << "</CabilConfiguration>" << std::endl;
}


/**
 * Ensure that this cmCabilCommand has a valid m_CabilData pointer.
 */
void cmCabilCommand::SetupCabilData()
{
  // Only do something if the pointer is invalid.
  if(m_CabilData)
    { return; }
  
  // Look through the vector of commands from the makefile.
  const std::vector<cmCommand*>& usedCommands =
    m_Makefile->GetUsedCommands();  
  for(std::vector<cmCommand*>::const_iterator commandIter =
        usedCommands.begin(); commandIter != usedCommands.end(); ++commandIter)
    {
    // If this command is a cmCabilCommand, see if it has a cmCabilData
    // instance.
    cmCabilCommand* command = cmCabilCommand::SafeDownCast(*commandIter);
    if(command)
      { m_CabilData = command->m_CabilData; }
    
    // If we found an instance of cmCabilData, then we are done.
    if(m_CabilData)
      { return; }
    }
  
  // We didn't find another cmCabilCommand with a valid cmCabilData.
  // We must allocate the new cmCabilData ourselves, and with this
  // command as its owner.
  m_CabilData = new cmCabilData(this);
}
