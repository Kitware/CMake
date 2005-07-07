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
#include "cmConfigureFileCommand.h"

#include <cmsys/RegularExpression.hxx>

// cmConfigureFileCommand
bool cmConfigureFileCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }
  m_InputFile = args[0];
  m_OuputFile = args[1];
  m_CopyOnly = false;
  m_EscapeQuotes = false;

  
  // for CMake 2.0 and earlier CONFIGURE_FILE defaults to the FinalPass,
  // after 2.0 it only does InitialPass
  m_Immediate = false;
  const char* versionValue
    = m_Makefile->GetRequiredDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (atof(versionValue) > 2.0)
    {
    m_Immediate = true;
    }

  
  m_AtOnly = false;
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "COPYONLY")
      {
      m_CopyOnly = true;
      }
    else if(args[i] == "ESCAPE_QUOTES")
      {
      m_EscapeQuotes = true;
      }
    else if(args[i] == "@ONLY")
      {
      m_AtOnly = true;
      }
    else if(args[i] == "IMMEDIATE")
      {
      m_Immediate = true;
      }
    }
  
  // If we were told to copy the file immediately, then do it on the
  // first pass (now).
  if(m_Immediate)
    {
    if ( !this->ConfigureFile() )
      {
      this->SetError("Problem configuring file");
      return false;
      }
    }
  
  return true;
}

void cmConfigureFileCommand::FinalPass()
{
  if(!m_Immediate)
    {
    this->ConfigureFile();
    }
}

int cmConfigureFileCommand::ConfigureFile()
{
  std::string inFile = m_InputFile;
  if (!cmSystemTools::FileIsFullPath(inFile.c_str()))
    {
    inFile = m_Makefile->GetStartDirectory();
    inFile += "/";
    inFile += m_InputFile;
    }
  return m_Makefile->ConfigureFile(inFile.c_str(),
    m_OuputFile.c_str(),
    m_CopyOnly,
    m_AtOnly,
    m_EscapeQuotes);
}


