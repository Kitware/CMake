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

void cmGlobalUnixMakefileGenerator::EnableLanguage(const char* lang, 
                                                   cmMakefile *mf)
{
  // if no lang specified use CXX
  if(!lang )
    {
    lang = "CXX";
    }
  std::string root 
    = cmSystemTools::ConvertToOutputPath(mf->GetDefinition("CMAKE_ROOT"));
  std::string rootBin = mf->GetHomeOutputDirectory();
  if(m_ConfiguredFilesPath.size())
    {
    rootBin = m_ConfiguredFilesPath;
    }
  bool needCBackwards = false;
  bool needCXXBackwards = false;
  
  // check for a C compiler and configure it
  if(!m_CMakeInstance->GetLocal() &&
     !this->GetLanguageEnabled("C") && 
     lang[0] == 'C')
    {
    if (m_CMakeInstance->GetIsInTryCompile())
      {
      cmSystemTools::Error("This should not have happen. "
                           "If you see this message, you are probably using a "
                           "broken CMakeLists.txt file or a problematic release of "
                           "CMake");
      }
    needCBackwards = true;
    // Read the DetermineSystem file
    std::string systemFile = root;
    systemFile += "/Modules/CMakeDetermineSystem.cmake";
    mf->ReadListFile(0, systemFile.c_str());
    // read determine C compiler
    std::string determineCFile = root;
    determineCFile += "/Modules/CMakeDetermineCCompiler.cmake";
    mf->ReadListFile(0,determineCFile.c_str());
    this->SetLanguageEnabled("C");
    } 
  
  // check for a CXX compiler and configure it
  if(!m_CMakeInstance->GetLocal() &&
     !this->GetLanguageEnabled("CXX") &&
     strcmp(lang, "CXX") == 0)
    {
    needCXXBackwards = true;
    std::string determineCFile = root;
    determineCFile += "/Modules/CMakeDetermineCXXCompiler.cmake";
    mf->ReadListFile(0,determineCFile.c_str());
    this->SetLanguageEnabled("CXX");
    }
 
    
  std::string fpath = rootBin;
  if(!mf->GetDefinition("CMAKE_SYSTEM_LOADED"))
    {
    fpath += "/CMakeSystem.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  // if C,  then enable C
  if(lang[0] == 'C' && !mf->GetDefinition("CMAKE_C_COMPILER_LOADED"))
    {
    fpath = rootBin;
    fpath += "/CMakeCCompiler.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  if(strcmp(lang, "CXX") == 0 && !mf->GetDefinition("CMAKE_CXX_COMPILER_LOADED"))
    {
    fpath = rootBin;
    fpath += "/CMakeCXXCompiler.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  if(!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED"))
    {
    fpath = root;
    fpath += "/Modules/CMakeSystemSpecificInformation.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  
  if(!m_CMakeInstance->GetLocal())
    {
    // At this point we should have enough info for a try compile
    // which is used in the backward stuff
    if(needCBackwards)
      {
      if (!m_CMakeInstance->GetIsInTryCompile())
        {
        // for old versions of CMake ListFiles
        const char* versionValue
          = mf->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
        if (!versionValue || atof(versionValue) <= 1.4)
          {
          std::string ifpath = root + "/Modules/CMakeBackwardCompatibilityC.cmake";
          mf->ReadListFile(0,ifpath.c_str()); 
          }
        }
      }
    if(needCXXBackwards)
      {
      if (!m_CMakeInstance->GetIsInTryCompile())
        {
        // for old versions of CMake ListFiles
        const char* versionValue
          = mf->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
        if (!versionValue || atof(versionValue) <= 1.4)
          {
          std::string fpath = root + "/Modules/CMakeBackwardCompatibilityCXX.cmake";
          mf->ReadListFile(0,fpath.c_str()); 
          }
        }
      }
    // if we are from the top, always define this
    mf->AddDefinition("RUN_CONFIGURE", true);
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalUnixMakefileGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalUnixMakefileGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

void cmGlobalUnixMakefileGenerator::EnableLanguagesFromGenerator(cmGlobalGenerator *gen)
{
  this->SetConfiguredFilesPath(gen->GetCMakeInstance()->GetHomeOutputDirectory());
  // if C,  then enable C
  if(gen->GetLanguageEnabled("C"))
    {
    this->SetLanguageEnabled("C");
    }
  
  // if CXX 
  if(gen->GetLanguageEnabled("CXX"))
    {
    this->SetLanguageEnabled("CXX");
    }
}
