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
#include "cmAddLibraryCommand.h"

// cmLibraryCommand
bool cmAddLibraryCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // Library type defaults to value of BUILD_SHARED_LIBS, if it exists,
  // otherwise it defaults to static library.
  int shared = 
    !cmSystemTools::IsOff(this->Makefile->GetDefinition("BUILD_SHARED_LIBS"));
  
  std::vector<std::string>::const_iterator s = args.begin();

  this->LibName = *s;

  ++s;
  
  // If the second argument is "SHARED" or "STATIC", then it controls
  // the type of library.  Otherwise, it is treated as a source or
  // source list name.
  if(s != args.end())
    {
    std::string libType = *s;
    if(libType == "STATIC")
      {
      ++s;
      shared = 0;
      }
    else if(libType == "SHARED")
      {
      ++s;
      shared = 1;
      }
    else if(libType == "MODULE")
      {
      ++s;
      shared = 2;
      }
    }

  if (s == args.end())
    {
    std::string msg = "You have called ADD_LIBRARY for library ";
    msg += args[0];
    msg += " without any source files. This typically indicates a problem ";
    msg += "with your CMakeLists.txt file";
    cmSystemTools::Message(msg.c_str() ,"Warning");
    }

  std::vector<std::string> srclists;
  while (s != args.end()) 
    {
    srclists.push_back(*s);  
    ++s;
    }

  this->Makefile->AddLibrary(this->LibName.c_str(), shared, srclists);
  
  return true;
}


