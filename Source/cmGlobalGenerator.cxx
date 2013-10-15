/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#if defined(_WIN32) && !defined(__CYGWIN__)
#include "windows.h" // this must be first to define GetCurrentDirectory
#endif

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmQtAutomoc.h"
#include "cmSourceFile.h"
#include "cmVersion.h"
#include "cmTargetExport.h"
#include "cmComputeTargetDepends.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmExportBuildFileGenerator.h"

#include <cmsys/Directory.hxx>

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cmsys/MD5.h>
#endif

#include <stdlib.h> // required for atof

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
  this->CurrentLocalGenerator = 0;
  this->TryCompileOuterMakefile = 0;
}

cmGlobalGenerator::~cmGlobalGenerator()
{
  // Delete any existing cmLocalGenerators
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i)
    {
    delete this->LocalGenerators[i];
    }
  for(std::vector<cmGeneratorExpressionEvaluationFile*>::const_iterator
      li = this->EvaluationFiles.begin();
      li != this->EvaluationFiles.end();
      ++li)
    {
    delete *li;
    }
  for(std::map<std::string, cmExportBuildFileGenerator*>::iterator
        i = this->BuildExportSets.begin();
      i != this->BuildExportSets.end(); ++i)
    {
    delete i->second;
    }
  this->LocalGenerators.clear();

  if (this->ExtraGenerator)
    {
    delete this->ExtraGenerator;
    }

  this->ClearGeneratorTargets();
}

bool cmGlobalGenerator::SetGeneratorToolset(std::string const& ts)
{
  cmOStringStream e;
  e <<
    "Generator\n"
    "  " << this->GetName() << "\n"
    "does not support toolset specification, but toolset\n"
    "  " << ts << "\n"
    "was specified.";
  this->CMakeInstance->IssueMessage(cmake::FATAL_ERROR, e.str(),
                                    cmListFileBacktrace());
  return false;
}

void cmGlobalGenerator::ResolveLanguageCompiler(const std::string &lang,
                                                cmMakefile *mf,
                                                bool optional)
{
  std::string langComp = "CMAKE_";
  langComp += lang;
  langComp += "_COMPILER";

  if(!mf->GetDefinition(langComp.c_str()))
    {
    if(!optional)
      {
      cmSystemTools::Error(langComp.c_str(),
                           " not set, after EnableLanguage");
      }
    return;
    }
  const char* name = mf->GetRequiredDefinition(langComp.c_str());
  std::string path;
  if(!cmSystemTools::FileIsFullPath(name))
    {
    path = cmSystemTools::FindProgram(name);
    }
  else
    {
    path = name;
    }
  if((path.size() == 0 || !cmSystemTools::FileExists(path.c_str()))
      && (optional==false))
    {
    std::string message = "your ";
    message += lang;
    message += " compiler: \"";
    message +=  name;
    message += "\" was not found.   Please set ";
    message += langComp;
    message += " to a valid compiler path or name.";
    cmSystemTools::Error(message.c_str());
    path = name;
    }
  std::string doc = lang;
  doc += " compiler.";
  const char* cname = this->GetCMakeInstance()->
    GetCacheManager()->GetCacheValue(langComp.c_str());
  std::string changeVars;
  if(cname && !optional)
    {
    std::string cnameString;
    if(!cmSystemTools::FileIsFullPath(cname))
      {
      cnameString = cmSystemTools::FindProgram(cname);
      }
    else
      {
      cnameString = cname;
      }
    std::string pathString = path;
    // get rid of potentially multiple slashes:
    cmSystemTools::ConvertToUnixSlashes(cnameString);
    cmSystemTools::ConvertToUnixSlashes(pathString);
    if (cnameString != pathString)
      {
      const char* cvars =
        this->GetCMakeInstance()->GetProperty(
          "__CMAKE_DELETE_CACHE_CHANGE_VARS_");
      if(cvars)
        {
        changeVars += cvars;
        changeVars += ";";
        }
      changeVars += langComp;
      changeVars += ";";
      changeVars += cname;
      this->GetCMakeInstance()->SetProperty(
        "__CMAKE_DELETE_CACHE_CHANGE_VARS_",
        changeVars.c_str());
      }
    }
  mf->AddCacheDefinition(langComp.c_str(), path.c_str(),
                         doc.c_str(), cmCacheManager::FILEPATH);
}

void cmGlobalGenerator::AddBuildExportSet(cmExportBuildFileGenerator* gen)
{
  this->BuildExportSets[gen->GetMainExportFileName()] = gen;
}

bool cmGlobalGenerator::GenerateImportFile(const std::string &file)
{
  std::map<std::string, cmExportBuildFileGenerator*>::iterator it
                                          = this->BuildExportSets.find(file);
  if (it != this->BuildExportSets.end())
    {
    bool result = it->second->GenerateImportFile();
    delete it->second;
    it->second = 0;
    this->BuildExportSets.erase(it);
    return result;
    }
  return false;
}

