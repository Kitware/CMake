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
#include "cmGlobalMinGWMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"

cmGlobalMinGWMakefileGenerator::cmGlobalMinGWMakefileGenerator()
{
  m_FindMakeProgramFile = "CMakeMinGWFindMake.cmake";
  m_ForceUnixPaths = true;
  
}

void cmGlobalMinGWMakefileGenerator::EnableLanguage(std::vector<std::string>const& l,
                                                    cmMakefile *mf)
{ 
  this->FindMakeProgram(mf);
  std::string makeProgram = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::vector<std::string> locations;
  locations.push_back(cmSystemTools::GetProgramPath(makeProgram.c_str()));
  locations.push_back("c:/mingw/bin");
  locations.push_back("/mingw/bin");
  std::string gcc = "gcc.exe";
  std::string gxx = "g++.exe";
  std::string slash = "/";
  for(std::vector<std::string>::iterator i = locations.begin();
      i != locations.end(); ++i)
    {
    std::string tgcc = *i + slash + gcc;
    std::string tgxx = *i + slash + gxx;
    if(cmSystemTools::FileExists(tgcc.c_str()))
      {
      gcc = tgcc;
      gxx = tgxx;
      break;
      }
    }
  mf->AddDefinition("CMAKE_GENERATOR_CC", gcc.c_str());
  mf->AddDefinition("CMAKE_GENERATOR_CXX", gxx.c_str());
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf);
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalMinGWMakefileGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetWindowsShell(true);
  lg->SetGlobalGenerator(this);
  lg->SetIgnoreLibPrefix(true);
  lg->SetPassMakeflags(false);
  lg->SetUnixCD(true);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalMinGWMakefileGenerator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates a make file for use with mingw32-make.";
  entry.full = "The makefiles generated use cmd.exe as the shell.  "
    "They do not require msys or a unix shell.";
}
