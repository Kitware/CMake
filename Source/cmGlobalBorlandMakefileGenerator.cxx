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
#include "cmGlobalBorlandMakefileGenerator.h"
#include "cmLocalBorlandMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

void cmGlobalBorlandMakefileGenerator::EnableLanguage(const char*,
                                                      cmMakefile *mf)
{
  // now load the settings
  if(!mf->GetDefinition("CMAKE_ROOT"))
    {
    cmSystemTools::Error(
      "CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
    }
  std::string outdir = m_CMakeInstance->GetStartOutputDirectory();
  if(outdir.find('-') != std::string::npos)
    {
    std::string message = "The Borland command line tools do not support path names that have - in them.  Please re-name your output directory and use _ instead of -.";
    message += "\nYour path currently is: ";
    message += outdir;
    cmSystemTools::Error(message.c_str());
    }
  if(!this->GetLanguageEnabled("CXX"))
    {
    std::string fpath = 
      mf->GetDefinition("CMAKE_ROOT");
    fpath += "/Templates/CMakeBorlandWindowsSystemConfig.cmake";
    mf->ReadListFile(NULL,fpath.c_str());
    this->SetLanguageEnabled("CXX");
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalBorlandMakefileGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalBorlandMakefileGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}
