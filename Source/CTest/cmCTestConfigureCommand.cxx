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
#include "cmCTestConfigureCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

bool cmCTestConfigureCommand::InitialPass(
  std::vector<std::string> const& args)
{
  const char* source_dir = 0;
  const char* build_dir = 0;
  const char* res_var = 0;

  bool havereturn_variable = false;
  bool havesource = false;
  bool havebuild = false;
  for(size_t i=0; i < args.size(); ++i)
    {
    if ( havereturn_variable )
      {
      res_var = args[i].c_str();
      havereturn_variable = false;
      }
    else if ( havebuild )
      {
      build_dir = args[i].c_str();
      havebuild = false;
      }
    else if ( havesource )
      {
      source_dir = args[i].c_str();
      havesource = false;
      }
    else if(args[i] == "RETURN_VALUE")
      {
      if ( res_var )
        {
        this->SetError("called with incorrect number of arguments. RETURN_VALUE specified twice.");
        return false;
        }
      havereturn_variable = true;
      }    
    else if(args[i] == "SOURCE")
      {
      if ( source_dir )
        {
        this->SetError("called with incorrect number of arguments. SOURCE specified twice.");
        return false;
        }
      havesource = true;
      }
    else if(args[i] == "BUILD")
      {
      if ( build_dir )
        {
        this->SetError("called with incorrect number of arguments. BUILD specified twice.");
        return false;
        }
      havebuild = true;
      }
    else
      {
      cmOStringStream str;
      str << "called with incorrect number of arguments. Extra argument is: " << args[i].c_str() << ".";
      this->SetError(str.str().c_str());
      return false;
      }
    }

  if ( source_dir )
    {
    m_CTest->SetCTestConfiguration("SourceDirectory", source_dir);
    }
  else
    {
    source_dir = m_Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY");
    }

  if ( build_dir )
    {
    m_CTest->SetCTestConfiguration("BuildDirectory", build_dir);
    }
  else
    {
    build_dir = m_Makefile->GetDefinition("CTEST_BINARY_DIRECTORY");
    if ( !build_dir )
      {
      this->SetError("Build directory not specified. Either use BUILD argument to CTEST_CONFIGURE command or set CTEST_BINARY_DIRECTORY variable");
      return false;
      }
    }


  const char* ctestConfigureCommand = m_Makefile->GetDefinition("CTEST_CONFIGURE_COMMAND");
  if ( ctestConfigureCommand && *ctestConfigureCommand )
    {
    m_CTest->SetCTestConfiguration("ConfigureCommand", ctestConfigureCommand);
    }
  else
    {
    const char* cmakeGeneratorName = m_Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    if ( cmakeGeneratorName && *cmakeGeneratorName )
      {
      std::string cmakeConfigureCommand = "\"";
      cmakeConfigureCommand += m_CTest->GetCMakeExecutable();
      cmakeConfigureCommand += "\" \"-G";
      cmakeConfigureCommand += cmakeGeneratorName;
      cmakeConfigureCommand += "\" \"";
      cmakeConfigureCommand += source_dir;
      cmakeConfigureCommand += "\"";
      m_CTest->SetCTestConfiguration("ConfigureCommand", cmakeConfigureCommand.c_str());
      }
    else
      {
      this->SetError("Configure command is not specified. If this is a CMake project, specify CTEST_CMAKE_GENERATOR, or if this is not CMake project, specify CTEST_CONFIGURE_COMMAND.");
      return false;
      }
    }

  cmCTestGenericHandler* handler = m_CTest->GetInitializedHandler("configure");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate configure handler");
    return false;
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


