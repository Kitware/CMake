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
#include "cmGlobalNMakeMakefileGenerator.h"
#include "cmLocalNMakeMakefileGenerator.h"
#include "cmMakefile.h"

void cmGlobalNMakeMakefileGenerator::EnableLanguage(const char*,
                                                    cmMakefile *mf)
{
  // now load the settings
  if(!mf->GetDefinition("CMAKE_ROOT"))
    {
    cmSystemTools::Error(
      "CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
    }
  if(!this->GetLanguageEnabled("CXX"))
    {
    std::string fpath = 
      mf->GetDefinition("CMAKE_ROOT");
    fpath += "/Templates/CMakeNMakeWindowsSystemConfig.cmake";
    mf->ReadListFile(NULL,fpath.c_str());
    this->SetLanguageEnabled("CXX");
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalNMakeMakefileGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalNMakeMakefileGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}
