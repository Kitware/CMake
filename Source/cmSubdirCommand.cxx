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
#include "cmSubdirCommand.h"

// cmSubdirCommand
bool cmSubdirCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  bool res = true;

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    std::string directory = std::string(m_Makefile->GetCurrentDirectory()) + 
      "/" + i->c_str();
    if ( cmSystemTools::FileIsDirectory(directory.c_str()) )
      {
      m_Makefile->AddSubDirectory(i->c_str());
      }
    else
      {
      std::string error = "Incorrect SUBDIRS command. Directory: ";
      error += directory + " does not exists.";
      this->SetError(error.c_str());   
      res = false;
      }
    }
  return res;
}

