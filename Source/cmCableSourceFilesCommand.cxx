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
#include "cmCableSourceFilesCommand.h"
#include "cmCacheManager.h"

void cmCableSourceFilesCommand::FinalPass()
{
  // Get the index of the current package's cmClassFile.
  // If it doesn't exist, ignore this command.
  int index = m_CableData->GetPackageClassIndex();
  if(index < 0)
    { return; }
  
  // The package's file has not yet been generated yet.  The dependency
  // finder will need hints.  Add one for each source file.
  cmClassFile& cFile = m_Makefile->GetClasses()[index];
  
  std::string curPath = m_Makefile->GetCurrentDirectory();
  curPath += "/";
  
  for(Entries::const_iterator f = m_Entries.begin();
      f != m_Entries.end(); ++f)
    {
    std::string header = curPath+*f+".h";
    cFile.m_Depends.push_back(header);
    }
}


/**
 * Write the CABLE configuration code to indicate header dependencies for
 * a package.
 */
void cmCableSourceFilesCommand::WriteConfiguration() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  
  cmRegularExpression needCdataBlock("[&<>]");
  
  // Look for the files on a path relative to the current CMakeLists.txt.
  std::string curPath = m_Makefile->GetCurrentDirectory();
  curPath += "/";
  
  os << indent << "<Headers>" << std::endl;
  for(Entries::const_iterator f = m_Entries.begin();
      f != m_Entries.end(); ++f)
    {
    std::string file = curPath+*f;
    
    // Look for the normal include file.
    std::string header = file+".h";
    if(cmSystemTools::FileExists(header.c_str()))
      {
      os << indent << "  <File name=\"" << header.c_str() << "\"/>"
         << std::endl;
      }
    else
      {
      cmSystemTools::Error("Unable to find source file ", header.c_str());
      }
    
    // Look for an instantiation file.
    std::string instantiation = file+".txx";
    if(cmSystemTools::FileExists(instantiation.c_str()))
      {
      os << indent << "  <File name=\"" << instantiation.c_str()
         << "\" purpose=\"instantiate\"/>" << std::endl;
      }
    }
  os << indent << "</Headers>" << std::endl;
}
