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
  // cmCablePackageCommand's WritePackageFooter().  This will call
  // this cmCablePackageCommand's WritePackageHeader().
  m_CableData->BeginPackage(this);
  
  // Add custom rules to the makefile to generate this package's source
  // files.
  std::string command = "${CABLE}";
  m_Makefile->ExpandVariablesInString(command);  
  std::vector<std::string> depends;  
  depends.push_back(command);
  command += " cable_config.xml";
  
  std::string packageFile = "Cxx/"+m_PackageName+"_cxx";
  std::string packageHeader = packageFile+".h";
  std::string packageSource = packageFile+".cxx";
  
  // A rule for the package's header file.
  m_Makefile->AddCustomCommand("cable_config.xml",
                               packageHeader.c_str(),
                               command.c_str(),
                               depends);
  
  // A rule for the package's source file.
  m_Makefile->AddCustomCommand("cable_config.xml",
                               packageSource.c_str(),
                               command.c_str(),
                               depends);
  
  return true;
}


void cmCablePackageCommand::FinalPass()
{
  // Add a rule to build the generated package.
  std::string fileName = "Cxx/"+m_PackageName+"_cxx";
  std::string filePath = m_Makefile->GetStartOutputDirectory();
  cmClassFile file;
  file.m_AbstractClass = false;
  file.SetName(fileName.c_str(), filePath.c_str(), "cxx", false);
  
  m_CableData->SetPackageClassIndex(m_Makefile->GetClasses().size());
  m_Makefile->AddClass(file);
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

