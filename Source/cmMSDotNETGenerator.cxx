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
#include "cmMSDotNETGenerator.h"
#include "cmSLNWriter.h"
#include "cmVCProjWriter.h"
#include "cmCacheManager.h"

cmMSDotNETGenerator::cmMSDotNETGenerator()
{
  m_SLNWriter = 0;
  m_VCProjWriter = 0;
  BuildProjOn();
}

void cmMSDotNETGenerator::GenerateMakefile()
{
  if(m_BuildSLN)
    {
    delete m_SLNWriter;
    m_SLNWriter = 0;
    m_SLNWriter = new cmSLNWriter(m_Makefile);
    m_SLNWriter->OutputSLNFile();
    }
  else
    {
    delete m_VCProjWriter;
    m_VCProjWriter = 0;
    m_VCProjWriter = new cmVCProjWriter(m_Makefile);
    m_VCProjWriter->OutputVCProjFile();
    }
}

cmMSDotNETGenerator::~cmMSDotNETGenerator()
{
  delete m_VCProjWriter;
  delete m_SLNWriter;
}

void cmMSDotNETGenerator::SetLocal(bool local)
{
  m_BuildSLN = !local;
}

void cmMSDotNETGenerator::ComputeSystemInfo()
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
