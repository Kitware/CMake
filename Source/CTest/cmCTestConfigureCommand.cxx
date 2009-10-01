/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestConfigureCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestConfigureCommand::cmCTestConfigureCommand()
{
  this->Arguments[ctc_OPTIONS] = "OPTIONS";
  this->Arguments[ctc_LAST] = 0;
  this->Last = ctc_LAST;
}

cmCTestGenericHandler* cmCTestConfigureCommand::InitializeHandler()
{
  std::vector<std::string> options;

  if (this->Values[ctc_OPTIONS])
    {
    cmSystemTools::ExpandListArgument(this->Values[ctc_OPTIONS], options);
    }

  if ( this->Values[ct_BUILD] )
    {
    this->CTest->SetCTestConfiguration("BuildDirectory",
      cmSystemTools::CollapseFullPath(
        this->Values[ct_BUILD]).c_str());
    }
  else
    {
    this->CTest->SetCTestConfiguration("BuildDirectory",
      cmSystemTools::CollapseFullPath(
       this->Makefile->GetDefinition("CTEST_BINARY_DIRECTORY")).c_str());
    }

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
      cmakeConfigureCommand += "\"";

      std::vector<std::string>::const_iterator it;
      std::string option;
      for (it= options.begin(); it!=options.end(); ++it)
        {
        option = *it;
        cmakeConfigureCommand += " \"";
        cmakeConfigureCommand += option;
        cmakeConfigureCommand += "\"";
        }

      cmakeConfigureCommand += " \"-G";
      cmakeConfigureCommand += cmakeGeneratorName;
      cmakeConfigureCommand += "\"";

      cmakeConfigureCommand += " \"";
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
