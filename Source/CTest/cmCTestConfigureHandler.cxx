/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCTestConfigureHandler.h"

#include "cmCTest.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"
#include "cmXMLSafe.h"
#include <cmsys/Process.h>


//----------------------------------------------------------------------
cmCTestConfigureHandler::cmCTestConfigureHandler()
{
}

//----------------------------------------------------------------------
void cmCTestConfigureHandler::Initialize()
{
  this->Superclass::Initialize();
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestConfigureHandler::ProcessHandler()
{
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "Configure project" << std::endl);
  std::string cCommand
    = this->CTest->GetCTestConfiguration("ConfigureCommand");
  if ( cCommand.size() == 0 )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot find ConfigureCommand key in the DartConfiguration.tcl"
      << std::endl);
    return -1;
    }

  std::string buildDirectory
    = this->CTest->GetCTestConfiguration("BuildDirectory");
  if ( buildDirectory.size() == 0 )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot find BuildDirectory  key in the DartConfiguration.tcl"
      << std::endl);
    return -1;
    }

  double elapsed_time_start = cmSystemTools::GetTime();
  std::string output;
  int retVal = 0;
  int res = 0;
  if ( !this->CTest->GetShowOnly() )
    {
    cmGeneratedFileStream os;
    if(!this->StartResultingXML(cmCTest::PartConfigure, "Configure", os))
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot open configure file"
        << std::endl);
      return 1;
      }
    std::string start_time = this->CTest->CurrentTime();
    unsigned int start_time_time = static_cast<unsigned int>(
      cmSystemTools::GetTime());

    cmGeneratedFileStream ofs;
    this->StartLogFile("Configure", ofs);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Configure with command: "
      << cCommand.c_str() << std::endl);
    res = this->CTest->RunMakeCommand(cCommand.c_str(), &output,
      &retVal, buildDirectory.c_str(),
      0, ofs);

    if ( ofs )
      {
      ofs.close();
      }

    if ( os )
      {
      this->CTest->StartXML(os, this->AppendXML);
      os << "<Configure>\n"
         << "\t<StartDateTime>" << start_time << "</StartDateTime>"
         << std::endl
         << "\t<StartConfigureTime>" << start_time_time
         << "</StartConfigureTime>\n";

      if ( res == cmsysProcess_State_Exited && retVal )
        {
        os << retVal;
        }
      os << "<ConfigureCommand>" << cCommand.c_str() << "</ConfigureCommand>"
        << std::endl;
      cmCTestLog(this->CTest, DEBUG, "End" << std::endl);
      os << "<Log>" << cmXMLSafe(output) << "</Log>" << std::endl;
      std::string end_time = this->CTest->CurrentTime();
      os << "\t<ConfigureStatus>" << retVal << "</ConfigureStatus>\n"
         << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
         << "\t<EndConfigureTime>" << 
        static_cast<unsigned int>(cmSystemTools::GetTime())
         << "</EndConfigureTime>\n"
         << "<ElapsedMinutes>"
         << static_cast<int>(
           (cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
         << "</ElapsedMinutes>"
         << "</Configure>" << std::endl;
      this->CTest->EndXML(os);
      }
    }
  else
    {
    cmCTestLog(this->CTest, DEBUG, "Configure with command: " << cCommand
      << std::endl);
    }
  if (! res || retVal )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Error(s) when configuring the project" << std::endl);
    return -1;
    }
  return 0;
}
