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
#include "cmLocalUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"
cmGlobalBorlandMakefileGenerator::cmGlobalBorlandMakefileGenerator()
{
  m_FindMakeProgramFile = "CMakeBorlandFindMake.cmake";
}

void cmGlobalBorlandMakefileGenerator::EnableLanguage(const char* l,
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
  mf->AddDefinition("BORLAND", "1");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "bcc32");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "bcc32"); 
  this->cmGlobalUnixMakefileGenerator::EnableLanguage(l, mf);
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalBorlandMakefileGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator *lg = new cmLocalUnixMakefileGenerator;
  lg->SetIncludeDirective("!include");
  lg->SetWindowsShell(true);
  lg->SetMakefileVariableSize(32);

  lg->SetGlobalGenerator(this);
  return lg;
}


//----------------------------------------------------------------------------
void cmGlobalBorlandMakefileGenerator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Borland makefiles.";
  entry.full = "";
}
