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
  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();
  std::vector<std::string>::const_iterator j = args.begin();
  std::string flags = *j;
  ++j;
  for(;j != args.end(); ++j)
    {   
    for(cmMakefile::SourceMap::iterator l = Classes.begin(); 
        l != Classes.end(); l++)
      {
      for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
          i != l->second.end(); i++)
        {
        if(i->GetSourceName() == (*j))
          {
          i->SetCompileFlags(flags.c_str());
          }
        }
      }
    }
  return true;
}

