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
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"

cmGlobalGenerator::cmGlobalGenerator()
{
// do nothing duh
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


void cmGlobalGenerator::EnableLanguage(const char* lang, 
                                                   cmMakefile *mf)
{
  if(m_FindMakeProgramFile.size() == 0)
    {
    cmSystemTools::Error(
      "Generator implementation error, "
      "all generators must specify m_FindMakeProgramFile");
    }
  std::string root = mf->GetDefinition("CMAKE_ROOT");
  if(!mf->GetDefinition("CMAKE_MAKE_PROGRAM")
     || cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM")))
    {
    std::string setMakeProgram = root;
    setMakeProgram += "/Modules/";
    setMakeProgram += m_FindMakeProgramFile;
    mf->ReadListFile(0, setMakeProgram.c_str());
    } 
  if(!mf->GetDefinition("CMAKE_MAKE_PROGRAM")
     || cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM")))
    {
    cmOStringStream err;
    err << "CMake was unable to find a build program corresponding to \""
        << this->GetName() << "\".  CMAKE_MAKE_PROGRAM is not set.  You "
        << "probably need to select a different build tool.";
    cmSystemTools::Error(err.str().c_str());
    cmSystemTools::SetFatalErrorOccured();
    return;
    }
  std::string makeProgram = mf->GetDefinition("CMAKE_MAKE_PROGRAM");
  if(makeProgram.find(' ') != makeProgram.npos)
    {
    cmSystemTools::GetShortPath(makeProgram.c_str(), makeProgram);
    this->GetCMakeInstance()->AddCacheEntry("CMAKE_MAKE_PROGRAM", makeProgram.c_str(),
                                            "make program",
                                            cmCacheManager::FILEPATH);
    }
    
  bool isLocal = m_CMakeInstance->GetLocal();
  // if no lang specified use CXX
  if(!lang )
    {
    lang = "CXX";
    }
  std::string rootBin = mf->GetHomeOutputDirectory();
  if(m_ConfiguredFilesPath.size())
    {
    rootBin = m_ConfiguredFilesPath;
    }
  bool needCBackwards = false;
  bool needCXXBackwards = false;

  if (!isLocal &&
      !this->GetLanguageEnabled("C") && !this->GetLanguageEnabled("CXX") &&
      !this->GetLanguageEnabled("JAVA"))
    {
    // Read the DetermineSystem file
    std::string systemFile = root;
    systemFile += "/Modules/CMakeDetermineSystem.cmake";
    mf->ReadListFile(0, systemFile.c_str());
    }
  
  // check for a C compiler and configure it
  if(!isLocal &&
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
    // read determine C compiler
    std::string determineCFile = root;
    determineCFile += "/Modules/CMakeDetermineCCompiler.cmake";
    mf->ReadListFile(0,determineCFile.c_str());
    this->SetLanguageEnabled("C");
    // put CC in the environment in case user scripts want
    // to run configure
    // see man putenv for explaination of this stupid code...
    if(mf->GetDefinition("CMAKE_C_COMPILER"))
      { 
      static char envCC[5000];
      std::string env = "CC=${CMAKE_C_COMPILER}";
      mf->ExpandVariablesInString(env);
      unsigned int size = static_cast<unsigned int>(env.size());
      if(size > 4999)
        {
        size = 4999;
        }
      strncpy(envCC, env.c_str(), size);
      envCC[size] = 0;
      putenv(envCC);
      }
    } 
  
  // check for a CXX compiler and configure it
  if(!isLocal &&
     !this->GetLanguageEnabled("CXX") &&
     strcmp(lang, "CXX") == 0)
    {
    needCXXBackwards = true;
    std::string determineCFile = root;
    determineCFile += "/Modules/CMakeDetermineCXXCompiler.cmake";
    mf->ReadListFile(0,determineCFile.c_str());
    this->SetLanguageEnabled("CXX");
    // put CXX in the environment in case user scripts want
    // to run configure
    // see man putenv for explaination of this stupid code...
    static char envCXX[5000];
    if(mf->GetDefinition("CMAKE_CXX_COMPILER"))
      {
      std::string env = "CXX=${CMAKE_CXX_COMPILER}";
      mf->ExpandVariablesInString(env); 
      unsigned int size =  static_cast<unsigned int>(env.size());
      if(size > 4999)
        {
        size = 4999;
        }
      strncpy(envCXX, env.c_str(), size);
      envCXX[4999] = 0;
      putenv(envCXX);
      }
    }
    // check for a Java compiler and configure it
  if(!isLocal &&
     !this->GetLanguageEnabled("JAVA") &&
     strcmp(lang, "JAVA") == 0)
    {
    std::string determineCFile = root;
    determineCFile += "/Modules/CMakeDetermineJavaCompiler.cmake";
    mf->ReadListFile(0,determineCFile.c_str());
    this->SetLanguageEnabled("JAVA");
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
    this->SetLanguageEnabled("C");
    }
  if(strcmp(lang, "CXX") == 0 && !mf->GetDefinition("CMAKE_CXX_COMPILER_LOADED"))
    {
    fpath = rootBin;
    fpath += "/CMakeCXXCompiler.cmake";
    mf->ReadListFile(0,fpath.c_str());
    this->SetLanguageEnabled("CXX");

    }
  if(strcmp(lang, "JAVA") == 0 && !mf->GetDefinition("CMAKE_JAVA_COMPILER_LOADED"))
    {
    fpath = rootBin;
    fpath += "/CMakeJavaCompiler.cmake";
    mf->ReadListFile(0,fpath.c_str());
    this->SetLanguageEnabled("JAVA");
    }
  if ( lang[0] == 'C' && !mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED"))
    {
    fpath = root;
    fpath += "/Modules/CMakeSystemSpecificInformation.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  
  if(!isLocal)
    {
    // At this point we should have enough info for a try compile
    // which is used in the backward stuff
    if(needCBackwards)
      {
      if (!m_CMakeInstance->GetIsInTryCompile())
        {
        std::string ifpath = root + "/Modules/CMakeTestCCompiler.cmake";
        mf->ReadListFile(0,ifpath.c_str());
        // for old versions of CMake ListFiles
        const char* versionValue
          = mf->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
        if (atof(versionValue) <= 1.4)
          {
          ifpath = root + "/Modules/CMakeBackwardCompatibilityC.cmake";
          mf->ReadListFile(0,ifpath.c_str()); 
          }
        }
      }
    if(needCXXBackwards)
      {
      if (!m_CMakeInstance->GetIsInTryCompile())
        {
        std::string ifpath = root + "/Modules/CMakeTestCXXCompiler.cmake";
        mf->ReadListFile(0,ifpath.c_str());
        // for old versions of CMake ListFiles
        const char* versionValue
          = mf->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
        if (atof(versionValue) <= 1.4)
          {
          std::string nfpath = root + "/Modules/CMakeBackwardCompatibilityCXX.cmake";
          mf->ReadListFile(0,nfpath.c_str()); 
          }
        }
      }
    // if we are from the top, always define this
    mf->AddDefinition("RUN_CONFIGURE", true);
    }
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
  this->RecursiveConfigure(lg,0.0f,0.9f);

  std::set<std::string> notFoundMap;
  // after it is all done do a ConfigureFinalPass
  cmCacheManager* manager = 0;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    manager = m_LocalGenerators[i]->GetMakefile()->GetCacheManager();
    m_LocalGenerators[i]->ConfigureFinalPass();
    cmTargets const& targets = m_LocalGenerators[i]->GetMakefile()->GetTargets(); 
    for (cmTargets::const_iterator l = targets.begin();
         l != targets.end(); l++)
      {
      cmTarget::LinkLibraries libs = l->second.GetLinkLibraries();
      for(cmTarget::LinkLibraries::iterator lib = libs.begin();
          lib != libs.end(); ++lib)
        {
        if(lib->first.size() > 9 && 
           cmSystemTools::IsNOTFOUND(lib->first.c_str()))
          {
          std::string varName = lib->first.substr(0, lib->first.size()-9);
          notFoundMap.insert(varName);
          }
        }
      std::vector<std::string>& incs = 
        m_LocalGenerators[i]->GetMakefile()->GetIncludeDirectories();
      
      for( std::vector<std::string>::iterator lib = incs.begin();
           lib != incs.end(); ++lib)
        {
        if(lib->size() > 9 && 
           cmSystemTools::IsNOTFOUND(lib->c_str()))
          {
          std::string varName = lib->substr(0, lib->size()-9); 
          notFoundMap.insert(varName);
          }
        }
      m_CMakeInstance->UpdateProgress("Configuring", 
                                    0.9f+0.1f*(i+1.0f)/m_LocalGenerators.size());
      }
    }

  if(notFoundMap.size())
    {
    std::string notFoundVars;
    for(std::set<std::string>::iterator ii = notFoundMap.begin();
        ii != notFoundMap.end(); ++ii)
      { 
      notFoundVars += *ii;
      if(manager)
        {
        cmCacheManager::CacheIterator it = 
          manager->GetCacheIterator(ii->c_str());
        if(it.GetPropertyAsBool("ADVANCED"))
          {
          notFoundVars += " (ADVANCED)";
          }
        }
      notFoundVars += "\n";
      }
    cmSystemTools::Error("This project requires some variables to be set,\n"
                         "and cmake can not find them.\n"
                         "Please set the following variables:\n",
                         notFoundVars.c_str());
    }
  m_CMakeInstance->UpdateProgress("Configuring done", -1);
}

// loop through the directories creating cmLocalGenerators and Configure()
void cmGlobalGenerator::RecursiveConfigure(cmLocalGenerator *lg, 
                                           float startProgress, 
                                           float endProgress)
{
  // configure the current directory
  lg->Configure();
                                  
  // get all the subdirectories
  std::vector<std::string> subdirs = lg->GetMakefile()->GetSubDirectories();
  float progressPiece = (endProgress - startProgress)/(1.0f+subdirs.size());
  m_CMakeInstance->UpdateProgress("Configuring",
                                  startProgress + progressPiece);
  
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
  
    this->RecursiveConfigure(lg2, 
                             startProgress + (i+1.0f)*progressPiece,
                             startProgress + (i+2.0f)*progressPiece);
    }
}

void cmGlobalGenerator::Generate()
{
  // For each existing cmLocalGenerator
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    m_LocalGenerators[i]->Generate(true);
    m_CMakeInstance->UpdateProgress("Generating", 
                                    (i+1.0f)/m_LocalGenerators.size());
    }
  m_CMakeInstance->UpdateProgress("Generating done", -1);
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
  lg->ConfigureFinalPass();
  lg->Generate(false);
  delete lg;
}

int cmGlobalGenerator::TryCompile(const char *, const char *bindir, 
                                  const char *, const char *target,
                                  std::string *output)
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
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  // Since we have full control over the invocation of nmake, let us
  // make it quiet.
  if ( strcmp(this->GetName(), "NMake Makefiles") == 0 )
    {
    makeCommand += " /NOLOGO ";
    }
  
  // now build
  if (target)
    {
    makeCommand += " ";
    makeCommand += target;
#if defined(_WIN32) || defined(__CYGWIN__)
    makeCommand += ".exe";
#endif // WIN32
    }
  else
    {
    makeCommand += " all";
    }
  int retVal;
  if (!cmSystemTools::RunCommand(makeCommand.c_str(), *output, retVal, 0, false))
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

void cmGlobalGenerator::EnableLanguagesFromGenerator(cmGlobalGenerator *gen )
{
  this->SetConfiguredFilesPath(
    gen->GetCMakeInstance()->GetHomeOutputDirectory());
  const char* make =
    gen->GetCMakeInstance()->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_MAKE_PROGRAM", make,
                                          "make program",
                                          cmCacheManager::FILEPATH);
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

