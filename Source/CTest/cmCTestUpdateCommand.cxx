/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestUpdateCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestGenericHandler* cmCTestUpdateCommand::InitializeHandler()
{
  if ( this->Values[ct_SOURCE] )
    {
    this->CTest->SetCTestConfiguration("SourceDirectory",
      cmSystemTools::CollapseFullPath(
        this->Values[ct_SOURCE]).c_str());
    }
  else
    {
    this->CTest->SetCTestConfiguration("SourceDirectory",
      cmSystemTools::CollapseFullPath(
        this->Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY")).c_str());
    }
  std::string source_dir
    = this->CTest->GetCTestConfiguration("SourceDirectory");

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "UpdateCommand", "CTEST_UPDATE_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "UpdateOptions", "CTEST_UPDATE_OPTIONS");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CVSCommand", "CTEST_CVS_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CVSUpdateOptions", "CTEST_CVS_UPDATE_OPTIONS");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "SVNCommand", "CTEST_SVN_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "SVNUpdateOptions", "CTEST_SVN_UPDATE_OPTIONS");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "BZRCommand", "CTEST_BZR_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "BZRUpdateOptions", "CTEST_BZR_UPDATE_OPTIONS");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "GITCommand", "CTEST_GIT_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "GITUpdateOptions", "CTEST_GIT_UPDATE_OPTIONS");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "HGCommand", "CTEST_HG_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "HGUpdateOptions", "CTEST_HG_UPDATE_OPTIONS");

  const char* initialCheckoutCommand
    = this->Makefile->GetDefinition("CTEST_CHECKOUT_COMMAND");
  if ( !initialCheckoutCommand )
    {
    initialCheckoutCommand = 
      this->Makefile->GetDefinition("CTEST_CVS_CHECKOUT");
    }

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("update");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate update handler");
    return 0;
    }
  handler->SetCommand(this);
  if ( source_dir.empty() )
    {
    this->SetError("source directory not specified. Please use SOURCE tag");
    return 0;
    }
  handler->SetOption("SourceDirectory", source_dir.c_str());
  if ( initialCheckoutCommand )
    {
    handler->SetOption("InitialCheckout", initialCheckoutCommand);
    }
  if ( (!cmSystemTools::FileExists(source_dir.c_str()) ||
      !cmSystemTools::FileIsDirectory(source_dir.c_str()))
    && !initialCheckoutCommand )
    {
    cmOStringStream str;
    str << "cannot find source directory: " << source_dir.c_str() << ".";
    if ( !cmSystemTools::FileExists(source_dir.c_str()) )
      {
      str << " Looks like it is not checked out yet. Please specify "
        "CTEST_CHECKOUT_COMMAND.";
      }
    this->SetError(str.str().c_str());
    return 0;
    }
  return handler;
}


