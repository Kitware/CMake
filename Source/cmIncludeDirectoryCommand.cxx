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
#include "cmIncludeDirectoryCommand.h"

// cmIncludeDirectoryCommand
bool cmIncludeDirectoryCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    return true;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  bool before = false;
  if ((*i) == "BEFORE")
    {
    before = true;
    ++i;
    }

  for(; i != args.end(); ++i)
    {
    if(i->size() == 0)
      {
      cmSystemTools::Error("Empty Include Directory Passed into INCLUDE_DIRECTORIES command.");
      }
    std::string unixPath = *i;
    cmSystemTools::ConvertToUnixSlashes(unixPath);
    this->Makefile->AddIncludeDirectory(unixPath.c_str(), before);
    }
  return true;
}

