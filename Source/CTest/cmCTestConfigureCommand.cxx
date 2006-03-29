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

cmCTestGenericHandler* cmCTestConfigureCommand::InitializeHandler()
{
  if ( this->Values[ct_BUILD] )
    {
    this->CTest->SetCTestConfiguration("BuildDirectory",
      this->Values[ct_BUILD]);
    }
  else
    {
    this->CTest->SetCTestConfiguration("BuildDirectory",
      this->Makefile->GetDefinition("CTEST_BINARY_DIRECTORY"));
    }
  if ( this->Values[ct_SOURCE] )
    {
    this->CTest->SetCTestConfiguration("SourceDirectory",
      this->Values[ct_SOURCE]);
    }
  else
    {
    this->CTest->SetCTestConfiguration("SourceDirectory",
      this->Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY"));
    }
  if ( this->CTest->GetCTestConfiguration("BuildDirectory").empty() )
    {
    this->SetError("Build directory not specified. Either use BUILD "
      "argument to CTEST_CONFIGURE command or set CTEST_BINARY_DIRECTORY "
      "variable");
    return 0;
    }

  const char* ctestConfigureCommand
    = this->Makefile->GetDefinition("CTEST_CONFIGURE_COMMAND");
  if ( ctestConfigureCommand && *ctestConfigureCommand )
    {
    this->CTest->SetCTestConfiguration("ConfigureCommand",
      ctestConfigureCommand);
    }
  else
    {
    const char* cmakeGeneratorName
      = this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    if ( cmakeGeneratorName && *cmakeGeneratorName )
      {
      const std::string& source_dir
        = this->CTest->GetCTestConfiguration("SourceDirectory");
      if ( source_dir.empty() )
        {
        this->SetError("Source directory not specified. Either use SOURCE "
          "argument to CTEST_CONFIGURE command or set CTEST_SOURCE_DIRECTORY "
          "variable");
        return 0;
        }
      std::string cmakeConfigureCommand = "\"";
      cmakeConfigureCommand += this->CTest->GetCMakeExecutable();
      cmakeConfigureCommand += "\" \"-G";
      cmakeConfigureCommand += cmakeGeneratorName;
      cmakeConfigureCommand += "\" \"";
      cmakeConfigureCommand += source_dir;
      cmakeConfigureCommand += "\"";
      this->CTest->SetCTestConfiguration("ConfigureCommand",
        cmakeConfigureCommand.c_str());
      }
    else
      {
      this->SetError("Configure command is not specified. If this is a CMake "
        "project, specify CTEST_CMAKE_GENERATOR, or if this is not CMake "
        "project, specify CTEST_CONFIGURE_COMMAND.");
      return 0;
      }
    }

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("configure");
  if ( !handler )
    {
    this->SetError(
      "internal CTest error. Cannot instantiate configure handler");
    return 0;
    }
  return handler;
}


