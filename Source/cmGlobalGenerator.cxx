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
  this->ForceUnixPaths = false;
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
    std::string setMakeProgram = mf->GetModulesFile(this->FindMakeProgramFile.c_str());
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
//
// The following files are loaded in this order:
// 
// First figure out what OS we are running on:
// 
// CMakeSystem.cmake          // configured file created by CMakeDetermineSystem.cmake
//   CMakeDetermineSystem.cmake // figure out os info and create CMakeSystem.cmake IFF CMAKE_SYSTEM_NAME not set
//   CMakeSystem.cmake          // configured file created by CMakeDetermineSystem.cmake IFF CMAKE_SYSTEM_LOADED 
//    TODO: CMakeDetermineSystem.cmake and CMakeSystem.cmake should be in the same if

// Next try and enable all languages found in the languages vector
// 
// FOREACH LANG in languages
//   CMake(LANG)Compiler.cmake  // configured file create by CMakeDetermine(LANG)Compiler.cmake
//     CMakeDetermine(LANG)Compiler.cmake  // Finds compiler for LANG and creates CMake(LANG)Compiler.cmake
//     CMake(LANG)Compiler.cmake   // configured file create by CMakeDetermine(LANG)Compiler.cmake       
//
// CMakeSystemSpecificInformation.cmake  // inludes Platform/${CMAKE_SYSTEM_NAME}.cmake may use compiler stuff

// FOREACH LANG in languages
//   CMake(LANG)Information.cmake   // loads Platform/${CMAKE_SYSTEM_NAME}-${COMPILER}.cmake
//   CMakeTest(LANG)Compiler.cmake  // Make sure the compiler works with a try compile if CMakeDetermine(LANG) was loaded
// 
// Now load a few files that can override values set in any of the above
// CMake(PROJECTNAME)Compatibility.cmake  // load any backwards compatibility stuff for current project
// ${CMAKE_USER_MAKE_RULES_OVERRIDE}      // allow users a chance to override system variables 
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
  rootBin += "/CMakeFiles";
  
  // If the configuration files path has been set,
  // then we are in a try compile and need to copy the enable language
  // files from the parent cmake bin dir, into the try compile bin dir
  if(this->ConfiguredFilesPath.size())
    {
    std::string src = this->ConfiguredFilesPath;
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
      std::string src2 = this->ConfiguredFilesPath;
      src2 += "/CMake";
      src2 += lang;
      src2 += "Compiler.cmake";
      std::string dst2 = rootBin;
      dst2 += "/CMake";
      dst2 += lang;
      dst2 += "Compiler.cmake";
      cmSystemTools::CopyFileIfDifferent(src2.c_str(), dst2.c_str()); 
      src2 = this->ConfiguredFilesPath;
      src2 += "/CMake";
      src2 += lang;
      src2 += "Platform.cmake";
      dst2 = rootBin;
      dst2 += "/CMake";
      dst2 += lang;
      dst2 += "Platform.cmake";
      cmSystemTools::CopyFileIfDifferent(src2.c_str(), dst2.c_str());
      }
    rootBin = this->ConfiguredFilesPath;
    }

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
    mf->AddDefinition("CMAKE_SYSTEM_VERSION", windowsVersionString.str().c_str());
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
    unsigned int selfMajor = cmMakefile::GetMajorVersion();
    unsigned int selfMinor = cmMakefile::GetMinorVersion();
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
          cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
          }
        // if this file was found then the language was already determined to be working
        needTestLanguage[lang] = false;
        this->SetLanguageEnabled(lang, mf); // this can only be called after loading CMake(LANG)Compiler.cmake
        }
      }
      
    if(!this->GetLanguageEnabled(lang) )
      {  
      if (this->CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happen. "
                             "If you see this message, you are probably using a "
                             "broken CMakeLists.txt file or a problematic release of "
                             "CMake");
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
        cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
        }
      this->SetLanguageEnabled(lang, mf); // this can only be called after loading CMake(LANG)Compiler.cmake
      // the language must be enabled for try compile to work, but
      // we do not know if it is a working compiler yet so set the test language flag
      needTestLanguage[lang] = true;
      }
    }  // end loop over languages
  
  // **** Load the system specific information if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED"))
    {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if(!mf->ReadListFile(0,fpath.c_str()))
      {
      cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
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
        cmSystemTools::Error("Could not find cmake module file:", fpath.c_str());
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
        // if the compiler did not work, then remove the CMake(LANG)Compiler.cmake file
        // so that it will get tested the next time cmake is run
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
        } // end if in try compile
      } // end need test language
    } // end for each language
  
  // Now load files that can override any settings on the platform or
  // for the project
  // First load the project compatibility file if it is in cmake
  std::string projectCompatibility = mf->GetDefinition("CMAKE_ROOT");
  projectCompatibility += "/Modules/";
  projectCompatibility += mf->GetDefinition("PROJECT_NAME");
  projectCompatibility += "Compatibility.cmake";
  if(cmSystemTools::FileExists(projectCompatibility.c_str()))
    {
    mf->ReadListFile(0,projectCompatibility.c_str()); 
    }
}

