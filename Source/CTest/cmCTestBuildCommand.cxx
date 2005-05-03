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

bool cmCTestBuildCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if (args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* build_dir = args[0].c_str();
  const char* res_var = args[1].c_str();

  m_CTest->SetCTestConfiguration("BuildDirectory", build_dir);
  cmCTestGenericHandler* handler = m_CTest->GetHandler("build");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate build handler");
    return false;
    }
 
  const char* ctestBuildCommand = m_Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if ( ctestBuildCommand && *ctestBuildCommand )
    {
    m_CTest->SetCTestConfiguration("MakeCommand", ctestBuildCommand);
    }
  else
    {
    const char* cmakeGeneratorName = m_Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    const char* cmakeProjectName = m_Makefile->GetDefinition("CTEST_PROJECT_NAME");
    const char* cmakeBuildConfiguration = m_Makefile->GetDefinition("CTEST_BUILD_CONFIGURATION");
    if ( cmakeGeneratorName && *cmakeGeneratorName &&
      cmakeProjectName && *cmakeProjectName )
      {
      if ( !cmakeBuildConfiguration )
        {
        cmakeBuildConfiguration = "Release";
        }
      cmGlobalGenerator* gen = 
        m_Makefile->GetCMakeInstance()->CreateGlobalGenerator(cmakeGeneratorName);
      gen->FindMakeProgram(m_Makefile);
      const char* cmakeMakeProgram = m_Makefile->GetDefinition("CMAKE_MAKE_PROGRAM");
      std::cout << "CMake Make program is: " << cmakeMakeProgram << std::endl;
      std::string buildCommand = gen->GenerateBuildCommand(cmakeMakeProgram, cmakeProjectName,
        0, cmakeBuildConfiguration, true);

      m_CTest->SetCTestConfiguration("MakeCommand", buildCommand.c_str());
      }
    else
      {
      cmOStringStream ostr;
      ostr << "CTEST_BUILD_COMMAND or CTEST_CMAKE_GENERATOR not specified. Please specify the CTEST_CMAKE_GENERATOR if this is a CMake project, or specify the CTEST_BUILD_COMMAND for cmake or any other project.";
      this->SetError(ostr.str().c_str());
      return false;
      }
    }
  
  int res = handler->ProcessHandler();
  cmOStringStream str;
  str << res;
  m_Makefile->AddDefinition(res_var, str.str().c_str());
  return true;
}


