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
  std::string fpath = 
    m_Makefile->GetDefinition("CMAKE_ROOT");
  fpath += "/Templates/CMakeWindowsSystemConfig.cmake";
  m_Makefile->ReadListFile(NULL,fpath.c_str());
}
