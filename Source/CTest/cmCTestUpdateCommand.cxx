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
#include "cmCTestUpdateCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

bool cmCTestUpdateCommand::InitialPass(
  std::vector<std::string> const& args)
{
  const char* source_dir = 0;
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
      source_dir = args[i].c_str();
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
    else if(args[i] == "SOURCE")
      {
      if ( source_dir )
        {
        this->SetError("called with incorrect number of arguments. SOURCE "
          "specified twice.");
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

  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile,
    "UpdateCommand", "CTEST_UPDATE_COMMAND");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile,
    "UpdateOptions", "CTEST_UPDATE_OPTIONS");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile,
    "CVSCommand", "CTEST_CVS_COMMAND");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile,
    "CVSUpdateOptions", "CTEST_CVS_UPDATE_OPTIONS");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile,
    "SVNCommand", "CTEST_SVN_COMMAND");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile,
    "SVNUpdateOptions", "CTEST_SVN_UPDATE_OPTIONS");

  const char* initialCheckoutCommand
    = m_Makefile->GetDefinition("CTEST_CHECKOUT_COMMAND");
  if ( !initialCheckoutCommand )
    {
    initialCheckoutCommand = m_Makefile->GetDefinition("CTEST_CVS_CHECKOUT");
    }

  cmCTestGenericHandler* handler = m_CTest->GetInitializedHandler("update");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate update handler");
    return false;
    }
  handler->SetCommand(this);
  if ( !source_dir )
    {
    this->SetError("source directory not specified. Please use SOURCE tag");
    return false;
    }
  if ( initialCheckoutCommand )
    {
    handler->SetOption("InitialCheckout", initialCheckoutCommand);
    }
  if ( (!cmSystemTools::FileExists(source_dir) ||
      !cmSystemTools::FileIsDirectory(source_dir))
    && !initialCheckoutCommand )
    {
    cmOStringStream str;
    str << "cannot find source directory: " << source_dir << ".";
    if ( !cmSystemTools::FileExists(source_dir) )
      {
      str << " Looks like it is not checked out yet. Please specify "
        "CTEST_CHECKOUT_COMMAND.";
      }
    this->SetError(str.str().c_str());
    return false;
    }
  handler->SetOption("SourceDirectory", source_dir);
  int res = handler->ProcessHandler();
  if ( res_var )
    {
    cmOStringStream str;
    str << res;
    m_Makefile->AddDefinition(res_var, str.str().c_str());
    }
  return true;
}


