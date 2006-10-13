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
bool cmIncludeDirectoryCommand
::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    return true;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  bool before = this->Makefile->IsOn("CMAKE_INCLUDE_DIRECTORIES_BEFORE");
  bool system = false;

  if ((*i) == "BEFORE")
    {
    before = true;
    ++i;
    }
  else if ((*i) == "AFTER")
    {
    before = false;
    ++i;
    }

  for(; i != args.end(); ++i)
    {
    if(*i == "SYSTEM")
      {
      system = true;
      continue;
      }
    if(i->size() == 0)
      {
      cmSystemTools::Error
        ("Empty Include Directory Passed into INCLUDE_DIRECTORIES command.");
      }
    std::string unixPath = *i;
    if (!cmSystemTools::IsOff(unixPath.c_str()))
      {
    cmSystemTools::ConvertToUnixSlashes(unixPath);
    if(!cmSystemTools::FileIsFullPath(unixPath.c_str()))
      {
      std::string tmp = this->Makefile->GetStartDirectory();
      tmp += "/";
      tmp += unixPath;
      unixPath = tmp;
      }
      }
    this->Makefile->AddIncludeDirectory(unixPath.c_str(), before);
    if(system)
      {
      this->Makefile->AddSystemIncludeDirectory(unixPath.c_str());
      }
    }
  return true;
}

