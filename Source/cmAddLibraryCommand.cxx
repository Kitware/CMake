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
#include "cmAddLibraryCommand.h"
#include "cmCacheManager.h"

// cmLibraryCommand
bool cmAddLibraryCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  // Library type defaults to value of BUILD_SHARED_LIBS, if it exists,
  // otherwise it defaults to static library.
  int shared = !cmSystemTools::IsOff(m_Makefile->GetDefinition("BUILD_SHARED_LIBS"));
  
  std::vector<std::string>::const_iterator s = args.begin();

  m_LibName = *s;

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

  std::vector<std::string> srclists;
  while (s != args.end()) 
    {
    srclists.push_back(*s);  
    ++s;
    }

  m_Makefile->AddLibrary(m_LibName.c_str(), shared, srclists);
  
  return true;
}


