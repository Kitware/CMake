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
#include <stdlib.h> // required for atof

#if defined(_WIN32) && !defined(__CYGWIN__) 
#include <windows.h>
#endif

int cmGlobalGenerator::s_TryCompileTimeout = 0;

cmGlobalGenerator::cmGlobalGenerator()
{
  // by default use the native paths
  m_ForceUnixPaths = false;
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

// Find the make program for the generator, required for try compiles
void cmGlobalGenerator::FindMakeProgram(cmMakefile* mf)
{
  if(m_FindMakeProgramFile.size() == 0)
    {
    cmSystemTools::Error(
      "Generator implementation error, "
      "all generators must specify m_FindMakeProgramFile");
    }
  if(!mf->GetDefinition("CMAKE_MAKE_PROGRAM")
     || cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM")))
    {
    std::string setMakeProgram = mf->GetModulesFile(m_FindMakeProgramFile.c_str());
    if(setMakeProgram.size())
      {
      mf->ReadListFile(0, setMakeProgram.c_str());
      }
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
  std::string makeProgram = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  // if there are spaces in the make program use short path
  // but do not short path the actual program name, as
  // this can cause trouble with VSExpress
  if(makeProgram.find(' ') != makeProgram.npos)
    {
    std::string dir;
    std::string file;
    cmSystemTools::SplitProgramPath(makeProgram.c_str(),
                                    dir, file);
    std::string saveFile = file;
    cmSystemTools::GetShortPath(makeProgram.c_str(), makeProgram);
    cmSystemTools::SplitProgramPath(makeProgram.c_str(),
                                    dir, file);
    makeProgram = dir;
    makeProgram += "/";
    makeProgram += saveFile;
    this->GetCMakeInstance()->AddCacheEntry("CMAKE_MAKE_PROGRAM", makeProgram.c_str(),
                                            "make program",
                                            cmCacheManager::FILEPATH);
    }
  
}

// enable the given language
void cmGlobalGenerator::EnableLanguage(std::vector<std::string>const& languages,
                                       cmMakefile *mf)
{  
  if(languages.size() == 0)
    {
    cmSystemTools::Error("EnableLanguage must have a lang specified!");
    cmSystemTools::SetFatalErrorOccured();
    return;
    }
  // setup some variables for the EnableLanguage function
  bool isLocal = m_CMakeInstance->GetLocal();
  // if we are from the top, always define this
  if(!isLocal)
    {
    mf->AddDefinition("RUN_CONFIGURE", true);
    }
  bool needTestLanguage = false;
  std::string rootBin = mf->GetHomeOutputDirectory();
  // If the configuration files path has been set,
  // then we are in a try compile and need to copy the enable language
  // files into the try compile directory
  if(m_ConfiguredFilesPath.size())
    {
    std::string src = m_ConfiguredFilesPath;
    src += "/CMakeSystem.cmake";
    std::string dst = rootBin;
    dst += "/CMakeSystem.cmake";
    cmSystemTools::CopyFileIfDifferent(src.c_str(), dst.c_str());
    for(std::vector<std::string>::const_iterator l = languages.begin();
        l != languages.end(); ++l)
      {
      const char* lang = l->c_str();
      std::string src2 = m_ConfiguredFilesPath;
      src2 += "/CMake";
      src2 += lang;
      src2 += "Compiler.cmake";
      std::string dst2 = rootBin;
      dst2 += "/CMake";
      dst2 += lang;
      dst2 += "Compiler.cmake";
      cmSystemTools::CopyFileIfDifferent(src2.c_str(), dst2.c_str());
      }
    rootBin = m_ConfiguredFilesPath;
    }

  // **** Step 1, find and make sure CMAKE_MAKE_PROGRAM is defined
  this->FindMakeProgram(mf);

  // **** Step 2, Load the CMakeDetermineSystem.cmake file and find out
  // what platform we are running on
  if (!isLocal &&  !mf->GetDefinition("CMAKE_SYSTEM_NAME"))
    {
#if defined(_WIN32) && !defined(__CYGWIN__) 
    /* Windows version number data.  */
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx (&osvi);
    cmOStringStream windowsVersionString;
    windowsVersionString << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
    windowsVersionString.str();
    mf->AddDefinition("CMAKE_SYSTEM_VERSION", windowsVersionString.str().c_str());
#endif
    // Read the DetermineSystem file
    std::string systemFile = mf->GetModulesFile("CMakeDetermineSystem.cmake");
    mf->ReadListFile(0, systemFile.c_str());
    }
  // **** Step 3, load the CMakeSystem.cmake from the binary directory
  // this file is configured by the CMakeDetermineSystem.cmake file
  std::string fpath = rootBin;
  if(!mf->GetDefinition("CMAKE_SYSTEM_LOADED"))
    {
    fpath += "/CMakeSystem.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  // **** Step 4, foreach language 
  // load the CMakeDetermine(LANG)Compiler.cmake file to find
  // the compiler 
  for(std::vector<std::string>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    const char* lang = l->c_str();
    if(!isLocal && !this->GetLanguageEnabled(lang) )
      {  
      if (m_CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happen. "
                             "If you see this message, you are probably using a "
                             "broken CMakeLists.txt file or a problematic release of "
                             "CMake");
        }
      needTestLanguage = true; // must test a language after finding it
      // read determine LANG compiler
      std::string determinCompiler = "CMakeDetermine";
      determinCompiler += lang;
      determinCompiler += "Compiler.cmake";
      std::string determineFile = mf->GetModulesFile(determinCompiler.c_str());
      if(!mf->ReadListFile(0,determineFile.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file:", determineFile.c_str());
        }
      
      // put ${CMake_(LANG)_COMPILER_ENV_VAR}=${CMAKE_(LANG)_COMPILER into the
      // environment, in case user scripts want to run configure, or sub cmakes
      std::string compilerName = "CMAKE_";
      compilerName += lang;
      compilerName += "_COMPILER";
      std::string compilerEnv = "CMAKE_";
      compilerEnv += lang;
      compilerEnv += "_COMPILER_ENV_VAR";
      std::string envVar = mf->GetRequiredDefinition(compilerEnv.c_str());
      std::string envVarValue = mf->GetRequiredDefinition(compilerName.c_str());
      std::string env = envVar;
      env += "=";
      env += envVarValue;
      cmSystemTools::PutEnv(env.c_str());
      }
    
    // **** Step 5, Load the configured language compiler file, if not loaded.
    // look to see if CMAKE_(LANG)_COMPILER_LOADED is set, 
    // if not then load the CMake(LANG)Compiler.cmake file from the
    // binary tree, this is a configured file provided by
    // CMakeDetermine(LANG)Compiler.cmake
    std::string loadedLang = "CMAKE_";
    loadedLang +=  lang;
    loadedLang += "_COMPILER_LOADED";
    if(!mf->GetDefinition(loadedLang.c_str()))
      {
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";
      if(!mf->ReadListFile(0,fpath.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
        }
      this->SetLanguageEnabled(lang, mf);
      }
    }
  
  // **** Step 6, Load the system specific information if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED"))
    {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if(!mf->ReadListFile(0,fpath.c_str()))
      {
      cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
      }
    }
  for(std::vector<std::string>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    const char* lang = l->c_str();
    std::string langLoadedVar = "CMAKE_";
    langLoadedVar += lang;
    langLoadedVar += "_INFORMATION_LOADED";
    if (!mf->GetDefinition(langLoadedVar.c_str()))
      { 
      fpath = "CMake";
      fpath +=  lang;
      fpath += "Information.cmake";
      fpath = mf->GetModulesFile(fpath.c_str());
      if(!mf->ReadListFile(0,fpath.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
        }
      }
    // **** Step 7, Test the compiler for the language just setup
    // At this point we should have enough info for a try compile
    // which is used in the backward stuff
    if(!isLocal)
      {
      if(needTestLanguage)
        {
        if (!m_CMakeInstance->GetIsInTryCompile())
          {
          std::string testLang = "CMakeTest";
          testLang += lang;
          testLang += "Compiler.cmake";
          std::string ifpath = mf->GetModulesFile(testLang.c_str());
          if(!mf->ReadListFile(0,ifpath.c_str()))
            {
            cmSystemTools::Error("Could not find cmake module file:", ifpath.c_str());
            }
          // **** Step 8, load backwards compatibility stuff for C and CXX
          // for old versions of CMake ListFiles C and CXX had some
          // backwards compatibility files they have to load
          const char* versionValue
            = mf->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
          if (atof(versionValue) <= 1.4)
            {
            if(strcmp(lang, "C") == 0)
              {
              ifpath =  mf->GetModulesFile("CMakeBackwardCompatibilityC.cmake");
              mf->ReadListFile(0,ifpath.c_str()); 
              }
            if(strcmp(lang, "CXX") == 0)
              {
              ifpath =  mf->GetModulesFile("CMakeBackwardCompatibilityCXX.cmake");
              mf->ReadListFile(0,ifpath.c_str()); 
              }
            
            }
          }
        }
      }
    }
}

const char* cmGlobalGenerator::GetLanguageFromExtension(const char* ext)
{
  if(m_ExtensionToLanguage.count(ext) > 0)
    {
    return m_ExtensionToLanguage[ext].c_str();
    }
  return 0;
}

void cmGlobalGenerator::SetLanguageEnabled(const char* l, cmMakefile* mf)
{
  if(m_LanguageEnabled.count(l) > 0)
    {
    return;
    }
  std::string extensionsVar = std::string("CMAKE_") + 
    std::string(l) + std::string("_SOURCE_FILE_EXTENSIONS");
  std::string exts = mf->GetSafeDefinition(extensionsVar.c_str());
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(exts, extensionList);
  for(std::vector<std::string>::iterator i = extensionList.begin();
      i != extensionList.end(); ++i)
    {
    m_ExtensionToLanguage[*i] = l;
    }
  
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
      m_LocalGenerators[i]->GetMakefile()->CheckInfiniteLoops();
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
  if ( !m_CMakeInstance->GetScriptMode() )
    {
    m_CMakeInstance->UpdateProgress("Configuring done", -1);
    }
}

// loop through the directories creating cmLocalGenerators and Configure()
void cmGlobalGenerator::RecursiveConfigure(cmLocalGenerator *lg, 
                                           float startProgress, 
                                           float endProgress)
{
  // configure the current directory
  lg->Configure();
                                  
  // get all the subdirectories
  std::vector<std::pair<cmStdString, bool> > subdirs = lg->GetMakefile()->GetSubDirectories();
  float progressPiece = (endProgress - startProgress)/(1.0f+subdirs.size());
  m_CMakeInstance->UpdateProgress("Configuring",
                                  startProgress + progressPiece);
  
  // for each subdir recurse
  unsigned int i;
  for (i = 0; i < subdirs.size(); ++i)
    {
    cmLocalGenerator *lg2 = this->CreateLocalGenerator();
    lg2->SetParent(lg);
    m_LocalGenerators.push_back(lg2);
    
    // add the subdir to the start output directory
    std::string outdir = lg->GetMakefile()->GetStartOutputDirectory();
    outdir += "/";
    outdir += subdirs[i].first;
    lg2->GetMakefile()->SetStartOutputDirectory(outdir.c_str());
    lg2->SetExcludeAll(!subdirs[i].second);
    // add the subdir to the start source directory
    std::string currentDir = lg->GetMakefile()->GetStartDirectory();
    currentDir += "/";
    currentDir += subdirs[i].first;
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
    m_LocalGenerators[i]->GenerateInstallRules();
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
                                  std::string *output, cmMakefile*)
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
    std::string tmp = target;
    // if the target does not already end in . something 
    // then assume .exe
    if(tmp.size() < 4 || tmp[tmp.size()-4] != '.')
      {
      makeCommand += ".exe";
      }
#endif // WIN32
    }
  else
    {
    makeCommand += " all";
    }
  int retVal;
  int timeout = cmGlobalGenerator::s_TryCompileTimeout;
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();
  cmSystemTools::SetRunCommandHideConsole(true);
  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), output, 
      &retVal, 0, false, timeout))
    {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    cmSystemTools::Error("Generator: execution of make failed.");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmSystemTools::SetRunCommandHideConsole(hideconsole);
  
  // The SGI MipsPro 7.3 compiler does not return an error code when
  // the source has a #error in it!  This is a work-around for such
  // compilers.
  if((retVal == 0) && (output->find("#error") != std::string::npos))
    {
    retVal = 1;
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
  // copy the enabled languages
  this->m_LanguageEnabled = gen->m_LanguageEnabled;
  this->m_ExtensionToLanguage = gen->m_ExtensionToLanguage;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "";
  entry.full = "";
}

bool cmGlobalGenerator::IsExcluded(cmLocalGenerator* root, 
                                                cmLocalGenerator* gen)
{
  cmLocalGenerator* cur = gen->GetParent();
  while(cur && cur != root)
    {
    if(cur->GetExcludeAll())
      {
      return true;
      }
    cur = cur->GetParent();
    }
  return false;
}


