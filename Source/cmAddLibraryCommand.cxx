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
  bool excludeFromAll = false;
  
  std::vector<std::string>::const_iterator s = args.begin();

  this->LibName = *s;

  ++s;
  
  // If the second argument is "SHARED" or "STATIC", then it controls
  // the type of library.  Otherwise, it is treated as a source or
  // source list name. There man be two keyword arguments, check for them
  while ( s != args.end() )
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
    else if(*s == "EXCLUDE_FROM_ALL")
      {
      ++s;
      excludeFromAll = true;
      }
    else
      {
      break;
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

  /* ideally we should check whether for the linker language of the target 
    CMAKE_${LANG}_CREATE_SHARED_LIBRARY is defined and if not default to
    STATIC. But at this point we know only the name of the target, but not 
    yet its linker language. */
  if ((shared != 0) && 
       (this->Makefile->IsOn("CMAKE_TARGET_SUPPORTS_ONLY_STATIC_LIBS")))
    {
    std::string msg = "ADD_LIBRARY for library ";
    msg += args[0];
    msg += " is used with the SHARED or MODULE option, but the target "
        "platform supports only STATIC libraries. Building it STATIC instead. "
        "This may lead to problems.";
    cmSystemTools::Message(msg.c_str() ,"Warning");
    shared = 0;
    }

  std::vector<std::string> srclists;
  while (s != args.end()) 
    {
    srclists.push_back(*s);  
    ++s;
    }

  this->Makefile->AddLibrary(this->LibName.c_str(), shared, srclists,
                             excludeFromAll);
  
  return true;
}


