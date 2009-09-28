/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestStartCommand.h"

#include "cmCTest.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

bool cmCTestStartCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if (args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  size_t cnt = 0;
  const char* smodel = args[cnt].c_str();
  const char* src_dir = 0;
  const char* bld_dir = 0;

  cnt++;

  this->CTest->SetSpecificTrack(0);
  if ( cnt < args.size() -1 )
    {
    if ( args[cnt] == "TRACK" )
      {
      cnt ++;
      this->CTest->SetSpecificTrack(args[cnt].c_str());
      cnt ++;
      }
    }

  if ( cnt < args.size() )
    {
    src_dir = args[cnt].c_str();
    cnt ++;
    if ( cnt < args.size() )
      {
      bld_dir = args[cnt].c_str();
      }
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

  cmSystemTools::AddKeepPath(src_dir);
  cmSystemTools::AddKeepPath(bld_dir);

  this->CTest->EmptyCTestConfiguration();
  this->CTest->SetCTestConfiguration("SourceDirectory",
    cmSystemTools::CollapseFullPath(src_dir).c_str());
  this->CTest->SetCTestConfiguration("BuildDirectory",
    cmSystemTools::CollapseFullPath(bld_dir).c_str());

  cmCTestLog(this->CTest, HANDLER_OUTPUT, "Run dashboard with model "
    << smodel << std::endl
    << "   Source directory: " << src_dir << std::endl
    << "   Build directory: " << bld_dir << std::endl);
  const char* track = this->CTest->GetSpecificTrack();
  if ( track )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
      "   Track: " << track << std::endl);
    }

  this->Makefile->AddDefinition("CTEST_RUN_CURRENT_SCRIPT", "OFF");
  this->CTest->SetSuppressUpdatingCTestConfiguration(true);
  int model = this->CTest->GetTestModelFromString(smodel);
  this->CTest->SetTestModel(model);
  this->CTest->SetProduceXML(true);

  return this->CTest->InitializeFromCommand(this, true);
}


