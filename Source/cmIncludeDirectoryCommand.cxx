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
#include "cmIncludeDirectoryCommand.h"
#include "cmCacheManager.h"
// cmIncludeDirectoryCommand
bool cmIncludeDirectoryCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  std::vector<std::string>::const_iterator i = args.begin();

  bool before = false;
  if ((*i) == "BEFORE")
    {
    before = true;
    ++i;
    }

  for(; i != args.end(); ++i)
    {
    m_Makefile->AddIncludeDirectory((*i).c_str(), before);
    }
  return true;
}

