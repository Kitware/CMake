/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmWrapExcludeFilesCommand.h"

// cmWrapExcludeFilesCommand
bool cmWrapExcludeFilesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (versionValue && atof(versionValue) > 1.2)
    {
    this->SetError("The WRAP_EXCLUDE_FILES command has been deprecated in CMake version 1.4. You should use the SET_SOURCE_FILES_PROPERTIES command instead.\n");
    return false;
    }

  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args; 
  m_Makefile->ExpandSourceListArguments(argsIn, args, 0);

  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    // if the file is already in the makefile just set properites on it
    cmSourceFile* sf = m_Makefile->GetSource(j->c_str());
    if(sf)
      {
      sf->SetProperty("WRAP_EXCLUDE","1");
      }
    // if file is not already in the makefile, then add it
    else
      { 
      std::string newfile = *j;
      cmSourceFile file; 
      std::string path = cmSystemTools::GetFilenamePath(newfile);
      // set the flags
      file.SetProperty("WRAP_EXCLUDE","1");
      // if this is a full path then 
      if((path.size() && path[0] == '/') ||
         (path.size() > 1 && path[1] == ':'))
        {
        file.SetName(cmSystemTools::GetFilenameName(newfile.c_str()).c_str(), 
                     path.c_str(),
                     m_Makefile->GetSourceExtensions(),
                     m_Makefile->GetHeaderExtensions());
        }
      else
        {
        file.SetName(newfile.c_str(), m_Makefile->GetCurrentDirectory(),
                     m_Makefile->GetSourceExtensions(),
                     m_Makefile->GetHeaderExtensions());
        }    
      // add the source file to the makefile
      m_Makefile->AddSource(file);
      }
    }
  return true;
}

