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
#include "cmWriteFileCommand.h"

// cmLibraryCommand
bool cmWriteFileCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  std::string fileName = *i;
  i++;

  for(;i != args.end(); ++i)
    {
    message += *i;
    }

  std::ofstream file(fileName.c_str(), std::ios::app);
  if ( !file )
    {
    cmSystemTools::Error("Internal CMake error when trying to open file: ",
                         fileName.c_str());
    return false;
    }
  file << message << std::endl;
  file.close();

  return true;
}

