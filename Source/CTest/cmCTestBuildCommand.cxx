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
#include "cmCTestBuildCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmake.h"
#include "cmGlobalGenerator.h"


//----------------------------------------------------------------------------
cmCTestBuildCommand::cmCTestBuildCommand()
{
  this->GlobalGenerator = 0;
}

//----------------------------------------------------------------------------
cmCTestBuildCommand::~cmCTestBuildCommand()
{
  if ( this->GlobalGenerator )
    {
    delete this->GlobalGenerator;
    this->GlobalGenerator = 0;
    }
}

//----------------------------------------------------------------------------
cmCTestGenericHandler* cmCTestBuildCommand::InitializeHandler()
{
  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("build");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate build handler");
    return false;
    }

  const char* ctestBuildCommand
    = this->Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if ( ctestBuildCommand && *ctestBuildCommand )
    {
    this->CTest->SetCTestConfiguration("MakeCommand", ctestBuildCommand);
    }
  else
    {
    const char* cmakeGeneratorName
      = this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    const char* cmakeProjectName
      = this->Makefile->GetDefinition("CTEST_PROJECT_NAME");
    const char* cmakeBuildConfiguration
      = this->Makefile->GetDefinition("CTEST_BUILD_CONFIGURATION");
    const char* cmakeBuildAdditionalFlags
      = this->Makefile->GetDefinition("CTEST_BUILD_FLAGS");
    if ( cmakeGeneratorName && *cmakeGeneratorName &&
      cmakeProjectName && *cmakeProjectName )
      {
      if ( !cmakeBuildConfiguration )
        {
        cmakeBuildConfiguration = "Release";
        }
      if ( this->GlobalGenerator )
        {
        if ( strcmp(this->GlobalGenerator->GetName(),
            cmakeGeneratorName) != 0 )
          {
          delete this->GlobalGenerator;
          this->GlobalGenerator = 0;
          }
        }
      if ( !this->GlobalGenerator )
        {
        this->GlobalGenerator =
          this->Makefile->GetCMakeInstance()->CreateGlobalGenerator(
            cmakeGeneratorName);
        }
      this->GlobalGenerator->FindMakeProgram(this->Makefile);
      const char* cmakeMakeProgram
        = this->Makefile->GetDefinition("CMAKE_MAKE_PROGRAM");
      std::string buildCommand
        = this->GlobalGenerator->GenerateBuildCommand(cmakeMakeProgram,
          cmakeProjectName,
          cmakeBuildAdditionalFlags, 0, cmakeBuildConfiguration, true);
      this->CTest->SetCTestConfiguration("MakeCommand", buildCommand.c_str());
      }
    else
      {
      cmOStringStream ostr;
      ostr << "CTEST_BUILD_COMMAND or CTEST_CMAKE_GENERATOR not specified. "
        "Please specify the CTEST_CMAKE_GENERATOR and CTEST_PROJECT_NAME if "
        "this is a CMake project, or specify the CTEST_BUILD_COMMAND for "
        "cmake or any other project.";
      this->SetError(ostr.str().c_str());
      return false;
      }
    }

  return handler;
}


