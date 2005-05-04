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
  const char* res_var = 0;

  bool havereturn_variable = false;
  for(size_t i=0; i < args.size(); ++i)
    {
    if ( havereturn_variable )
      {
      res_var = args[i].c_str();
      havereturn_variable = false;
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
    else
      {
      cmOStringStream str;
      str << "called with incorrect number of arguments. Extra argument is: " << args[i].c_str() << ".";
      this->SetError(str.str().c_str());
      return false;
      }
    }

  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "DropMethod", "CTEST_DROP_METHOD");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "DropSite", "CTEST_DROP_SITE");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "DropLocation", "CTEST_DROP_LOCATION");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "DropSiteUser", "CTEST_DROP_SITE_USER");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "DropSitePassword", "CTEST_DROP_SITE_PASSWORD");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "TriggerSite", "CTEST_TRIGGER_SITE");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "ScpCommand", "CTEST_SCP_COMMAND");

  cmCTestGenericHandler* handler = m_CTest->GetHandler("submit");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate submit handler");
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


