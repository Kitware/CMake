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

#include "cmCTestConfigureHandler.h"

#include "cmCTest.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"
#include <cmsys/Process.h>


//----------------------------------------------------------------------
cmCTestConfigureHandler::cmCTestConfigureHandler()
{
  m_Verbose = false; 
  m_CTest = 0;
}


//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestConfigureHandler::ConfigureDirectory()
{
  std::cout << "Configure project" << std::endl;
  std::string cCommand = m_CTest->GetDartConfiguration("ConfigureCommand");
  if ( cCommand.size() == 0 )
    {
    std::cerr << "Cannot find ConfigureCommand key in the DartConfiguration.tcl" 
              << std::endl;
    return 1;
    }

  std::string buildDirectory = m_CTest->GetDartConfiguration("BuildDirectory");
  if ( buildDirectory.size() == 0 )
    {
    std::cerr << "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl;
    return 1;
    }

  double elapsed_time_start = cmSystemTools::GetTime();
  std::string output;
  int retVal = 0;
  int res = 0;
  if ( !m_CTest->GetShowOnly() )
    {
    cmGeneratedFileStream os; 
    if ( !m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), "Configure.xml", os, true) )
      {
      std::cerr << "Cannot open configure file" << std::endl;
      return 1;
      }
    std::string start_time = m_CTest->CurrentTime();

    cmGeneratedFileStream ofs;
    m_CTest->OpenOutputFile("Temporary", "LastConfigure.log", ofs);
    res = m_CTest->RunMakeCommand(cCommand.c_str(), &output, 
      &retVal, buildDirectory.c_str(),
      m_Verbose, 0, ofs);

    if ( ofs )
      {
      ofs.close();
      }
    
    if ( os )
      {
      m_CTest->StartXML(os);
      os << "<Configure>\n"
         << "\t<StartDateTime>" << start_time << "</StartDateTime>" << std::endl;
      if ( res == cmsysProcess_State_Exited && retVal )
        {
        os << retVal;
        }
      os << "<ConfigureCommand>" << cCommand.c_str() << "</ConfigureCommand>" << std::endl;
      //std::cout << "End" << std::endl;
      os << "<Log>" << cmCTest::MakeXMLSafe(output) << "</Log>" << std::endl;
      std::string end_time = m_CTest->CurrentTime();
      os << "\t<ConfigureStatus>" << retVal << "</ConfigureStatus>\n"
         << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
         << "<ElapsedMinutes>" 
         << static_cast<int>(
           (cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
         << "</ElapsedMinutes>"
         << "</Configure>" << std::endl;
      m_CTest->EndXML(os);
      }    
    }
  else
    {
    std::cout << "Configure with command: " << cCommand << std::endl;
    }
  if (! res || retVal )
    {
    std::cerr << "Error(s) when updating the project" << std::endl;
    return 1;
    }
  return 0;
}
