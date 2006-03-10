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
bool cmCTestBuildCommand::InitialPass(
  std::vector<std::string> const& args)
{
  const char* build_dir = 0;
  const char* res_var = 0;

  bool havereturn_variable = false;
  bool havesource = false;
  for(size_t i=0; i < args.size(); ++i)
    {
    if ( havereturn_variable )
      {
      res_var = args[i].c_str();
      havereturn_variable = false;
      }
    else if ( havesource )
      {
      build_dir = args[i].c_str();
      havesource = false;
      }
    else if(args[i] == "RETURN_VALUE")
      {
      if ( res_var )
        {
        this->SetError("called with incorrect number of arguments. "
          "RETURN_VALUE specified twice.");
        return false;
        }
      havereturn_variable = true;
      }
    else if(args[i] == "BUILD")
      {
      if ( build_dir )
        {
        this->SetError("called with incorrect number of arguments. "
          "BUILD specified twice.");
        return false;
        }
      havesource = true;
      }
    else
      {
      cmOStringStream str;
      str << "called with incorrect number of arguments. Extra argument is: "
        << args[i].c_str() << ".";
      this->SetError(str.str().c_str());
      return false;
      }
    }

  if ( build_dir )
    {
    this->CTest->SetCTestConfiguration("BuildDirectory", build_dir);
    }

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("build");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate build handler");
    return false;
    }

  const char* ctestBuildCommand
    = m_Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if ( ctestBuildCommand && *ctestBuildCommand )
    {
    this->CTest->SetCTestConfiguration("MakeCommand", ctestBuildCommand);
    }
  else
    {
    const char* cmakeGeneratorName
      = m_Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    const char* cmakeProjectName
      = m_Makefile->GetDefinition("CTEST_PROJECT_NAME");
    const char* cmakeBuildConfiguration
      = m_Makefile->GetDefinition("CTEST_BUILD_CONFIGURATION");
    const char* cmakeBuildAdditionalFlags
      = m_Makefile->GetDefinition("CTEST_BUILD_FLAGS");
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
          m_Makefile->GetCMakeInstance()->CreateGlobalGenerator(
            cmakeGeneratorName);
        }
      this->GlobalGenerator->FindMakeProgram(m_Makefile);
      const char* cmakeMakeProgram
        = m_Makefile->GetDefinition("CMAKE_MAKE_PROGRAM");
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

  int res = handler->ProcessHandler();
  if ( res_var )
    {
    cmOStringStream str;
    str << res;
    m_Makefile->AddDefinition(res_var, str.str().c_str());
    }
  return true;
}


