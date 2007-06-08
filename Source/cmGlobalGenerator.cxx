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
#include "cmExternalMakefileProjectGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmVersion.h"

#include <stdlib.h> // required for atof

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#endif

#include <assert.h>

cmGlobalGenerator::cmGlobalGenerator()
{
  // By default the .SYMBOLIC dependency is not needed on symbolic rules.
  this->NeedSymbolicMark = false;

  // by default use the native paths
  this->ForceUnixPaths = false;

  // By default do not try to support color.
  this->ToolSupportsColor = false;

  // By default do not use link scripts.
  this->UseLinkScript = false;

  // Whether an install target is needed.
  this->InstallTargetEnabled = false;

  // how long to let try compiles run
  this->TryCompileTimeout = 0;

  this->ExtraGenerator = 0;
}

cmGlobalGenerator::~cmGlobalGenerator()
{
  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    delete this->LocalGenerators[i];
    }
  this->LocalGenerators.clear();

  if (this->ExtraGenerator)
    {
    delete this->ExtraGenerator;
    }
}

// Find the make program for the generator, required for try compiles
void cmGlobalGenerator::FindMakeProgram(cmMakefile* mf)
{
  if(this->FindMakeProgramFile.size() == 0)
    {
    cmSystemTools::Error(
      "Generator implementation error, "
      "all generators must specify this->FindMakeProgramFile");
    }
  if(!mf->GetDefinition("CMAKE_MAKE_PROGRAM")
     || cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM")))
    {
    std::string setMakeProgram =
      mf->GetModulesFile(this->FindMakeProgramFile.c_str());
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
    mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", makeProgram.c_str(),
                           "make program",
                           cmCacheManager::FILEPATH);
    }

  if(makeProgram.find("xcodebuild") != makeProgram.npos)
    {
    // due to the text file busy /bin/sh problem with xcodebuild
    // use the cmakexbuild wrapper instead.  This program
    // will run xcodebuild and if it sees the error text file busy
    // it will stop forwarding output, and let the build finish.
    // Then it will retry the build.  It will continue this
    // untill no text file busy errors occur.
    std::string cmakexbuild =
      this->CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_COMMAND");
    cmakexbuild = cmakexbuild.substr(0, cmakexbuild.length()-5);
    cmakexbuild += "cmakexbuild";

    mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM",
                           cmakexbuild.c_str(),
                           "make program",
                           cmCacheManager::FILEPATH);
    }
}

// enable the given language
//
// The following files are loaded in this order:
//
// First figure out what OS we are running on:
//
// CMakeSystem.cmake - configured file created by CMakeDetermineSystem.cmake
//   CMakeDetermineSystem.cmake - figure out os info and create
//                                CMakeSystem.cmake IFF CMAKE_SYSTEM_NAME
//                                not set
//   CMakeSystem.cmake - configured file created by
//                       CMakeDetermineSystem.cmake IFF CMAKE_SYSTEM_LOADED

// Next try and enable all languages found in the languages vector
//
// FOREACH LANG in languages
//   CMake(LANG)Compiler.cmake - configured file create by
//                               CMakeDetermine(LANG)Compiler.cmake
//     CMakeDetermine(LANG)Compiler.cmake - Finds compiler for LANG and
//                                          creates CMake(LANG)Compiler.cmake
//     CMake(LANG)Compiler.cmake - configured file created by
//                                 CMakeDetermine(LANG)Compiler.cmake
//
// CMakeSystemSpecificInformation.cmake
//   - includes Platform/${CMAKE_SYSTEM_NAME}.cmake
//     may use compiler stuff

// FOREACH LANG in languages
//   CMake(LANG)Information.cmake
//     - loads Platform/${CMAKE_SYSTEM_NAME}-${COMPILER}.cmake
//   CMakeTest(LANG)Compiler.cmake
//     - Make sure the compiler works with a try compile if
//       CMakeDetermine(LANG) was loaded
//
// Now load a few files that can override values set in any of the above
// CMake(PROJECTNAME)Compatibility.cmake
//   - load any backwards compatibility stuff for current project
// ${CMAKE_USER_MAKE_RULES_OVERRIDE}
//   - allow users a chance to override system variables
//
//

