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
#include "cmSourceFilesCommand.h"

// cmSourceFilesCommand
bool cmSourceFilesCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string name = args[0];
  m_Makefile->ExpandVariablesInString(name);
  
  int generated = 0;

  for(std::vector<std::string>::const_iterator i = (args.begin() + 1);
      i != args.end(); ++i)
    {
    std::string copy = *i;
    // Keyword GENERATED in the source file list means that 
    // from here on files will be generated
    if ( copy == "GENERATED" )
      {
      generated = 1;
      continue;
      }
    cmSourceFile file;
    m_Makefile->ExpandVariablesInString(copy);
    file.SetIsAnAbstractClass(false);
    std::string path = cmSystemTools::GetFilenamePath(copy);
    if ( generated )
      {
      // This file will be generated, so we should not check
      // if it exist. 
      std::string ext = cmSystemTools::GetFilenameExtension(copy);
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
      // if this is a full path then 
      if((path.size() && path[0] == '/') ||
	 (path.size() > 1 && path[1] == ':'))
	{
	file.SetName(cmSystemTools::GetFilenameName(copy.c_str()).c_str(), 
		     path.c_str(),
		     m_Makefile->GetSourceExtensions(),
		     m_Makefile->GetHeaderExtensions());
	}
      else
	{
	file.SetName(copy.c_str(), m_Makefile->GetCurrentDirectory(),
		     m_Makefile->GetSourceExtensions(),
		     m_Makefile->GetHeaderExtensions());
	}    
    m_Makefile->AddSource(file, name.c_str());
    }

  return true;
}

