/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSourceFilesCommand.h"
#include <stdlib.h> // required for atof

bool cmSourceFilesCommand::IsDeprecated(int major, int minor)
{
  if ( major >= 1 && minor >= 4 )
    {
    this->SetError("The SOURCE_FILES command was deprecated in CMake version 1.4 and will be removed in later versions of CMake. You should modify your CMakeLists.txt files to use the SET command instead, or set the cache value of CMAKE_BACKWARDS_COMPATIBILITY to 1.2 or less.\n");
    return true;
    }
  return false;
}

// cmSourceFilesCommand
bool cmSourceFilesCommand::InitialPass(std::vector<std::string> const& args)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (atof(versionValue) > 1.2)
    {
    cmSystemTools::Message("The SOURCE_FILES command was deprecated in CMake version 1.4 and will be removed in later versions. You should modify your CMakeLists.txt files to use the SET command instead, or set the cache value of CMAKE_BACKWARDS_COMPATIBILITY to 1.2 or less.\n","Warning");
    }

  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string sourceListValue;
  
  // was the list already populated
  std::string name = args[0];
  const char *def = m_Makefile->GetDefinition(name.c_str());  
  if (def)
    {
    sourceListValue = def;
    }
  
  
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
    cmSourceFile* sf = m_Makefile->GetSource(copy.c_str());
    if(sf)
      {
      // if the source file is already in the makefile,
      // then add the pointer to the source list without creating cmSourceFile
      if (sourceListValue.size() > 0)
        {
        sourceListValue += ";";
        }
      sourceListValue += copy;
      continue;
      }
    cmSourceFile file;
    file.SetProperty("ABSTRACT","0");
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
        file.SetName(name_no_ext.c_str(), 
                     m_Makefile->GetCurrentOutputDirectory(), 
                     ext.c_str(), false);
        }
      }
    else
      {
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
      }
    m_Makefile->AddSource(file);
    if (sourceListValue.size() > 0)
      {
      sourceListValue += ";";
      }
    sourceListValue += copy;
    }

  m_Makefile->AddDefinition(name.c_str(), sourceListValue.c_str());  
  return true;
}

