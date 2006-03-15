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
#include "cmCTestMemCheckCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"


cmCTestGenericHandler* cmCTestMemCheckCommand::InitializeActualHandler()
{
  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("memcheck");

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckCommand", "CTEST_MEMORYCHECK_COMMAND");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckCommandOptions", "CTEST_MEMORYCHECK_COMMAND_OPTIONS");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckSuppressionFile", "CTEST_MEMORYCHECK_SUPPRESSIONS_FILE");

  return handler;
}

