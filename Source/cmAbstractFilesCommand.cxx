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
#include "cmAbstractFilesCommand.h"

// cmAbstractFilesCommand
bool cmAbstractFilesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  bool ret = true;
  std::string m = "could not find source file(s):\n";

  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {  
    cmSourceFile* sf = m_Makefile->GetSource(j->c_str());
    if(sf)
      {
      sf->SetIsAnAbstractClass(true);
      }
    else
      {
      // for VTK 4.0 we have to support missing abstract sources
      if(m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION"))
        {
        m += *j;
        m += "\n";
        ret = false;
        } 
      }
    }
  if(!ret)
    {
    this->SetError(m.c_str());
    }
  return ret;
}

