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
#include "cmCTestStartCommand.h"

#include "cmCTest.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

bool cmCTestStartCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if (args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* smodel = args[0].c_str();
  const char* src_dir = 0;
  const char* bld_dir = 0;

  if ( args.size() >= 2 )
    {
    src_dir = args[1].c_str();
    if ( args.size() == 3 )
      {
      bld_dir = args[2].c_str();
      }
    }
  if ( args.size() > 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  if ( !src_dir )
    {
    src_dir = this->Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY");
    }
  if ( !bld_dir)
    {
    bld_dir = this->Makefile->GetDefinition("CTEST_BINARY_DIRECTORY");
    }
  if ( !src_dir )
    {
    this->SetError("source directory not specified. Specify source directory "
      "as an argument or set CTEST_SOURCE_DIRECTORY");
    return false;
    }
  if ( !bld_dir)
    {
    this->SetError("binary directory not specified. Specify binary directory "
      "as an argument or set CTEST_BINARY_DIRECTORY");
    return false;
    }
  this->CTest->EmptyCTestConfiguration();
  this->CTest->SetCTestConfiguration("SourceDirectory", src_dir);
  this->CTest->SetCTestConfiguration("BuildDirectory", bld_dir);

  cmCTestLog(this->CTest, HANDLER_OUTPUT, "Run dashboard with model "
    << smodel << std::endl
    << "   Source directory: " << src_dir << std::endl
    << "   Build directory: " << bld_dir << std::endl);

  this->Makefile->AddDefinition("CTEST_RUN_CURRENT_SCRIPT", "OFF");
  this->CTest->SetSuppressUpdatingCTestConfiguration(true);
  int model = this->CTest->GetTestModelFromString(smodel);
  this->CTest->SetTestModel(model);
  this->CTest->SetProduceXML(true);

  return this->CTest->InitializeFromCommand(this, true);
}


