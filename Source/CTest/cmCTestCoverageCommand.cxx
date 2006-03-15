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
#include "cmCTestCoverageCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

bool cmCTestCoverageCommand::InitialPass(
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

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CoverageCommand", "CTEST_COVERAGE_COMMAND");

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("coverage");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate test handler");
    return false;
    }
  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(
    this->CTest->GetCTestConfiguration("BuildDirectory").c_str());
  int res = handler->ProcessHandler();
  if ( res_var )
    {
    cmOStringStream str;
    str << res;
    this->Makefile->AddDefinition(res_var, str.str().c_str());
    }
  cmSystemTools::ChangeDirectory(current_dir.c_str());
  return true;
}



