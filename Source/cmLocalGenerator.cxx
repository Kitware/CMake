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
#include "cmLocalGenerator.h"
#include "cmMakefile.h"

cmLocalGenerator::cmLocalGenerator()
{
  m_Makefile = new cmMakefile;
}

cmLocalGenerator::~cmLocalGenerator()
{
  delete m_Makefile;
}

void cmLocalGenerator::Configure()
{
  // find & read the list file
  std::string currentStart = m_Makefile->GetStartDirectory();
  currentStart += "/CMakeLists.txt";
  m_Makefile->ReadListFile(currentStart.c_str());
}
