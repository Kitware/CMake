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
  if (args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* source_dir = args[0].c_str();
  const char* res_var = args[1].c_str();

  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "CVSCommand", "CTEST_CVS_COMMAND");
  m_CTest->SetCTestConfigurationFromCMakeVariable(m_Makefile, "SVNCommand", "CTEST_SVN_COMMAND");

  cmCTestGenericHandler* handler = m_CTest->GetHandler("update");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate update handler");
    return false;
    }
  handler->SetOption("SourceDirectory", source_dir);
  int res = handler->ProcessHandler();
  cmOStringStream str;
  str << res;
  m_Makefile->AddDefinition(res_var, str.str().c_str());
  return true;
}


