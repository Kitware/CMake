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
#include "cmProjectCommand.h"

// cmProjectCommand
bool cmProjectCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    } 
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  m_Makefile->SetProjectName(args[0].c_str());

  std::string bindir = args[0];
  bindir += "_BINARY_DIR";
  std::string srcdir = args[0];
  srcdir += "_SOURCE_DIR";
  
  m_Makefile->AddCacheDefinition(bindir.c_str(),
                                 m_Makefile->GetCurrentOutputDirectory(),
                                 "Value Computed by CMake", cmCacheManager::STATIC);
  m_Makefile->AddCacheDefinition(srcdir.c_str(),
                                 m_Makefile->GetCurrentDirectory(),
                                 "Value Computed by CMake", cmCacheManager::STATIC);
  
  bindir = "PROJECT_BINARY_DIR";
  srcdir = "PROJECT_SOURCE_DIR";

  m_Makefile->AddDefinition(bindir.c_str(),
          m_Makefile->GetCurrentOutputDirectory());
  m_Makefile->AddDefinition(srcdir.c_str(),
          m_Makefile->GetCurrentDirectory());

  m_Makefile->AddDefinition("PROJECT_NAME", args[0].c_str());

  if(args.size() > 1)
    {
    for(size_t i =1; i < args.size(); ++i)
      {
      m_Makefile->EnableLanguage(args[i].c_str());
      }
    }
  else
    {
    m_Makefile->EnableLanguage(0);
    }
  return true;
}

