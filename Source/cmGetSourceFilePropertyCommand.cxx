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
#include "cmGetSourceFilePropertyCommand.h"

// cmSetSourceFilePropertyCommand
bool cmGetSourceFilePropertyCommand::InitialPass(std::vector<std::string> const& 
                                                 args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* var = args[0].c_str();
  const char* file = args[1].c_str();
  cmSourceFile* sf = m_Makefile->GetSource(file);
  if(sf)
    {
    if(args[2] == "ABSTRACT")
      {
      m_Makefile->AddDefinition(var, sf->IsAnAbstractClass());
      }
    if(args[2] == "WRAP_EXCLUDE")
      {
      m_Makefile->AddDefinition(var, sf->GetWrapExclude());
      }
    if(args[2] == "FLAGS")
      {
      m_Makefile->AddDefinition(var, sf->GetCompileFlags());
      }
    }
  else
    {
    std::string m = "Could not find source file: ";
    m += file;
    this->SetError(m.c_str());
    return false;
    }
  return true;
}