bool
cmGlobalGenerator::IsExportedTargetsFile(const std::string &filename) const
{
  const std::map<std::string, cmExportBuildFileGenerator*>::const_iterator it
                                      = this->BuildExportSets.find(filename);
  return it != this->BuildExportSets.end();
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
//                                CMakeSystem.cmake IF CMAKE_SYSTEM
//                                not set
//   CMakeSystem.cmake - configured file created by
//                       CMakeDetermineSystem.cmake IF CMAKE_SYSTEM_LOADED

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
// (PROJECTNAME)Compatibility.cmake
//   - load any backwards compatibility stuff for current project
// ${CMAKE_USER_MAKE_RULES_OVERRIDE}
//   - allow users a chance to override system variables
//
//

void
cmGlobalGenerator::EnableLanguage(std::vector<std::string>const& languages,
                                  cmMakefile *mf, bool)
{
  if(languages.size() == 0)
    {
    cmSystemTools::Error("EnableLanguage must have a lang specified!");
    cmSystemTools::SetFatalErrorOccured();
    return;
    }

  if(this->TryCompileOuterMakefile)
    {
    // In a try-compile we can only enable languages provided by caller.
    for(std::vector<std::string>::const_iterator li = languages.begin();
        li != languages.end(); ++li)
      {
      if(*li == "NONE")
        {
        this->SetLanguageEnabled("NONE", mf);
        }
      else
        {
        const char* lang = li->c_str();
        if(this->LanguagesReady.find(lang) == this->LanguagesReady.end())
          {
          cmOStringStream e;
          e << "The test project needs language "
            << lang << " which is not enabled.";
          this->TryCompileOuterMakefile
            ->IssueMessage(cmake::FATAL_ERROR, e.str());
          cmSystemTools::SetFatalErrorOccured();
          return;
          }
        }
      }
    }

  mf->AddDefinition("RUN_CONFIGURE", true);
  std::string rootBin = mf->GetHomeOutputDirectory();
  rootBin += cmake::GetCMakeFilesDirectory();

  // If the configuration files path has been set,
  // then we are in a try compile and need to copy the enable language
  // files from the parent cmake bin dir, into the try compile bin dir
  if(this->ConfiguredFilesPath.size())
    {
    rootBin = this->ConfiguredFilesPath;
    }
  rootBin += "/";
  rootBin += cmVersion::GetCMakeVersion();

  // set the dir for parent files so they can be used by modules
  mf->AddDefinition("CMAKE_PLATFORM_INFO_DIR",rootBin.c_str());

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
  if (!mf->GetDefinition("CMAKE_SYSTEM"))
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
  std::map<cmStdString, bool> needSetLanguageEnabledMaps;
  // foreach language
  // load the CMakeDetermine(LANG)Compiler.cmake file to find
  // the compiler

  for(std::vector<std::string>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    const char* lang = l->c_str();
    needSetLanguageEnabledMaps[lang] = false;
    if(*l == "NONE")
      {
      this->SetLanguageEnabled("NONE", mf);
      continue;
      }
    std::string loadedLang = "CMAKE_";
    loadedLang +=  lang;
    loadedLang += "_COMPILER_LOADED";
    if(!mf->GetDefinition(loadedLang.c_str()))
      {
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";

      // If the existing build tree was already configured with this
      // version of CMake then try to load the configured file first
      // to avoid duplicate compiler tests.
      if(cmSystemTools::FileExists(fpath.c_str()))
        {
        if(!mf->ReadListFile(0,fpath.c_str()))
          {
          cmSystemTools::Error("Could not find cmake module file: ",
                               fpath.c_str());
          }
        // if this file was found then the language was already determined
        // to be working
        needTestLanguage[lang] = false;
        this->SetLanguageEnabledFlag(lang, mf);
        needSetLanguageEnabledMaps[lang] = true;
        // this can only be called after loading CMake(LANG)Compiler.cmake
        }
      }

    if(!this->GetLanguageEnabled(lang) )
      {
      if (this->CMakeInstance->GetIsInTryCompile())
        {
        cmSystemTools::Error("This should not have happened. "
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
        cmSystemTools::Error("Could not find cmake module file: ",
                             determineFile.c_str());
        }
      needTestLanguage[lang] = true;
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

      // if determineLanguage was called then load the file it
      // configures CMake(LANG)Compiler.cmake
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";
      if(!mf->ReadListFile(0,fpath.c_str()))
        {
        cmSystemTools::Error("Could not find cmake module file: ",
                             fpath.c_str());
        }
      this->SetLanguageEnabledFlag(lang, mf);
      needSetLanguageEnabledMaps[lang] = true;
      // this can only be called after loading CMake(LANG)Compiler.cmake
      // the language must be enabled for try compile to work, but we do
      // not know if it is a working compiler yet so set the test language
      // flag
      needTestLanguage[lang] = true;
      } // end if(!this->GetLanguageEnabled(lang) )
    }  // end loop over languages

  // **** Load the system specific information if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED"))
    {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if(!mf->ReadListFile(0,fpath.c_str()))
      {
      cmSystemTools::Error("Could not find cmake module file: ",
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
      std::string informationFile = mf->GetModulesFile(fpath.c_str());
      if (informationFile.empty())
        {
        cmSystemTools::Error("Could not find cmake module file: ",
                             fpath.c_str());
        }
      else if(!mf->ReadListFile(0, informationFile.c_str()))
        {
        cmSystemTools::Error("Could not process cmake module file: ",
                             informationFile.c_str());
        }
      }
    if (needSetLanguageEnabledMaps[lang])
      {
      this->SetLanguageEnabledMaps(lang, mf);
      }
    this->LanguagesReady.insert(lang);

    std::string compilerName = "CMAKE_";
    compilerName += lang;
    compilerName += "_COMPILER";
    std::string compilerLangFile = rootBin;
    compilerLangFile += "/CMake";
    compilerLangFile += lang;
    compilerLangFile += "Compiler.cmake";
    // Test the compiler for the language just setup
    // (but only if a compiler has been actually found)
    // At this point we should have enough info for a try compile
    // which is used in the backward stuff
    // If the language is untested then test it now with a try compile.
    if (!mf->IsSet(compilerName.c_str()))
      {
      // if the compiler did not work, then remove the
      // CMake(LANG)Compiler.cmake file so that it will get tested the
      // next time cmake is run
      cmSystemTools::RemoveFile(compilerLangFile.c_str());
      }
    else if(needTestLanguage[lang])
      {
      if (!this->CMakeInstance->GetIsInTryCompile())
        {
        std::string testLang = "CMakeTest";
        testLang += lang;
        testLang += "Compiler.cmake";
        std::string ifpath = mf->GetModulesFile(testLang.c_str());
        if(!mf->ReadListFile(0,ifpath.c_str()))
          {
          cmSystemTools::Error("Could not find cmake module file: ",
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
          cmSystemTools::RemoveFile(compilerLangFile.c_str());
          }
        else
          {
          // load backwards compatibility stuff for C and CXX
          // for old versions of CMake ListFiles C and CXX had some
          // backwards compatibility files they have to load
          // These files have a bunch of try compiles in them so
          // should only be done
          if (mf->NeedBackwardsCompatibility(1,4))
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
    // Store the shared library flags so that we can satisfy CMP0018
    std::string sharedLibFlagsVar = "CMAKE_SHARED_LIBRARY_";
    sharedLibFlagsVar += lang;
    sharedLibFlagsVar += "_FLAGS";
    const char* sharedLibFlags =
      mf->GetSafeDefinition(sharedLibFlagsVar.c_str());
    if (sharedLibFlags)
      {
      this->LanguageToOriginalSharedLibFlags[lang] = sharedLibFlags;
      }

    // Translate compiler ids for compatibility.
    this->CheckCompilerIdCompatibility(mf, lang);
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

//----------------------------------------------------------------------------
void cmGlobalGenerator::CheckCompilerIdCompatibility(cmMakefile* mf,
                                                     std::string lang)
{
  std::string compilerIdVar = "CMAKE_" + lang + "_COMPILER_ID";
  const char* compilerId = mf->GetDefinition(compilerIdVar.c_str());
  if(compilerId && strcmp(compilerId, "AppleClang") == 0)
    {
    cmPolicies* policies = this->CMakeInstance->GetPolicies();
    switch(mf->GetPolicyStatus(cmPolicies::CMP0025))
      {
      case cmPolicies::WARN:
        if(!this->CMakeInstance->GetIsInTryCompile())
          {
          cmOStringStream w;
          w << policies->GetPolicyWarning(cmPolicies::CMP0025) << "\n"
            "Converting " << lang <<
            " compiler id \"AppleClang\" to \"Clang\" for compatibility."
            ;
          mf->IssueMessage(cmake::AUTHOR_WARNING, w.str());
          }
      case cmPolicies::OLD:
        // OLD behavior is to convert AppleClang to Clang.
        mf->AddDefinition(compilerIdVar.c_str(), "Clang");
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        mf->IssueMessage(
          cmake::FATAL_ERROR,
          policies->GetRequiredPolicyError(cmPolicies::CMP0025)
          );
      case cmPolicies::NEW:
        // NEW behavior is to keep AppleClang.
        break;
      }
    }
}

//----------------------------------------------------------------------------
const char*
cmGlobalGenerator::GetLanguageOutputExtension(cmSourceFile const& source)
{
  if(const char* lang = source.GetLanguage())
    {
    if(this->LanguageToOutputExtension.count(lang) > 0)
      {
      return this->LanguageToOutputExtension[lang].c_str();
      }
    }
  else
    {
    // if no language is found then check to see if it is already an
    // ouput extension for some language.  In that case it should be ignored
    // and in this map, so it will not be compiled but will just be used.
    std::string const& ext = source.GetExtension();
    if(!ext.empty())
      {
      if(this->OutputExtensions.count(ext))
        {
        return ext.c_str();
        }
      }
    }
  return "";
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

/* SetLanguageEnabled() is now split in two parts:
at first the enabled-flag is set. This can then be used in EnabledLanguage()
for checking whether the language is already enabled. After setting this
flag still the values from the cmake variables have to be copied into the
internal maps, this is done in SetLanguageEnabledMaps() which is called
after the system- and compiler specific files have been loaded.

This split was done originally so that compiler-specific configuration
files could change the object file extension
(CMAKE_<LANG>_OUTPUT_EXTENSION) before the CMake variables were copied
to the C++ maps.
*/
void cmGlobalGenerator::SetLanguageEnabled(const char* l, cmMakefile* mf)
{
  this->SetLanguageEnabledFlag(l, mf);
  this->SetLanguageEnabledMaps(l, mf);
}

void cmGlobalGenerator::SetLanguageEnabledFlag(const char* l, cmMakefile* mf)
{
  this->LanguageEnabled[l] = true;

  // Fill the language-to-extension map with the current variable
  // settings to make sure it is available for the try_compile()
  // command source file signature.  In SetLanguageEnabledMaps this
  // will be done again to account for any compiler- or
  // platform-specific entries.
  this->FillExtensionToLanguageMap(l, mf);
}

void cmGlobalGenerator::SetLanguageEnabledMaps(const char* l, cmMakefile* mf)
{
  // use LanguageToLinkerPreference to detect whether this functions has
  // run before
  if (this->LanguageToLinkerPreference.find(l) !=
                                        this->LanguageToLinkerPreference.end())
    {
    return;
    }

  std::string linkerPrefVar = std::string("CMAKE_") +
    std::string(l) + std::string("_LINKER_PREFERENCE");
  const char* linkerPref = mf->GetDefinition(linkerPrefVar.c_str());
  int preference = 0;
  if(linkerPref)
    {
    if (sscanf(linkerPref, "%d", &preference)!=1)
      {
      // backward compatibility: before 2.6 LINKER_PREFERENCE
      // was either "None" or "Prefered", and only the first character was
      // tested. So if there is a custom language out there and it is
      // "Prefered", set its preference high
      if (linkerPref[0]=='P')
        {
        preference = 100;
        }
      else
        {
        preference = 0;
        }
      }
    }

  if (preference < 0)
    {
    std::string msg = linkerPrefVar;
    msg += " is negative, adjusting it to 0";
    cmSystemTools::Message(msg.c_str(), "Warning");
    preference = 0;
    }

  this->LanguageToLinkerPreference[l] = preference;

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

  // The map was originally filled by SetLanguageEnabledFlag, but
  // since then the compiler- and platform-specific files have been
  // loaded which might have added more entries.
  this->FillExtensionToLanguageMap(l, mf);

  std::string ignoreExtensionsVar = std::string("CMAKE_") +
    std::string(l) + std::string("_IGNORE_EXTENSIONS");
  std::string ignoreExts = mf->GetSafeDefinition(ignoreExtensionsVar.c_str());
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(ignoreExts, extensionList);
  for(std::vector<std::string>::iterator i = extensionList.begin();
      i != extensionList.end(); ++i)
    {
    this->IgnoreExtensions[*i] = true;
    }

}

void cmGlobalGenerator::FillExtensionToLanguageMap(const char* l,
                                                   cmMakefile* mf)
{
  std::string extensionsVar = std::string("CMAKE_") +
    std::string(l) + std::string("_SOURCE_FILE_EXTENSIONS");
  std::string exts = mf->GetSafeDefinition(extensionsVar.c_str());
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(exts, extensionList);
  for(std::vector<std::string>::iterator i = extensionList.begin();
      i != extensionList.end(); ++i)
    {
    this->ExtensionToLanguage[*i] = l;
    }
}

bool cmGlobalGenerator::IgnoreFile(const char* l)
{
  if(this->GetLanguageFromExtension(l))
    {
    return false;
    }
  return (this->IgnoreExtensions.count(l) > 0);
}

bool cmGlobalGenerator::GetLanguageEnabled(const char* l) const
{
  return (this->LanguageEnabled.find(l)!= this->LanguageEnabled.end());
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
      TargetDependSet const& tgtdeps = this->GetTargetDirectDepends(target);
      if(tgtdeps.count(targetIn))
        {
        return true;
        }
      }
    }
  return false;
}

void cmGlobalGenerator::Configure()
{
  this->FirstTimeProgress = 0.0f;
  this->ClearGeneratorTargets();
  this->ExportSets.clear();
  // Delete any existing cmLocalGenerators
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    delete this->LocalGenerators[i];
    }
  this->LocalGenerators.clear();
  this->TargetDependencies.clear();
  this->TotalTargets.clear();
  this->ImportedTargets.clear();
  this->LocalGeneratorToTargetMap.clear();
  this->ProjectMap.clear();
  this->RuleHashes.clear();
  this->DirectoryContentMap.clear();
  this->BinaryDirectories.clear();

  // start with this directory
  cmLocalGenerator *lg = this->CreateLocalGenerator();
  this->LocalGenerators.push_back(lg);

  // set the Start directories
  cmMakefile* mf = lg->GetMakefile();
  lg->GetMakefile()->SetStartDirectory
    (this->CMakeInstance->GetStartDirectory());
  lg->GetMakefile()->SetStartOutputDirectory
    (this->CMakeInstance->GetStartOutputDirectory());
  lg->GetMakefile()->MakeStartDirectoriesCurrent();

  this->BinaryDirectories.insert(mf->GetStartOutputDirectory());

  // now do it
  lg->Configure();

  // update the cache entry for the number of local generators, this is used
  // for progress
  char num[100];
  sprintf(num,"%d",static_cast<int>(this->LocalGenerators.size()));
  this->GetCMakeInstance()->AddCacheEntry
    ("CMAKE_NUMBER_OF_LOCAL_GENERATORS", num,
     "number of local generators", cmCacheManager::INTERNAL);

  // check for link libraries and include directories containing "NOTFOUND"
  // and for infinite loops
  this->CheckLocalGenerators();

  // at this point this->LocalGenerators has been filled,
  // so create the map from project name to vector of local generators
  this->FillProjectMap();

  if ( this->CMakeInstance->GetWorkingMode() == cmake::NORMAL_MODE)
    {
    cmOStringStream msg;
    if(cmSystemTools::GetErrorOccuredFlag())
      {
      msg << "Configuring incomplete, errors occurred!";
      const char* logs[] = {"CMakeOutput.log", "CMakeError.log", 0};
      for(const char** log = logs; *log; ++log)
        {
        std::string f = this->CMakeInstance->GetHomeOutputDirectory();
        f += this->CMakeInstance->GetCMakeFilesDirectory();
        f += "/";
        f += *log;
        if(cmSystemTools::FileExists(f.c_str()))
          {
          msg << "\nSee also \"" << f << "\".";
          }
        }
      }
    else
      {
      msg << "Configuring done";
      }
    this->CMakeInstance->UpdateProgress(msg.str().c_str(), -1);
    }
}

cmExportBuildFileGenerator*
cmGlobalGenerator::GetExportedTargetsFile(const std::string &filename) const
{
  std::map<std::string, cmExportBuildFileGenerator*>::const_iterator it
    = this->BuildExportSets.find(filename);
  return it == this->BuildExportSets.end() ? 0 : it->second;
}

bool cmGlobalGenerator::CheckALLOW_DUPLICATE_CUSTOM_TARGETS()
{
  // If the property is not enabled then okay.
  if(!this->CMakeInstance
     ->GetPropertyAsBool("ALLOW_DUPLICATE_CUSTOM_TARGETS"))
    {
    return true;
    }

  // This generator does not support duplicate custom targets.
  cmOStringStream e;
  e << "This project has enabled the ALLOW_DUPLICATE_CUSTOM_TARGETS "
    << "global property.  "
    << "The \"" << this->GetName() << "\" generator does not support "
    << "duplicate custom targets.  "
    << "Consider using a Makefiles generator or fix the project to not "
    << "use duplicate target names.";
  cmSystemTools::Error(e.str().c_str());
  return false;
}

void cmGlobalGenerator::Generate()
{
  // Some generators track files replaced during the Generate.
  // Start with an empty vector:
  this->FilesReplacedDuringGenerate.clear();

  // Check whether this generator is allowed to run.
  if(!this->CheckALLOW_DUPLICATE_CUSTOM_TARGETS())
    {
    return;
    }

  // Check that all targets are valid.
  if(!this->CheckTargets())
    {
    return;
    }

  // Iterate through all targets and set up automoc for those which have
  // the AUTOMOC property set
  this->CreateAutomocTargets();

  // For each existing cmLocalGenerator
  unsigned int i;

  // Put a copy of each global target in every directory.
  cmTargets globalTargets;
  this->CreateDefaultGlobalTargets(&globalTargets);
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

    for ( tit = targets->begin(); tit != targets->end(); ++ tit )
      {
      tit->second.AppendBuildInterfaceIncludes();
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
    this->LocalGenerators[i]->GenerateTargetManifest();
    }

  // Create per-target generator information.
  this->CreateGeneratorTargets();

  this->ProcessEvaluationFiles();

  // Compute the inter-target dependencies.
  if(!this->ComputeTargetDepends())
    {
    return;
    }

  // Create a map from local generator to the complete set of targets
  // it builds by default.
  this->FillLocalGeneratorToTargetMap();

  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = this->LocalGenerators[i]->GetMakefile();
    cmTargets* targets = &(mf->GetTargets());
    for ( cmTargets::iterator it = targets->begin();
        it != targets->end(); ++ it )
      {
      it->second.FinalizeSystemIncludeDirectories();
      }
    }

  // Generate project files
  for (i = 0; i < this->LocalGenerators.size(); ++i)
    {
    this->LocalGenerators[i]->GetMakefile()->SetGeneratingBuildSystem();
    this->SetCurrentLocalGenerator(this->LocalGenerators[i]);
    this->LocalGenerators[i]->Generate();
    this->LocalGenerators[i]->GenerateInstallRules();
    this->LocalGenerators[i]->GenerateTestFiles();
    this->CMakeInstance->UpdateProgress("Generating",
      (static_cast<float>(i)+1.0f)/
       static_cast<float>(this->LocalGenerators.size()));
    }
  this->SetCurrentLocalGenerator(0);

  for (std::map<std::string, cmExportBuildFileGenerator*>::iterator
      it = this->BuildExportSets.begin(); it != this->BuildExportSets.end();
      ++it)
    {
    if (!it->second->GenerateImportFile()
        && !cmSystemTools::GetErrorOccuredFlag())
      {
      this->GetCMakeInstance()
          ->IssueMessage(cmake::FATAL_ERROR, "Could not write export file.",
                        cmListFileBacktrace());
      return;
      }
    }
  // Update rule hashes.
  this->CheckRuleHashes();

  this->WriteSummary();

  if (this->ExtraGenerator != 0)
    {
    this->ExtraGenerator->Generate();
    }

  this->CMakeInstance->UpdateProgress("Generating done", -1);
}

//----------------------------------------------------------------------------
bool cmGlobalGenerator::ComputeTargetDepends()
{
  cmComputeTargetDepends ctd(this);
  if(!ctd.Compute())
    {
    return false;
    }
  std::vector<cmTarget*> const& targets = ctd.GetTargets();
  for(std::vector<cmTarget*>::const_iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    ctd.GetTargetDirectDepends(*ti, this->TargetDependencies[*ti]);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmGlobalGenerator::CheckTargets()
{
  // Make sure all targets can find their source files.
  for(unsigned int i=0; i < this->LocalGenerators.size(); ++i)
    {
    cmTargets& targets =
      this->LocalGenerators[i]->GetMakefile()->GetTargets();
    for(cmTargets::iterator ti = targets.begin();
        ti != targets.end(); ++ti)
      {
      cmTarget& target = ti->second;
      if(target.GetType() == cmTarget::EXECUTABLE ||
         target.GetType() == cmTarget::STATIC_LIBRARY ||
         target.GetType() == cmTarget::SHARED_LIBRARY ||
         target.GetType() == cmTarget::MODULE_LIBRARY ||
         target.GetType() == cmTarget::UTILITY)
        {
        if(!target.FindSourceFiles())
          {
          return false;
          }
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::CreateAutomocTargets()
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  typedef std::vector<std::pair<cmQtAutomoc, cmTarget*> > Automocs;
  Automocs automocs;
  for(unsigned int i=0; i < this->LocalGenerators.size(); ++i)
    {
    cmTargets& targets =
      this->LocalGenerators[i]->GetMakefile()->GetTargets();
    for(cmTargets::iterator ti = targets.begin();
        ti != targets.end(); ++ti)
      {
      cmTarget& target = ti->second;
      if(target.GetType() == cmTarget::EXECUTABLE ||
         target.GetType() == cmTarget::STATIC_LIBRARY ||
         target.GetType() == cmTarget::SHARED_LIBRARY ||
         target.GetType() == cmTarget::MODULE_LIBRARY ||
         target.GetType() == cmTarget::OBJECT_LIBRARY)
        {
        if(target.GetPropertyAsBool("AUTOMOC") && !target.IsImported())
          {
          cmQtAutomoc automoc;
          if(automoc.InitializeMocSourceFile(&target))
            {
            automocs.push_back(std::make_pair(automoc, &target));
            }
          }
        }
      }
    }
  for (Automocs::iterator it = automocs.begin(); it != automocs.end();
       ++it)
    {
    it->first.SetupAutomocTarget(it->second);
    }
#endif
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::CreateGeneratorTargets()
{
  // Construct per-target generator information.
  for(unsigned int i=0; i < this->LocalGenerators.size(); ++i)
    {
    cmGeneratorTargetsType generatorTargets;

    cmMakefile *mf = this->LocalGenerators[i]->GetMakefile();

    const std::vector<cmValueWithOrigin> noconfig_compile_definitions =
                                mf->GetCompileDefinitionsEntries();

    std::vector<std::string> configs;
    mf->GetConfigurations(configs);

    cmTargets& targets = mf->GetTargets();
    for(cmTargets::iterator ti = targets.begin();
        ti != targets.end(); ++ti)
      {
      cmTarget* t = &ti->second;

      {
      for (std::vector<cmValueWithOrigin>::const_iterator it
                                      = noconfig_compile_definitions.begin();
          it != noconfig_compile_definitions.end(); ++it)
        {
        t->InsertCompileDefinition(*it);
        }

      for(std::vector<std::string>::const_iterator ci = configs.begin();
          ci != configs.end(); ++ci)
        {
        std::string defPropName = "COMPILE_DEFINITIONS_";
        defPropName += cmSystemTools::UpperCase(*ci);
        t->AppendProperty(defPropName.c_str(),
                          mf->GetProperty(defPropName.c_str()));
        }
      }

      cmGeneratorTarget* gt = new cmGeneratorTarget(t);
      this->GeneratorTargets[t] = gt;
      this->ComputeTargetObjects(gt);
      generatorTargets[t] = gt;
      }

    for(std::vector<cmTarget*>::const_iterator
          j = mf->GetOwnedImportedTargets().begin();
        j != mf->GetOwnedImportedTargets().end(); ++j)
      {
      cmGeneratorTarget* gt = new cmGeneratorTarget(*j);
      this->GeneratorTargets[*j] = gt;
      generatorTargets[*j] = gt;
      }

    mf->SetGeneratorTargets(generatorTargets);
    }
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::ClearGeneratorTargets()
{
  for(cmGeneratorTargetsType::iterator i = this->GeneratorTargets.begin();
      i != this->GeneratorTargets.end(); ++i)
    {
    delete i->second;
    }
  this->GeneratorTargets.clear();
}

//----------------------------------------------------------------------------
cmGeneratorTarget* cmGlobalGenerator::GetGeneratorTarget(cmTarget* t) const
{
  cmGeneratorTargetsType::const_iterator ti = this->GeneratorTargets.find(t);
  if(ti == this->GeneratorTargets.end())
    {
    this->CMakeInstance->IssueMessage(
      cmake::INTERNAL_ERROR, "Missing cmGeneratorTarget instance!",
      cmListFileBacktrace());
    return 0;
    }
  return ti->second;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::ComputeTargetObjects(cmGeneratorTarget*) const
{
  // Implemented in generator subclasses that need this.
}

void cmGlobalGenerator::CheckLocalGenerators()
{
  std::map<cmStdString, cmStdString> notFoundMap;
//  std::set<cmStdString> notFoundMap;
  // after it is all done do a ConfigureFinalPass
  cmCacheManager* manager = 0;
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i)
    {
    manager = this->LocalGenerators[i]->GetMakefile()->GetCacheManager();
    this->LocalGenerators[i]->ConfigureFinalPass();
    cmTargets &targets =
      this->LocalGenerators[i]->GetMakefile()->GetTargets();
    for (cmTargets::iterator l = targets.begin();
         l != targets.end(); l++)
      {
      const cmTarget::LinkLibraryVectorType& libs =
        l->second.GetOriginalLinkLibraries();
      for(cmTarget::LinkLibraryVectorType::const_iterator lib = libs.begin();
          lib != libs.end(); ++lib)
        {
        if(lib->first.size() > 9 &&
           cmSystemTools::IsNOTFOUND(lib->first.c_str()))
          {
          std::string varName = lib->first.substr(0, lib->first.size()-9);
          cmCacheManager::CacheIterator it =
            manager->GetCacheIterator(varName.c_str());
          if(it.GetPropertyAsBool("ADVANCED"))
            {
            varName += " (ADVANCED)";
            }
          std::string text = notFoundMap[varName];
          text += "\n    linked by target \"";
          text += l->second.GetName();
          text += "\" in directory ";
          text+=this->LocalGenerators[i]->GetMakefile()->GetCurrentDirectory();
          notFoundMap[varName] = text;
          }
        }
      std::vector<std::string> incs;
      const char *incDirProp = l->second.GetProperty("INCLUDE_DIRECTORIES");
      if (!incDirProp)
        {
        continue;
        }

      std::string incDirs = cmGeneratorExpression::Preprocess(incDirProp,
                        cmGeneratorExpression::StripAllGeneratorExpressions);

      cmSystemTools::ExpandListArgument(incDirs.c_str(), incs);

      for( std::vector<std::string>::const_iterator incDir = incs.begin();
            incDir != incs.end(); ++incDir)
        {
        if(incDir->size() > 9 &&
            cmSystemTools::IsNOTFOUND(incDir->c_str()))
          {
          std::string varName = incDir->substr(0, incDir->size()-9);
          cmCacheManager::CacheIterator it =
            manager->GetCacheIterator(varName.c_str());
          if(it.GetPropertyAsBool("ADVANCED"))
            {
            varName += " (ADVANCED)";
            }
          std::string text = notFoundMap[varName];
          text += "\n   used as include directory in directory ";
          text += this->LocalGenerators[i]
                      ->GetMakefile()->GetCurrentDirectory();
          notFoundMap[varName] = text;
          }
        }
      }
    this->CMakeInstance->UpdateProgress
      ("Configuring", 0.9f+0.1f*(static_cast<float>(i)+1.0f)/
        static_cast<float>(this->LocalGenerators.size()));
    }

  if(notFoundMap.size())
    {
    std::string notFoundVars;
    for(std::map<cmStdString, cmStdString>::const_iterator
        ii = notFoundMap.begin();
        ii != notFoundMap.end();
        ++ii)
      {
      notFoundVars += ii->first;
      notFoundVars += ii->second;
      notFoundVars += "\n";
      }
    cmSystemTools::Error("The following variables are used in this project, "
                         "but they are set to NOTFOUND.\n"
                         "Please set them or make sure they are set and "
                         "tested correctly in the CMake files:\n",
                         notFoundVars.c_str());
    }
}

int cmGlobalGenerator::TryCompile(const char *srcdir, const char *bindir,
                                  const char *projectName,
                                  const char *target, bool fast,
                                  std::string *output, cmMakefile *mf)
{
  // if this is not set, then this is a first time configure
  // and there is a good chance that the try compile stuff will
  // take the bulk of the time, so try and guess some progress
  // by getting closer and closer to 100 without actually getting there.
  if (!this->CMakeInstance->GetCacheManager()->GetCacheValue
      ("CMAKE_NUMBER_OF_LOCAL_GENERATORS"))
    {
    // If CMAKE_NUMBER_OF_LOCAL_GENERATORS is not set
    // we are in the first time progress and we have no
    // idea how long it will be.  So, just move 1/10th of the way
    // there each time, and don't go over 95%
    this->FirstTimeProgress += ((1.0f - this->FirstTimeProgress) /30.0f);
    if(this->FirstTimeProgress > 0.95f)
      {
      this->FirstTimeProgress = 0.95f;
      }
    this->CMakeInstance->UpdateProgress("Configuring",
                                        this->FirstTimeProgress);
    }

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
                     output,makeCommand.c_str(),config,false,fast,
                     this->TryCompileTimeout);
}

std::string cmGlobalGenerator
::GenerateBuildCommand(const char* makeProgram, const char *projectName,
                       const char *projectDir, const char* additionalOptions,
                       const char *targetName, const char* config,
                       bool ignoreErrors, bool)
{
  // Project name & dir and config are not used yet.
  (void)projectName;
  (void)projectDir;
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
  double timeout,
  cmSystemTools::OutputOption outputflag,
  const char* extraOptions,
  std::vector<std::string> const& nativeOptions)
{
  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);
  if(output)
    {
    *output += "Change Dir: ";
    *output += bindir;
    *output += "\n";
    }

  int retVal;
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();
  cmSystemTools::SetRunCommandHideConsole(true);
  std::string outputBuffer;
  std::string* outputPtr = 0;
  if(output)
    {
    outputPtr = &outputBuffer;
    }

  // should we do a clean first?
  if (clean)
    {
    std::string cleanCommand =
      this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir,
      0, "clean", config, false, fast);
    if(output)
      {
      *output += "\nRun Clean Command:";
      *output += cleanCommand;
      *output += "\n";
      }

    if (!cmSystemTools::RunSingleCommand(cleanCommand.c_str(), outputPtr,
                                         &retVal, 0, outputflag, timeout))
      {
      cmSystemTools::SetRunCommandHideConsole(hideconsole);
      cmSystemTools::Error("Generator: execution of make clean failed.");
      if (output)
        {
        *output += *outputPtr;
        *output += "\nGenerator: execution of make clean failed.\n";
        }

      // return to the original directory
      cmSystemTools::ChangeDirectory(cwd.c_str());
      return 1;
      }
    if (output)
      {
      *output += *outputPtr;
      }
    }

  // now build
  std::string makeCommand =
    this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir,
                               extraOptions, target,
                               config, false, fast);
  if(output)
    {
    *output += "\nRun Build Command:";
    *output += makeCommand;
    *output += "\n";
    }

  std::vector<cmStdString> command =
    cmSystemTools::ParseArguments(makeCommand.c_str());
  for(std::vector<std::string>::const_iterator ni = nativeOptions.begin();
      ni != nativeOptions.end(); ++ni)
    {
    command.push_back(*ni);
    }

  if (!cmSystemTools::RunSingleCommand(command, outputPtr,
                                       &retVal, 0, outputflag, timeout))
    {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    cmSystemTools::Error
      ("Generator: execution of make failed. Make command was: ",
       makeCommand.c_str());
    if (output)
      {
      *output += *outputPtr;
      *output += "\nGenerator: execution of make failed. Make command was: "
        + makeCommand + "\n";
      }

    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  if (output)
    {
    *output += *outputPtr;
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
    // If CMAKE_NUMBER_OF_LOCAL_GENERATORS is not set
    // we are in the first time progress and we have no
    // idea how long it will be.  So, just move half way
    // there each time, and don't go over 95%
    this->FirstTimeProgress += ((1.0f - this->FirstTimeProgress) /30.0f);
    if(this->FirstTimeProgress > 0.95f)
      {
      this->FirstTimeProgress = 0.95f;
      }
    this->CMakeInstance->UpdateProgress("Configuring",
                                        this->FirstTimeProgress);
    return;
    }

  int numGen = atoi(numGenC);
  float prog = 0.9f*static_cast<float>(this->LocalGenerators.size())/
    static_cast<float>(numGen);
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

void cmGlobalGenerator::EnableLanguagesFromGenerator(cmGlobalGenerator *gen,
                                                     cmMakefile* mf)
{
  this->SetConfiguredFilesPath(gen);
  this->TryCompileOuterMakefile = mf;
  const char* make =
    gen->GetCMakeInstance()->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_MAKE_PROGRAM", make,
                                          "make program",
                                          cmCacheManager::FILEPATH);
  // copy the enabled languages
  this->LanguageEnabled = gen->LanguageEnabled;
  this->LanguagesReady = gen->LanguagesReady;
  this->ExtensionToLanguage = gen->ExtensionToLanguage;
  this->IgnoreExtensions = gen->IgnoreExtensions;
  this->LanguageToOutputExtension = gen->LanguageToOutputExtension;
  this->LanguageToLinkerPreference = gen->LanguageToLinkerPreference;
  this->OutputExtensions = gen->OutputExtensions;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::SetConfiguredFilesPath(cmGlobalGenerator* gen)
{
  if(!gen->ConfiguredFilesPath.empty())
    {
    this->ConfiguredFilesPath = gen->ConfiguredFilesPath;
    }
  else
    {
    this->ConfiguredFilesPath = gen->CMakeInstance->GetHomeOutputDirectory();
    this->ConfiguredFilesPath += cmake::GetCMakeFilesDirectory();
    }
}

bool cmGlobalGenerator::IsExcluded(cmLocalGenerator* root,
                                   cmLocalGenerator* gen)
{
  if(!gen || gen == root)
    {
    // No directory excludes itself.
    return false;
    }

  if(gen->GetMakefile()->GetPropertyAsBool("EXCLUDE_FROM_ALL"))
    {
    // This directory is excluded from its parent.
    return true;
    }

  // This directory is included in its parent.  Check whether the
  // parent is excluded.
  return this->IsExcluded(root, gen->GetParent());
}

bool cmGlobalGenerator::IsExcluded(cmLocalGenerator* root,
                                   cmTarget& target)
{
  if(target.GetPropertyAsBool("EXCLUDE_FROM_ALL"))
    {
    // This target is excluded from its directory.
    return true;
    }
  else
    {
    // This target is included in its directory.  Check whether the
    // directory is excluded.
    return this->IsExcluded(root, target.GetMakefile()->GetLocalGenerator());
    }
}

void cmGlobalGenerator::GetEnabledLanguages(std::vector<std::string>& lang)
{
  for(std::map<cmStdString, bool>::iterator i =
        this->LanguageEnabled.begin(); i != this->LanguageEnabled.end(); ++i)
    {
    lang.push_back(i->first);
    }
}

int cmGlobalGenerator::GetLinkerPreference(const char* lang)
{
  std::map<cmStdString, int>::const_iterator it =
                                   this->LanguageToLinkerPreference.find(lang);
  if (it != this->LanguageToLinkerPreference.end())
    {
    return it->second;
    }
  return 0;
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


// Build a map that contains a the set of targets used by each local
// generator directory level.
void cmGlobalGenerator::FillLocalGeneratorToTargetMap()
{
  this->LocalGeneratorToTargetMap.clear();
  // Loop over all targets in all local generators.
  for(std::vector<cmLocalGenerator*>::const_iterator
        lgi = this->LocalGenerators.begin();
      lgi != this->LocalGenerators.end(); ++lgi)
    {
    cmLocalGenerator* lg = *lgi;
    cmMakefile* mf = lg->GetMakefile();
    cmTargets& targets = mf->GetTargets();
    for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
      {
      cmTarget& target = t->second;

      // Consider the directory containing the target and all its
      // parents until something excludes the target.
      for(cmLocalGenerator* clg = lg; clg && !this->IsExcluded(clg, target);
          clg = clg->GetParent())
        {
        // This local generator includes the target.
        std::set<cmTarget*>& targetSet =
          this->LocalGeneratorToTargetMap[clg];
        targetSet.insert(&target);

        // Add dependencies of the included target.  An excluded
        // target may still be included if it is a dependency of a
        // non-excluded target.
        TargetDependSet const& tgtdeps = this->GetTargetDirectDepends(target);
        for(TargetDependSet::const_iterator ti = tgtdeps.begin();
            ti != tgtdeps.end(); ++ti)
          {
          targetSet.insert(*ti);
          }
        }
      }
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

//----------------------------------------------------------------------------
void cmGlobalGenerator::AddAlias(const char *name, cmTarget *tgt)
{
  this->AliasTargets[name] = tgt;
}

//----------------------------------------------------------------------------
bool cmGlobalGenerator::IsAlias(const char *name)
{
  return this->AliasTargets.find(name) != this->AliasTargets.end();
}

//----------------------------------------------------------------------------
cmTarget*
cmGlobalGenerator::FindTarget(const char* project, const char* name,
                              bool excludeAliases)
{
  // if project specific
  if(project)
    {
    std::vector<cmLocalGenerator*>* gens = &this->ProjectMap[project];
    for(unsigned int i = 0; i < gens->size(); ++i)
      {
      cmTarget* ret = (*gens)[i]->GetMakefile()->FindTarget(name,
                                                            excludeAliases);
      if(ret)
        {
        return ret;
        }
      }
    }
  // if all projects/directories
  else
    {
    if (!excludeAliases)
      {
      std::map<cmStdString, cmTarget*>::iterator ai
                                              = this->AliasTargets.find(name);
      if (ai != this->AliasTargets.end())
        {
        return ai->second;
        }
      }
    std::map<cmStdString,cmTarget *>::iterator i =
      this->TotalTargets.find ( name );
    if ( i != this->TotalTargets.end() )
      {
      return i->second;
      }
    i = this->ImportedTargets.find(name);
    if ( i != this->ImportedTargets.end() )
      {
      return i->second;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
bool cmGlobalGenerator::NameResolvesToFramework(const std::string& libname)
{
  if(cmSystemTools::IsPathToFramework(libname.c_str()))
    {
    return true;
    }

  if(cmTarget* tgt = this->FindTarget(0, libname.c_str()))
    {
    if(tgt->IsFrameworkOnApple())
       {
       return true;
       }
    }

  return false;
}

//----------------------------------------------------------------------------
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
  const char* cmakeCfgIntDir = this->GetCMakeCFGIntDir();
  const char* cmakeCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");

  // CPack
  std::string workingDir =  mf->GetStartOutputDirectory();
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
  std::string relConfigFile = "./CPackConfig.cmake";
  singleLine.push_back(relConfigFile);
  cpackCommandLines.push_back(singleLine);
  if ( this->GetPreinstallTargetName() )
    {
    depends.push_back(this->GetPreinstallTargetName());
    }
  else
    {
    const char* noPackageAll =
      mf->GetDefinition("CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY");
    if(!noPackageAll || cmSystemTools::IsOff(noPackageAll))
      {
      depends.push_back(this->GetAllTargetName());
      }
    }
  if(cmSystemTools::FileExists(configFile.c_str()))
    {
    (*targets)[this->GetPackageTargetName()]
      = this->CreateGlobalTarget(this->GetPackageTargetName(),
                                 "Run CPack packaging tool...",
                                 &cpackCommandLines, depends,
                                 workingDir.c_str());
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
    relConfigFile = "./CPackSourceConfig.cmake";
    singleLine.push_back(relConfigFile);
    if(cmSystemTools::FileExists(configFile.c_str()))
      {
      singleLine.push_back(configFile);
      cpackCommandLines.push_back(singleLine);
      (*targets)[packageSourceTargetName]
        = this->CreateGlobalTarget(packageSourceTargetName,
                                   "Run CPack packaging tool for source...",
                                   &cpackCommandLines, depends,
                                   workingDir.c_str()
                                   );
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
        "Running tests...", &cpackCommandLines, depends, 0);
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
          &cpackCommandLines, depends, 0);
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
          &cpackCommandLines, depends, 0);
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
        &cpackCommandLines, depends, 0);
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
          &cpackCommandLines, depends, 0);
      }
    std::string cmd = cmakeCommand;
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
      // executable to install over itself.  The generator will
      // automatically convert this name to the build-time location.
      cmd = "cmake";
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
        &cpackCommandLines, depends, 0);

    // install_local
    if(const char* install_local = this->GetInstallLocalTargetName())
      {
      cmCustomCommandLine localCmdLine = singleLine;

      localCmdLine.insert(localCmdLine.begin()+1,
                                               "-DCMAKE_INSTALL_LOCAL_ONLY=1");
      cpackCommandLines.erase(cpackCommandLines.begin(),
                                                      cpackCommandLines.end());
      cpackCommandLines.push_back(localCmdLine);

      (*targets)[install_local] =
        this->CreateGlobalTarget(
          install_local, "Installing only the local directory...",
          &cpackCommandLines, depends, 0);
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
          &cpackCommandLines, depends, 0);
      }
    }
}

//----------------------------------------------------------------------------
const char* cmGlobalGenerator::GetPredefinedTargetsFolder()
{
  const char* prop =
    this->GetCMakeInstance()->GetProperty("PREDEFINED_TARGETS_FOLDER");

  if (prop)
    {
    return prop;
    }

  return "CMakePredefinedTargets";
}

//----------------------------------------------------------------------------
bool cmGlobalGenerator::UseFolderProperty()
{
  const char* prop = this->GetCMakeInstance()->GetProperty("USE_FOLDERS");

  // If this property is defined, let the setter turn this on or off...
  //
  if (prop)
    {
    return cmSystemTools::IsOn(prop);
    }

  // By default, this feature is OFF, since it is not supported in the
  // Visual Studio Express editions until VS11:
  //
  return false;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::EnableMinGWLanguage(cmMakefile *mf)
{
  this->FindMakeProgram(mf);
  std::string makeProgram = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::vector<std::string> locations;
  locations.push_back(cmSystemTools::GetProgramPath(makeProgram.c_str()));
  locations.push_back("/mingw/bin");
  locations.push_back("c:/mingw/bin");
  std::string tgcc = cmSystemTools::FindProgram("gcc", locations);
  std::string gcc = "gcc.exe";
  if(tgcc.size())
    {
    gcc = tgcc;
    }
  std::string tgxx = cmSystemTools::FindProgram("g++", locations);
  std::string gxx = "g++.exe";
  if(tgxx.size())
    {
    gxx = tgxx;
    }
  std::string trc = cmSystemTools::FindProgram("windres", locations);
  std::string rc = "windres.exe";
  if(trc.size())
    {
    rc = trc;
    }
  mf->AddDefinition("CMAKE_GENERATOR_CC", gcc.c_str());
  mf->AddDefinition("CMAKE_GENERATOR_CXX", gxx.c_str());
  mf->AddDefinition("CMAKE_GENERATOR_RC", rc.c_str());
}

//----------------------------------------------------------------------------
cmTarget cmGlobalGenerator::CreateGlobalTarget(
  const char* name, const char* message,
  const cmCustomCommandLines* commandLines,
  std::vector<std::string> depends,
  const char* workingDirectory)
{
  // Package
  cmTarget target;
  target.GetProperties().SetCMakeInstance(this->CMakeInstance);
  target.SetType(cmTarget::GLOBAL_TARGET, name);
  target.SetProperty("EXCLUDE_FROM_ALL","TRUE");

  std::vector<std::string> no_outputs;
  std::vector<std::string> no_depends;
  // Store the custom command in the target.
  cmCustomCommand cc(0, no_outputs, no_depends, *commandLines, 0,
                     workingDirectory);
  target.GetPostBuildCommands().push_back(cc);
  target.SetProperty("EchoString", message);
  std::vector<std::string>::iterator dit;
  for ( dit = depends.begin(); dit != depends.end(); ++ dit )
    {
    target.AddUtility(dit->c_str());
    }

  // Organize in the "predefined targets" folder:
  //
  if (this->UseFolderProperty())
    {
    target.SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
    }

  return target;
}

//----------------------------------------------------------------------------
std::string
cmGlobalGenerator::GenerateRuleFile(std::string const& output) const
{
  std::string ruleFile = output;
  ruleFile += ".rule";
  const char* dir = this->GetCMakeCFGIntDir();
  if(dir && dir[0] == '$')
    {
    cmSystemTools::ReplaceString(ruleFile, dir,
                                 cmake::GetCMakeFilesDirectory());
    }
  return ruleFile;
}

//----------------------------------------------------------------------------
std::string cmGlobalGenerator::GetSharedLibFlagsForLanguage(
                                                        std::string const& l)
{
  if(this->LanguageToOriginalSharedLibFlags.count(l) > 0)
    {
    return this->LanguageToOriginalSharedLibFlags[l];
    }
  return "";
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
cmGlobalGenerator::TargetDependSet const&
cmGlobalGenerator::GetTargetDirectDepends(cmTarget & target)
{
  return this->TargetDependencies[&target];
}

void cmGlobalGenerator::AddTarget(cmTarget* t)
{
  if(t->IsImported())
    {
    this->ImportedTargets[t->GetName()] = t;
    }
  else
    {
    this->TotalTargets[t->GetName()] = t;
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

const char* cmGlobalGenerator::GetExtraGeneratorName() const
{
  return this->ExtraGenerator==0 ? 0 : this->ExtraGenerator->GetName();
}

void cmGlobalGenerator::FileReplacedDuringGenerate(const std::string& filename)
{
  this->FilesReplacedDuringGenerate.push_back(filename);
}

void
cmGlobalGenerator
::GetFilesReplacedDuringGenerate(std::vector<std::string>& filenames)
{
  filenames.clear();
  std::copy(
    this->FilesReplacedDuringGenerate.begin(),
    this->FilesReplacedDuringGenerate.end(),
    std::back_inserter(filenames));
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::GetTargetSets(TargetDependSet& projectTargets,
                                      TargetDependSet& originalTargets,
                                      cmLocalGenerator* root,
                                      GeneratorVector const& generators)
{
  // loop over all local generators
  for(std::vector<cmLocalGenerator*>::const_iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    // check to make sure generator is not excluded
    if(this->IsExcluded(root, *i))
      {
      continue;
      }
    cmMakefile* mf = (*i)->GetMakefile();
    // Get the targets in the makefile
    cmTargets &tgts = mf->GetTargets();
    // loop over all the targets
    for (cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
      {
      cmTarget* target = &l->second;
      if(this->IsRootOnlyTarget(target) &&
         target->GetMakefile() != root->GetMakefile())
        {
        continue;
        }
      // put the target in the set of original targets
      originalTargets.insert(target);
      // Get the set of targets that depend on target
      this->AddTargetDepends(target, projectTargets);
      }
    }
}

//----------------------------------------------------------------------------
bool cmGlobalGenerator::IsRootOnlyTarget(cmTarget* target)
{
  return (target->GetType() == cmTarget::GLOBAL_TARGET ||
          strcmp(target->GetName(), this->GetAllTargetName()) == 0);
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::AddTargetDepends(cmTarget* target,
                                         TargetDependSet& projectTargets)
{
  // add the target itself
  if(projectTargets.insert(target).second)
    {
    // This is the first time we have encountered the target.
    // Recursively follow its dependencies.
    TargetDependSet const& ts = this->GetTargetDirectDepends(*target);
    for(TargetDependSet::const_iterator i = ts.begin(); i != ts.end(); ++i)
      {
      cmTarget* dtarget = *i;
      this->AddTargetDepends(dtarget, projectTargets);
      }
    }
}


//----------------------------------------------------------------------------
void cmGlobalGenerator::AddToManifest(const char* config,
                                      std::string const& f)
{
  // Add to the main manifest for this configuration.
  this->TargetManifest[config].insert(f);

  // Add to the content listing for the file's directory.
  std::string dir = cmSystemTools::GetFilenamePath(f);
  std::string file = cmSystemTools::GetFilenameName(f);
  this->DirectoryContentMap[dir].insert(file);
}

//----------------------------------------------------------------------------
std::set<cmStdString> const&
cmGlobalGenerator::GetDirectoryContent(std::string const& dir, bool needDisk)
{
  DirectoryContent& dc = this->DirectoryContentMap[dir];
  if(needDisk && !dc.LoadedFromDisk)
    {
    // Load the directory content from disk.
    cmsys::Directory d;
    if(d.Load(dir.c_str()))
      {
      unsigned long n = d.GetNumberOfFiles();
      for(unsigned long i = 0; i < n; ++i)
        {
        const char* f = d.GetFile(i);
        if(strcmp(f, ".") != 0 && strcmp(f, "..") != 0)
          {
          dc.insert(f);
          }
        }
      }
    dc.LoadedFromDisk = true;
    }
  return dc;
}

//----------------------------------------------------------------------------
void
cmGlobalGenerator::AddRuleHash(const std::vector<std::string>& outputs,
                               std::string const& content)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Ignore if there are no outputs.
  if(outputs.empty())
    {
    return;
    }

  // Compute a hash of the rule.
  RuleHash hash;
  {
  unsigned char const* data =
    reinterpret_cast<unsigned char const*>(content.c_str());
  int length = static_cast<int>(content.length());
  cmsysMD5* sum = cmsysMD5_New();
  cmsysMD5_Initialize(sum);
  cmsysMD5_Append(sum, data, length);
  cmsysMD5_FinalizeHex(sum, hash.Data);
  cmsysMD5_Delete(sum);
  }

  // Shorten the output name (in expected use case).
  cmLocalGenerator* lg = this->GetLocalGenerators()[0];
  std::string fname = lg->Convert(outputs[0].c_str(),
                                  cmLocalGenerator::HOME_OUTPUT);

  // Associate the hash with this output.
  this->RuleHashes[fname] = hash;
#else
  (void)outputs;
  (void)content;
#endif
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::CheckRuleHashes()
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  std::string home = this->GetCMakeInstance()->GetHomeOutputDirectory();
  std::string pfile = home;
  pfile += this->GetCMakeInstance()->GetCMakeFilesDirectory();
  pfile += "/CMakeRuleHashes.txt";
  this->CheckRuleHashes(pfile, home);
  this->WriteRuleHashes(pfile);
#endif
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::CheckRuleHashes(std::string const& pfile,
                                        std::string const& home)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  std::ifstream fin(pfile.c_str(), std::ios::in | std::ios::binary);
#else
  std::ifstream fin(pfile.c_str(), std::ios::in);
#endif
  if(!fin)
    {
    return;
    }
  std::string line;
  std::string fname;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    // Line format is a 32-byte hex string followed by a space
    // followed by a file name (with no escaping).

    // Skip blank and comment lines.
    if(line.size() < 34 || line[0] == '#')
      {
      continue;
      }

    // Get the filename.
    fname = line.substr(33, line.npos);

    // Look for a hash for this file's rule.
    std::map<cmStdString, RuleHash>::const_iterator rhi =
      this->RuleHashes.find(fname);
    if(rhi != this->RuleHashes.end())
      {
      // Compare the rule hash in the file to that we were given.
      if(strncmp(line.c_str(), rhi->second.Data, 32) != 0)
        {
        // The rule has changed.  Delete the output so it will be
        // built again.
        fname = cmSystemTools::CollapseFullPath(fname.c_str(), home.c_str());
        cmSystemTools::RemoveFile(fname.c_str());
        }
      }
    else
      {
      // We have no hash for a rule previously listed.  This may be a
      // case where a user has turned off a build option and might
      // want to turn it back on later, so do not delete the file.
      // Instead, we keep the rule hash as long as the file exists so
      // that if the feature is turned back on and the rule has
      // changed the file is still rebuilt.
      std::string fpath =
        cmSystemTools::CollapseFullPath(fname.c_str(), home.c_str());
      if(cmSystemTools::FileExists(fpath.c_str()))
        {
        RuleHash hash;
        strncpy(hash.Data, line.c_str(), 32);
        this->RuleHashes[fname] = hash;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::WriteRuleHashes(std::string const& pfile)
{
  // Now generate a new persistence file with the current hashes.
  if(this->RuleHashes.empty())
    {
    cmSystemTools::RemoveFile(pfile.c_str());
    }
  else
    {
    cmGeneratedFileStream fout(pfile.c_str());
    fout << "# Hashes of file build rules.\n";
    for(std::map<cmStdString, RuleHash>::const_iterator
          rhi = this->RuleHashes.begin(); rhi != this->RuleHashes.end(); ++rhi)
      {
      fout.write(rhi->second.Data, 32);
      fout << " " << rhi->first << "\n";
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::WriteSummary()
{
  cmMakefile* mf = this->LocalGenerators[0]->GetMakefile();

  // Record all target directories in a central location.
  std::string fname = mf->GetHomeOutputDirectory();
  fname += cmake::GetCMakeFilesDirectory();
  fname += "/TargetDirectories.txt";
  cmGeneratedFileStream fout(fname.c_str());

  // Generate summary information files for each target.
  std::string dir;
  for(std::map<cmStdString,cmTarget *>::const_iterator ti =
        this->TotalTargets.begin(); ti != this->TotalTargets.end(); ++ti)
    {
    if ((ti->second)->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }
    this->WriteSummary(ti->second);
    fout << ti->second->GetSupportDirectory() << "\n";
    }
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::WriteSummary(cmTarget* target)
{
  // Place the labels file in a per-target support directory.
  std::string dir = target->GetSupportDirectory();
  std::string file = dir;
  file += "/Labels.txt";

  // Check whether labels are enabled for this target.
  if(const char* value = target->GetProperty("LABELS"))
    {
    cmSystemTools::MakeDirectory(dir.c_str());
    cmGeneratedFileStream fout(file.c_str());

    // List the target-wide labels.  All sources in the target get
    // these labels.
    std::vector<std::string> labels;
    cmSystemTools::ExpandListArgument(value, labels);
    if(!labels.empty())
      {
      fout << "# Target labels\n";
      for(std::vector<std::string>::const_iterator li = labels.begin();
          li != labels.end(); ++li)
        {
        fout << " " << *li << "\n";
        }
      }

    // List the source files with any per-source labels.
    fout << "# Source files and their labels\n";
    std::vector<cmSourceFile*> const& sources = target->GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
        si != sources.end(); ++si)
      {
      cmSourceFile* sf = *si;
      fout << sf->GetFullPath() << "\n";
      if(const char* svalue = sf->GetProperty("LABELS"))
        {
        labels.clear();
        cmSystemTools::ExpandListArgument(svalue, labels);
        for(std::vector<std::string>::const_iterator li = labels.begin();
            li != labels.end(); ++li)
          {
          fout << " " << *li << "\n";
          }
        }
      }
    }
  else
    {
    cmSystemTools::RemoveFile(file.c_str());
    }
}

//----------------------------------------------------------------------------
// static
std::string cmGlobalGenerator::EscapeJSON(const std::string& s) {
  std::string result;
  for (std::string::size_type i = 0; i < s.size(); ++i) {
    if (s[i] == '"' || s[i] == '\\') {
      result += '\\';
    }
    result += s[i];
  }
  return result;
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::AddEvaluationFile(const std::string &inputFile,
                    cmsys::auto_ptr<cmCompiledGeneratorExpression> outputExpr,
                    cmMakefile *makefile,
                    cmsys::auto_ptr<cmCompiledGeneratorExpression> condition,
                    bool inputIsContent)
{
  this->EvaluationFiles.push_back(
              new cmGeneratorExpressionEvaluationFile(inputFile, outputExpr,
                                                      makefile, condition,
                                                      inputIsContent));
}

//----------------------------------------------------------------------------
void cmGlobalGenerator::ProcessEvaluationFiles()
{
  std::set<std::string> generatedFiles;
  for(std::vector<cmGeneratorExpressionEvaluationFile*>::const_iterator
      li = this->EvaluationFiles.begin();
      li != this->EvaluationFiles.end();
      ++li)
    {
    (*li)->Generate();
    if (cmSystemTools::GetFatalErrorOccured())
      {
      return;
      }
    std::vector<std::string> files = (*li)->GetFiles();
    for(std::vector<std::string>::const_iterator fi = files.begin();
        fi != files.end(); ++fi)
      {
      if (!generatedFiles.insert(*fi).second)
        {
        cmSystemTools::Error("File to be generated by multiple different "
          "commands: ", fi->c_str());
        return;
        }
      }
    }
}