const char* cmGlobalGenerator::GetLanguageOutputExtensionForLanguage(const char* lang)
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
    if(this->OutputExtensions.count(ext))
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

void cmGlobalGenerator::Configure()
{
  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    delete this->LocalGenerators[i];
    }
  this->LocalGenerators.clear();

  // Setup relative path generation.
  this->ConfigureRelativePaths();

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
      this->CMakeInstance->UpdateProgress("Configuring", 
                                      0.9f+0.1f*(i+1.0f)/this->LocalGenerators.size());
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
    cmTargets* targets = &(this->LocalGenerators[i]->GetMakefile()->GetTargets());
    cmTargets::iterator tarIt;
    for ( tarIt = targets->begin(); tarIt != targets->end(); ++ tarIt )
      {
      if ( tarIt->second.GetType() == cmTarget::GLOBAL_TARGET )
        {
        globalTargets[tarIt->first] = tarIt->second;
        }
      }
    }
  
  // Generate project files
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    cmTargets* targets = &(this->LocalGenerators[i]->GetMakefile()->GetTargets());
    cmTargets::iterator tit;
    for ( tit = globalTargets.begin(); tit != globalTargets.end(); ++ tit )
      {
      (*targets)[tit->first] = tit->second;
      }
    this->LocalGenerators[i]->Generate();
    this->LocalGenerators[i]->GenerateInstallRules();
    this->LocalGenerators[i]->GenerateTestFiles();
    this->CMakeInstance->UpdateProgress("Generating", 
                                    (i+1.0f)/this->LocalGenerators.size());
    }
  this->CMakeInstance->UpdateProgress("Generating done", -1);
}

int cmGlobalGenerator::TryCompile(const char *srcdir, const char *bindir, 
                                  const char *projectName, 
                                  const char *target,
                                  std::string *output, cmMakefile *mf)
{
  std::string makeCommand = 
    this->CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
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
  const char *projectName, const char* additionalOptions, const char *targetName,
  const char* config, bool ignoreErrors)
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
    std::string cleanCommand = this->GenerateBuildCommand(makeCommandCSTR, projectName,
      0, "clean", config, false);
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
  std::string makeCommand = this->GenerateBuildCommand(makeCommandCSTR, projectName,
    0, target, config, false);

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

cmLocalGenerator *cmGlobalGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

