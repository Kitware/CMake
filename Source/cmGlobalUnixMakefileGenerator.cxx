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

#include "cmGlobalUnixMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

void cmGlobalUnixMakefileGenerator::EnableLanguage(const char* lang, 
                                                   cmMakefile *mf)
{
  // only do for global runs 
  if (!m_CMakeInstance->GetLocal())
    {
    std::string output;
    std::string root 
      = cmSystemTools::ConvertToOutputPath(mf->GetDefinition("CMAKE_ROOT"));
    // if no lang specified use CXX
    if(!lang )
      {
      lang = "CXX";
      }
    // if CXX or C,  then enable C
    if((!this->GetLanguageEnabled("C") && lang[0] == 'C'))
      {
      static char envCC[5000];
      if(mf->GetDefinition("CMAKE_C_COMPILER"))
        {
        std::string env = "CC=${CMAKE_C_COMPILER}";
        mf->ExpandVariablesInString(env);
        strncpy(envCC, env.c_str(), 4999);
        envCC[4999] = 0;
        putenv(envCC);
        }
      if (m_CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happen. "
                             "If you see this message, you are probably using a "
                             "broken CMakeLists.txt file or a problematic release of "
                             "CMake");
        }

      std::string cmd = root;
      cmd += "/Templates/cconfigure";
      cmSystemTools::RunCommand(cmd.c_str(), output, 
                                cmSystemTools::ConvertToOutputPath(
                                  mf->GetHomeOutputDirectory()).c_str());
        
      std::string fpath = mf->GetHomeOutputDirectory();
      fpath += "/CCMakeSystemConfig.cmake";
      mf->ReadListFile(0,fpath.c_str());
      this->SetLanguageEnabled("C");
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
    // if CXX 
    if(!this->GetLanguageEnabled("CXX")  && strcmp(lang, "CXX") == 0)
      {
      // see man putenv for explaination of this stupid code....
      static char envCXX[5000];
      if(mf->GetDefinition("CMAKE_CXX_COMPILER"))
        {
        std::string env = "CXX=${CMAKE_CXX_COMPILER}";
        mf->ExpandVariablesInString(env);
        strncpy(envCXX, env.c_str(), 4999);
        envCXX[4999] = 0;
        putenv(envCXX);
        }
      std::string cmd = root;
      if (m_CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happen. "
                             "If you see this message, you are probably using a "
                             "broken CMakeLists.txt file or a problematic release of "
                             "CMake");
        }
      cmd += "/Templates/cxxconfigure";
      cmSystemTools::RunCommand(cmd.c_str(), output, 
                                cmSystemTools::ConvertToOutputPath(
                                  mf->GetHomeOutputDirectory()).c_str());
      
      std::string fpath = mf->GetHomeOutputDirectory();
      fpath += "/CXXCMakeSystemConfig.cmake";
      mf->ReadListFile(0,fpath.c_str());
      this->SetLanguageEnabled("CXX");
      
      if (!m_CMakeInstance->GetIsInTryCompile())
        {
        // for old versions of CMake ListFiles
        const char* versionValue
          = mf->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
        if (!versionValue || atof(versionValue) <= 1.4)
          {
          fpath = root + "/Modules/CMakeBackwardCompatibilityCXX.cmake";
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

void cmGlobalUnixMakefileGenerator::EnableLanguagesFromGenerator(
  cmGlobalGenerator *gen, cmMakefile *mf)
{
  // for UNIX we just want to read in the configured files
  cmLocalGenerator *lg = this->CreateLocalGenerator();
  
  // set the Start directories
  lg->GetMakefile()->SetStartDirectory(m_CMakeInstance->GetStartDirectory());
  lg->GetMakefile()->SetStartOutputDirectory(m_CMakeInstance->GetStartOutputDirectory());
  lg->GetMakefile()->MakeStartDirectoriesCurrent();
  
  // if C,  then enable C
  if(gen->GetLanguageEnabled("C"))
    {
    std::string fpath = mf->GetHomeOutputDirectory();
    fpath += "/CCMakeSystemConfig.cmake";
    lg->GetMakefile()->ReadListFile(0,fpath.c_str());
    this->SetLanguageEnabled("C");
    }
  
  // if CXX 
  if(gen->GetLanguageEnabled("CXX"))
    {
    std::string fpath = mf->GetHomeOutputDirectory();
    fpath += "/CXXCMakeSystemConfig.cmake";
    lg->GetMakefile()->ReadListFile(0,fpath.c_str());
    this->SetLanguageEnabled("CXX");
    }
  delete lg;
}
