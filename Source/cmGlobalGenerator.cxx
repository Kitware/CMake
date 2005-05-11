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

#include <assert.h>

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

  mf->AddDefinition("RUN_CONFIGURE", true);
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
      if(*l == "NONE")
        {
        this->SetLanguageEnabled("NONE", mf);
        continue;
        }
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
      src2 = m_ConfiguredFilesPath;
      src2 += "/CMake";
      src2 += lang;
      src2 += "Platform.cmake";
      dst2 = rootBin;
      dst2 += "/CMake";
      dst2 += lang;
      dst2 += "Platform.cmake";
      cmSystemTools::CopyFileIfDifferent(src2.c_str(), dst2.c_str());
      }
    rootBin = m_ConfiguredFilesPath;
    }

  // **** Step 1, find and make sure CMAKE_MAKE_PROGRAM is defined
  this->FindMakeProgram(mf);

  // try and load the CMakeSystem.cmake if it is there
  std::string fpath = rootBin;
  if(!mf->GetDefinition("CMAKE_SYSTEM_LOADED"))
    {
    fpath += "/CMakeSystem.cmake";
    if(cmSystemTools::FileExists(fpath.c_str()))
      {
      mf->ReadListFile(0,fpath.c_str());
      }
    }
  // **** Step 2, Load the CMakeDetermineSystem.cmake file and find out
  // what platform we are running on
  if (!mf->GetDefinition("CMAKE_SYSTEM_NAME"))
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
  fpath = rootBin;
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
    if(*l == "NONE")
      {
      this->SetLanguageEnabled("NONE", mf);
      continue;
      }

    if(!this->GetLanguageEnabled(lang) )
      {  
      if (m_CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happen. "
                             "If you see this message, you are probably using a "
                             "broken CMakeLists.txt file or a problematic release of "
                             "CMake");
        }

      // If the existing build tree was already configured with this
      // version of CMake then try to load the configured file first
      // to avoid duplicate compiler tests.
      unsigned int cacheMajor = mf->GetCacheMajorVersion();
      unsigned int cacheMinor = mf->GetCacheMinorVersion();
      unsigned int selfMajor = cmMakefile::GetMajorVersion();
      unsigned int selfMinor = cmMakefile::GetMinorVersion();
      if(selfMajor == cacheMajor && selfMinor == cacheMinor)
        {
        std::string loadedLang = "CMAKE_";
        loadedLang +=  lang;
        loadedLang += "_COMPILER_LOADED";
        if(!mf->GetDefinition(loadedLang.c_str()))
          {
          fpath = rootBin;
          fpath += "/CMake";
          fpath += lang;
          fpath += "Compiler.cmake";
          if(cmSystemTools::FileExists(fpath.c_str()))
            {
            if(!mf->ReadListFile(0,fpath.c_str()))
              {
              cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
              }
            this->SetLanguageEnabled(lang, mf);
            }
          }
        }
      
      needTestLanguage = true; // must test a language after finding it
      // read determine LANG compiler
      std::string determineCompiler = "CMakeDetermine";
      determineCompiler += lang;
      determineCompiler += "Compiler.cmake";
      std::string determineFile = mf->GetModulesFile(determineCompiler.c_str());
      if(!mf->ReadListFile(0,determineFile.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file:", determineFile.c_str());
        }
      // Some generators like visual studio should not use the env variables
      // So the global generator can specify that in this variable
      if(!mf->GetDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV"))
        {
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
    if(*l == "NONE")
      {
      this->SetLanguageEnabled("NONE", mf);
      continue;
      }
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

const char* cmGlobalGenerator::GetLanguageOutputExtensionForLanguage(const char* lang)
{
  if(!lang) 
    {
    return "";
    }
  if(m_LanguageToOutputExtension.count(lang) > 0)
    {
    return m_LanguageToOutputExtension[lang].c_str();
    }
  return "";
}

const char* cmGlobalGenerator::GetLanguageOutputExtensionFromExtension(const char* ext)
{
  if(!ext)
    {
    return "";
    }
  const char* lang = this->GetLanguageFromExtension(ext);
  if(!lang || *lang == 0)
    {
    // if no language is found then check to see if it is already an
    // ouput extension for some language.  In that case it should be ignored
    // and in this map, so it will not be compiled but will just be used.
    if(m_OutputExtensions.count(ext))
      {
      return ext;
      }
    }
  return this->GetLanguageOutputExtensionForLanguage(lang);
}


const char* cmGlobalGenerator::GetLanguageFromExtension(const char* ext)
{
  // if there is an extension and it starts with . then
  // move past the . because the extensions are not stored with a .
  // in the map
  if(ext && *ext == '.')
    {
    ++ext;
    }
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
  std::string outputExtensionVar = std::string("CMAKE_") + 
    std::string(l) + std::string("_OUTPUT_EXTENSION");
  const char* outputExtension = mf->GetDefinition(outputExtensionVar.c_str());
  if(outputExtension)
    {
    m_LanguageToOutputExtension[l] = outputExtension;
    m_OutputExtensions[outputExtension] = outputExtension;
    if(outputExtension[0] == '.')
      {
      m_OutputExtensions[outputExtension+1] = outputExtension+1;
      }
    }
  
  std::string linkerPrefVar = std::string("CMAKE_") + 
    std::string(l) + std::string("_LINKER_PREFERENCE");
  const char* linkerPref = mf->GetDefinition(linkerPrefVar.c_str());
  if(!linkerPref)
    {
    linkerPref = "None";
    }
  m_LanguageToLinkerPreference[l] = linkerPref;
  
  std::string extensionsVar = std::string("CMAKE_") + 
    std::string(l) + std::string("_SOURCE_FILE_EXTENSIONS");
  std::string ignoreExtensionsVar = std::string("CMAKE_") + 
    std::string(l) + std::string("_IGNORE_EXTENSIONS");
  std::string ignoreExts = mf->GetSafeDefinition(ignoreExtensionsVar.c_str());
  std::string exts = mf->GetSafeDefinition(extensionsVar.c_str());
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(exts, extensionList);
  for(std::vector<std::string>::iterator i = extensionList.begin();
      i != extensionList.end(); ++i)
    {
    m_ExtensionToLanguage[*i] = l;
    }
  cmSystemTools::ExpandListArgument(ignoreExts, extensionList);
  for(std::vector<std::string>::iterator i = extensionList.begin();
      i != extensionList.end(); ++i)
    {
    m_IgnoreExtensions[*i] = true;
    }
  m_LanguageEnabled[l] = true;

}
bool cmGlobalGenerator::IgnoreFile(const char* l)
{
  if(this->GetLanguageFromExtension(l))
    {
    return false;
    }
  return (m_IgnoreExtensions.count(l) > 0);
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
  // Setup the current output directory components for use by
  // ConvertToRelativePath.
  std::string outdir =
    cmSystemTools::CollapseFullPath(m_CMakeInstance->GetHomeOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), m_HomeOutputDirectoryComponents);

  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    delete m_LocalGenerators[i];
    }
  m_LocalGenerators.clear();

  // Setup relative path generation.
  this->ConfigureRelativePaths();

  // start with this directory
  cmLocalGenerator *lg = this->CreateLocalGenerator();
  m_LocalGenerators.push_back(lg);

  // set the Start directories
  lg->GetMakefile()->SetStartDirectory
    (m_CMakeInstance->GetStartDirectory());
  lg->GetMakefile()->SetStartOutputDirectory
    (m_CMakeInstance->GetStartOutputDirectory());
  lg->GetMakefile()->MakeStartDirectoriesCurrent();
  
  // now do it
  lg->Configure();
  
  // update the cache entry for the number of local generators, this is used
  // for progress
  char num[100];
  sprintf(num,"%d",static_cast<int>(m_LocalGenerators.size()));
  this->GetCMakeInstance()->AddCacheEntry
    ("CMAKE_NUMBER_OF_LOCAL_GENERATORS", num,
     "number of local generators", cmCacheManager::INTERNAL);
  
  std::set<cmStdString> notFoundMap;
  // after it is all done do a ConfigureFinalPass
  cmCacheManager* manager = 0;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    manager = m_LocalGenerators[i]->GetMakefile()->GetCacheManager();
    m_LocalGenerators[i]->ConfigureFinalPass();
    cmTargets const& targets = 
      m_LocalGenerators[i]->GetMakefile()->GetTargets(); 
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
    for(std::set<cmStdString>::iterator ii = notFoundMap.begin();
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
  // at this point m_LocalGenerators has been filled,
  // so create the map from project name to vector of local generators
  this->FillProjectMap();
  if ( !m_CMakeInstance->GetScriptMode() )
    {
    m_CMakeInstance->UpdateProgress("Configuring done", -1);
    }
}

void cmGlobalGenerator::Generate()
{
  // For each existing cmLocalGenerator
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    m_LocalGenerators[i]->Generate();
    m_LocalGenerators[i]->GenerateInstallRules();
    m_LocalGenerators[i]->GenerateTestFiles();
    m_CMakeInstance->UpdateProgress("Generating", 
                                    (i+1.0f)/m_LocalGenerators.size());
    }
  m_CMakeInstance->UpdateProgress("Generating done", -1);
}

int cmGlobalGenerator::TryCompile(const char *srcdir, const char *bindir, 
                                  const char *projectName, 
                                  const char *target,
                                  std::string *output, cmMakefile *mf)
{
  std::string makeCommand = 
    m_CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
  if(makeCommand.size() == 0)
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return 1;
    }

  std::string newTarget;
  if (target && strlen(target))
    {
    newTarget += target;
#if 0
#if defined(_WIN32) || defined(__CYGWIN__)
    std::string tmp = target;
    // if the target does not already end in . something 
    // then assume .exe
    if(tmp.size() < 4 || tmp[tmp.size()-4] != '.')
      {
      newTarget += ".exe";
      }
#endif // WIN32
#endif
    }
  const char* config = mf->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
  return this->Build(srcdir,bindir,projectName,
                     newTarget.c_str(),
                     output,makeCommand.c_str(),config,false);
}

