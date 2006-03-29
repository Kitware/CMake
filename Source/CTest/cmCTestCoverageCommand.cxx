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

cmCTestGenericHandler* cmCTestCoverageCommand::InitializeHandler()
{
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CoverageCommand", "CTEST_COVERAGE_COMMAND");

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("coverage");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate test handler");
    return 0;
    }
  return handler;
}