void cmGlobalGenerator::EnableLanguagesFromGenerator(cmGlobalGenerator *gen )
{
  std::string cfp = gen->GetCMakeInstance()->GetHomeOutputDirectory();
  cfp += "/CMakeFiles";
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
  for(std::map<cmStdString, bool>::iterator i = this->LanguageEnabled.begin(); 
      i != this->LanguageEnabled.end(); ++i)
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
                                        const char* name)
{
  std::vector<cmLocalGenerator*>* gens = &this->LocalGenerators;
  if(project)
    {
    gens = &this->ProjectMap[project];
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
  // The current working directory on Windows cannot be a network
  // path.  Therefore relative paths cannot work when the build tree
  // is a network path.
  std::string source = this->CMakeInstance->GetHomeDirectory();
  std::string binary = this->CMakeInstance->GetHomeOutputDirectory();
  if(binary.size() < 2 || binary.substr(0, 2) != "//")
    {
    this->RelativePathTopSource = source;
    this->RelativePathTopBinary = binary;
    }
  else
    {
    this->RelativePathTopSource = "";
    this->RelativePathTopBinary = "";
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

  // If the path is already relative then just return the path.
  if(!cmSystemTools::FileIsFullPath(in_remote))
    {
    return in_remote;
    }

  // Skip conversion if the path is not in the source or binary tree.
  std::string original = in_remote;
  if((original.size() < this->RelativePathTopSource.size() ||
      !cmSystemTools::ComparePath(
        original.substr(0, this->RelativePathTopSource.size()).c_str(),
        this->RelativePathTopSource.c_str())) &&
     (original.size() < this->RelativePathTopBinary.size() ||
      !cmSystemTools::ComparePath(
        original.substr(0, this->RelativePathTopBinary.size()).c_str(),
        this->RelativePathTopBinary.c_str())))
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

  // If no part of the path is in common then return the full path.
  if(common == 0)
    {
    return in_remote;
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
    this->LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
  ctest = removeQuotes(ctest);
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  ctest += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      this->LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Debug/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      this->LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
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
    fname = this->CMakeInstance->GetStartOutputDirectory();
    fname += "/";
    if ( this->LocalGenerators[0]->GetMakefile()->IsSet("CTEST_NEW_FORMAT") )
      {
      fname += "CTestTestfile.txt";
      }
    else
      {
      fname += "DartTestfile.txt";
      }

    // Add run_test only if any tests are foun
    long total_tests = 0;
    unsigned int i;
    for (i = 0; i < this->LocalGenerators.size(); ++i)
      {
      total_tests += this->LocalGenerators[i]->GetMakefile()->GetTests()->size();
      }

    // If the file doesn't exist, then ENABLE_TESTING hasn't been run
    if (total_tests > 0)
      {
      const char* no_output = 0;
      const char* no_working_dir = 0;
      std::vector<std::string> no_depends;
      std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
      for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
        {
        std::vector<cmLocalGenerator*>& gen = it->second;
        // add the RUN_TESTS to the first local generator of each project
        if(gen.size())
          {
          cmMakefile* mf = gen[0]->GetMakefile();
          if(const char* outDir = mf->GetDefinition("CMAKE_CFG_INTDIR"))
            {
            mf->AddUtilityCommand("RUN_TESTS", false, no_output, no_depends,
                                  no_working_dir, 
                                  ctest.c_str(), "-C", outDir);
            }
          }
        }
      }
    }
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
  cpackCommandLines.erase(cpackCommandLines.begin(), cpackCommandLines.end());
  singleLine.erase(singleLine.begin(), singleLine.end());
  depends.erase(depends.begin(), depends.end());
  singleLine.push_back(this->GetCMakeInstance()->GetCPackCommand());
  if ( cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.' )
    {
    singleLine.push_back("-C");
    singleLine.push_back(mf->GetDefinition("CMAKE_CFG_INTDIR"));
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
  (*targets)[this->GetPackageTargetName()]
    = this->CreateGlobalTarget(this->GetPackageTargetName(),
      "Run CPack packaging tool...", &cpackCommandLines, depends);

  // Test
  if(mf->IsOn("CMAKE_TESTING_ENABLED"))
    {
    cpackCommandLines.erase(cpackCommandLines.begin(), cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(this->GetCMakeInstance()->GetCTestCommand());
    singleLine.push_back("--force-new-ctest-process");
    cpackCommandLines.push_back(singleLine);
    (*targets)[this->GetTestTargetName()]
      = this->CreateGlobalTarget(this->GetTestTargetName(),
        "Running tests...", &cpackCommandLines, depends);
    }

  //Edit Cache
  const char* editCacheTargetName = this->GetEditCacheTargetName();
  if ( editCacheTargetName )
    {
    cpackCommandLines.erase(cpackCommandLines.begin(), cpackCommandLines.end());
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
      singleLine.push_back("-H$(CMAKE_SOURCE_DIR)");
      singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
      singleLine.push_back("-i");
      cpackCommandLines.push_back(singleLine);
      (*targets)[editCacheTargetName] =
        this->CreateGlobalTarget(
          editCacheTargetName, "Running interactive CMake command-line interface...",
          &cpackCommandLines, depends);
      }
    }

  //Rebuild Cache
  const char* rebuildCacheTargetName = this->GetRebuildCacheTargetName();
  if ( rebuildCacheTargetName )
    {
    cpackCommandLines.erase(cpackCommandLines.begin(), cpackCommandLines.end());
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
  std::string cmd;
  cpackCommandLines.erase(cpackCommandLines.begin(), cpackCommandLines.end());
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
}

cmTarget cmGlobalGenerator::CreateGlobalTarget(
  const char* name, const char* message,
  const cmCustomCommandLines* commandLines,
  std::vector<std::string> depends,
  bool depends_on_all /* = false */)
{
  // Package
  cmTarget target;
  target.SetType(cmTarget::GLOBAL_TARGET, name);
  target.SetInAll(false);

  std::vector<std::string> fileDepends;
  // Store the custom command in the target.
  cmCustomCommand cc(0, fileDepends, *commandLines, 0, 0);
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