void
cmGlobalGenerator::EnableLanguage(std::vector<std::string>const& languages,
                                  cmMakefile *mf)
{
  if(languages.size() == 0)
    {
    cmSystemTools::Error("EnableLanguage must have a lang specified!");
    cmSystemTools::SetFatalErrorOccured();
    return;
    }
  mf->AddDefinition("RUN_CONFIGURE", true);
  std::string rootBin = mf->GetHomeOutputDirectory();
  rootBin += cmake::GetCMakeFilesDirectory();

  // If the configuration files path has been set,
  // then we are in a try compile and need to copy the enable language
  // files from the parent cmake bin dir, into the try compile bin dir
  if(this->ConfiguredFilesPath.size())
    {
    for(std::vector<std::string>::const_iterator l = languages.begin();
        l != languages.end(); ++l)
      {
      if(*l == "NONE")
        {
        this->SetLanguageEnabled("NONE", mf);
        break;
        }
      }
    rootBin = this->ConfiguredFilesPath;
    }

  // set the dir for parent files so they can be used by modules
  mf->AddDefinition("CMAKE_PLATFORM_ROOT_BIN",rootBin.c_str());

  // find and make sure CMAKE_MAKE_PROGRAM is defined
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
  //  Load the CMakeDetermineSystem.cmake file and find out
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
    mf->AddDefinition("CMAKE_HOST_SYSTEM_VERSION", 
                      windowsVersionString.str().c_str());
#endif
    // Read the DetermineSystem file
    std::string systemFile = mf->GetModulesFile("CMakeDetermineSystem.cmake");
    mf->ReadListFile(0, systemFile.c_str());
    // load the CMakeSystem.cmake from the binary directory
    // this file is configured by the CMakeDetermineSystem.cmake file
    fpath = rootBin;
    fpath += "/CMakeSystem.cmake";
    mf->ReadListFile(0,fpath.c_str());
    }
  std::map<cmStdString, bool> needTestLanguage;
  // foreach language
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
    bool determineLanguageCalled = false;
    std::string loadedLang = "CMAKE_";
    loadedLang +=  lang;
    loadedLang += "_COMPILER_LOADED";
    // If the existing build tree was already configured with this
    // version of CMake then try to load the configured file first
    // to avoid duplicate compiler tests.
    unsigned int cacheMajor = mf->GetCacheMajorVersion();
    unsigned int cacheMinor = mf->GetCacheMinorVersion();
    unsigned int selfMajor = cmVersion::GetMajorVersion();
    unsigned int selfMinor = cmVersion::GetMinorVersion();
    if((this->CMakeInstance->GetIsInTryCompile() ||
        (selfMajor == cacheMajor && selfMinor == cacheMinor))
       && !mf->GetDefinition(loadedLang.c_str()))
      {
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";
      if(cmSystemTools::FileExists(fpath.c_str()))
        {
        if(!mf->ReadListFile(0,fpath.c_str()))
          {
          cmSystemTools::Error("Could not find cmake module file:",
                               fpath.c_str());
          }
        // if this file was found then the language was already determined
        // to be working
        needTestLanguage[lang] = false;
        this->SetLanguageEnabled(lang, mf);
        // this can only be called after loading CMake(LANG)Compiler.cmake
        }
      }

    if(!this->GetLanguageEnabled(lang) )
      {
      if (this->CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happen. "
                             "If you see this message, you are probably "
                             "using a broken CMakeLists.txt file or a "
                             "problematic release of CMake");
        }
      // if the CMake(LANG)Compiler.cmake file was not found then
      // load CMakeDetermine(LANG)Compiler.cmake
      std::string determineCompiler = "CMakeDetermine";
      determineCompiler += lang;
      determineCompiler += "Compiler.cmake";
      std::string determineFile =
        mf->GetModulesFile(determineCompiler.c_str());
      if(!mf->ReadListFile(0,determineFile.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file:",
                             determineFile.c_str());
        }
      needTestLanguage[lang] = true;
      determineLanguageCalled = true;
      // Some generators like visual studio should not use the env variables
      // So the global generator can specify that in this variable
      if(!mf->GetDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV"))
        {
        // put ${CMake_(LANG)_COMPILER_ENV_VAR}=${CMAKE_(LANG)_COMPILER
        // into the environment, in case user scripts want to run
        // configure, or sub cmakes
        std::string compilerName = "CMAKE_";
        compilerName += lang;
        compilerName += "_COMPILER";
        std::string compilerEnv = "CMAKE_";
        compilerEnv += lang;
        compilerEnv += "_COMPILER_ENV_VAR";
        std::string envVar = mf->GetRequiredDefinition(compilerEnv.c_str());
        std::string envVarValue =
          mf->GetRequiredDefinition(compilerName.c_str());
        std::string env = envVar;
        env += "=";
        env += envVarValue;
        cmSystemTools::PutEnv(env.c_str());
        }
      } // end if(!this->GetLanguageEnabled(lang) )
    // if determineLanguage was called then load the file it
    // configures CMake(LANG)Compiler.cmake
    if(determineLanguageCalled)
      {
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";
      if(!mf->ReadListFile(0,fpath.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file:",
                             fpath.c_str());
        }
      this->SetLanguageEnabled(lang, mf);
      // this can only be called after loading CMake(LANG)Compiler.cmake
      // the language must be enabled for try compile to work, but we do
      // not know if it is a working compiler yet so set the test language
      // flag
      needTestLanguage[lang] = true;
      }
    }  // end loop over languages

  // **** Load the system specific information if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED"))
    {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if(!mf->ReadListFile(0,fpath.c_str()))
      {
      cmSystemTools::Error("Could not find cmake module file:",
                           fpath.c_str());
      }
    }
  // loop over languages again loading CMake(LANG)Information.cmake
  //
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
        cmSystemTools::Error("Could not find cmake module file:",
                             fpath.c_str());
        }
      }
    // Test the compiler for the language just setup
    // At this point we should have enough info for a try compile
    // which is used in the backward stuff
    // If the language is untested then test it now with a try compile.
    if(needTestLanguage[lang])
      {
      if (!this->CMakeInstance->GetIsInTryCompile())
        {
        std::string testLang = "CMakeTest";
        testLang += lang;
        testLang += "Compiler.cmake";
        std::string ifpath = mf->GetModulesFile(testLang.c_str());
        if(!mf->ReadListFile(0,ifpath.c_str()))
          {
          cmSystemTools::Error("Could not find cmake module file:",
                               ifpath.c_str());
          }
        std::string compilerWorks = "CMAKE_";
        compilerWorks += lang;
        compilerWorks += "_COMPILER_WORKS";
        // if the compiler did not work, then remove the
        // CMake(LANG)Compiler.cmake file so that it will get tested the
        // next time cmake is run
        if(!mf->IsOn(compilerWorks.c_str()))
          {
          fpath = rootBin;
          fpath += "/CMake";
          fpath += lang;
          fpath += "Compiler.cmake";
          cmSystemTools::RemoveFile(fpath.c_str());
          }
        else
          {
          // load backwards compatibility stuff for C and CXX
          // for old versions of CMake ListFiles C and CXX had some
          // backwards compatibility files they have to load
          // These files have a bunch of try compiles in them so
          // should only be done
          const char* versionValue
            = mf->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
          if (atof(versionValue) <= 1.4)
            {
            if(strcmp(lang, "C") == 0)
              {
              ifpath =
                mf->GetModulesFile("CMakeBackwardCompatibilityC.cmake");
              mf->ReadListFile(0,ifpath.c_str());
              }
            if(strcmp(lang, "CXX") == 0)
              {
              ifpath =
                mf->GetModulesFile("CMakeBackwardCompatibilityCXX.cmake");
              mf->ReadListFile(0,ifpath.c_str());
              }
            }
          }
        } // end if in try compile
      } // end need test language
    } // end for each language

  // Now load files that can override any settings on the platform or for
  // the project First load the project compatibility file if it is in
  // cmake
  std::string projectCompatibility = mf->GetDefinition("CMAKE_ROOT");
  projectCompatibility += "/Modules/";
  projectCompatibility += mf->GetSafeDefinition("PROJECT_NAME");
  projectCompatibility += "Compatibility.cmake";
  if(cmSystemTools::FileExists(projectCompatibility.c_str()))
    {
    mf->ReadListFile(0,projectCompatibility.c_str());
    }
}

