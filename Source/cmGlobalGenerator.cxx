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
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"

cmGlobalGenerator::cmGlobalGenerator()
{
  m_LanguagesEnabled = false;
}

cmGlobalGenerator::~cmGlobalGenerator()
{
  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    delete m_LocalGenerators[i];
    }
  m_LocalGenerators.clear();
}

void cmGlobalGenerator::SetLanguageEnabled(const char* l)
{
  m_LanguageEnabled[l] = true;
}

bool cmGlobalGenerator::GetLanguageEnabled(const char* l)
{
  return (m_LanguageEnabled.count(l) > 0);
}

void cmGlobalGenerator::ClearEnabledLanguages()
{
  m_LanguageEnabled.clear();
}

void cmGlobalGenerator::Configure()
{
  // reset theLanguages
  m_LanguagesEnabled = false;
  
  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    delete m_LocalGenerators[i];
    }
  m_LocalGenerators.clear();
  
  // start with this directory
  cmLocalGenerator *lg = this->CreateLocalGenerator();
  m_LocalGenerators.push_back(lg);

  // set the Start directories
  lg->GetMakefile()->SetStartDirectory(m_CMakeInstance->GetStartDirectory());
  lg->GetMakefile()->SetStartOutputDirectory(m_CMakeInstance->GetStartOutputDirectory());
  lg->GetMakefile()->MakeStartDirectoriesCurrent();
  
  // now do it
  this->RecursiveConfigure(lg);
}

// loop through the directories creating cmLocalGenerators and Configure()
void cmGlobalGenerator::RecursiveConfigure(cmLocalGenerator *lg)
{
  // configure the current directory
  lg->Configure();
  
  // get all the subdirectories
  std::vector<std::string> subdirs = lg->GetMakefile()->GetSubDirectories();
  
  // for each subdir recurse
  unsigned int i;
  for (i = 0; i < subdirs.size(); ++i)
    {
    cmLocalGenerator *lg2 = this->CreateLocalGenerator();
    m_LocalGenerators.push_back(lg2);
    
    // add the subdir to the start output directory
    std::string outdir = lg->GetMakefile()->GetStartOutputDirectory();
    outdir += "/";
    outdir += subdirs[i];
    lg2->GetMakefile()->SetStartOutputDirectory(outdir.c_str());
    
    // add the subdir to the start source directory
    std::string currentDir = lg->GetMakefile()->GetStartDirectory();
    currentDir += "/";
    currentDir += subdirs[i];
    lg2->GetMakefile()->SetStartDirectory(currentDir.c_str());
    lg2->GetMakefile()->MakeStartDirectoriesCurrent();
  
    this->RecursiveConfigure(lg2);
    }
}

void cmGlobalGenerator::Generate()
{
  // For each existing cmLocalGenerator
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    m_LocalGenerators[i]->Generate(true);
    }
}

void cmGlobalGenerator::LocalGenerate()
{
  // for this case we create one LocalGenerator
  // configure it, and then Generate it
  // start with this directory
  cmLocalGenerator *lg = this->CreateLocalGenerator();

  // set the Start directories
  lg->GetMakefile()->SetStartDirectory(m_CMakeInstance->GetStartDirectory());
  lg->GetMakefile()->SetStartOutputDirectory(m_CMakeInstance->GetStartOutputDirectory());
  lg->GetMakefile()->MakeStartDirectoriesCurrent();
  
  // now do trhe configure
  lg->Configure();
  lg->Generate(false);
  delete lg;
}

int cmGlobalGenerator::TryCompile(const char *, const char *bindir, 
                                  const char *, const char *target)
{
  // now build the test
  std::string makeCommand = 
    m_CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
  if(makeCommand.size() == 0)
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return 1;
    }
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand.c_str());

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string output;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);
  
  // now build
  if (target)
    {
    makeCommand += " ";
    makeCommand += target;
    }
  else
    {
    makeCommand += " all";
    }
  int retVal;
  if (!cmSystemTools::RunCommand(makeCommand.c_str(), output, retVal))
    {
    cmSystemTools::Error("Generator: execution of make failed.");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return retVal;
}

cmLocalGenerator *cmGlobalGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}
