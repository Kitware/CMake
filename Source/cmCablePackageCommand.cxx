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


cmCablePackageCommand::~cmCablePackageCommand()
{
  // If we are the owner of the cmCableData, we must delete it here.
  // For most cmCableCommands, the cmCableCommand destructor will take
  // care of this.  If this package happens to be the last one, and is
  // the owner, then the destructor of cmCableData will call back to a method
  // in this class after the package part of it has been freed!
  if(m_CableData && m_CableData->OwnerIs(this))
    {
    delete m_CableData;
    // Make sure our superclass's destructor doesn't try to delete the
    // cmCableData too.
    m_CableData = NULL;
    }
}

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

  // Tell the makefile that it needs the "cable" utility.  
  m_Makefile->AddUtility("cable");

  // Add custom rules to the makefile to generate this package's source
  // files.
  std::string command = "${CABLE}";
  m_Makefile->ExpandVariablesInString(command);
  std::vector<std::string> depends;
  depends.push_back(command);
  command += " cable_config.xml";
  
  std::vector<std::string> outputs;
  outputs.push_back("Cxx/"+m_PackageName+"_cxx.cxx");
  outputs.push_back("Cxx/"+m_PackageName+"_cxx.h");
  
  // A rule for the package's source files.
  m_Makefile->AddCustomCommand("cable_config.xml",
                               command.c_str(),
                               depends,
                               outputs);
  
  return true;
}


void cmCablePackageCommand::FinalPass()
{
  // Add a rule to build the generated package.
  std::string fileName = "Cxx/"+m_PackageName+"_cxx";
  std::string filePath = m_Makefile->GetStartOutputDirectory();
  cmClassFile file;
  file.m_AbstractClass = false;
  file.m_HeaderFileOnly = false;
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

