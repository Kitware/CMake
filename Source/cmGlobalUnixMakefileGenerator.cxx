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

void cmGlobalUnixMakefileGenerator::EnableLanguage(const char* lang, 
                                                   cmMakefile *mf)
{
  if (!m_LanguagesEnabled)
    {
    m_LanguagesEnabled = true;
    
    // see man putenv for explaination of this stupid code....
    static char envCXX[5000];
    static char envCC[5000];
    if(mf->GetDefinition("CMAKE_CXX_COMPILER"))
      {
      std::string env = "CXX=${CMAKE_CXX_COMPILER}";
      mf->ExpandVariablesInString(env);
      strncpy(envCXX, env.c_str(), 4999);
      envCXX[4999] = 0;
      putenv(envCXX);
      }
    if(mf->GetDefinition("CMAKE_C_COMPILER"))
      {
      std::string env = "CC=${CMAKE_C_COMPILER}";
      mf->ExpandVariablesInString(env);
      strncpy(envCC, env.c_str(), 4999);
      envCC[4999] = 0;
      putenv(envCC);
      }
    std::string output;
    std::string root 
      = cmSystemTools::ConvertToOutputPath(mf->GetDefinition("CMAKE_ROOT"));
    // if no lang specified use CXX
    if(!lang )
      {
      lang = "CXX";
      }
    // if CXX or C,  then enable C
    if((!this->GetLanguageEnabled(lang) && lang[0] == 'C'))
      {
      std::string cmd = root;
      cmd += "/Templates/cconfigure";
      cmSystemTools::RunCommand(cmd.c_str(), output, 
                                cmSystemTools::ConvertToOutputPath(mf->GetHomeOutputDirectory()).c_str());
      std::string fpath = mf->GetHomeOutputDirectory();
      fpath += "/CCMakeSystemConfig.cmake";
      mf->ReadListFile(NULL,fpath.c_str());
      this->SetLanguageEnabled("C");
      }
    // if CXX 
    if(!this->GetLanguageEnabled(lang)  || strcmp(lang, "CXX") == 0)
      {
      std::string cmd = root;
      cmd += "/Templates/cxxconfigure";
      cmSystemTools::RunCommand(cmd.c_str(), output, 
                                cmSystemTools::ConvertToOutputPath(mf->GetHomeOutputDirectory()).c_str());
      std::string fpath = mf->GetHomeOutputDirectory();
      fpath += "/CXXCMakeSystemConfig.cmake";
      mf->ReadListFile(NULL,fpath.c_str());
      this->SetLanguageEnabled("CXX");
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