const char* cmGlobalGenerator
::GetLanguageOutputExtensionForLanguage(const char* lang)
{
  if(!lang)
    {
    return "";
    }
  if(this->LanguageToOutputExtension.count(lang) > 0)
    {
    return this->LanguageToOutputExtension[lang].c_str();
    }
  return "";
}

const char* cmGlobalGenerator
::GetLanguageOutputExtensionFromExtension(const char* ext)
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
    if(this->OutputExtensions.count(ext))
      {
      return ext;
      }
    }
  return this->GetLanguageOutputExtensionForLanguage(lang);
}


const char* cmGlobalGenerator::GetLanguageFromExtension(const char* ext)
{
  // if there is an extension and it starts with . then move past the
  // . because the extensions are not stored with a .  in the map
  if(ext && *ext == '.')
    {
    ++ext;
    }
  if(this->ExtensionToLanguage.count(ext) > 0)
    {
    return this->ExtensionToLanguage[ext].c_str();
    }
  return 0;
}

void cmGlobalGenerator::SetLanguageEnabled(const char* l, cmMakefile* mf)
{
  if(this->LanguageEnabled.count(l) > 0)
    {
    return;
    }
  std::string outputExtensionVar = std::string("CMAKE_") +
    std::string(l) + std::string("_OUTPUT_EXTENSION");
  const char* outputExtension = mf->GetDefinition(outputExtensionVar.c_str());
  if(outputExtension)
    {
    this->LanguageToOutputExtension[l] = outputExtension;
    this->OutputExtensions[outputExtension] = outputExtension;
    if(outputExtension[0] == '.')
      {
      this->OutputExtensions[outputExtension+1] = outputExtension+1;
      }
    }

  std::string linkerPrefVar = std::string("CMAKE_") +
    std::string(l) + std::string("_LINKER_PREFERENCE");
  const char* linkerPref = mf->GetDefinition(linkerPrefVar.c_str());
  if(!linkerPref)
    {
    linkerPref = "None";
    }
  this->LanguageToLinkerPreference[l] = linkerPref;

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
    this->ExtensionToLanguage[*i] = l;
    }
  cmSystemTools::ExpandListArgument(ignoreExts, extensionList);
  for(std::vector<std::string>::iterator i = extensionList.begin();
      i != extensionList.end(); ++i)
    {
    this->IgnoreExtensions[*i] = true;
    }
  this->LanguageEnabled[l] = true;

}
bool cmGlobalGenerator::IgnoreFile(const char* l)
{
  if(this->GetLanguageFromExtension(l))
    {
    return false;
    }
  return (this->IgnoreExtensions.count(l) > 0);
}

bool cmGlobalGenerator::GetLanguageEnabled(const char* l)
{
  return (this->LanguageEnabled.count(l) > 0);
}

void cmGlobalGenerator::ClearEnabledLanguages()
{
  this->LanguageEnabled.clear();
}

bool cmGlobalGenerator::IsDependedOn(const char* project,
                                     cmTarget* targetIn)
{
  // Get all local gens for this project
  std::vector<cmLocalGenerator*>* gens = &this->ProjectMap[project];
  // loop over local gens and get the targets for each one
  for(unsigned int i = 0; i < gens->size(); ++i)
    {
    cmTargets& targets = (*gens)[i]->GetMakefile()->GetTargets();
    for (cmTargets::iterator l = targets.begin();
         l != targets.end(); l++)
      {
      cmTarget& target = l->second;
      std::set<cmStdString>::const_iterator pos =
        target.GetUtilities().find(targetIn->GetName());
      if(pos != target.GetUtilities().end())
        {
        return true;
        }
      }
    }
  return false;
}

