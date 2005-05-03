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
#include "cmCTestSubmitCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

bool cmCTestSubmitCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if (args.size() != 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* res_var = args[0].c_str();

  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "DropMethod", "CTEST_DROP_METHOD");
  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "DropSite", "CTEST_DROP_SITE");
  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "DropLocation", "CTEST_DROP_LOCATION");
  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "DropSiteUser", "CTEST_DROP_SITE_USER");
  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "DropSitePassword", "CTEST_DROP_SITE_PASSWORD");
  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "TriggerSite", "CTEST_TRIGGER_SITE");
  m_CTest->SetDartConfigurationFromCMakeVariable(m_Makefile, "ScpCommand", "CTEST_SCP_COMMAND");

  cmCTestGenericHandler* handler = m_CTest->GetHandler("submit");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate submit handler");
    return false;
    }
  int res = handler->ProcessHandler();
  cmOStringStream str;
  str << res;
  m_Makefile->AddDefinition(res_var, str.str().c_str());
  return true;
}


