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
#include "cmSourceFilesRemoveCommand.h"

// cmSourceFilesRemoveCommand
bool cmSourceFilesRemoveCommand::InitialPass(std::vector<std::string> const& args)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (versionValue && atof(versionValue) > 1.2)
    {
    this->SetError("The SOURCE_FILES_REMOVE command has been deprecated in CMake version 1.4. You should use the REMOVE command instead.\n");
    return false;
    }
  
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* variable = args[0].c_str(); // VAR is always first
  // get the old value
  const char* cacheValue
    = m_Makefile->GetDefinition(variable);

  // expand the variable
  std::vector<std::string> varArgsExpanded;
  cmSystemTools::ExpandListArgument(cacheValue, varArgsExpanded);
  
  // expand the args
  // check for REMOVE(VAR v1 v2 ... vn) 
  std::vector<std::string> argsExpanded;
  std::vector<std::string> temp;
  for(unsigned int j = 1; j < args.size(); ++j)
    {
    temp.push_back(args[j]);
    }
  cmSystemTools::ExpandList(temp, argsExpanded);
  
  // now create the new value
  std::string value;
  for(unsigned int j = 0; j < varArgsExpanded.size(); ++j)
    {
    int found = 0;
    for(unsigned int k = 0; k < argsExpanded.size(); ++k)
      {
      if (varArgsExpanded[j] == argsExpanded[k])
        {
        found = 1;
        break;
        }
      }
    if (!found)
      {
      if (value.size())
        {
        value += ";";
        }
      value += varArgsExpanded[j];
      }
    }
  
  // add the definition
  m_Makefile->AddDefinition(variable, value.c_str());

  return true;
}