void cmGlobalGenerator::Configure()
{
  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    delete this->LocalGenerators[i];
    }
  this->LocalGenerators.clear();
  this->TargetDependencies.clear();
  this->TotalTargets.clear();
  this->ImportedTotalTargets.clear();
  this->ProjectToTargetMap.clear();
  this->ProjectMap.clear();

  // start with this directory
  cmLocalGenerator *lg = this->CreateLocalGenerator();
  this->LocalGenerators.push_back(lg);

  // set the Start directories
  lg->GetMakefile()->SetStartDirectory
    (this->CMakeInstance->GetStartDirectory());
  lg->GetMakefile()->SetStartOutputDirectory
    (this->CMakeInstance->GetStartOutputDirectory());
  lg->GetMakefile()->MakeStartDirectoriesCurrent();

  // now do it
  lg->Configure();

  // update the cache entry for the number of local generators, this is used
  // for progress
  char num[100];
  sprintf(num,"%d",static_cast<int>(this->LocalGenerators.size()));
  this->GetCMakeInstance()->AddCacheEntry
    ("CMAKE_NUMBER_OF_LOCAL_GENERATORS", num,
     "number of local generators", cmCacheManager::INTERNAL);

  std::set<cmStdString> notFoundMap;
  // after it is all done do a ConfigureFinalPass
  cmCacheManager* manager = 0;
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    manager = this->LocalGenerators[i]->GetMakefile()->GetCacheManager();
    this->LocalGenerators[i]->ConfigureFinalPass();
    cmTargets & targets =
      this->LocalGenerators[i]->GetMakefile()->GetTargets();
    for (cmTargets::iterator l = targets.begin();
         l != targets.end(); l++)
      {
      cmTarget::LinkLibraryVectorType libs = l->second.GetLinkLibraries();
      for(cmTarget::LinkLibraryVectorType::iterator lib = libs.begin();
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
        this->LocalGenerators[i]->GetMakefile()->GetIncludeDirectories();

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
      this->CMakeInstance->UpdateProgress
        ("Configuring", 0.9f+0.1f*(i+1.0f)/this->LocalGenerators.size());
      this->LocalGenerators[i]->GetMakefile()->CheckInfiniteLoops();
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
  // at this point this->LocalGenerators has been filled,
  // so create the map from project name to vector of local generators
    this->FillProjectMap();
  // now create project to target map
  // This will make sure that targets have all the
  // targets they depend on as part of the build.
    this->FillProjectToTargetMap();

  if ( !this->CMakeInstance->GetScriptMode() )
    {
    this->CMakeInstance->UpdateProgress("Configuring done", -1);
    }
}

void cmGlobalGenerator::Generate()
{
  // For each existing cmLocalGenerator
  unsigned int i;

  // Consolidate global targets
  cmTargets globalTargets;
  this->CreateDefaultGlobalTargets(&globalTargets);
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    cmTargets* targets =
      &(this->LocalGenerators[i]->GetMakefile()->GetTargets());
    cmTargets::iterator tarIt;
    for ( tarIt = targets->begin(); tarIt != targets->end(); ++ tarIt )
      {
      if ( tarIt->second.GetType() == cmTarget::GLOBAL_TARGET )
        {
        globalTargets[tarIt->first] = tarIt->second;
        }
      }
    }
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = this->LocalGenerators[i]->GetMakefile();
    cmTargets* targets = &(mf->GetTargets());
    cmTargets::iterator tit;
    for ( tit = globalTargets.begin(); tit != globalTargets.end(); ++ tit )
      {
      (*targets)[tit->first] = tit->second;
      (*targets)[tit->first].SetMakefile(mf);
      }
    }

  // Add generator specific helper commands
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    this->LocalGenerators[i]->AddHelperCommands();
    }

  // Trace the dependencies, after that no custom commands should be added
  // because their dependencies might not be handled correctly
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    this->LocalGenerators[i]->TraceDependencies();
    }

  // Compute the manifest of main targets generated.
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    this->LocalGenerators[i]->GenerateTargetManifest(this->TargetManifest);
    }

  // Generate project files
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    this->LocalGenerators[i]->Generate();
    this->LocalGenerators[i]->GenerateInstallRules();
    this->LocalGenerators[i]->GenerateTestFiles();
    this->CMakeInstance->UpdateProgress("Generating",
                                    (i+1.0f)/this->LocalGenerators.size());
    }

  if (this->ExtraGenerator != 0)
    {
    this->ExtraGenerator->Generate();
    }

  this->CMakeInstance->UpdateProgress("Generating done", -1);
}

int cmGlobalGenerator::TryCompile(const char *srcdir, const char *bindir,
                                  const char *projectName,
                                  const char *target,
                                  std::string *output, cmMakefile *mf)
{
  std::string makeCommand = this->CMakeInstance->
    GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
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
                     output,makeCommand.c_str(),config,false,true,
                     this->TryCompileTimeout);
}

