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
#include "cmCTestCoverageHandler.h"

cmCTestGenericHandler* cmCTestCoverageCommand::InitializeHandler()
{
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CoverageCommand", "CTEST_COVERAGE_COMMAND");

  cmCTestCoverageHandler* handler = static_cast<cmCTestCoverageHandler*>(
    this->CTest->GetInitializedHandler("coverage"));
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate test handler");
    return 0;
    }

  // If a LABELS option was given, select only files with the labels.
  if(this->LabelsMentioned)
    {
    handler->SetLabelFilter(this->Labels);
    }

  return handler;
}

//----------------------------------------------------------------------------
bool cmCTestCoverageCommand::CheckArgumentKeyword(std::string const& arg)
{
  // Look for arguments specific to this command.
  if(arg == "LABELS")
    {
    this->ArgumentDoing = ArgumentDoingLabels;
    this->LabelsMentioned = true;
    return true;
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentKeyword(arg);
}

//----------------------------------------------------------------------------
bool cmCTestCoverageCommand::CheckArgumentValue(std::string const& arg)
{
  // Handle states specific to this command.
  if(this->ArgumentDoing == ArgumentDoingLabels)
    {
    this->Labels.insert(arg);
    return true;
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentValue(arg);
}
