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
#include "cmSourceFilesFlagsCommand.h"

// cmSourceFilesFlagsCommand
bool cmSourceFilesFlagsCommand::InitialPass(std::vector<std::string> const& 
                                             args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::const_iterator j = args.begin();
  std::string flags = *j;
  ++j;
  for(;j != args.end(); ++j)
    {   
    cmSourceFile* sf = m_Makefile->GetSource(j->c_str());
    if(sf)
      {
      sf->SetCompileFlags(flags.c_str());
      }
    else
      {
      std::string m = "could not find source file ";
      m += *j;
      this->SetError(m.c_str());
      return false;
      }
    }
  return true;
}