std::string cmGlobalGenerator
::GenerateBuildCommand(const char* makeProgram, const char *projectName,
                       const char* additionalOptions, const char *targetName,
                       const char* config, bool ignoreErrors, bool)
{
  // Project name and config are not used yet.
  (void)projectName;
  (void)config;

  std::string makeCommand =
    cmSystemTools::ConvertToUnixOutputPath(makeProgram);

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
  if ( additionalOptions )
    {
    makeCommand += " ";
    makeCommand += additionalOptions;
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
  bool clean, bool fast,
  double timeout)
{
  *output += "\nTesting TryCompileWithoutMakefile\n";

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  int retVal;
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();
  cmSystemTools::SetRunCommandHideConsole(true);

  // should we do a clean first?
  if (clean)
    {
    std::string cleanCommand =
      this->GenerateBuildCommand(makeCommandCSTR, projectName,
      0, "clean", config, false, fast);
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
  std::string makeCommand =
    this->GenerateBuildCommand(makeCommandCSTR, projectName,
                               0, target, config, false, fast);

  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), output,
                                       &retVal, 0, false, timeout))
    {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    cmSystemTools::Error
      ("Generator: execution of make failed. Make command was: ",
       makeCommand.c_str());
    if (output)
      {
      *output += "\nGenerator: execution of make failed. Make command was: "
        + makeCommand + "\n";
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
  this->LocalGenerators.push_back(lg);

  // update progress
  // estimate how many lg there will be
  const char *numGenC =
    this->CMakeInstance->GetCacheManager()->GetCacheValue
    ("CMAKE_NUMBER_OF_LOCAL_GENERATORS");

  if (!numGenC)
    {
    return;
    }

  int numGen = atoi(numGenC);
  float prog = 0.9f*this->LocalGenerators.size()/numGen;
  if (prog > 0.9f)
    {
    prog = 0.9f;
    }
  this->CMakeInstance->UpdateProgress("Configuring", prog);
}

void cmGlobalGenerator::AddInstallComponent(const char* component)
{
  if(component && *component)
    {
    this->InstallComponents.insert(component);
    }
}

void cmGlobalGenerator::EnableInstallTarget()
{
  this->InstallTargetEnabled = true;
}

cmLocalGenerator *cmGlobalGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

void cmGlobalGenerator::EnableLanguagesFromGenerator(cmGlobalGenerator *gen )
{
  std::string cfp = gen->GetCMakeInstance()->GetHomeOutputDirectory();
  cfp += cmake::GetCMakeFilesDirectory();
  this->SetConfiguredFilesPath(cfp.c_str());
  const char* make =
    gen->GetCMakeInstance()->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_MAKE_PROGRAM", make,
                                          "make program",
                                          cmCacheManager::FILEPATH);
  // copy the enabled languages
  this->LanguageEnabled = gen->LanguageEnabled;
  this->ExtensionToLanguage = gen->ExtensionToLanguage;
  this->IgnoreExtensions = gen->IgnoreExtensions;
  this->LanguageToOutputExtension = gen->LanguageToOutputExtension;
  this->LanguageToLinkerPreference = gen->LanguageToLinkerPreference;
  this->OutputExtensions = gen->OutputExtensions;
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
  if(gen == root)
    {
    return false;
    }
  cmLocalGenerator* cur = gen->GetParent();
  while(cur && cur != root)
    {
    if(cur->GetMakefile()->GetPropertyAsBool("EXCLUDE_FROM_ALL"))
      {
      return true;
      }
    cur = cur->GetParent();
    }
  return gen->GetMakefile()->GetPropertyAsBool("EXCLUDE_FROM_ALL");
}


void cmGlobalGenerator::GetEnabledLanguages(std::vector<std::string>& lang)
{
  for(std::map<cmStdString, bool>::iterator i =
        this->LanguageEnabled.begin(); i != this->LanguageEnabled.end(); ++i)
    {
    lang.push_back(i->first);
    }
}

const char* cmGlobalGenerator::GetLinkerPreference(const char* lang)
{
  if(this->LanguageToLinkerPreference.count(lang))
    {
    return this->LanguageToLinkerPreference[lang].c_str();
    }
  return "None";
}

void cmGlobalGenerator::FillProjectMap()
{
  this->ProjectMap.clear(); // make sure we start with a clean map
  unsigned int i;
  for(i = 0; i < this->LocalGenerators.size(); ++i)
    {
    // for each local generator add all projects
    cmLocalGenerator *lg = this->LocalGenerators[i];
    std::string name;
    do
      {
      if (name != lg->GetMakefile()->GetProjectName())
        {
        name = lg->GetMakefile()->GetProjectName();
        this->ProjectMap[name].push_back(this->LocalGenerators[i]);
        }
      lg = lg->GetParent();
      }
    while (lg);
    }
}


