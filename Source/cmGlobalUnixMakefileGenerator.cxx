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

#include "cmGlobalUnixMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalUnixMakefileGenerator::cmGlobalUnixMakefileGenerator()
{
  m_FindMakeProgramFile = "CMakeUnixFindMake.cmake";
}

void cmGlobalUnixMakefileGenerator::EnableLanguage(const char* lang, 
                                                   cmMakefile *mf)
{
  mf->AddDefinition("CMAKE_CFG_INTDIR",".");
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
  if(!lang)
    {
    lang = "CXX";
    }
  if(lang[0] == 'C')
    {
    if(!mf->GetDefinition("CMAKE_C_COMPILER"))
      {
      cmSystemTools::Error("CMAKE_C_COMPILER not set, after EnableLanguage");
      return;
      }
    const char* cc = mf->GetDefinition("CMAKE_C_COMPILER");
    std::string path = cmSystemTools::FindProgram(cc);
    if(path.size() == 0)
      {
      std::string message = "your C compiler: ";
      if(cc)
        {
        message +=  cc;
        }
      else
        {
        message += "(NULL)";
        }
      message += " was not found in your path.   "
          "For CMake to correctly use try compile commands, the compiler must "
          "be in your path.   Please add the compiler to your PATH environment,"
          " and re-run CMake.";
      cmSystemTools::Error(message.c_str());
      }
    if(strcmp(lang, "CXX") == 0)
      {
      const char* cxx = mf->GetDefinition("CMAKE_CXX_COMPILER");
      path = cmSystemTools::FindProgram(cxx);
      if(path.size() == 0)
        {
        std::string message = "your C++ compiler: ";
        if(cxx)
          {
          message +=  cxx;
          }
        else
          {
          message +=  "(NULL)";
          }
        
        message += " was not found in your path.   "
          "For CMake to correctly use try compile commands, the compiler must "
          "be in your path.   Please add the compiler to your PATH environment,"
          " and re-run CMake.";
        cmSystemTools::Error(message.c_str());
        }
      }
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalUnixMakefileGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalUnixMakefileGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

