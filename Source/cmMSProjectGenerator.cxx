/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMSProjectGenerator.h"
#include "cmDSWWriter.h"
#include "cmDSPWriter.h"
#include "cmCacheManager.h"

cmMSProjectGenerator::cmMSProjectGenerator()
{
  m_DSWWriter = 0;
  m_DSPWriter = 0;
  BuildDSWOn();
}

void cmMSProjectGenerator::GenerateMakefile()
{
  this->EnableLanguage("CXX");
  if(m_BuildDSW)
    {
    delete m_DSWWriter;
    m_DSWWriter = 0;
    m_DSWWriter = new cmDSWWriter(m_Makefile);
    m_DSWWriter->OutputDSWFile();
    }
  else
    {
    delete m_DSPWriter;
    m_DSPWriter = 0;
    m_DSPWriter = new cmDSPWriter(m_Makefile);
    m_DSPWriter->OutputDSPFile();
    }
}

cmMSProjectGenerator::~cmMSProjectGenerator()
{
  delete m_DSPWriter;
  delete m_DSWWriter;
}

void cmMSProjectGenerator::SetLocal(bool local)
{
  m_BuildDSW = !local;
}

void cmMSProjectGenerator::EnableLanguage(const char*)
{
  // now load the settings
  if(!m_Makefile->GetDefinition("CMAKE_ROOT"))
    {
    cmSystemTools::Error(
      "CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
    }
  if(!this->GetLanguageEnabled("CXX"))
    {
    std::string fpath = 
      m_Makefile->GetDefinition("CMAKE_ROOT");
    fpath += "/Templates/CMakeWindowsSystemConfig.cmake";
    m_Makefile->ReadListFile(NULL,fpath.c_str());
    this->SetLanguageEnabled("CXX");
    }
}

int cmMSProjectGenerator::TryCompile(const char *srcdir, 
                                     const char *bindir,
                                     const char *projectName)
{
  // now build the test
  std::string makeCommand = m_Makefile->GetDefinition("CMAKE_MAKE_PROGRAM");
  if(makeCommand.size() == 0)
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return 1;
    }
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand.c_str());
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string output;

  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  // if there are spaces in the makeCommand, assume a full path
  // and convert it to a path with no spaces in it as the
  // RunCommand does not like spaces
#if defined(_WIN32) && !defined(__CYGWIN__)      
  if(makeCommand.find(' ') != std::string::npos)
    {
    cmSystemTools::GetShortPath(makeCommand.c_str(), makeCommand);
    }
#endif
  makeCommand += " ";
  makeCommand += projectName;
  makeCommand += ".dsw /MAKE \"ALL_BUILD - Debug\" /REBUILD";
  
  if (!cmSystemTools::RunCommand(makeCommand.c_str(), output))
    {
    cmSystemTools::Error("Generator: execution of msdev failed.");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return 0;
}