// Build a map that contains a the set of targets used by each project
void cmGlobalGenerator::FillProjectToTargetMap()
{
  // loop over each project in the build
  for(std::map<cmStdString,
        std::vector<cmLocalGenerator*> >::iterator m =
        this->ProjectMap.begin();
      m != this->ProjectMap.end(); ++m)
    {
    std::vector<cmLocalGenerator*>& lgs = m->second;
    if(lgs.size() == 0)
      {
      continue;
      }
    cmStdString const & projectName = m->first;
    cmMakefile* projectMakefile = lgs[0]->GetMakefile();
    // get the current EXCLUDE_FROM_ALL value from projectMakefile
    const char* exclude = 0;
    std::string excludeSave;
    bool chain = false;
    exclude =
      projectMakefile->GetProperties().
      GetPropertyValue("EXCLUDE_FROM_ALL",
                       cmProperty::DIRECTORY, chain);
    if(exclude)
      {
      excludeSave = exclude;
      }
    // Set EXCLUDE_FROM_ALL to FALSE for the top level makefile because
    // in the current project nothing is excluded yet
    projectMakefile->SetProperty("EXCLUDE_FROM_ALL", "FALSE");
    // now loop over all cmLocalGenerators in this project and pull
    // out all the targets that depend on each other, even if those
    // targets come from a target that is excluded.
    for(std::vector<cmLocalGenerator*>::iterator lg =
          lgs.begin(); lg != lgs.end(); ++lg)
      {
      cmMakefile* mf = (*lg)->GetMakefile();
      cmTargets& targets = mf->GetTargets();
      for(cmTargets::iterator t = targets.begin();
          t != targets.end(); ++t)
        {
        cmTarget& target = t->second;
        // if the target is in all then add it to the project
        if(!target.GetPropertyAsBool("EXCLUDE_FROM_ALL"))
          {
          // add this target to the project
          this->ProjectToTargetMap[projectName].insert(&target);
          // get the target's dependencies
          std::vector<cmTarget *>& tgtdeps = this->GetTargetDepends(target);
          this->ProjectToTargetMap[projectName].insert(tgtdeps.begin(),
                                                       tgtdeps.end());
          }
        }
      }
    // Now restore the EXCLUDE_FROM_ALL property on the project top
    // makefile
    if(exclude)
      {
      exclude = excludeSave.c_str();
      }
    projectMakefile->SetProperty("EXCLUDE_FROM_ALL", exclude);
    }
  // dump the map for debug purposes
  // right now this map is not being used, but it was
  // checked in to avoid constant conflicts.
  // It is also the first step to creating sub projects
  // that contain all of the targets they need.
#if 0
  std::map<cmStdString, std::set<cmTarget*> >::iterator i =
    this->ProjectToTargetMap.begin();
  for(; i != this->ProjectToTargetMap.end(); ++i)
    {
    std::cerr << i->first << "\n";
    std::set<cmTarget*>::iterator t = i->second.begin();
    for(; t != i->second.end(); ++t)
      {
      cmTarget* target = *t;
      std::cerr << "\t" << target->GetName() << "\n";
      }
    }
#endif
}


///! Find a local generator by its startdirectory
cmLocalGenerator* cmGlobalGenerator::FindLocalGenerator(const char* start_dir)
{
  std::vector<cmLocalGenerator*>* gens = &this->LocalGenerators;
  for(unsigned int i = 0; i < gens->size(); ++i)
    {
    std::string sd = (*gens)[i]->GetMakefile()->GetStartDirectory();
    if (sd == start_dir)
      {
      return (*gens)[i];
      }
    }
  return 0;
}


cmTarget* cmGlobalGenerator::FindTarget(const char* project,
                                        const char* name,
                                        bool useImportedTargets)
{
  // if project specific
  if(project)
    {
    std::vector<cmLocalGenerator*>* gens = &this->ProjectMap[project];
    for(unsigned int i = 0; i < gens->size(); ++i)
      {
      cmTarget* ret = (*gens)[i]->GetMakefile()->FindTarget(name,
                                                            useImportedTargets);
      if(ret)
        {
        return ret;
        }
      }
    }
  // if all projects/directories
  else
    {
    std::map<cmStdString,cmTarget *>::iterator i =
      this->TotalTargets.find ( name );
    if ( i != this->TotalTargets.end() )
      {
      return i->second;
      }

    if ( useImportedTargets )
      {
      std::map<cmStdString,cmTarget *>::iterator importedTarget =
        this->ImportedTotalTargets.find ( name );
      if ( importedTarget != this->ImportedTotalTargets.end() )
        {
        return importedTarget->second;
        }
      }
    }
  return 0;
}

inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}

void cmGlobalGenerator::SetCMakeInstance(cmake* cm)
{
  // Store a pointer to the cmake object instance.
  this->CMakeInstance = cm;
}

