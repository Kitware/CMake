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
  m_Immediate = false;
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
  return m_Makefile->ConfigureFile(m_InputFile.c_str(),
    m_OuputFile.c_str(),
    m_CopyOnly,
    m_AtOnly,
    m_EscapeQuotes);
}


