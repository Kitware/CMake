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
#include "cmFindProgramCommand.h"
#include "cmCacheManager.h"
#include <stdlib.h>
  
cmFindProgramCommand::cmFindProgramCommand()
{
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "FIND_XXX", "FIND_PROGRAM");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_PATH", "CMAKE_PROGRAM_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SYSTEM", "");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_PATH", "CMAKE_SYSTEM_PROGRAM_PATH"); 
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX_DESC", "program");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX", "program");
}

// cmFindProgramCommand
bool cmFindProgramCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  this->VariableDocumentation = "Path to a program.";
  this->CMakePathName = "PROGRAM";
  // call cmFindBase::ParseArguments
  if(!this->ParseArguments(argsIn))
    {
    return false;
    }
  if(this->AlreadyInCache)
    {
    return true;
    }
  std::string result = cmSystemTools::FindProgram(this->Names,
                                                  this->SearchPaths);
  if(result != "")
    {
    // Save the value in the cache
    m_Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                   result.c_str(),
                                   this->VariableDocumentation.c_str(),
                                   cmCacheManager::FILEPATH);
    
    return true;
    }
  m_Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                 (this->VariableName + "-NOTFOUND").c_str(),
                                 this->VariableDocumentation.c_str(),
                                 cmCacheManager::FILEPATH);
  return true;
}

