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
#include "cmSourceFilesRemoveCommand.h"

// cmSourceFilesRemoveCommand
bool cmSourceFilesRemoveCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (versionValue && atof(versionValue) > 1.2)
    {
    this->SetError("The SOURCE_FILES_REMOVE command has been deprecated in CMake version 1.4. You should use the REMOVE command instead.\n");
    return false;
    }
  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  int generated = 0;
  for(std::vector<std::string>::const_iterator i = (args.begin() + 1);
      i != args.end(); ++i)
    {
    std::string copy = *i;
    if ( copy == "GENERATED" )
      {
      generated = 1;
      continue;
      }
    cmSourceFile file;
    if ( generated )
      {
      // This file will be generated, so we should not check
      // if it exist. 
      std::string ext = cmSystemTools::GetFilenameExtension(copy);
      std::string path = cmSystemTools::GetFilenamePath(copy);
      std::string name_no_ext = cmSystemTools::GetFilenameName(copy.c_str());
      name_no_ext = name_no_ext.substr(0, name_no_ext.length()-ext.length());
      if ( ext.length() && ext[0] == '.' )
	{
	ext = ext.substr(1);
	}
      if((path.size() && path[0] == '/') ||
	 (path.size() > 1 && path[1] == ':'))
	{
	file.SetName(name_no_ext.c_str(), path.c_str(), ext.c_str(), false);
	}
      else
	{
	file.SetName(name_no_ext.c_str(), m_Makefile->GetCurrentOutputDirectory(), 
		     ext.c_str(), false);
	}
      }
    else
      {
      file.SetName((*i).c_str(), m_Makefile->GetCurrentDirectory(),
		   m_Makefile->GetSourceExtensions(),
		   m_Makefile->GetHeaderExtensions());
      }
    m_Makefile->RemoveSource(file, args[0].c_str());
    }
  return true;
}

