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
#include "cmCableCloseNamespaceCommand.h"
#include "cmCacheManager.h"


// cmCableCloseNamespaceCommand
bool cmCableCloseNamespaceCommand::Invoke(std::vector<std::string>& args)
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
  
  // Ask the cable data to close the namespace.
  m_CableData->CloseNamespace(m_NamespaceName);

  // Write the configuration for this command.
  this->WriteNamespaceFooter();  
  
  return true;
}


/**
 * Generate a CABLE Namespace close tag.
 */
void cmCableCloseNamespaceCommand::WriteNamespaceFooter() const
{
  m_CableData->Unindent();
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  os << indent << "</Namespace> <!-- \"" << m_NamespaceName.c_str()
     << "\" -->" << std::endl;
}
