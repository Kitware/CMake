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
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"

cmLocalGenerator::cmLocalGenerator()
{
  m_Makefile = new cmMakefile;
  m_Makefile->SetLocalGenerator(this);
}

cmLocalGenerator::~cmLocalGenerator()
{
  delete m_Makefile;
}

void cmLocalGenerator::Configure()
{
  // set the PROJECT_SOURCE_DIR and PROJECT_BIN_DIR to default values
  // just in case the project does not include a PROJECT command
  m_Makefile->AddDefinition("PROJECT_BINARY_DIR",
                            m_Makefile->GetHomeOutputDirectory());
  m_Makefile->AddDefinition("PROJECT_SOURCE_DIR",
                            m_Makefile->GetHomeDirectory());
  
  // find & read the list file
  std::string currentStart = m_Makefile->GetStartDirectory();
  currentStart += "/CMakeLists.txt";
  m_Makefile->ReadListFile(currentStart.c_str());
}

void cmLocalGenerator::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  m_GlobalGenerator = gg; 

  // setup the home directories
  m_Makefile->SetHomeDirectory(
    gg->GetCMakeInstance()->GetHomeDirectory());
  m_Makefile->SetHomeOutputDirectory(
    gg->GetCMakeInstance()->GetHomeOutputDirectory());
}

void cmLocalGenerator::ConfigureFinalPass()
{ 
  m_Makefile->ConfigureFinalPass(); 
}
