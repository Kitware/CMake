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
#include "cmGetFilenameComponentCommand.h"
#include "cmSystemTools.h"

// cmGetFilenameComponentCommand
bool cmGetFilenameComponentCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Check and see if the value has been stored in the cache
  // already, if so use that value
  if(args.size() == 4 && args[3] == "CACHE")
    {
    const char* cacheValue = m_Makefile->GetDefinition(args[0].c_str());
    if(cacheValue && strcmp(cacheValue, "NOTFOUND"))
      {
      return true;
      }
    }
  
  std::string result;
  std::string filename = args[1];
  m_Makefile->ExpandVariablesInString(filename);

  if (args[2] == "PATH")
    {
    result = cmSystemTools::GetFilenamePath(filename);
    }
  else if (args[2] == "NAME")
    {
    result = cmSystemTools::GetFilenameName(filename);
    }
  else if (args[2] == "EXT")
    {
    result = cmSystemTools::GetFilenameExtension(filename);
    }
  else if (args[2] == "NAME_WE")
    {
    result = cmSystemTools::GetFilenameNameWithoutExtension(filename);
    }
  else 
    {
    std::string err = "unknow component " + args[2];
    this->SetError(err.c_str());
    return false;
    }

  if(args.size() == 4 && args[3] == "CACHE")
    {
    m_Makefile->AddCacheDefinition(args[0].c_str(),
                                   result.c_str(),
                                   "",
                                   args[2] == "PATH" ? cmCacheManager::FILEPATH
                                                     : cmCacheManager::STRING);
    }
  else 
    {
    m_Makefile->AddDefinition(args[0].c_str(), result.c_str());
    }

  return true;
}

