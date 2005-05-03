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
  if (args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* build_dir = args[0].c_str();
  const char* res_var = args[1].c_str();

  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "ConfigureCommand", "CTEST_CONFIGURE_COMMAND");
  m_CTest->SetCTestConfiguration("BuildDirectory", build_dir);

  cmCTestGenericHandler* handler = m_CTest->GetHandler("configure");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate configure handler");
    return false;
    }
  int res = handler->ProcessHandler();
  cmOStringStream str;
  str << res;
  m_Makefile->AddDefinition(res_var, str.str().c_str());
  return true;
}