std::string cmGlobalGenerator::GenerateBuildCommand(const char* makeProgram,
  const char *projectName, const char *targetName, const char* config,
  bool ignoreErrors)
{
  // Project name and config are not used yet.
  (void)projectName;
  (void)config;

  std::string makeCommand = cmSystemTools::ConvertToUnixOutputPath(makeProgram);

  // Since we have full control over the invocation of nmake, let us
  // make it quiet.
  if ( strcmp(this->GetName(), "NMake Makefiles") == 0 )
    {
    makeCommand += " /NOLOGO ";
    }
  if ( ignoreErrors )
    {
    makeCommand += " -i";
    }
  if ( targetName )
    {
    makeCommand += " ";
    makeCommand += targetName;
    }
  return makeCommand;
}
  
int cmGlobalGenerator::Build(
  const char *, const char *bindir, 
  const char *projectName, const char *target,
  std::string *output, 
  const char *makeCommandCSTR,
  const char *config,
  bool clean)
{
  *output += "\nTesting TryCompileWithoutMakefile\n";
  
  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  int retVal;
  int timeout = cmGlobalGenerator::s_TryCompileTimeout;
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();
  cmSystemTools::SetRunCommandHideConsole(true);

  // should we do a clean first?
  if (clean)
    {
    std::string cleanCommand = this->GenerateBuildCommand(makeCommandCSTR, projectName, "clean", config, false);
    if (!cmSystemTools::RunSingleCommand(cleanCommand.c_str(), output, 
                                         &retVal, 0, false, timeout))
      {
      cmSystemTools::SetRunCommandHideConsole(hideconsole);
      cmSystemTools::Error("Generator: execution of make clean failed.");
      if (output)
        {
        *output += "\nGenerator: execution of make clean failed.\n";
        }
      
      // return to the original directory
      cmSystemTools::ChangeDirectory(cwd.c_str());
      return 1;
      }
    }
  
  // now build
  std::string makeCommand = this->GenerateBuildCommand(makeCommandCSTR, projectName, target, config, false);

  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), output, 
                                       &retVal, 0, false, timeout))
    {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    cmSystemTools::Error("Generator: execution of make failed. Make command was: ",
      makeCommand.c_str());
    if (output)
      {
      *output += "\nGenerator: execution of make failed. Make command was: " +
        makeCommand + "\n";
      }
    
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

void cmGlobalGenerator::AddLocalGenerator(cmLocalGenerator *lg)
{
  m_LocalGenerators.push_back(lg); 

  // update progress
  // estimate how many lg there will be
  const char *numGenC = 
    m_CMakeInstance->GetCacheManager()->GetCacheValue
    ("CMAKE_NUMBER_OF_LOCAL_GENERATORS");
  
  if (!numGenC)
    {
    return;
    }
  
  int numGen = atoi(numGenC);
  float prog = 0.9f*m_LocalGenerators.size()/numGen;
  if (prog > 0.9f)
    {
    prog = 0.9f;
    }
  m_CMakeInstance->UpdateProgress("Configuring", prog);
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
  this->m_IgnoreExtensions = gen->m_IgnoreExtensions;
  this->m_LanguageToOutputExtension = gen->m_LanguageToOutputExtension;
  this->m_LanguageToLinkerPreference = gen->m_LanguageToLinkerPreference;
  this->m_OutputExtensions = gen->m_OutputExtensions;
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


void cmGlobalGenerator::GetEnabledLanguages(std::vector<std::string>& lang)
{
  for(std::map<cmStdString, bool>::iterator i = m_LanguageEnabled.begin(); 
      i != m_LanguageEnabled.end(); ++i)
    {
    lang.push_back(i->first);
    }
}

const char* cmGlobalGenerator::GetLinkerPreference(const char* lang)
{
  if(m_LanguageToLinkerPreference.count(lang))
    {
    return m_LanguageToLinkerPreference[lang].c_str();
    }
  return "None";
}


void cmGlobalGenerator::FillProjectMap()
{ 
  m_ProjectMap.clear(); // make sure we start with a clean map
  unsigned int i;
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    // for each local generator add all projects 
    cmLocalGenerator *lg = m_LocalGenerators[i];
    std::string name;
    do 
      {
      if (name != lg->GetMakefile()->GetProjectName())
        {
        name = lg->GetMakefile()->GetProjectName();
        m_ProjectMap[name].push_back(m_LocalGenerators[i]);
        }
      lg = lg->GetParent();
      }
    while (lg);
    }
}


cmTarget* cmGlobalGenerator::FindTarget(const char* project, 
                                        const char* name)
{
  std::vector<cmLocalGenerator*>* gens = &m_LocalGenerators;
  if(project)
    {
    gens = &m_ProjectMap[project];
    }
  for(unsigned int i = 0; i < gens->size(); ++i)
    {
    cmTarget* ret = (*gens)[i]->GetMakefile()->FindTarget(name);
    if(ret)
      {
      return ret;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::ConfigureRelativePaths()
{
  // Identify the longest shared path component between the source
  // directory and the build directory.
  std::vector<std::string> source;
  std::vector<std::string> binary;
  cmSystemTools::SplitPath(m_CMakeInstance->GetHomeDirectory(), source);
  cmSystemTools::SplitPath(m_CMakeInstance->GetHomeOutputDirectory(), binary);
  unsigned int common=0;
  while(common < source.size() && common < binary.size() &&
        cmSystemTools::ComparePath(source[common].c_str(),
                                   binary[common].c_str()))
    {
    ++common;
    }

  // Require more than just the root portion of the path to be in
  // common before allowing relative paths.  Also disallow relative
  // paths if the build tree is a network path.  The current working
  // directory on Windows cannot be a network path.  Therefore
  // relative paths cannot work with network paths.
  if(common > 1 && source[0] != "//")
    {
    // Build the minimum prefix required of a path to be converted to
    // a relative path.
    source.erase(source.begin()+common, source.end());
    m_RelativePathTop = cmSystemTools::JoinPath(source);
    }
  else
    {
    // Disable relative paths.
    m_RelativePathTop = "";
    }
}

//----------------------------------------------------------------------------
std::string
cmGlobalGenerator::ConvertToRelativePath(const std::vector<std::string>& local,
                                         const char* in_remote)
{
  // The path should never be quoted.
  assert(in_remote[0] != '\"');

  // The local path should never have a trailing slash.
  assert(local.size() > 0 && !(local[local.size()-1] == ""));

  // If the path is already relative or relative paths are disabled
  // then just return the path.
  if(m_RelativePathTop.size() == 0 ||
     !cmSystemTools::FileIsFullPath(in_remote))
    {
    return in_remote;
    }

  // If the path does not begin with the minimum relative path prefix
  // then do not convert it.
  std::string original = in_remote;
  if(original.size() < m_RelativePathTop.size() ||
     !cmSystemTools::ComparePath(
       original.substr(0, m_RelativePathTop.size()).c_str(),
       m_RelativePathTop.c_str()))
    {
    return in_remote;
    }

  // Identify the longest shared path component between the remote
  // path and the local path.
  std::vector<std::string> remote;
  cmSystemTools::SplitPath(in_remote, remote);
  unsigned int common=0;
  while(common < remote.size() &&
        common < local.size() &&
        cmSystemTools::ComparePath(remote[common].c_str(),
                                   local[common].c_str()))
    {
    ++common;
    }

  // If the entire path is in common then just return a ".".
  if(common == remote.size() &&
     common == local.size())
    {
    return ".";
    }

  // If the entire path is in common except for a trailing slash then
  // just return a "./".
  if(common+1 == remote.size() &&
     remote[common].size() == 0 &&
     common == local.size())
    {
    return "./";
    }

  // Construct the relative path.
  std::string relative;

  // First add enough ../ to get up to the level of the shared portion
  // of the path.  Leave off the trailing slash.  Note that the last
  // component of local will never be empty because local should never
  // have a trailing slash.
  for(unsigned int i=common; i < local.size(); ++i)
    {
    relative += "..";
    if(i < local.size()-1)
      {
      relative += "/";
      }
    }

  // Now add the portion of the destination path that is not included
  // in the shared portion of the path.  Add a slash the first time
  // only if there was already something in the path.  If there was a
  // trailing slash in the input then the last iteration of the loop
  // will add a slash followed by an empty string which will preserve
  // the trailing slash in the output.
  for(unsigned int i=common; i < remote.size(); ++i)
    {
    if(relative.size() > 0)
      {
      relative += "/";
      }
    relative += remote[i];
    }

  // Finally return the path.
  return relative;
}

inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}

void cmGlobalGenerator::SetupTests()
{
  std::string ctest = 
    m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
  ctest = removeQuotes(ctest);
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  ctest += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Debug/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Release/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  // if we found ctest
  if (cmSystemTools::FileExists(ctest.c_str()))
    {
    // Create a full path filename for output Testfile
    std::string fname;
    fname = m_CMakeInstance->GetStartOutputDirectory();
    fname += "/";
    if ( m_LocalGenerators[0]->GetMakefile()->IsSet("CTEST_NEW_FORMAT") )
      {
      fname += "CTestTestfile.txt";
      }
    else
      {
      fname += "DartTestfile.txt";
      }
    
    // If the file doesn't exist, then ENABLE_TESTING hasn't been run
    if (cmSystemTools::FileExists(fname.c_str()))
      {
      const char* no_output = 0;
      std::vector<std::string> no_depends;
      std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
      for(it = m_ProjectMap.begin(); it!= m_ProjectMap.end(); ++it)
        {
        std::vector<cmLocalGenerator*>& gen = it->second;
        // add the ALL_BUILD to the first local generator of each project
        if(gen.size())
          {
          gen[0]->GetMakefile()->
            AddUtilityCommand("RUN_TESTS", false, no_output, no_depends,
                              ctest.c_str(), "-C", "$(IntDir)");
          }
        }
      }
    }
}


//----------------------------------------------------------------------------
std::string cmGlobalGenerator::ConvertToHomeRelativePath(const char* remote)
{
  return (this->ConvertToRelativePath(m_HomeOutputDirectoryComponents,remote));
}

//----------------------------------------------------------------------------
std::string
cmGlobalGenerator::ConvertToHomeRelativeOutputPath(const char* remote)
{
  return cmSystemTools::ConvertToOutputPath
    (this->ConvertToRelativePath(m_HomeOutputDirectoryComponents,remote).c_str());
}

