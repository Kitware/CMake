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
#include "cmCablePackageCommand.h"
#include "cmCacheManager.h"

// cmCablePackageCommand
bool cmCablePackageCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() != 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // This command needs to access the Cable data.
  this->SetupCableData();
  
  // The argument is the package name.
  m_PackageName = args[0];
  
  // Ask the cable data to begin the package.  This may call another
  // cmCablePackageCommand's WritePackageFooter().
  m_CableData->BeginPackage(this);
  
  // Write the configuration for this command.
  // The cmCableData::EndPackage() later on will call WritePackageFooter().
  this->WritePackageHeader();
  
  return true;
}


/**
 * Write a CABLE package header.
 */
void cmCablePackageCommand::WritePackageHeader() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  os << indent << "<Package name=\"" << m_PackageName.c_str() << "\">"
     << std::endl;
  m_CableData->Indent();
}


/**
 * Write a CABLE package footer.
 */
void cmCablePackageCommand::WritePackageFooter() const
{
  m_CableData->Unindent();
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  os << indent << "</Package> <!-- \"" << m_PackageName.c_str() << "\" -->"
     << std::endl;
}