void cmGlobalGenerator::CreateDefaultGlobalTargets(cmTargets* targets)
{
  cmMakefile* mf = this->LocalGenerators[0]->GetMakefile();
  const char* cmakeCfgIntDir = this->GetCMakeCFGInitDirectory();
  const char* cmakeCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");

  // CPack
  cmCustomCommandLines cpackCommandLines;
  std::vector<std::string> depends;
  cmCustomCommandLine singleLine;
  singleLine.push_back(this->GetCMakeInstance()->GetCPackCommand());
  if ( cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.' )
    {
    singleLine.push_back("-C");
    singleLine.push_back(cmakeCfgIntDir);
    }
  singleLine.push_back("--config");
  std::string configFile = mf->GetStartOutputDirectory();;
  configFile += "/CPackConfig.cmake";
  singleLine.push_back(configFile);
  cpackCommandLines.push_back(singleLine);
  if ( this->GetPreinstallTargetName() )
    {
    depends.push_back("preinstall");
    }
  if(cmSystemTools::FileExists(configFile.c_str()))
    {
    (*targets)[this->GetPackageTargetName()]
      = this->CreateGlobalTarget(this->GetPackageTargetName(),
                                 "Run CPack packaging tool...",
                                 &cpackCommandLines, depends);
    }
  // CPack source
  const char* packageSourceTargetName = this->GetPackageSourceTargetName();
  if ( packageSourceTargetName )
    {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(this->GetCMakeInstance()->GetCPackCommand());
    singleLine.push_back("--config");
    configFile = mf->GetStartOutputDirectory();;
    configFile += "/CPackSourceConfig.cmake";
    if(cmSystemTools::FileExists(configFile.c_str()))
      {
      singleLine.push_back(configFile);
      cpackCommandLines.push_back(singleLine);
      (*targets)[packageSourceTargetName]
        = this->CreateGlobalTarget(packageSourceTargetName,
                                   "Run CPack packaging tool for source...",
                                   &cpackCommandLines, depends);
      }
    }

  // Test
  if(mf->IsOn("CMAKE_TESTING_ENABLED"))
    {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(this->GetCMakeInstance()->GetCTestCommand());
    singleLine.push_back("--force-new-ctest-process");
    if(cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.')
      {
      singleLine.push_back("-C");
      singleLine.push_back(cmakeCfgIntDir);
      }
    else // TODO: This is a hack. Should be something to do with the generator
      {
      singleLine.push_back("$(ARGS)");
      }
    cpackCommandLines.push_back(singleLine);
    (*targets)[this->GetTestTargetName()]
      = this->CreateGlobalTarget(this->GetTestTargetName(),
        "Running tests...", &cpackCommandLines, depends);
    }

  //Edit Cache
  const char* editCacheTargetName = this->GetEditCacheTargetName();
  if ( editCacheTargetName )
    {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());

    // Use CMAKE_EDIT_COMMAND for the edit_cache rule if it is defined.
    // Otherwise default to the interactive command-line interface.
    if(mf->GetDefinition("CMAKE_EDIT_COMMAND"))
      {
      singleLine.push_back(mf->GetDefinition("CMAKE_EDIT_COMMAND"));
      singleLine.push_back("-H$(CMAKE_SOURCE_DIR)");
      singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
      cpackCommandLines.push_back(singleLine);
      (*targets)[editCacheTargetName] =
        this->CreateGlobalTarget(
          editCacheTargetName, "Running CMake cache editor...",
          &cpackCommandLines, depends);
      }
    else
      {
      singleLine.push_back(cmakeCommand);
      singleLine.push_back("-i");
      singleLine.push_back(".");
      cpackCommandLines.push_back(singleLine);
      (*targets)[editCacheTargetName] =
        this->CreateGlobalTarget(
          editCacheTargetName,
          "Running interactive CMake command-line interface...",
          &cpackCommandLines, depends);
      }
    }

  //Rebuild Cache
  const char* rebuildCacheTargetName = this->GetRebuildCacheTargetName();
  if ( rebuildCacheTargetName )
    {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(cmakeCommand);
    singleLine.push_back("-H$(CMAKE_SOURCE_DIR)");
    singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
    cpackCommandLines.push_back(singleLine);
    (*targets)[rebuildCacheTargetName] =
      this->CreateGlobalTarget(
        rebuildCacheTargetName, "Running CMake to regenerate build system...",
        &cpackCommandLines, depends);
    }

  //Install
  if(this->InstallTargetEnabled)
    {
    if(!cmakeCfgIntDir || !*cmakeCfgIntDir || cmakeCfgIntDir[0] == '.')
      {
      std::set<cmStdString>* componentsSet = &this->InstallComponents;
      cpackCommandLines.erase(cpackCommandLines.begin(),
        cpackCommandLines.end());
      depends.erase(depends.begin(), depends.end());
      cmOStringStream ostr;
      if ( componentsSet->size() > 0 )
        {
        ostr << "Available install components are:";
        std::set<cmStdString>::iterator it;
        for (
          it = componentsSet->begin();
          it != componentsSet->end();
          ++ it )
          {
          ostr << " \"" << it->c_str() << "\"";
          }
        }
      else
        {
        ostr << "Only default component available";
        }
      singleLine.push_back(ostr.str().c_str());
      (*targets)["list_install_components"]
        = this->CreateGlobalTarget("list_install_components",
          ostr.str().c_str(),
          &cpackCommandLines, depends);
      }
    std::string cmd;
    cpackCommandLines.erase(cpackCommandLines.begin(),
      cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    if ( this->GetPreinstallTargetName() )
      {
      depends.push_back(this->GetPreinstallTargetName());
      }
    else
      {
      const char* noall =
        mf->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
      if(!noall || cmSystemTools::IsOff(noall))
        {
        depends.push_back(this->GetAllTargetName());
        }
      }
    if(mf->GetDefinition("CMake_BINARY_DIR"))
      {
      // We are building CMake itself.  We cannot use the original
      // executable to install over itself.
      cmd = mf->GetDefinition("EXECUTABLE_OUTPUT_PATH");
      if(cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.')
        {
        cmd += "/";
        cmd += cmakeCfgIntDir;
        }
      cmd += "/cmake";
      }
    else
      {
      cmd = cmakeCommand;
      }
    singleLine.push_back(cmd.c_str());
    if ( cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.' )
      {
      std::string cfgArg = "-DBUILD_TYPE=";
      cfgArg += mf->GetDefinition("CMAKE_CFG_INTDIR");
      singleLine.push_back(cfgArg);
      }
    singleLine.push_back("-P");
    singleLine.push_back("cmake_install.cmake");
    cpackCommandLines.push_back(singleLine);
    (*targets)[this->GetInstallTargetName()] =
      this->CreateGlobalTarget(
        this->GetInstallTargetName(), "Install the project...",
        &cpackCommandLines, depends);

    // install_local
    if(const char* install_local = this->GetInstallLocalTargetName())
      {
      cmCustomCommandLine localCmdLine = singleLine;

      localCmdLine.insert(localCmdLine.begin()+1, "-DCMAKE_INSTALL_LOCAL_ONLY=1");
      cpackCommandLines.erase(cpackCommandLines.begin(),
        cpackCommandLines.end());
      cpackCommandLines.push_back(localCmdLine);

      (*targets)[install_local] =
        this->CreateGlobalTarget(
          install_local, "Installing only the local directory...",
          &cpackCommandLines, depends);
      }

    // install_strip
    const char* install_strip = this->GetInstallStripTargetName();
    if((install_strip !=0) && (mf->IsSet("CMAKE_STRIP")))
      {
      cmCustomCommandLine stripCmdLine = singleLine;

      stripCmdLine.insert(stripCmdLine.begin()+1,"-DCMAKE_INSTALL_DO_STRIP=1");
      cpackCommandLines.erase(cpackCommandLines.begin(),
        cpackCommandLines.end());
      cpackCommandLines.push_back(stripCmdLine);

      (*targets)[install_strip] =
        this->CreateGlobalTarget(
          install_strip, "Installing the project stripped...",
          &cpackCommandLines, depends);
      }
    }
}

cmTarget cmGlobalGenerator::CreateGlobalTarget(
  const char* name, const char* message,
  const cmCustomCommandLines* commandLines,
  std::vector<std::string> depends,
  bool depends_on_all /* = false */)
{
  // Package
  cmTarget target;
  target.GetProperties().SetCMakeInstance(this->CMakeInstance);
  target.SetType(cmTarget::GLOBAL_TARGET, name);
  target.SetProperty("EXCLUDE_FROM_ALL","TRUE");

  std::vector<std::string> no_outputs;
  std::vector<std::string> no_depends;
  // Store the custom command in the target.
  cmCustomCommand cc(no_outputs, no_depends, *commandLines, 0, 0);
  target.GetPostBuildCommands().push_back(cc);
  target.SetProperty("EchoString", message);
  if ( depends_on_all )
    {
    target.AddUtility("all");
    }
  std::vector<std::string>::iterator dit;
  for ( dit = depends.begin(); dit != depends.end(); ++ dit )
    {
    target.AddUtility(dit->c_str());
    }
  return target;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::AppendDirectoryForConfig(const char*, const char*,
                                                 const char*, std::string&)
{
  // Subclasses that support multiple configurations should implement
  // this method to append the subdirectory for the given build
  // configuration.
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::CheckMultipleOutputs(cmMakefile*, bool)
{
  // Only certain generators need this check.  They define this
  // method.
}

//----------------------------------------------------------------------------
std::vector<cmTarget *>& cmGlobalGenerator
::GetTargetDepends(cmTarget& target)
{
  // if the depends are already in the map then return
  std::map<cmStdString, std::vector<cmTarget *> >::iterator tgtI =
    this->TargetDependencies.find(target.GetName());
  if (tgtI != this->TargetDependencies.end())
    {
    return tgtI->second;
    }

  // A target should not depend on itself.
  std::set<cmStdString> emitted;
  emitted.insert(target.GetName());

  // the vector of results
  std::vector<cmTarget *>& result =
    this->TargetDependencies[target.GetName()];

  // Loop over all library dependencies but not for static libs
  if (target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    const cmTarget::LinkLibraryVectorType& tlibs = target.GetLinkLibraries();
    for(cmTarget::LinkLibraryVectorType::const_iterator lib = tlibs.begin();
        lib != tlibs.end(); ++lib)
      {
      // Don't emit the same library twice for this target.
      if(emitted.insert(lib->first).second)
        {
        cmTarget *target2 =
          target.GetMakefile()->FindTarget(lib->first.c_str(), false);

        // search each local generator until a match is found
        if (!target2)
          {
          target2 = this->FindTarget(0,lib->first.c_str(), false);
          }

        // if a match was found then ...
        if (target2)
          {
          // Add this dependency.
          result.push_back(target2);
          }
        }
      }
    }

  // Loop over all utility dependencies.
  const std::set<cmStdString>& tutils = target.GetUtilities();
  for(std::set<cmStdString>::const_iterator util = tutils.begin();
      util != tutils.end(); ++util)
    {
    // Don't emit the same utility twice for this target.
    if(emitted.insert(*util).second)
      {
      cmTarget *target2 = target.GetMakefile()->FindTarget(util->c_str(), false);

      // search each local generator until a match is found
      if (!target2)
        {
        target2 = this->FindTarget(0,util->c_str(), false);
        }

      // if a match was found then ...
      if (target2)
        {
        // Add this dependency.
        result.push_back(target2);
        }
      }
    }
  return result;
}

void cmGlobalGenerator::AddTarget(cmTargets::value_type &v)
{
  if (v.second.IsImported())
    {
    this->ImportedTotalTargets[v.first] = &v.second;
    }
  else
    {
    this->TotalTargets[v.first] = &v.second;
    }
}

void cmGlobalGenerator::SetExternalMakefileProjectGenerator(
                            cmExternalMakefileProjectGenerator *extraGenerator)
{
  this->ExtraGenerator = extraGenerator;
  if (this->ExtraGenerator!=0)
    {
    this->ExtraGenerator->SetGlobalGenerator(this);
    }
}

