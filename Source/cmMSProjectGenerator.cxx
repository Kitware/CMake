/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmMSProjectGenerator.h"
#include "cmDSWMakefile.h"
#include "cmDSPMakefile.h"

cmMSProjectGenerator::cmMSProjectGenerator()
{
  m_DSWMakefile = 0;
  m_DSPMakefile = 0;
  BuildDSWOn();
}

void cmMSProjectGenerator::GenerateMakefile()
{
  if(m_BuildDSW)
    {
    m_DSWMakefile = new cmDSWMakefile(m_Makefile);
    m_DSWMakefile->OutputDSWFile();
    }
  else
    {
    m_DSPMakefile = new cmDSPMakefile(m_Makefile);
    m_DSPMakefile->OutputDSPFile();
    }
}

cmMSProjectGenerator::~cmMSProjectGenerator()
{
  delete m_DSPMakefile;
  delete m_DSWMakefile;
}

