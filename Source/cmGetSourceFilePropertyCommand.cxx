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
bool cmGetSourceFilePropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() != 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* var = args[0].c_str();
  const char* file = args[1].c_str();
  cmSourceFile* sf = m_Makefile->GetSource(file);

  if(sf)
    {
    const char *prop = sf->GetProperty(args[2].c_str());
    if (prop)
      {
      m_Makefile->AddDefinition(var, prop);
      return true;
      }
    }

  m_Makefile->AddDefinition(var, "NOT_FOUND");
  return true;
}

