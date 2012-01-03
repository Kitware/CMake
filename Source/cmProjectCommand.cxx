/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmProjectCommand.h"

// cmProjectCommand
bool cmProjectCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    } 
  this->Makefile->SetProjectName(args[0].c_str());

  std::string bindir = args[0];
  bindir += "_BINARY_DIR";
  std::string srcdir = args[0];
  srcdir += "_SOURCE_DIR";
  
  this->Makefile->AddCacheDefinition
    (bindir.c_str(),
     this->Makefile->GetCurrentOutputDirectory(),
     "Value Computed by CMake", cmCacheManager::STATIC);
  this->Makefile->AddCacheDefinition
    (srcdir.c_str(),
     this->Makefile->GetCurrentDirectory(),
     "Value Computed by CMake", cmCacheManager::STATIC);
  
  bindir = "PROJECT_BINARY_DIR";
  srcdir = "PROJECT_SOURCE_DIR";

  this->Makefile->AddDefinition(bindir.c_str(),
          this->Makefile->GetCurrentOutputDirectory());
  this->Makefile->AddDefinition(srcdir.c_str(),
          this->Makefile->GetCurrentDirectory());

  this->Makefile->AddDefinition("PROJECT_NAME", args[0].c_str());

  // Set the CMAKE_PROJECT_NAME variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one, so that
  // CMAKE_PROJECT_NAME will match PROJECT_NAME, and cmake --build
  // will work.
  if(!this->Makefile->GetDefinition("CMAKE_PROJECT_NAME")
     || (this->Makefile->GetLocalGenerator()->GetParent() == 0) )
    {
    this->Makefile->AddDefinition("CMAKE_PROJECT_NAME", args[0].c_str());
    this->Makefile->AddCacheDefinition
      ("CMAKE_PROJECT_NAME",
       args[0].c_str(),
       "Value Computed by CMake", cmCacheManager::STATIC);
    }

  std::vector<std::string> languages;
  if(args.size() > 1)
    {
    for(size_t i =1; i < args.size(); ++i)
      {
      languages.push_back(args[i]);
      }
    }
  else
    {
    // if no language is specified do c and c++
    languages.push_back("C");
    languages.push_back("CXX");
    }
  this->Makefile->EnableLanguage(languages, false);
  return true;
}

