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
#include "cmCableOpenNamespaceCommand.h"
#include "cmCacheManager.h"


// cmCableOpenNamespaceCommand
bool cmCableOpenNamespaceCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() != 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // This command needs to access the Cable data.
  this->SetupCableData();
  
  // The argument is the namespace name.
  m_NamespaceName = args[0];
  
  // Write the configuration for this command.
  this->WriteNamespaceHeader();
  
  // Ask the cable data to open the namespace.
  m_CableData->OpenNamespace(m_NamespaceName);
  
  return true;
}


/**
 * Generate a CABLE Namespace open tag.
 */
void cmCableOpenNamespaceCommand::WriteNamespaceHeader() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  os << indent << "<Namespace name=\"" << m_NamespaceName.c_str()
     << "\">" << std::endl;
  m_CableData->Indent();
}
