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
#include "cmIncludeCommand.h"


// cmIncludeCommand
bool cmIncludeCommand::InitialPass(std::vector<std::string> const& args)
{
  if (args.size()< 1 || args.size() > 2)
    {
      this->SetError("called with wrong number of arguments.  "
                     "Include only takes one file.");
    }
  bool exists = cmSystemTools::FileExists(args[0].c_str());
  if(args.size() == 2 && args[1] == "OPTIONAL" && !exists)
    {
    return true;
    }
  if(!exists)
    {
    std::string error = "Include file not found: " + args[0] + "\n";
    this->SetError(error.c_str());
    return false;
    }
  m_Makefile->ReadListFile( m_Makefile->GetCurrentListFile(), 
                            args[0].c_str());
  return true;
}


