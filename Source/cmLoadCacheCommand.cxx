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
#include "cmLoadCacheCommand.h"


// cmLoadcacheCommand
bool cmLoadCacheCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if (argsIn.size()< 1)
    {
    this->SetError("called with wrong number of arguments.");
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  // Cache entries to be excluded from the import list.
  // If this set is empty, all cache entries are brought in
  // and they can not be overridden.
  bool excludeFiles=false;
  unsigned int i;
  std::set<std::string> excludes;

  for(i=0; i<args.size(); i++)
    {
    if (excludeFiles)
      {
      excludes.insert(args[i]);
      }
    if (args[i] == "EXCLUDE")
      {
      excludeFiles=true;
      }
    if (excludeFiles && (args[i] == "INCLUDE_INTERNALS"))
      {
      break;
      }
    }

  // Internal cache entries to be imported.
  // If this set is empty, no internal cache entries are
  // brought in.
  bool includeFiles=false;
  std::set<std::string> includes;

  for(i=0; i<args.size(); i++)
    {
    if (includeFiles)
      {
      includes.insert(args[i]);
      }
    if (args[i] == "INCLUDE_INTERNALS")
      {
      includeFiles=true;
      }
    if (includeFiles && (args[i] == "EXCLUDE"))
      {
      break;
      }
    }

  // Loop over each build directory listed in the arguments.  Each
  // directory has a cache file.
  for(i=0; i<args.size(); i++)
    {
    if ((args[i] == "EXCLUDE") || (args[i] == "INCLUDE_INTERNALS"))
      {
      break;
      }
    m_Makefile->GetCacheManager()->LoadCache(args[i].c_str(), false,
                                             excludes, includes);
    }


  return true;
}


