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
#include "cmCTestHandlerCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestHandlerCommand::cmCTestHandlerCommand()
{
  const size_t INIT_SIZE = 100;
  size_t cc;
  this->Arguments.reserve(INIT_SIZE);
  for ( cc = 0; cc < INIT_SIZE; ++ cc )
    {
    this->Arguments.push_back(0);
    }
  this->Arguments[ct_RETURN_VALUE] = "RETURN_VALUE";
  this->Arguments[ct_SOURCE] = "SOURCE";
  this->Arguments[ct_BUILD] = "BUILD";
  this->Arguments[ct_SUBMIT_INDEX] = "SUBMIT_INDEX";
  this->Last = ct_LAST;
}

bool cmCTestHandlerCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if ( !this->ProcessArguments(args, (unsigned int)this->Last, 
                               &*this->Arguments.begin(),this->Values) )
    {
    return false;
    }

  cmCTestLog(this->CTest, DEBUG, "Initialize handler" << std::endl;);
  cmCTestGenericHandler* handler = this->InitializeHandler();
  if ( !handler )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot instantiate test handler " << this->GetName()
               << std::endl);
    return false;
    }

  handler->PopulateCustomVectors(this->Makefile);
  if ( this->Values[ct_BUILD] )
    {
    this->CTest->SetCTestConfiguration("BuildDirectory",
      cmSystemTools::CollapseFullPath(
        this->Values[ct_BUILD]).c_str());
    }
  else
    {
    const char* bdir = 
      this->Makefile->GetSafeDefinition("CTEST_BINARY_DIRECTORY");
    if(bdir)
      {
      this->
        CTest->SetCTestConfiguration("BuildDirectory",
          cmSystemTools::CollapseFullPath(bdir).c_str());
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "CTEST_BINARY_DIRECTORY not set" << std::endl;);
      }
    }
  if ( this->Values[ct_SOURCE] )
    {
    cmCTestLog(this->CTest, DEBUG,
      "Set source directory to: " << this->Values[ct_SOURCE] << std::endl);
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
  if ( this->Values[ct_SUBMIT_INDEX] )
    {
    if ( this->CTest->GetDartVersion() <= 1 )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Dart before version 2.0 does not support collecting submissions."
        << std::endl
        << "Please upgrade the server to Dart 2 or higher, or do not use "
        "SUBMIT_INDEX." << std::endl);
      }
    else
      {
      handler->SetSubmitIndex(atoi(this->Values[ct_SUBMIT_INDEX]));
      }
    }
  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(
    this->CTest->GetCTestConfiguration("BuildDirectory").c_str());
  int res = handler->ProcessHandler();
  if ( this->Values[ct_RETURN_VALUE] && *this->Values[ct_RETURN_VALUE])
    {
    cmOStringStream str;
    str << res;
    this->Makefile->AddDefinition(
      this->Values[ct_RETURN_VALUE], str.str().c_str());
    }
  cmSystemTools::ChangeDirectory(current_dir.c_str());
  return true;
}

bool cmCTestHandlerCommand::ProcessArguments(
  std::vector<std::string> const& args, int last, const char** strings,
  std::vector<const char*>& values)
{
  int state = 0;
  int cc;
  values.resize(last);
  for ( cc = 0; cc < last; ++ cc )
    {
    values[cc] = 0;
    }

  for(size_t i=0; i < args.size(); ++i)
    {
    if ( state > 0 && state < last )
      {
      values[state] = args[i].c_str();
      cmCTestLog(this->CTest, DEBUG, "Set " << strings[state] << " to "
        << args[i].c_str() << std::endl);
      state = 0;
      }
    else
      {
      bool found = false;
      for ( cc = 0; cc < last; ++ cc )
        {
        if ( strings[cc] && args[i] == strings[cc] )
          {
          state = cc;
          if ( values[state] )
            {
            cmOStringStream ostr;
            ostr << "called with incorrect number of arguments. "
              << strings[state] << " specified twice.";
            this->SetError(ostr.str().c_str());
            return false;
            }
          found = true;
          break;
          }
        }
      if ( !found )
        {
        cmOStringStream str;
        str
          << "called with incorrect number of arguments. Extra argument is: "
          << args[i].c_str() << ".";
        this->SetError(str.str().c_str());
        return false;
        }
      }
    }
  return true;
}
