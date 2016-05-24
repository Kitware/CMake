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
#if defined(_MSC_VER) && _MSC_VER >= 1800
#define KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#endif
#endif

#include "cmGlobalGenerator.h"

#include "cmAlgorithms.h"
#include "cmCPackPropertiesGenerator.h"
#include "cmComputeTargetDepends.h"
#include "cmExportBuildFileGenerator.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmInstallGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmQtAutoGeneratorInitializer.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmTargetExport.h"
#include "cmVersion.h"
#include "cmake.h"

#include <cmsys/Directory.hxx>
#include <cmsys/FStream.hxx>

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_value.h"
#include "cm_jsoncpp_writer.h"
#include <cmsys/MD5.h>
#endif

#include <stdlib.h> // required for atof

#include <assert.h>

bool cmTarget::StrictTargetComparison::operator()(cmTarget const* t1,
                                                  cmTarget const* t2) const
{
  int nameResult = strcmp(t1->GetName().c_str(), t2->GetName().c_str());
  if (nameResult == 0) {
    return strcmp(t1->GetMakefile()->GetCurrentBinaryDirectory(),
                  t2->GetMakefile()->GetCurrentBinaryDirectory()) < 0;
  }
  return nameResult < 0;
}

cmGlobalGenerator::cmGlobalGenerator(cmake* cm)
  : CMakeInstance(cm)
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
  this->CurrentMakefile = 0;
  this->TryCompileOuterMakefile = 0;

  this->ConfigureDoneCMP0026AndCMP0024 = false;

  cm->GetState()->SetMinGWMake(false);
  cm->GetState()->SetMSYSShell(false);
  cm->GetState()->SetNMake(false);
  cm->GetState()->SetWatcomWMake(false);
  cm->GetState()->SetWindowsShell(false);
  cm->GetState()->SetWindowsVSIDE(false);
}

cmGlobalGenerator::~cmGlobalGenerator()
{
  this->ClearGeneratorMembers();
  delete this->ExtraGenerator;
}

bool cmGlobalGenerator::SetGeneratorPlatform(std::string const& p,
                                             cmMakefile* mf)
{
  if (p.empty()) {
    return true;
  }

  std::ostringstream e;
  /* clang-format off */
  e <<
    "Generator\n"
    "  " << this->GetName() << "\n"
    "does not support platform specification, but platform\n"
    "  " << p << "\n"
    "was specified.";
  /* clang-format on */
  mf->IssueMessage(cmake::FATAL_ERROR, e.str());
  return false;
}

bool cmGlobalGenerator::SetGeneratorToolset(std::string const& ts,
                                            cmMakefile* mf)
{
  if (ts.empty()) {
    return true;
  }
  std::ostringstream e;
  /* clang-format off */
  e <<
    "Generator\n"
    "  " << this->GetName() << "\n"
    "does not support toolset specification, but toolset\n"
    "  " << ts << "\n"
    "was specified.";
  /* clang-format on */
  mf->IssueMessage(cmake::FATAL_ERROR, e.str());
  return false;
}

std::string cmGlobalGenerator::SelectMakeProgram(
  const std::string& inMakeProgram, const std::string& makeDefault) const
{
  std::string makeProgram = inMakeProgram;
  if (cmSystemTools::IsOff(makeProgram.c_str())) {
    const char* makeProgramCSTR =
      this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
    if (cmSystemTools::IsOff(makeProgramCSTR)) {
      makeProgram = makeDefault;
    } else {
      makeProgram = makeProgramCSTR;
    }
    if (cmSystemTools::IsOff(makeProgram.c_str()) && !makeProgram.empty()) {
      makeProgram = "CMAKE_MAKE_PROGRAM-NOTFOUND";
    }
  }
  return makeProgram;
}

void cmGlobalGenerator::ResolveLanguageCompiler(const std::string& lang,
                                                cmMakefile* mf,
                                                bool optional) const
{
  std::string langComp = "CMAKE_";
  langComp += lang;
  langComp += "_COMPILER";

  if (!mf->GetDefinition(langComp)) {
    if (!optional) {
      cmSystemTools::Error(langComp.c_str(), " not set, after EnableLanguage");
    }
    return;
  }
  const char* name = mf->GetRequiredDefinition(langComp);
  std::string path;
  if (!cmSystemTools::FileIsFullPath(name)) {
    path = cmSystemTools::FindProgram(name);
  } else {
    path = name;
  }
  if ((path.empty() || !cmSystemTools::FileExists(path.c_str())) &&
      (optional == false)) {
    return;
  }
  const char* cname =
    this->GetCMakeInstance()->GetState()->GetInitializedCacheValue(langComp);
  std::string changeVars;
  if (cname && !optional) {
    std::string cnameString;
    if (!cmSystemTools::FileIsFullPath(cname)) {
      cnameString = cmSystemTools::FindProgram(cname);
    } else {
      cnameString = cname;
    }
    std::string pathString = path;
    // get rid of potentially multiple slashes:
    cmSystemTools::ConvertToUnixSlashes(cnameString);
    cmSystemTools::ConvertToUnixSlashes(pathString);
    if (cnameString != pathString) {
      const char* cvars =
        this->GetCMakeInstance()->GetState()->GetGlobalProperty(
          "__CMAKE_DELETE_CACHE_CHANGE_VARS_");
      if (cvars) {
        changeVars += cvars;
        changeVars += ";";
      }
      changeVars += langComp;
      changeVars += ";";
      changeVars += cname;
      this->GetCMakeInstance()->GetState()->SetGlobalProperty(
        "__CMAKE_DELETE_CACHE_CHANGE_VARS_", changeVars.c_str());
    }
  }
}

void cmGlobalGenerator::AddBuildExportSet(cmExportBuildFileGenerator* gen)
{
  this->BuildExportSets[gen->GetMainExportFileName()] = gen;
}

void cmGlobalGenerator::AddBuildExportExportSet(
  cmExportBuildFileGenerator* gen)
{
  this->BuildExportSets[gen->GetMainExportFileName()] = gen;
  this->BuildExportExportSets[gen->GetMainExportFileName()] = gen;
}

bool cmGlobalGenerator::GenerateImportFile(const std::string& file)
{
  std::map<std::string, cmExportBuildFileGenerator*>::iterator it =
    this->BuildExportSets.find(file);
  if (it != this->BuildExportSets.end()) {
    bool result = it->second->GenerateImportFile();

    if (!this->ConfigureDoneCMP0026AndCMP0024) {
      for (std::vector<cmMakefile*>::const_iterator mit =
             this->Makefiles.begin();
           mit != this->Makefiles.end(); ++mit) {
        (*mit)->RemoveExportBuildFileGeneratorCMP0024(it->second);
      }
    }

    delete it->second;
    it->second = 0;
    this->BuildExportSets.erase(it);
    return result;
  }
  return false;
}

void cmGlobalGenerator::ForceLinkerLanguages()
{
}

bool cmGlobalGenerator::IsExportedTargetsFile(
  const std::string& filename) const
{
  const std::map<std::string, cmExportBuildFileGenerator*>::const_iterator it =
    this->BuildExportSets.find(filename);
  if (it == this->BuildExportSets.end()) {
    return false;
  }
  return this->BuildExportExportSets.find(filename) ==
    this->BuildExportExportSets.end();
}

// Find the make program for the generator, required for try compiles
void cmGlobalGenerator::FindMakeProgram(cmMakefile* mf)
{
  if (this->FindMakeProgramFile.empty()) {
    cmSystemTools::Error(
      "Generator implementation error, "
      "all generators must specify this->FindMakeProgramFile");
  }
  if (!mf->GetDefinition("CMAKE_MAKE_PROGRAM") ||
      cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::string setMakeProgram =
      mf->GetModulesFile(this->FindMakeProgramFile.c_str());
    if (!setMakeProgram.empty()) {
      mf->ReadListFile(setMakeProgram.c_str());
    }
  }
  if (!mf->GetDefinition("CMAKE_MAKE_PROGRAM") ||
      cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::ostringstream err;
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
  if (makeProgram.find(' ') != makeProgram.npos) {
    std::string dir;
    std::string file;
    cmSystemTools::SplitProgramPath(makeProgram, dir, file);
    std::string saveFile = file;
    cmSystemTools::GetShortPath(makeProgram, makeProgram);
    cmSystemTools::SplitProgramPath(makeProgram, dir, file);
    makeProgram = dir;
    makeProgram += "/";
    makeProgram += saveFile;
    mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", makeProgram.c_str(),
                           "make program", cmState::FILEPATH);
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

// CMakeSystemSpecificInitialize.cmake
//   - includes Platform/${CMAKE_SYSTEM_NAME}-Initialize.cmake

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

void cmGlobalGenerator::EnableLanguage(
  std::vector<std::string> const& languages, cmMakefile* mf, bool optional)
{
  if (languages.empty()) {
    cmSystemTools::Error("EnableLanguage must have a lang specified!");
    cmSystemTools::SetFatalErrorOccured();
    return;
  }

  std::set<std::string> cur_languages(languages.begin(), languages.end());
  for (std::set<std::string>::iterator li = cur_languages.begin();
       li != cur_languages.end(); ++li) {
    if (!this->LanguagesInProgress.insert(*li).second) {
      std::ostringstream e;
      e << "Language '" << *li << "' is currently being enabled.  "
                                  "Recursive call not allowed.";
      mf->IssueMessage(cmake::FATAL_ERROR, e.str());
      cmSystemTools::SetFatalErrorOccured();
      return;
    }
  }

  if (this->TryCompileOuterMakefile) {
    // In a try-compile we can only enable languages provided by caller.
    for (std::vector<std::string>::const_iterator li = languages.begin();
         li != languages.end(); ++li) {
      if (*li == "NONE") {
        this->SetLanguageEnabled("NONE", mf);
      } else {
        const char* lang = li->c_str();
        if (this->LanguagesReady.find(lang) == this->LanguagesReady.end()) {
          std::ostringstream e;
          e << "The test project needs language " << lang
            << " which is not enabled.";
          this->TryCompileOuterMakefile->IssueMessage(cmake::FATAL_ERROR,
                                                      e.str());
          cmSystemTools::SetFatalErrorOccured();
          return;
        }
      }
    }
  }

  bool fatalError = false;

  mf->AddDefinition("RUN_CONFIGURE", true);
  std::string rootBin = this->CMakeInstance->GetHomeOutputDirectory();
  rootBin += cmake::GetCMakeFilesDirectory();

  // If the configuration files path has been set,
  // then we are in a try compile and need to copy the enable language
  // files from the parent cmake bin dir, into the try compile bin dir
  if (!this->ConfiguredFilesPath.empty()) {
    rootBin = this->ConfiguredFilesPath;
  }
  rootBin += "/";
  rootBin += cmVersion::GetCMakeVersion();

  // set the dir for parent files so they can be used by modules
  mf->AddDefinition("CMAKE_PLATFORM_INFO_DIR", rootBin.c_str());

  // find and make sure CMAKE_MAKE_PROGRAM is defined
  this->FindMakeProgram(mf);

  // try and load the CMakeSystem.cmake if it is there
  std::string fpath = rootBin;
  bool const readCMakeSystem = !mf->GetDefinition("CMAKE_SYSTEM_LOADED");
  if (readCMakeSystem) {
    fpath += "/CMakeSystem.cmake";
    if (cmSystemTools::FileExists(fpath.c_str())) {
      mf->ReadListFile(fpath.c_str());
    }
  }
  //  Load the CMakeDetermineSystem.cmake file and find out
  // what platform we are running on
  if (!mf->GetDefinition("CMAKE_SYSTEM")) {
#if defined(_WIN32) && !defined(__CYGWIN__)
    /* Windows version number data.  */
    OSVERSIONINFOEXW osviex;
    ZeroMemory(&osviex, sizeof(osviex));
    osviex.dwOSVersionInfoSize = sizeof(osviex);

#ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    GetVersionExW((OSVERSIONINFOW*)&osviex);
#ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#pragma warning(pop)
#endif
    std::ostringstream windowsVersionString;
    windowsVersionString << osviex.dwMajorVersion << "."
                         << osviex.dwMinorVersion << "."
                         << osviex.dwBuildNumber;
    windowsVersionString.str();
    mf->AddDefinition("CMAKE_HOST_SYSTEM_VERSION",
                      windowsVersionString.str().c_str());
#endif
    // Read the DetermineSystem file
    std::string systemFile = mf->GetModulesFile("CMakeDetermineSystem.cmake");
    mf->ReadListFile(systemFile.c_str());
    // load the CMakeSystem.cmake from the binary directory
    // this file is configured by the CMakeDetermineSystem.cmake file
    fpath = rootBin;
    fpath += "/CMakeSystem.cmake";
    mf->ReadListFile(fpath.c_str());
  }

  if (readCMakeSystem) {
    // Tell the generator about the target system.
    std::string system = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (!this->SetSystemName(system, mf)) {
      cmSystemTools::SetFatalErrorOccured();
      return;
    }

    // Tell the generator about the platform, if any.
    std::string platform = mf->GetSafeDefinition("CMAKE_GENERATOR_PLATFORM");
    if (!this->SetGeneratorPlatform(platform, mf)) {
      cmSystemTools::SetFatalErrorOccured();
      return;
    }

    // Tell the generator about the toolset, if any.
    std::string toolset = mf->GetSafeDefinition("CMAKE_GENERATOR_TOOLSET");
    if (!this->SetGeneratorToolset(toolset, mf)) {
      cmSystemTools::SetFatalErrorOccured();
      return;
    }
  }

  // **** Load the system specific initialization if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INITIALIZE_LOADED")) {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInitialize.cmake");
    if (!mf->ReadListFile(fpath.c_str())) {
      cmSystemTools::Error("Could not find cmake module file: "
                           "CMakeSystemSpecificInitialize.cmake");
    }
  }

  std::map<std::string, bool> needTestLanguage;
  std::map<std::string, bool> needSetLanguageEnabledMaps;
  // foreach language
  // load the CMakeDetermine(LANG)Compiler.cmake file to find
  // the compiler

  for (std::vector<std::string>::const_iterator l = languages.begin();
       l != languages.end(); ++l) {
    const char* lang = l->c_str();
    needSetLanguageEnabledMaps[lang] = false;
    if (*l == "NONE") {
      this->SetLanguageEnabled("NONE", mf);
      continue;
    }
    std::string loadedLang = "CMAKE_";
    loadedLang += lang;
    loadedLang += "_COMPILER_LOADED";
    if (!mf->GetDefinition(loadedLang)) {
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";

      // If the existing build tree was already configured with this
      // version of CMake then try to load the configured file first
      // to avoid duplicate compiler tests.
      if (cmSystemTools::FileExists(fpath.c_str())) {
        if (!mf->ReadListFile(fpath.c_str())) {
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

    if (!this->GetLanguageEnabled(lang)) {
      if (this->CMakeInstance->GetIsInTryCompile()) {
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
      if (!mf->ReadListFile(determineFile.c_str())) {
        cmSystemTools::Error("Could not find cmake module file: ",
                             determineCompiler.c_str());
      }
      if (cmSystemTools::GetFatalErrorOccured()) {
        return;
      }
      needTestLanguage[lang] = true;
      // Some generators like visual studio should not use the env variables
      // So the global generator can specify that in this variable
      if (!mf->GetDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV")) {
        // put ${CMake_(LANG)_COMPILER_ENV_VAR}=${CMAKE_(LANG)_COMPILER
        // into the environment, in case user scripts want to run
        // configure, or sub cmakes
        std::string compilerName = "CMAKE_";
        compilerName += lang;
        compilerName += "_COMPILER";
        std::string compilerEnv = "CMAKE_";
        compilerEnv += lang;
        compilerEnv += "_COMPILER_ENV_VAR";
        std::string envVar = mf->GetRequiredDefinition(compilerEnv);
        std::string envVarValue = mf->GetRequiredDefinition(compilerName);
        std::string env = envVar;
        env += "=";
        env += envVarValue;
        cmSystemTools::PutEnv(env);
      }

      // if determineLanguage was called then load the file it
      // configures CMake(LANG)Compiler.cmake
      fpath = rootBin;
      fpath += "/CMake";
      fpath += lang;
      fpath += "Compiler.cmake";
      if (!mf->ReadListFile(fpath.c_str())) {
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
  }   // end loop over languages

  // **** Load the system specific information if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED")) {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if (!mf->ReadListFile(fpath.c_str())) {
      cmSystemTools::Error("Could not find cmake module file: "
                           "CMakeSystemSpecificInformation.cmake");
    }
  }
  // loop over languages again loading CMake(LANG)Information.cmake
  //
  for (std::vector<std::string>::const_iterator l = languages.begin();
       l != languages.end(); ++l) {
    const char* lang = l->c_str();
    if (*l == "NONE") {
      this->SetLanguageEnabled("NONE", mf);
      continue;
    }

    // Check that the compiler was found.
    std::string compilerName = "CMAKE_";
    compilerName += lang;
    compilerName += "_COMPILER";
    std::string compilerEnv = "CMAKE_";
    compilerEnv += lang;
    compilerEnv += "_COMPILER_ENV_VAR";
    std::ostringstream noCompiler;
    const char* compilerFile = mf->GetDefinition(compilerName);
    if (!compilerFile || !*compilerFile ||
        cmSystemTools::IsNOTFOUND(compilerFile)) {
      /* clang-format off */
      noCompiler <<
        "No " << compilerName << " could be found.\n"
        ;
      /* clang-format on */
    } else if (strcmp(lang, "RC") != 0 && strcmp(lang, "ASM_MASM") != 0) {
      if (!cmSystemTools::FileIsFullPath(compilerFile)) {
        /* clang-format off */
        noCompiler <<
          "The " << compilerName << ":\n"
          "  " << compilerFile << "\n"
          "is not a full path and was not found in the PATH.\n"
          ;
        /* clang-format on */
      } else if (!cmSystemTools::FileExists(compilerFile)) {
        /* clang-format off */
        noCompiler <<
          "The " << compilerName << ":\n"
          "  " << compilerFile << "\n"
          "is not a full path to an existing compiler tool.\n"
          ;
        /* clang-format on */
      }
    }
    if (!noCompiler.str().empty()) {
      // Skip testing this language since the compiler is not found.
      needTestLanguage[lang] = false;
      if (!optional) {
        // The compiler was not found and it is not optional.  Remove
        // CMake(LANG)Compiler.cmake so we try again next time CMake runs.
        std::string compilerLangFile = rootBin;
        compilerLangFile += "/CMake";
        compilerLangFile += lang;
        compilerLangFile += "Compiler.cmake";
        cmSystemTools::RemoveFile(compilerLangFile);
        if (!this->CMakeInstance->GetIsInTryCompile()) {
          this->PrintCompilerAdvice(noCompiler, lang,
                                    mf->GetDefinition(compilerEnv));
          mf->IssueMessage(cmake::FATAL_ERROR, noCompiler.str());
          fatalError = true;
        }
      }
    }

    std::string langLoadedVar = "CMAKE_";
    langLoadedVar += lang;
    langLoadedVar += "_INFORMATION_LOADED";
    if (!mf->GetDefinition(langLoadedVar)) {
      fpath = "CMake";
      fpath += lang;
      fpath += "Information.cmake";
      std::string informationFile = mf->GetModulesFile(fpath.c_str());
      if (informationFile.empty()) {
        cmSystemTools::Error("Could not find cmake module file: ",
                             fpath.c_str());
      } else if (!mf->ReadListFile(informationFile.c_str())) {
        cmSystemTools::Error("Could not process cmake module file: ",
                             informationFile.c_str());
      }
    }
    if (needSetLanguageEnabledMaps[lang]) {
      this->SetLanguageEnabledMaps(lang, mf);
    }
    this->LanguagesReady.insert(lang);

    // Test the compiler for the language just setup
    // (but only if a compiler has been actually found)
    // At this point we should have enough info for a try compile
    // which is used in the backward stuff
    // If the language is untested then test it now with a try compile.
    if (needTestLanguage[lang]) {
      if (!this->CMakeInstance->GetIsInTryCompile()) {
        std::string testLang = "CMakeTest";
        testLang += lang;
        testLang += "Compiler.cmake";
        std::string ifpath = mf->GetModulesFile(testLang.c_str());
        if (!mf->ReadListFile(ifpath.c_str())) {
          cmSystemTools::Error("Could not find cmake module file: ",
                               testLang.c_str());
        }
        std::string compilerWorks = "CMAKE_";
        compilerWorks += lang;
        compilerWorks += "_COMPILER_WORKS";
        // if the compiler did not work, then remove the
        // CMake(LANG)Compiler.cmake file so that it will get tested the
        // next time cmake is run
        if (!mf->IsOn(compilerWorks)) {
          std::string compilerLangFile = rootBin;
          compilerLangFile += "/CMake";
          compilerLangFile += lang;
          compilerLangFile += "Compiler.cmake";
          cmSystemTools::RemoveFile(compilerLangFile);
        }
      } // end if in try compile
    }   // end need test language
    // Store the shared library flags so that we can satisfy CMP0018
    std::string sharedLibFlagsVar = "CMAKE_SHARED_LIBRARY_";
    sharedLibFlagsVar += lang;
    sharedLibFlagsVar += "_FLAGS";
    const char* sharedLibFlags = mf->GetSafeDefinition(sharedLibFlagsVar);
    if (sharedLibFlags) {
      this->LanguageToOriginalSharedLibFlags[lang] = sharedLibFlags;
    }

    // Translate compiler ids for compatibility.
    this->CheckCompilerIdCompatibility(mf, lang);
  } // end for each language

  // Now load files that can override any settings on the platform or for
  // the project First load the project compatibility file if it is in
  // cmake
  std::string projectCompatibility = cmSystemTools::GetCMakeRoot();
  projectCompatibility += "/Modules/";
  projectCompatibility += mf->GetSafeDefinition("PROJECT_NAME");
  projectCompatibility += "Compatibility.cmake";
  if (cmSystemTools::FileExists(projectCompatibility.c_str())) {
    mf->ReadListFile(projectCompatibility.c_str());
  }
  // Inform any extra generator of the new language.
  if (this->ExtraGenerator) {
    this->ExtraGenerator->EnableLanguage(languages, mf, false);
  }

  if (fatalError) {
    cmSystemTools::SetFatalErrorOccured();
  }

  for (std::set<std::string>::iterator li = cur_languages.begin();
       li != cur_languages.end(); ++li) {
    this->LanguagesInProgress.erase(*li);
  }
}

void cmGlobalGenerator::PrintCompilerAdvice(std::ostream& os,
                                            std::string const& lang,
                                            const char* envVar) const
{
  // Subclasses override this method if they do not support this advice.
  os << "Tell CMake where to find the compiler by setting ";
  if (envVar) {
    os << "either the environment variable \"" << envVar << "\" or ";
  }
  os << "the CMake cache entry CMAKE_" << lang
     << "_COMPILER "
        "to the full path to the compiler, or to the compiler name "
        "if it is in the PATH.";
}

void cmGlobalGenerator::CheckCompilerIdCompatibility(
  cmMakefile* mf, std::string const& lang) const
{
  std::string compilerIdVar = "CMAKE_" + lang + "_COMPILER_ID";
  const char* compilerId = mf->GetDefinition(compilerIdVar);
  if (!compilerId) {
    return;
  }

  if (strcmp(compilerId, "AppleClang") == 0) {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0025)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0025")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0025) << "\n"
            "Converting " << lang <<
            " compiler id \"AppleClang\" to \"Clang\" for compatibility."
            ;
          /* clang-format on */
          mf->IssueMessage(cmake::AUTHOR_WARNING, w.str());
        }
      case cmPolicies::OLD:
        // OLD behavior is to convert AppleClang to Clang.
        mf->AddDefinition(compilerIdVar, "Clang");
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        mf->IssueMessage(
          cmake::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0025));
      case cmPolicies::NEW:
        // NEW behavior is to keep AppleClang.
        break;
    }
  }

  if (strcmp(compilerId, "QCC") == 0) {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0047)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0047")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0047) << "\n"
            "Converting " << lang <<
            " compiler id \"QCC\" to \"GNU\" for compatibility."
            ;
          /* clang-format on */
          mf->IssueMessage(cmake::AUTHOR_WARNING, w.str());
        }
      case cmPolicies::OLD:
        // OLD behavior is to convert QCC to GNU.
        mf->AddDefinition(compilerIdVar, "GNU");
        if (lang == "C") {
          mf->AddDefinition("CMAKE_COMPILER_IS_GNUCC", "1");
        } else if (lang == "CXX") {
          mf->AddDefinition("CMAKE_COMPILER_IS_GNUCXX", "1");
        }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        mf->IssueMessage(
          cmake::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0047));
      case cmPolicies::NEW:
        // NEW behavior is to keep QCC.
        break;
    }
  }
}

std::string cmGlobalGenerator::GetLanguageOutputExtension(
  cmSourceFile const& source) const
{
  const std::string& lang = source.GetLanguage();
  if (!lang.empty()) {
    std::map<std::string, std::string>::const_iterator it =
      this->LanguageToOutputExtension.find(lang);

    if (it != this->LanguageToOutputExtension.end()) {
      return it->second;
    }
  } else {
    // if no language is found then check to see if it is already an
    // ouput extension for some language.  In that case it should be ignored
    // and in this map, so it will not be compiled but will just be used.
    std::string const& ext = source.GetExtension();
    if (!ext.empty()) {
      if (this->OutputExtensions.count(ext)) {
        return ext;
      }
    }
  }
  return "";
}

std::string cmGlobalGenerator::GetLanguageFromExtension(const char* ext) const
{
  // if there is an extension and it starts with . then move past the
  // . because the extensions are not stored with a .  in the map
  if (ext && *ext == '.') {
    ++ext;
  }
  std::map<std::string, std::string>::const_iterator it =
    this->ExtensionToLanguage.find(ext);
  if (it != this->ExtensionToLanguage.end()) {
    return it->second;
  }
  return "";
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
void cmGlobalGenerator::SetLanguageEnabled(const std::string& l,
                                           cmMakefile* mf)
{
  this->SetLanguageEnabledFlag(l, mf);
  this->SetLanguageEnabledMaps(l, mf);
}

void cmGlobalGenerator::SetLanguageEnabledFlag(const std::string& l,
                                               cmMakefile* mf)
{
  this->CMakeInstance->GetState()->SetLanguageEnabled(l);

  // Fill the language-to-extension map with the current variable
  // settings to make sure it is available for the try_compile()
  // command source file signature.  In SetLanguageEnabledMaps this
  // will be done again to account for any compiler- or
  // platform-specific entries.
  this->FillExtensionToLanguageMap(l, mf);
}

void cmGlobalGenerator::SetLanguageEnabledMaps(const std::string& l,
                                               cmMakefile* mf)
{
  // use LanguageToLinkerPreference to detect whether this functions has
  // run before
  if (this->LanguageToLinkerPreference.find(l) !=
      this->LanguageToLinkerPreference.end()) {
    return;
  }

  std::string linkerPrefVar =
    std::string("CMAKE_") + std::string(l) + std::string("_LINKER_PREFERENCE");
  const char* linkerPref = mf->GetDefinition(linkerPrefVar);
  int preference = 0;
  if (linkerPref) {
    if (sscanf(linkerPref, "%d", &preference) != 1) {
      // backward compatibility: before 2.6 LINKER_PREFERENCE
      // was either "None" or "Preferred", and only the first character was
      // tested. So if there is a custom language out there and it is
      // "Preferred", set its preference high
      if (linkerPref[0] == 'P') {
        preference = 100;
      } else {
        preference = 0;
      }
    }
  }

  if (preference < 0) {
    std::string msg = linkerPrefVar;
    msg += " is negative, adjusting it to 0";
    cmSystemTools::Message(msg.c_str(), "Warning");
    preference = 0;
  }

  this->LanguageToLinkerPreference[l] = preference;

  std::string outputExtensionVar =
    std::string("CMAKE_") + std::string(l) + std::string("_OUTPUT_EXTENSION");
  const char* outputExtension = mf->GetDefinition(outputExtensionVar);
  if (outputExtension) {
    this->LanguageToOutputExtension[l] = outputExtension;
    this->OutputExtensions[outputExtension] = outputExtension;
    if (outputExtension[0] == '.') {
      this->OutputExtensions[outputExtension + 1] = outputExtension + 1;
    }
  }

  // The map was originally filled by SetLanguageEnabledFlag, but
  // since then the compiler- and platform-specific files have been
  // loaded which might have added more entries.
  this->FillExtensionToLanguageMap(l, mf);

  std::string ignoreExtensionsVar =
    std::string("CMAKE_") + std::string(l) + std::string("_IGNORE_EXTENSIONS");
  std::string ignoreExts = mf->GetSafeDefinition(ignoreExtensionsVar);
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(ignoreExts, extensionList);
  for (std::vector<std::string>::iterator i = extensionList.begin();
       i != extensionList.end(); ++i) {
    this->IgnoreExtensions[*i] = true;
  }
}

void cmGlobalGenerator::FillExtensionToLanguageMap(const std::string& l,
                                                   cmMakefile* mf)
{
  std::string extensionsVar = std::string("CMAKE_") + std::string(l) +
    std::string("_SOURCE_FILE_EXTENSIONS");
  std::string exts = mf->GetSafeDefinition(extensionsVar);
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(exts, extensionList);
  for (std::vector<std::string>::iterator i = extensionList.begin();
       i != extensionList.end(); ++i) {
    this->ExtensionToLanguage[*i] = l;
  }
}

bool cmGlobalGenerator::IgnoreFile(const char* ext) const
{
  if (!this->GetLanguageFromExtension(ext).empty()) {
    return false;
  }
  return (this->IgnoreExtensions.count(ext) > 0);
}

bool cmGlobalGenerator::GetLanguageEnabled(const std::string& l) const
{
  return this->CMakeInstance->GetState()->GetLanguageEnabled(l);
}

void cmGlobalGenerator::ClearEnabledLanguages()
{
  return this->CMakeInstance->GetState()->ClearEnabledLanguages();
}

void cmGlobalGenerator::CreateLocalGenerators()
{
  cmDeleteAll(this->LocalGenerators);
  this->LocalGenerators.clear();
  this->LocalGenerators.reserve(this->Makefiles.size());
  for (std::vector<cmMakefile*>::const_iterator it = this->Makefiles.begin();
       it != this->Makefiles.end(); ++it) {
    this->LocalGenerators.push_back(this->CreateLocalGenerator(*it));
  }
}

void cmGlobalGenerator::Configure()
{
  this->FirstTimeProgress = 0.0f;
  this->ClearGeneratorMembers();

  cmState::Snapshot snapshot = this->CMakeInstance->GetCurrentSnapshot();

  snapshot.GetDirectory().SetCurrentSource(
    this->CMakeInstance->GetHomeDirectory());
  snapshot.GetDirectory().SetCurrentBinary(
    this->CMakeInstance->GetHomeOutputDirectory());

  cmMakefile* dirMf = new cmMakefile(this, snapshot);
  this->Makefiles.push_back(dirMf);

  this->BinaryDirectories.insert(
    this->CMakeInstance->GetHomeOutputDirectory());

  // now do it
  this->ConfigureDoneCMP0026AndCMP0024 = false;
  dirMf->Configure();
  dirMf->EnforceDirectoryLevelRules();

  this->ConfigureDoneCMP0026AndCMP0024 = true;

  // Put a copy of each global target in every directory.
  cmTargets globalTargets;
  this->CreateDefaultGlobalTargets(&globalTargets);

  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    cmMakefile* mf = this->Makefiles[i];
    cmTargets* targets = &(mf->GetTargets());
    cmTargets::iterator tit;
    for (tit = globalTargets.begin(); tit != globalTargets.end(); ++tit) {
      (*targets)[tit->first] = tit->second;
      (*targets)[tit->first].SetMakefile(mf);
    }
  }

  // update the cache entry for the number of local generators, this is used
  // for progress
  char num[100];
  sprintf(num, "%d", static_cast<int>(this->Makefiles.size()));
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_NUMBER_OF_MAKEFILES", num,
                                          "number of local generators",
                                          cmState::INTERNAL);

  // check for link libraries and include directories containing "NOTFOUND"
  // and for infinite loops
  this->CheckTargetProperties();

  if (this->CMakeInstance->GetWorkingMode() == cmake::NORMAL_MODE) {
    std::ostringstream msg;
    if (cmSystemTools::GetErrorOccuredFlag()) {
      msg << "Configuring incomplete, errors occurred!";
      const char* logs[] = { "CMakeOutput.log", "CMakeError.log", 0 };
      for (const char** log = logs; *log; ++log) {
        std::string f = this->CMakeInstance->GetHomeOutputDirectory();
        f += this->CMakeInstance->GetCMakeFilesDirectory();
        f += "/";
        f += *log;
        if (cmSystemTools::FileExists(f.c_str())) {
          msg << "\nSee also \"" << f << "\".";
        }
      }
    } else {
      msg << "Configuring done";
    }
    this->CMakeInstance->UpdateProgress(msg.str().c_str(), -1);
  }
}

void cmGlobalGenerator::CreateGenerationObjects(TargetTypes targetTypes)
{
  this->CreateLocalGenerators();
  this->CreateGeneratorTargets(targetTypes);
  this->ComputeBuildFileGenerators();
}

void cmGlobalGenerator::CreateImportedGenerationObjects(
  cmMakefile* mf, const std::vector<std::string>& targets,
  std::vector<const cmGeneratorTarget*>& exports)
{
  this->CreateGenerationObjects(ImportedOnly);
  std::vector<cmMakefile*>::iterator mfit =
    std::find(this->Makefiles.begin(), this->Makefiles.end(), mf);
  cmLocalGenerator* lg =
    this->LocalGenerators[std::distance(this->Makefiles.begin(), mfit)];
  for (std::vector<std::string>::const_iterator it = targets.begin();
       it != targets.end(); ++it) {
    cmGeneratorTarget* gt = lg->FindGeneratorTargetToUse(*it);
    if (gt) {
      exports.push_back(gt);
    }
  }
}

cmExportBuildFileGenerator* cmGlobalGenerator::GetExportedTargetsFile(
  const std::string& filename) const
{
  std::map<std::string, cmExportBuildFileGenerator*>::const_iterator it =
    this->BuildExportSets.find(filename);
  return it == this->BuildExportSets.end() ? 0 : it->second;
}

void cmGlobalGenerator::AddCMP0042WarnTarget(const std::string& target)
{
  this->CMP0042WarnTargets.insert(target);
}

bool cmGlobalGenerator::CheckALLOW_DUPLICATE_CUSTOM_TARGETS() const
{
  // If the property is not enabled then okay.
  if (!this->CMakeInstance->GetState()->GetGlobalPropertyAsBool(
        "ALLOW_DUPLICATE_CUSTOM_TARGETS")) {
    return true;
  }

  // This generator does not support duplicate custom targets.
  std::ostringstream e;
  e << "This project has enabled the ALLOW_DUPLICATE_CUSTOM_TARGETS "
    << "global property.  "
    << "The \"" << this->GetName() << "\" generator does not support "
    << "duplicate custom targets.  "
    << "Consider using a Makefiles generator or fix the project to not "
    << "use duplicate target names.";
  cmSystemTools::Error(e.str().c_str());
  return false;
}

void cmGlobalGenerator::ComputeBuildFileGenerators()
{
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    std::vector<cmExportBuildFileGenerator*> gens =
      this->Makefiles[i]->GetExportBuildFileGenerators();
    for (std::vector<cmExportBuildFileGenerator*>::const_iterator it =
           gens.begin();
         it != gens.end(); ++it) {
      (*it)->Compute(this->LocalGenerators[i]);
    }
  }
}

bool cmGlobalGenerator::Compute()
{
  // Some generators track files replaced during the Generate.
  // Start with an empty vector:
  this->FilesReplacedDuringGenerate.clear();

  // clear targets to issue warning CMP0042 for
  this->CMP0042WarnTargets.clear();

  // Check whether this generator is allowed to run.
  if (!this->CheckALLOW_DUPLICATE_CUSTOM_TARGETS()) {
    return false;
  }
  this->FinalizeTargetCompileInfo();

  this->CreateGenerationObjects();

  // at this point this->LocalGenerators has been filled,
  // so create the map from project name to vector of local generators
  this->FillProjectMap();

#ifdef CMAKE_BUILD_WITH_CMAKE
  // Iterate through all targets and set up automoc for those which have
  // the AUTOMOC, AUTOUIC or AUTORCC property set
  std::vector<cmGeneratorTarget const*> autogenTargets =
    this->CreateQtAutoGeneratorsTargets();
#endif

  unsigned int i;

  // Add generator specific helper commands
  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    this->LocalGenerators[i]->AddHelperCommands();
  }

#ifdef CMAKE_BUILD_WITH_CMAKE
  for (std::vector<cmGeneratorTarget const*>::iterator it =
         autogenTargets.begin();
       it != autogenTargets.end(); ++it) {
    cmQtAutoGeneratorInitializer::SetupAutoGenerateTarget(*it);
  }
#endif

  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    cmMakefile* mf = this->LocalGenerators[i]->GetMakefile();
    std::vector<cmInstallGenerator*>& gens = mf->GetInstallGenerators();
    for (std::vector<cmInstallGenerator*>::const_iterator git = gens.begin();
         git != gens.end(); ++git) {
      (*git)->Compute(this->LocalGenerators[i]);
    }
  }

  this->AddExtraIDETargets();

  // Trace the dependencies, after that no custom commands should be added
  // because their dependencies might not be handled correctly
  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    this->LocalGenerators[i]->TraceDependencies();
  }

  this->ForceLinkerLanguages();

  // Compute the manifest of main targets generated.
  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    this->LocalGenerators[i]->ComputeTargetManifest();
  }

  // Compute the inter-target dependencies.
  if (!this->ComputeTargetDepends()) {
    return false;
  }

  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    this->LocalGenerators[i]->ComputeHomeRelativeOutputPath();
  }

  return true;
}

void cmGlobalGenerator::Generate()
{
  // Create a map from local generator to the complete set of targets
  // it builds by default.
  this->InitializeProgressMarks();

  this->ProcessEvaluationFiles();

  // Generate project files
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    this->SetCurrentMakefile(this->LocalGenerators[i]->GetMakefile());
    this->LocalGenerators[i]->Generate();
    if (!this->LocalGenerators[i]->GetMakefile()->IsOn(
          "CMAKE_SKIP_INSTALL_RULES")) {
      this->LocalGenerators[i]->GenerateInstallRules();
    }
    this->LocalGenerators[i]->GenerateTestFiles();
    this->CMakeInstance->UpdateProgress(
      "Generating", (static_cast<float>(i) + 1.0f) /
        static_cast<float>(this->LocalGenerators.size()));
  }
  this->SetCurrentMakefile(0);

  if (!this->GenerateCPackPropertiesFile()) {
    this->GetCMakeInstance()->IssueMessage(
      cmake::FATAL_ERROR, "Could not write CPack properties file.");
  }

  for (std::map<std::string, cmExportBuildFileGenerator*>::iterator it =
         this->BuildExportSets.begin();
       it != this->BuildExportSets.end(); ++it) {
    if (!it->second->GenerateImportFile() &&
        !cmSystemTools::GetErrorOccuredFlag()) {
      this->GetCMakeInstance()->IssueMessage(cmake::FATAL_ERROR,
                                             "Could not write export file.");
      return;
    }
  }
  // Update rule hashes.
  this->CheckRuleHashes();

  this->WriteSummary();

  if (this->ExtraGenerator != 0) {
    this->ExtraGenerator->Generate();
  }

  if (!this->CMP0042WarnTargets.empty()) {
    std::ostringstream w;
    w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0042) << "\n";
    w << "MACOSX_RPATH is not specified for"
         " the following targets:\n";
    for (std::set<std::string>::iterator iter =
           this->CMP0042WarnTargets.begin();
         iter != this->CMP0042WarnTargets.end(); ++iter) {
      w << " " << *iter << "\n";
    }
    this->GetCMakeInstance()->IssueMessage(cmake::AUTHOR_WARNING, w.str());
  }

  this->CMakeInstance->UpdateProgress("Generating done", -1);
}

bool cmGlobalGenerator::ComputeTargetDepends()
{
  cmComputeTargetDepends ctd(this);
  if (!ctd.Compute()) {
    return false;
  }
  std::vector<cmGeneratorTarget const*> const& targets = ctd.GetTargets();
  for (std::vector<cmGeneratorTarget const*>::const_iterator ti =
         targets.begin();
       ti != targets.end(); ++ti) {
    ctd.GetTargetDirectDepends(*ti, this->TargetDependencies[*ti]);
  }
  return true;
}

std::vector<const cmGeneratorTarget*>
cmGlobalGenerator::CreateQtAutoGeneratorsTargets()
{
  std::vector<const cmGeneratorTarget*> autogenTargets;

#ifdef CMAKE_BUILD_WITH_CMAKE
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    std::vector<cmGeneratorTarget*> targets =
      this->LocalGenerators[i]->GetGeneratorTargets();
    std::vector<cmGeneratorTarget*> filteredTargets;
    filteredTargets.reserve(targets.size());
    for (std::vector<cmGeneratorTarget*>::iterator ti = targets.begin();
         ti != targets.end(); ++ti) {
      if ((*ti)->GetType() == cmState::GLOBAL_TARGET) {
        continue;
      }
      if ((*ti)->GetType() != cmState::EXECUTABLE &&
          (*ti)->GetType() != cmState::STATIC_LIBRARY &&
          (*ti)->GetType() != cmState::SHARED_LIBRARY &&
          (*ti)->GetType() != cmState::MODULE_LIBRARY &&
          (*ti)->GetType() != cmState::OBJECT_LIBRARY) {
        continue;
      }
      if ((!(*ti)->GetPropertyAsBool("AUTOMOC") &&
           !(*ti)->GetPropertyAsBool("AUTOUIC") &&
           !(*ti)->GetPropertyAsBool("AUTORCC")) ||
          (*ti)->IsImported()) {
        continue;
      }
      // don't do anything if there is no Qt4 or Qt5Core (which contains moc):
      cmMakefile* mf = (*ti)->Target->GetMakefile();
      std::string qtMajorVersion = mf->GetSafeDefinition("QT_VERSION_MAJOR");
      if (qtMajorVersion == "") {
        qtMajorVersion = mf->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
      }
      if (qtMajorVersion != "4" && qtMajorVersion != "5") {
        continue;
      }

      cmGeneratorTarget* gt = *ti;

      cmQtAutoGeneratorInitializer::InitializeAutogenSources(gt);
      filteredTargets.push_back(gt);
    }
    for (std::vector<cmGeneratorTarget*>::iterator ti =
           filteredTargets.begin();
         ti != filteredTargets.end(); ++ti) {
      cmQtAutoGeneratorInitializer::InitializeAutogenTarget(
        this->LocalGenerators[i], *ti);
      autogenTargets.push_back(*ti);
    }
  }
#endif
  return autogenTargets;
}

void cmGlobalGenerator::FinalizeTargetCompileInfo()
{
  std::vector<std::string> const langs =
    this->CMakeInstance->GetState()->GetEnabledLanguages();

  // Construct per-target generator information.
  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    cmMakefile* mf = this->Makefiles[i];

    const cmStringRange noconfig_compile_definitions =
      mf->GetCompileDefinitionsEntries();
    const cmBacktraceRange noconfig_compile_definitions_bts =
      mf->GetCompileDefinitionsBacktraces();

    cmTargets& targets = mf->GetTargets();
    for (cmTargets::iterator ti = targets.begin(); ti != targets.end(); ++ti) {
      cmTarget* t = &ti->second;
      if (t->GetType() == cmState::GLOBAL_TARGET) {
        continue;
      }

      t->AppendBuildInterfaceIncludes();

      if (t->GetType() == cmState::INTERFACE_LIBRARY) {
        continue;
      }

      cmBacktraceRange::const_iterator btIt =
        noconfig_compile_definitions_bts.begin();
      for (cmStringRange::const_iterator
             it = noconfig_compile_definitions.begin();
           it != noconfig_compile_definitions.end(); ++it, ++btIt) {
        t->InsertCompileDefinition(*it, *btIt);
      }

      cmPolicies::PolicyStatus polSt =
        mf->GetPolicyStatus(cmPolicies::CMP0043);
      if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD) {
        std::vector<std::string> configs;
        mf->GetConfigurations(configs);

        for (std::vector<std::string>::const_iterator ci = configs.begin();
             ci != configs.end(); ++ci) {
          std::string defPropName = "COMPILE_DEFINITIONS_";
          defPropName += cmSystemTools::UpperCase(*ci);
          t->AppendProperty(defPropName, mf->GetProperty(defPropName));
        }
      }
    }

    // The standard include directories for each language
    // should be treated as system include directories.
    std::set<std::string> standardIncludesSet;
    for (std::vector<std::string>::const_iterator li = langs.begin();
         li != langs.end(); ++li) {
      std::string const standardIncludesVar =
        "CMAKE_" + *li + "_STANDARD_INCLUDE_DIRECTORIES";
      std::string const standardIncludesStr =
        mf->GetSafeDefinition(standardIncludesVar);
      std::vector<std::string> standardIncludesVec;
      cmSystemTools::ExpandListArgument(standardIncludesStr,
                                        standardIncludesVec);
      standardIncludesSet.insert(standardIncludesVec.begin(),
                                 standardIncludesVec.end());
    }
    mf->AddSystemIncludeDirectories(standardIncludesSet);
  }
}

void cmGlobalGenerator::CreateGeneratorTargets(
  TargetTypes targetTypes, cmMakefile* mf, cmLocalGenerator* lg,
  std::map<cmTarget*, cmGeneratorTarget*> const& importedMap)
{
  if (targetTypes == AllTargets) {
    cmTargets& targets = mf->GetTargets();
    for (cmTargets::iterator ti = targets.begin(); ti != targets.end(); ++ti) {
      cmTarget* t = &ti->second;
      cmGeneratorTarget* gt = new cmGeneratorTarget(t, lg);
      lg->AddGeneratorTarget(gt);
    }
  }

  std::vector<cmTarget*> itgts = mf->GetImportedTargets();

  for (std::vector<cmTarget*>::const_iterator j = itgts.begin();
       j != itgts.end(); ++j) {
    lg->AddImportedGeneratorTarget(importedMap.find(*j)->second);
  }
}

void cmGlobalGenerator::CreateGeneratorTargets(TargetTypes targetTypes)
{
  std::map<cmTarget*, cmGeneratorTarget*> importedMap;
  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    cmMakefile* mf = this->Makefiles[i];
    for (std::vector<cmTarget*>::const_iterator j =
           mf->GetOwnedImportedTargets().begin();
         j != mf->GetOwnedImportedTargets().end(); ++j) {
      cmLocalGenerator* lg = this->LocalGenerators[i];
      cmGeneratorTarget* gt = new cmGeneratorTarget(*j, lg);
      lg->AddOwnedImportedGeneratorTarget(gt);
      importedMap[*j] = gt;
    }
  }

  // Construct per-target generator information.
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    this->CreateGeneratorTargets(targetTypes, this->Makefiles[i],
                                 this->LocalGenerators[i], importedMap);
  }
}

void cmGlobalGenerator::ClearGeneratorMembers()
{
  cmDeleteAll(this->BuildExportSets);
  this->BuildExportSets.clear();

  cmDeleteAll(this->Makefiles);
  this->Makefiles.clear();

  cmDeleteAll(this->LocalGenerators);
  this->LocalGenerators.clear();

  this->ExportSets.clear();
  this->TargetDependencies.clear();
  this->TargetSearchIndex.clear();
  this->GeneratorTargetSearchIndex.clear();
  this->ProjectMap.clear();
  this->RuleHashes.clear();
  this->DirectoryContentMap.clear();
  this->BinaryDirectories.clear();
}

void cmGlobalGenerator::ComputeTargetObjectDirectory(cmGeneratorTarget*) const
{
}

void cmGlobalGenerator::CheckTargetProperties()
{
  std::map<std::string, std::string> notFoundMap;
  //  std::set<std::string> notFoundMap;
  // after it is all done do a ConfigureFinalPass
  cmState* state = this->GetCMakeInstance()->GetState();
  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    this->Makefiles[i]->ConfigureFinalPass();
    cmTargets& targets = this->Makefiles[i]->GetTargets();
    for (cmTargets::iterator l = targets.begin(); l != targets.end(); l++) {
      if (l->second.GetType() == cmState::INTERFACE_LIBRARY) {
        continue;
      }
      const cmTarget::LinkLibraryVectorType& libs =
        l->second.GetOriginalLinkLibraries();
      for (cmTarget::LinkLibraryVectorType::const_iterator lib = libs.begin();
           lib != libs.end(); ++lib) {
        if (lib->first.size() > 9 &&
            cmSystemTools::IsNOTFOUND(lib->first.c_str())) {
          std::string varName = lib->first.substr(0, lib->first.size() - 9);
          if (state->GetCacheEntryPropertyAsBool(varName, "ADVANCED")) {
            varName += " (ADVANCED)";
          }
          std::string text = notFoundMap[varName];
          text += "\n    linked by target \"";
          text += l->second.GetName();
          text += "\" in directory ";
          text += this->Makefiles[i]->GetCurrentSourceDirectory();
          notFoundMap[varName] = text;
        }
      }
      std::vector<std::string> incs;
      const char* incDirProp = l->second.GetProperty("INCLUDE_DIRECTORIES");
      if (!incDirProp) {
        continue;
      }

      std::string incDirs = cmGeneratorExpression::Preprocess(
        incDirProp, cmGeneratorExpression::StripAllGeneratorExpressions);

      cmSystemTools::ExpandListArgument(incDirs, incs);

      for (std::vector<std::string>::const_iterator incDir = incs.begin();
           incDir != incs.end(); ++incDir) {
        if (incDir->size() > 9 && cmSystemTools::IsNOTFOUND(incDir->c_str())) {
          std::string varName = incDir->substr(0, incDir->size() - 9);
          if (state->GetCacheEntryPropertyAsBool(varName, "ADVANCED")) {
            varName += " (ADVANCED)";
          }
          std::string text = notFoundMap[varName];
          text += "\n   used as include directory in directory ";
          text += this->Makefiles[i]->GetCurrentSourceDirectory();
          notFoundMap[varName] = text;
        }
      }
    }
    this->CMakeInstance->UpdateProgress(
      "Configuring", 0.9f +
        0.1f * (static_cast<float>(i) + 1.0f) /
          static_cast<float>(this->Makefiles.size()));
  }

  if (!notFoundMap.empty()) {
    std::string notFoundVars;
    for (std::map<std::string, std::string>::const_iterator ii =
           notFoundMap.begin();
         ii != notFoundMap.end(); ++ii) {
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

int cmGlobalGenerator::TryCompile(const std::string& srcdir,
                                  const std::string& bindir,
                                  const std::string& projectName,
                                  const std::string& target, bool fast,
                                  std::string& output, cmMakefile* mf)
{
  // if this is not set, then this is a first time configure
  // and there is a good chance that the try compile stuff will
  // take the bulk of the time, so try and guess some progress
  // by getting closer and closer to 100 without actually getting there.
  if (!this->CMakeInstance->GetState()->GetInitializedCacheValue(
        "CMAKE_NUMBER_OF_MAKEFILES")) {
    // If CMAKE_NUMBER_OF_MAKEFILES is not set
    // we are in the first time progress and we have no
    // idea how long it will be.  So, just move 1/10th of the way
    // there each time, and don't go over 95%
    this->FirstTimeProgress += ((1.0f - this->FirstTimeProgress) / 30.0f);
    if (this->FirstTimeProgress > 0.95f) {
      this->FirstTimeProgress = 0.95f;
    }
    this->CMakeInstance->UpdateProgress("Configuring",
                                        this->FirstTimeProgress);
  }

  std::string newTarget;
  if (!target.empty()) {
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
  std::string config =
    mf->GetSafeDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
  return this->Build(srcdir, bindir, projectName, newTarget, output, "",
                     config, false, fast, false, this->TryCompileTimeout);
}

void cmGlobalGenerator::GenerateBuildCommand(
  std::vector<std::string>& makeCommand, const std::string&,
  const std::string&, const std::string&, const std::string&,
  const std::string&, bool, bool, std::vector<std::string> const&)
{
  makeCommand.push_back(
    "cmGlobalGenerator::GenerateBuildCommand not implemented");
}

int cmGlobalGenerator::Build(const std::string&, const std::string& bindir,
                             const std::string& projectName,
                             const std::string& target, std::string& output,
                             const std::string& makeCommandCSTR,
                             const std::string& config, bool clean, bool fast,
                             bool verbose, double timeout,
                             cmSystemTools::OutputOption outputflag,
                             std::vector<std::string> const& nativeOptions)
{
  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);
  output += "Change Dir: ";
  output += bindir;
  output += "\n";

  int retVal;
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();
  cmSystemTools::SetRunCommandHideConsole(true);
  std::string outputBuffer;
  std::string* outputPtr = &outputBuffer;

  std::vector<std::string> makeCommand;
  this->GenerateBuildCommand(makeCommand, makeCommandCSTR, projectName, bindir,
                             target, config, fast, verbose, nativeOptions);

  // Workaround to convince VCExpress.exe to produce output.
  if (outputflag == cmSystemTools::OUTPUT_PASSTHROUGH &&
      !makeCommand.empty() &&
      cmSystemTools::LowerCase(
        cmSystemTools::GetFilenameName(makeCommand[0])) == "vcexpress.exe") {
    outputflag = cmSystemTools::OUTPUT_FORWARD;
  }

  // should we do a clean first?
  if (clean) {
    std::vector<std::string> cleanCommand;
    this->GenerateBuildCommand(cleanCommand, makeCommandCSTR, projectName,
                               bindir, "clean", config, fast, verbose);
    output += "\nRun Clean Command:";
    output += cmSystemTools::PrintSingleCommand(cleanCommand);
    output += "\n";

    if (!cmSystemTools::RunSingleCommand(cleanCommand, outputPtr, outputPtr,
                                         &retVal, 0, outputflag, timeout)) {
      cmSystemTools::SetRunCommandHideConsole(hideconsole);
      cmSystemTools::Error("Generator: execution of make clean failed.");
      output += *outputPtr;
      output += "\nGenerator: execution of make clean failed.\n";

      // return to the original directory
      cmSystemTools::ChangeDirectory(cwd);
      return 1;
    }
    output += *outputPtr;
  }

  // now build
  std::string makeCommandStr = cmSystemTools::PrintSingleCommand(makeCommand);
  output += "\nRun Build Command:";
  output += makeCommandStr;
  output += "\n";

  if (!cmSystemTools::RunSingleCommand(makeCommand, outputPtr, outputPtr,
                                       &retVal, 0, outputflag, timeout)) {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    cmSystemTools::Error(
      "Generator: execution of make failed. Make command was: ",
      makeCommandStr.c_str());
    output += *outputPtr;
    output += "\nGenerator: execution of make failed. Make command was: " +
      makeCommandStr + "\n";

    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd);
    return 1;
  }
  output += *outputPtr;
  cmSystemTools::SetRunCommandHideConsole(hideconsole);

  // The SGI MipsPro 7.3 compiler does not return an error code when
  // the source has a #error in it!  This is a work-around for such
  // compilers.
  if ((retVal == 0) && (output.find("#error") != std::string::npos)) {
    retVal = 1;
  }

  cmSystemTools::ChangeDirectory(cwd);
  return retVal;
}

std::string cmGlobalGenerator::GenerateCMakeBuildCommand(
  const std::string& target, const std::string& config,
  const std::string& native, bool ignoreErrors)
{
  std::string makeCommand = cmSystemTools::GetCMakeCommand();
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand.c_str());
  makeCommand += " --build .";
  if (!config.empty()) {
    makeCommand += " --config \"";
    makeCommand += config;
    makeCommand += "\"";
  }
  if (!target.empty()) {
    makeCommand += " --target \"";
    makeCommand += target;
    makeCommand += "\"";
  }
  const char* sep = " -- ";
  if (ignoreErrors) {
    const char* iflag = this->GetBuildIgnoreErrorsFlag();
    if (iflag && *iflag) {
      makeCommand += sep;
      makeCommand += iflag;
      sep = " ";
    }
  }
  if (!native.empty()) {
    makeCommand += sep;
    makeCommand += native;
  }
  return makeCommand;
}

void cmGlobalGenerator::AddMakefile(cmMakefile* mf)
{
  this->Makefiles.push_back(mf);

  // update progress
  // estimate how many lg there will be
  const char* numGenC =
    this->CMakeInstance->GetState()->GetInitializedCacheValue(
      "CMAKE_NUMBER_OF_MAKEFILES");

  if (!numGenC) {
    // If CMAKE_NUMBER_OF_MAKEFILES is not set
    // we are in the first time progress and we have no
    // idea how long it will be.  So, just move half way
    // there each time, and don't go over 95%
    this->FirstTimeProgress += ((1.0f - this->FirstTimeProgress) / 30.0f);
    if (this->FirstTimeProgress > 0.95f) {
      this->FirstTimeProgress = 0.95f;
    }
    this->CMakeInstance->UpdateProgress("Configuring",
                                        this->FirstTimeProgress);
    return;
  }

  int numGen = atoi(numGenC);
  float prog = 0.9f * static_cast<float>(this->Makefiles.size()) /
    static_cast<float>(numGen);
  if (prog > 0.9f) {
    prog = 0.9f;
  }
  this->CMakeInstance->UpdateProgress("Configuring", prog);
}

void cmGlobalGenerator::AddInstallComponent(const char* component)
{
  if (component && *component) {
    this->InstallComponents.insert(component);
  }
}

void cmGlobalGenerator::EnableInstallTarget()
{
  this->InstallTargetEnabled = true;
}

cmLocalGenerator* cmGlobalGenerator::CreateLocalGenerator(cmMakefile* mf)
{
  return new cmLocalGenerator(this, mf);
}

void cmGlobalGenerator::EnableLanguagesFromGenerator(cmGlobalGenerator* gen,
                                                     cmMakefile* mf)
{
  this->SetConfiguredFilesPath(gen);
  this->TryCompileOuterMakefile = mf;
  const char* make =
    gen->GetCMakeInstance()->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_MAKE_PROGRAM", make,
                                          "make program", cmState::FILEPATH);
  // copy the enabled languages
  this->GetCMakeInstance()->GetState()->SetEnabledLanguages(
    gen->GetCMakeInstance()->GetState()->GetEnabledLanguages());
  this->LanguagesReady = gen->LanguagesReady;
  this->ExtensionToLanguage = gen->ExtensionToLanguage;
  this->IgnoreExtensions = gen->IgnoreExtensions;
  this->LanguageToOutputExtension = gen->LanguageToOutputExtension;
  this->LanguageToLinkerPreference = gen->LanguageToLinkerPreference;
  this->OutputExtensions = gen->OutputExtensions;
}

void cmGlobalGenerator::SetConfiguredFilesPath(cmGlobalGenerator* gen)
{
  if (!gen->ConfiguredFilesPath.empty()) {
    this->ConfiguredFilesPath = gen->ConfiguredFilesPath;
  } else {
    this->ConfiguredFilesPath = gen->CMakeInstance->GetHomeOutputDirectory();
    this->ConfiguredFilesPath += cmake::GetCMakeFilesDirectory();
  }
}

bool cmGlobalGenerator::IsExcluded(cmState::Snapshot const& rootSnp,
                                   cmState::Snapshot const& snp_) const
{
  cmState::Snapshot snp = snp_;
  while (snp.IsValid()) {
    if (snp == rootSnp) {
      // No directory excludes itself.
      return false;
    }

    if (snp.GetDirectory().GetPropertyAsBool("EXCLUDE_FROM_ALL")) {
      // This directory is excluded from its parent.
      return true;
    }
    snp = snp.GetBuildsystemDirectoryParent();
  }
  return false;
}

bool cmGlobalGenerator::IsExcluded(cmLocalGenerator* root,
                                   cmLocalGenerator* gen) const
{
  assert(gen);

  cmState::Snapshot rootSnp = root->GetStateSnapshot();
  cmState::Snapshot snp = gen->GetStateSnapshot();

  return this->IsExcluded(rootSnp, snp);
}

bool cmGlobalGenerator::IsExcluded(cmLocalGenerator* root,
                                   cmGeneratorTarget* target) const
{
  if (target->GetType() == cmState::INTERFACE_LIBRARY ||
      target->GetPropertyAsBool("EXCLUDE_FROM_ALL")) {
    // This target is excluded from its directory.
    return true;
  }
  // This target is included in its directory.  Check whether the
  // directory is excluded.
  return this->IsExcluded(root, target->GetLocalGenerator());
}

void cmGlobalGenerator::GetEnabledLanguages(
  std::vector<std::string>& lang) const
{
  lang = this->CMakeInstance->GetState()->GetEnabledLanguages();
}

int cmGlobalGenerator::GetLinkerPreference(const std::string& lang) const
{
  std::map<std::string, int>::const_iterator it =
    this->LanguageToLinkerPreference.find(lang);
  if (it != this->LanguageToLinkerPreference.end()) {
    return it->second;
  }
  return 0;
}

void cmGlobalGenerator::FillProjectMap()
{
  this->ProjectMap.clear(); // make sure we start with a clean map
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    // for each local generator add all projects
    cmState::Snapshot snp = this->LocalGenerators[i]->GetStateSnapshot();
    std::string name;
    do {
      std::string snpProjName = snp.GetProjectName();
      if (name != snpProjName) {
        name = snpProjName;
        this->ProjectMap[name].push_back(this->LocalGenerators[i]);
      }
      snp = snp.GetBuildsystemDirectoryParent();
    } while (snp.IsValid());
  }
}

cmMakefile* cmGlobalGenerator::FindMakefile(const std::string& start_dir) const
{
  for (std::vector<cmMakefile*>::const_iterator it = this->Makefiles.begin();
       it != this->Makefiles.end(); ++it) {
    std::string sd = (*it)->GetCurrentSourceDirectory();
    if (sd == start_dir) {
      return *it;
    }
  }
  return 0;
}

///! Find a local generator by its startdirectory
cmLocalGenerator* cmGlobalGenerator::FindLocalGenerator(
  const std::string& start_dir) const
{
  for (std::vector<cmLocalGenerator*>::const_iterator it =
         this->LocalGenerators.begin();
       it != this->LocalGenerators.end(); ++it) {
    std::string sd = (*it)->GetCurrentSourceDirectory();
    if (sd == start_dir) {
      return *it;
    }
  }
  return 0;
}

void cmGlobalGenerator::AddAlias(const std::string& name,
                                 std::string const& tgtName)
{
  this->AliasTargets[name] = tgtName;
}

bool cmGlobalGenerator::IsAlias(const std::string& name) const
{
  return this->AliasTargets.find(name) != this->AliasTargets.end();
}

void cmGlobalGenerator::IndexTarget(cmTarget* t)
{
  if (!t->IsImported() || t->IsImportedGloballyVisible()) {
    this->TargetSearchIndex[t->GetName()] = t;
  }
}

void cmGlobalGenerator::IndexGeneratorTarget(cmGeneratorTarget* gt)
{
  if (!gt->IsImported() || gt->IsImportedGloballyVisible()) {
    this->GeneratorTargetSearchIndex[gt->GetName()] = gt;
  }
}

cmTarget* cmGlobalGenerator::FindTargetImpl(std::string const& name) const
{
  TargetMap::const_iterator i = this->TargetSearchIndex.find(name);
  if (i != this->TargetSearchIndex.end()) {
    return i->second;
  }
  return 0;
}

cmGeneratorTarget* cmGlobalGenerator::FindGeneratorTargetImpl(
  std::string const& name) const
{
  GeneratorTargetMap::const_iterator i =
    this->GeneratorTargetSearchIndex.find(name);
  if (i != this->GeneratorTargetSearchIndex.end()) {
    return i->second;
  }
  return 0;
}

cmTarget* cmGlobalGenerator::FindTarget(const std::string& name,
                                        bool excludeAliases) const
{
  if (!excludeAliases) {
    std::map<std::string, std::string>::const_iterator ai =
      this->AliasTargets.find(name);
    if (ai != this->AliasTargets.end()) {
      return this->FindTargetImpl(ai->second);
    }
  }
  return this->FindTargetImpl(name);
}

cmGeneratorTarget* cmGlobalGenerator::FindGeneratorTarget(
  const std::string& name) const
{
  std::map<std::string, std::string>::const_iterator ai =
    this->AliasTargets.find(name);
  if (ai != this->AliasTargets.end()) {
    return this->FindGeneratorTargetImpl(ai->second);
  }
  return this->FindGeneratorTargetImpl(name);
}

bool cmGlobalGenerator::NameResolvesToFramework(
  const std::string& libname) const
{
  if (cmSystemTools::IsPathToFramework(libname.c_str())) {
    return true;
  }

  if (cmTarget* tgt = this->FindTarget(libname)) {
    if (tgt->IsFrameworkOnApple()) {
      return true;
    }
  }

  return false;
}

inline std::string removeQuotes(const std::string& s)
{
  if (s[0] == '\"' && s[s.size() - 1] == '\"') {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

void cmGlobalGenerator::CreateDefaultGlobalTargets(cmTargets* targets)
{
  cmMakefile* mf = this->Makefiles[0];
  const char* cmakeCfgIntDir = this->GetCMakeCFGIntDir();

  // CPack
  std::string workingDir = mf->GetCurrentBinaryDirectory();
  cmCustomCommandLines cpackCommandLines;
  std::vector<std::string> depends;
  cmCustomCommandLine singleLine;
  singleLine.push_back(cmSystemTools::GetCPackCommand());
  if (cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.') {
    singleLine.push_back("-C");
    singleLine.push_back(cmakeCfgIntDir);
  }
  singleLine.push_back("--config");
  std::string configFile = mf->GetCurrentBinaryDirectory();
  ;
  configFile += "/CPackConfig.cmake";
  std::string relConfigFile = "./CPackConfig.cmake";
  singleLine.push_back(relConfigFile);
  cpackCommandLines.push_back(singleLine);
  if (this->GetPreinstallTargetName()) {
    depends.push_back(this->GetPreinstallTargetName());
  } else {
    const char* noPackageAll =
      mf->GetDefinition("CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY");
    if (!noPackageAll || cmSystemTools::IsOff(noPackageAll)) {
      depends.push_back(this->GetAllTargetName());
    }
  }
  if (cmSystemTools::FileExists(configFile.c_str())) {
    (*targets)[this->GetPackageTargetName()] = this->CreateGlobalTarget(
      this->GetPackageTargetName(), "Run CPack packaging tool...",
      &cpackCommandLines, depends, workingDir.c_str(), /*uses_terminal*/ true);
  }
  // CPack source
  const char* packageSourceTargetName = this->GetPackageSourceTargetName();
  if (packageSourceTargetName) {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(cmSystemTools::GetCPackCommand());
    singleLine.push_back("--config");
    configFile = mf->GetCurrentBinaryDirectory();
    ;
    configFile += "/CPackSourceConfig.cmake";
    relConfigFile = "./CPackSourceConfig.cmake";
    singleLine.push_back(relConfigFile);
    if (cmSystemTools::FileExists(configFile.c_str())) {
      singleLine.push_back(configFile);
      cpackCommandLines.push_back(singleLine);
      (*targets)[packageSourceTargetName] = this->CreateGlobalTarget(
        packageSourceTargetName, "Run CPack packaging tool for source...",
        &cpackCommandLines, depends, workingDir.c_str(),
        /*uses_terminal*/ true);
    }
  }

  // Test
  if (mf->IsOn("CMAKE_TESTING_ENABLED")) {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(cmSystemTools::GetCTestCommand());
    singleLine.push_back("--force-new-ctest-process");
    if (cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.') {
      singleLine.push_back("-C");
      singleLine.push_back(cmakeCfgIntDir);
    } else // TODO: This is a hack. Should be something to do with the
           // generator
    {
      singleLine.push_back("$(ARGS)");
    }
    cpackCommandLines.push_back(singleLine);
    (*targets)[this->GetTestTargetName()] =
      this->CreateGlobalTarget(this->GetTestTargetName(), "Running tests...",
                               &cpackCommandLines, depends, 0,
                               /*uses_terminal*/ true);
  }

  // Edit Cache
  const char* editCacheTargetName = this->GetEditCacheTargetName();
  if (editCacheTargetName) {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());

    // Use generator preference for the edit_cache rule if it is defined.
    std::string edit_cmd = this->GetEditCacheCommand();
    if (!edit_cmd.empty()) {
      singleLine.push_back(edit_cmd);
      singleLine.push_back("-H$(CMAKE_SOURCE_DIR)");
      singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
      cpackCommandLines.push_back(singleLine);
      (*targets)[editCacheTargetName] = this->CreateGlobalTarget(
        editCacheTargetName, "Running CMake cache editor...",
        &cpackCommandLines, depends, 0, /*uses_terminal*/ true);
    } else {
      singleLine.push_back(cmSystemTools::GetCMakeCommand());
      singleLine.push_back("-E");
      singleLine.push_back("echo");
      singleLine.push_back("No interactive CMake dialog available.");
      cpackCommandLines.push_back(singleLine);
      (*targets)[editCacheTargetName] = this->CreateGlobalTarget(
        editCacheTargetName, "No interactive CMake dialog available...",
        &cpackCommandLines, depends, 0, /*uses_terminal*/ false);
    }
  }

  // Rebuild Cache
  const char* rebuildCacheTargetName = this->GetRebuildCacheTargetName();
  if (rebuildCacheTargetName) {
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    singleLine.push_back(cmSystemTools::GetCMakeCommand());
    singleLine.push_back("-H$(CMAKE_SOURCE_DIR)");
    singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
    cpackCommandLines.push_back(singleLine);
    (*targets)[rebuildCacheTargetName] = this->CreateGlobalTarget(
      rebuildCacheTargetName, "Running CMake to regenerate build system...",
      &cpackCommandLines, depends, 0, /*uses_terminal*/ true);
  }

  // Install
  bool skipInstallRules = mf->IsOn("CMAKE_SKIP_INSTALL_RULES");
  if (this->InstallTargetEnabled && skipInstallRules) {
    mf->IssueMessage(cmake::WARNING,
                     "CMAKE_SKIP_INSTALL_RULES was enabled even though "
                     "installation rules have been specified");
  } else if (this->InstallTargetEnabled && !skipInstallRules) {
    if (!cmakeCfgIntDir || !*cmakeCfgIntDir || cmakeCfgIntDir[0] == '.') {
      std::set<std::string>* componentsSet = &this->InstallComponents;
      cpackCommandLines.erase(cpackCommandLines.begin(),
                              cpackCommandLines.end());
      depends.erase(depends.begin(), depends.end());
      std::ostringstream ostr;
      if (!componentsSet->empty()) {
        ostr << "Available install components are: ";
        ostr << cmWrap('"', *componentsSet, '"', " ");
      } else {
        ostr << "Only default component available";
      }
      singleLine.push_back(ostr.str());
      (*targets)["list_install_components"] = this->CreateGlobalTarget(
        "list_install_components", ostr.str().c_str(), &cpackCommandLines,
        depends, 0, /*uses_terminal*/ false);
    }
    std::string cmd = cmSystemTools::GetCMakeCommand();
    cpackCommandLines.erase(cpackCommandLines.begin(),
                            cpackCommandLines.end());
    singleLine.erase(singleLine.begin(), singleLine.end());
    depends.erase(depends.begin(), depends.end());
    if (this->GetPreinstallTargetName()) {
      depends.push_back(this->GetPreinstallTargetName());
    } else {
      const char* noall =
        mf->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
      if (!noall || cmSystemTools::IsOff(noall)) {
        depends.push_back(this->GetAllTargetName());
      }
    }
    if (mf->GetDefinition("CMake_BINARY_DIR") &&
        !mf->IsOn("CMAKE_CROSSCOMPILING")) {
      // We are building CMake itself.  We cannot use the original
      // executable to install over itself.  The generator will
      // automatically convert this name to the build-time location.
      cmd = "cmake";
    }
    singleLine.push_back(cmd);
    if (cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.') {
      std::string cfgArg = "-DBUILD_TYPE=";
      bool iosPlatform = mf->PlatformIsAppleIos();
      if (iosPlatform) {
        cfgArg += "$(CONFIGURATION)";
        singleLine.push_back(cfgArg);
        cfgArg = "-DEFFECTIVE_PLATFORM_NAME=$(EFFECTIVE_PLATFORM_NAME)";
      } else {
        cfgArg += mf->GetDefinition("CMAKE_CFG_INTDIR");
      }
      singleLine.push_back(cfgArg);
    }
    singleLine.push_back("-P");
    singleLine.push_back("cmake_install.cmake");
    cpackCommandLines.push_back(singleLine);
    (*targets)[this->GetInstallTargetName()] = this->CreateGlobalTarget(
      this->GetInstallTargetName(), "Install the project...",
      &cpackCommandLines, depends, 0, /*uses_terminal*/ true);

    // install_local
    if (const char* install_local = this->GetInstallLocalTargetName()) {
      cmCustomCommandLine localCmdLine = singleLine;

      localCmdLine.insert(localCmdLine.begin() + 1,
                          "-DCMAKE_INSTALL_LOCAL_ONLY=1");
      cpackCommandLines.erase(cpackCommandLines.begin(),
                              cpackCommandLines.end());
      cpackCommandLines.push_back(localCmdLine);

      (*targets)[install_local] = this->CreateGlobalTarget(
        install_local, "Installing only the local directory...",
        &cpackCommandLines, depends, 0, /*uses_terminal*/ true);
    }

    // install_strip
    const char* install_strip = this->GetInstallStripTargetName();
    if ((install_strip != 0) && (mf->IsSet("CMAKE_STRIP"))) {
      cmCustomCommandLine stripCmdLine = singleLine;

      stripCmdLine.insert(stripCmdLine.begin() + 1,
                          "-DCMAKE_INSTALL_DO_STRIP=1");
      cpackCommandLines.erase(cpackCommandLines.begin(),
                              cpackCommandLines.end());
      cpackCommandLines.push_back(stripCmdLine);

      (*targets)[install_strip] = this->CreateGlobalTarget(
        install_strip, "Installing the project stripped...",
        &cpackCommandLines, depends, 0, /*uses_terminal*/ true);
    }
  }
}

const char* cmGlobalGenerator::GetPredefinedTargetsFolder()
{
  const char* prop = this->GetCMakeInstance()->GetState()->GetGlobalProperty(
    "PREDEFINED_TARGETS_FOLDER");

  if (prop) {
    return prop;
  }

  return "CMakePredefinedTargets";
}

bool cmGlobalGenerator::UseFolderProperty()
{
  const char* prop =
    this->GetCMakeInstance()->GetState()->GetGlobalProperty("USE_FOLDERS");

  // If this property is defined, let the setter turn this on or off...
  //
  if (prop) {
    return cmSystemTools::IsOn(prop);
  }

  // By default, this feature is OFF, since it is not supported in the
  // Visual Studio Express editions until VS11:
  //
  return false;
}

cmTarget cmGlobalGenerator::CreateGlobalTarget(
  const std::string& name, const char* message,
  const cmCustomCommandLines* commandLines, std::vector<std::string> depends,
  const char* workingDirectory, bool uses_terminal)
{
  // Package
  cmTarget target;
  target.SetType(cmState::GLOBAL_TARGET, name);
  target.SetProperty("EXCLUDE_FROM_ALL", "TRUE");

  std::vector<std::string> no_outputs;
  std::vector<std::string> no_byproducts;
  std::vector<std::string> no_depends;
  // Store the custom command in the target.
  cmCustomCommand cc(0, no_outputs, no_byproducts, no_depends, *commandLines,
                     0, workingDirectory);
  cc.SetUsesTerminal(uses_terminal);
  target.AddPostBuildCommand(cc);
  target.SetProperty("EchoString", message);
  std::vector<std::string>::iterator dit;
  for (dit = depends.begin(); dit != depends.end(); ++dit) {
    target.AddUtility(*dit);
  }

  // Organize in the "predefined targets" folder:
  //
  if (this->UseFolderProperty()) {
    target.SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
  }

  return target;
}

std::string cmGlobalGenerator::GenerateRuleFile(
  std::string const& output) const
{
  std::string ruleFile = output;
  ruleFile += ".rule";
  const char* dir = this->GetCMakeCFGIntDir();
  if (dir && dir[0] == '$') {
    cmSystemTools::ReplaceString(ruleFile, dir,
                                 cmake::GetCMakeFilesDirectory());
  }
  return ruleFile;
}

std::string cmGlobalGenerator::GetSharedLibFlagsForLanguage(
  std::string const& l) const
{
  std::map<std::string, std::string>::const_iterator it =
    this->LanguageToOriginalSharedLibFlags.find(l);
  if (it != this->LanguageToOriginalSharedLibFlags.end()) {
    return it->second;
  }
  return "";
}

void cmGlobalGenerator::AppendDirectoryForConfig(const std::string&,
                                                 const std::string&,
                                                 const std::string&,
                                                 std::string&)
{
  // Subclasses that support multiple configurations should implement
  // this method to append the subdirectory for the given build
  // configuration.
}

cmGlobalGenerator::TargetDependSet const&
cmGlobalGenerator::GetTargetDirectDepends(cmGeneratorTarget const* target)
{
  return this->TargetDependencies[target];
}

bool cmGlobalGenerator::IsReservedTarget(std::string const& name)
{
  // The following is a list of targets reserved
  // by one or more of the cmake generators.

  // Adding additional targets to this list will require a policy!
  const char* reservedTargets[] = {
    "all",        "ALL_BUILD", "help",       "install",        "INSTALL",
    "preinstall", "clean",     "edit_cache", "rebuild_cache",  "test",
    "RUN_TESTS",  "package",   "PACKAGE",    "package_source", "ZERO_CHECK"
  };

  return std::find(cmArrayBegin(reservedTargets), cmArrayEnd(reservedTargets),
                   name) != cmArrayEnd(reservedTargets);
}

void cmGlobalGenerator::SetExternalMakefileProjectGenerator(
  cmExternalMakefileProjectGenerator* extraGenerator)
{
  this->ExtraGenerator = extraGenerator;
  if (this->ExtraGenerator != 0) {
    this->ExtraGenerator->SetGlobalGenerator(this);
  }
}

std::string cmGlobalGenerator::GetExtraGeneratorName() const
{
  return this->ExtraGenerator ? this->ExtraGenerator->GetName()
                              : std::string();
}

void cmGlobalGenerator::FileReplacedDuringGenerate(const std::string& filename)
{
  this->FilesReplacedDuringGenerate.push_back(filename);
}

void cmGlobalGenerator::GetFilesReplacedDuringGenerate(
  std::vector<std::string>& filenames)
{
  filenames.clear();
  std::copy(this->FilesReplacedDuringGenerate.begin(),
            this->FilesReplacedDuringGenerate.end(),
            std::back_inserter(filenames));
}

void cmGlobalGenerator::GetTargetSets(TargetDependSet& projectTargets,
                                      TargetDependSet& originalTargets,
                                      cmLocalGenerator* root,
                                      GeneratorVector const& generators)
{
  // loop over all local generators
  for (std::vector<cmLocalGenerator*>::const_iterator i = generators.begin();
       i != generators.end(); ++i) {
    // check to make sure generator is not excluded
    if (this->IsExcluded(root, *i)) {
      continue;
    }
    // Get the targets in the makefile
    std::vector<cmGeneratorTarget*> tgts = (*i)->GetGeneratorTargets();
    // loop over all the targets
    for (std::vector<cmGeneratorTarget*>::iterator l = tgts.begin();
         l != tgts.end(); ++l) {
      cmGeneratorTarget* target = *l;
      if (this->IsRootOnlyTarget(target) &&
          target->GetLocalGenerator() != root) {
        continue;
      }
      // put the target in the set of original targets
      originalTargets.insert(target);
      // Get the set of targets that depend on target
      this->AddTargetDepends(target, projectTargets);
    }
  }
}

bool cmGlobalGenerator::IsRootOnlyTarget(cmGeneratorTarget* target) const
{
  return (target->GetType() == cmState::GLOBAL_TARGET ||
          target->GetName() == this->GetAllTargetName());
}

void cmGlobalGenerator::AddTargetDepends(cmGeneratorTarget const* target,
                                         TargetDependSet& projectTargets)
{
  // add the target itself
  if (projectTargets.insert(target).second) {
    // This is the first time we have encountered the target.
    // Recursively follow its dependencies.
    TargetDependSet const& ts = this->GetTargetDirectDepends(target);
    for (TargetDependSet::const_iterator i = ts.begin(); i != ts.end(); ++i) {
      this->AddTargetDepends(*i, projectTargets);
    }
  }
}

void cmGlobalGenerator::AddToManifest(std::string const& f)
{
  // Add to the content listing for the file's directory.
  std::string dir = cmSystemTools::GetFilenamePath(f);
  std::string file = cmSystemTools::GetFilenameName(f);
  DirectoryContent& dc = this->DirectoryContentMap[dir];
  dc.Generated.insert(file);
  dc.All.insert(file);
}

std::set<std::string> const& cmGlobalGenerator::GetDirectoryContent(
  std::string const& dir, bool needDisk)
{
  DirectoryContent& dc = this->DirectoryContentMap[dir];
  if (needDisk) {
    long mt = cmSystemTools::ModifiedTime(dir);
    if (mt != dc.LastDiskTime) {
      // Reset to non-loaded directory content.
      dc.All = dc.Generated;

      // Load the directory content from disk.
      cmsys::Directory d;
      if (d.Load(dir)) {
        unsigned long n = d.GetNumberOfFiles();
        for (unsigned long i = 0; i < n; ++i) {
          const char* f = d.GetFile(i);
          if (strcmp(f, ".") != 0 && strcmp(f, "..") != 0) {
            dc.All.insert(f);
          }
        }
      }
      dc.LastDiskTime = mt;
    }
  }
  return dc.All;
}

void cmGlobalGenerator::AddRuleHash(const std::vector<std::string>& outputs,
                                    std::string const& content)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Ignore if there are no outputs.
  if (outputs.empty()) {
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
  cmOutputConverter converter(this->GetMakefiles()[0]->GetStateSnapshot());
  std::string fname =
    converter.Convert(outputs[0], cmLocalGenerator::HOME_OUTPUT);

  // Associate the hash with this output.
  this->RuleHashes[fname] = hash;
#else
  (void)outputs;
  (void)content;
#endif
}

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

void cmGlobalGenerator::CheckRuleHashes(std::string const& pfile,
                                        std::string const& home)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream fin(pfile.c_str(), std::ios::in | std::ios::binary);
#else
  cmsys::ifstream fin(pfile.c_str(), std::ios::in);
#endif
  if (!fin) {
    return;
  }
  std::string line;
  std::string fname;
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    // Line format is a 32-byte hex string followed by a space
    // followed by a file name (with no escaping).

    // Skip blank and comment lines.
    if (line.size() < 34 || line[0] == '#') {
      continue;
    }

    // Get the filename.
    fname = line.substr(33, line.npos);

    // Look for a hash for this file's rule.
    std::map<std::string, RuleHash>::const_iterator rhi =
      this->RuleHashes.find(fname);
    if (rhi != this->RuleHashes.end()) {
      // Compare the rule hash in the file to that we were given.
      if (strncmp(line.c_str(), rhi->second.Data, 32) != 0) {
        // The rule has changed.  Delete the output so it will be
        // built again.
        fname = cmSystemTools::CollapseFullPath(fname, home.c_str());
        cmSystemTools::RemoveFile(fname);
      }
    } else {
      // We have no hash for a rule previously listed.  This may be a
      // case where a user has turned off a build option and might
      // want to turn it back on later, so do not delete the file.
      // Instead, we keep the rule hash as long as the file exists so
      // that if the feature is turned back on and the rule has
      // changed the file is still rebuilt.
      std::string fpath = cmSystemTools::CollapseFullPath(fname, home.c_str());
      if (cmSystemTools::FileExists(fpath.c_str())) {
        RuleHash hash;
        strncpy(hash.Data, line.c_str(), 32);
        this->RuleHashes[fname] = hash;
      }
    }
  }
}

void cmGlobalGenerator::WriteRuleHashes(std::string const& pfile)
{
  // Now generate a new persistence file with the current hashes.
  if (this->RuleHashes.empty()) {
    cmSystemTools::RemoveFile(pfile);
  } else {
    cmGeneratedFileStream fout(pfile.c_str());
    fout << "# Hashes of file build rules.\n";
    for (std::map<std::string, RuleHash>::const_iterator rhi =
           this->RuleHashes.begin();
         rhi != this->RuleHashes.end(); ++rhi) {
      fout.write(rhi->second.Data, 32);
      fout << " " << rhi->first << "\n";
    }
  }
}

void cmGlobalGenerator::WriteSummary()
{
  // Record all target directories in a central location.
  std::string fname = this->CMakeInstance->GetHomeOutputDirectory();
  fname += cmake::GetCMakeFilesDirectory();
  fname += "/TargetDirectories.txt";
  cmGeneratedFileStream fout(fname.c_str());

  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    std::vector<cmGeneratorTarget*> tgts =
      this->LocalGenerators[i]->GetGeneratorTargets();
    for (std::vector<cmGeneratorTarget*>::iterator it = tgts.begin();
         it != tgts.end(); ++it) {
      if ((*it)->GetType() == cmState::INTERFACE_LIBRARY) {
        continue;
      }
      this->WriteSummary(*it);
      fout << (*it)->GetSupportDirectory() << "\n";
    }
  }
}

void cmGlobalGenerator::WriteSummary(cmGeneratorTarget* target)
{
  // Place the labels file in a per-target support directory.
  std::string dir = target->GetSupportDirectory();
  std::string file = dir;
  file += "/Labels.txt";
  std::string json_file = dir + "/Labels.json";

#ifdef CMAKE_BUILD_WITH_CMAKE
  // Check whether labels are enabled for this target.
  if (const char* value = target->GetProperty("LABELS")) {
    Json::Value lj_root(Json::objectValue);
    Json::Value& lj_target = lj_root["target"] = Json::objectValue;
    lj_target["name"] = target->GetName();
    Json::Value& lj_target_labels = lj_target["labels"] = Json::arrayValue;
    Json::Value& lj_sources = lj_root["sources"] = Json::arrayValue;

    cmSystemTools::MakeDirectory(dir.c_str());
    cmGeneratedFileStream fout(file.c_str());

    // List the target-wide labels.  All sources in the target get
    // these labels.
    std::vector<std::string> labels;
    cmSystemTools::ExpandListArgument(value, labels);
    if (!labels.empty()) {
      fout << "# Target labels\n";
      for (std::vector<std::string>::const_iterator li = labels.begin();
           li != labels.end(); ++li) {
        fout << " " << *li << "\n";
        lj_target_labels.append(*li);
      }
    }

    // List the source files with any per-source labels.
    fout << "# Source files and their labels\n";
    std::vector<cmSourceFile*> sources;
    std::vector<std::string> configs;
    target->Target->GetMakefile()->GetConfigurations(configs);
    if (configs.empty()) {
      configs.push_back("");
    }
    for (std::vector<std::string>::const_iterator ci = configs.begin();
         ci != configs.end(); ++ci) {
      target->GetSourceFiles(sources, *ci);
    }
    std::vector<cmSourceFile*>::const_iterator sourcesEnd =
      cmRemoveDuplicates(sources);
    for (std::vector<cmSourceFile*>::const_iterator si = sources.begin();
         si != sourcesEnd; ++si) {
      Json::Value& lj_source = lj_sources.append(Json::objectValue);
      cmSourceFile* sf = *si;
      std::string const& sfp = sf->GetFullPath();
      fout << sfp << "\n";
      lj_source["file"] = sfp;
      if (const char* svalue = sf->GetProperty("LABELS")) {
        labels.clear();
        Json::Value& lj_source_labels = lj_source["labels"] = Json::arrayValue;
        cmSystemTools::ExpandListArgument(svalue, labels);
        for (std::vector<std::string>::const_iterator li = labels.begin();
             li != labels.end(); ++li) {
          fout << " " << *li << "\n";
          lj_source_labels.append(*li);
        }
      }
    }
    cmGeneratedFileStream json_fout(json_file.c_str());
    json_fout << lj_root;
  } else
#endif
  {
    cmSystemTools::RemoveFile(file);
    cmSystemTools::RemoveFile(json_file);
  }
}

// static
std::string cmGlobalGenerator::EscapeJSON(const std::string& s)
{
  std::string result;
  for (std::string::size_type i = 0; i < s.size(); ++i) {
    if (s[i] == '"' || s[i] == '\\') {
      result += '\\';
    }
    result += s[i];
  }
  return result;
}

void cmGlobalGenerator::SetFilenameTargetDepends(
  cmSourceFile* sf, std::set<cmGeneratorTarget const*> tgts)
{
  this->FilenameTargetDepends[sf] = tgts;
}

std::set<cmGeneratorTarget const*> const&
cmGlobalGenerator::GetFilenameTargetDepends(cmSourceFile* sf) const
{
  return this->FilenameTargetDepends[sf];
}

void cmGlobalGenerator::CreateEvaluationSourceFiles(
  std::string const& config) const
{
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    this->LocalGenerators[i]->CreateEvaluationFileOutputs(config);
  }
}

void cmGlobalGenerator::ProcessEvaluationFiles()
{
  std::vector<std::string> generatedFiles;
  unsigned int i;
  for (i = 0; i < this->LocalGenerators.size(); ++i) {
    this->LocalGenerators[i]->ProcessEvaluationFiles(generatedFiles);
  }
}

std::string cmGlobalGenerator::ExpandCFGIntDir(
  const std::string& str, const std::string& /*config*/) const
{
  return str;
}

bool cmGlobalGenerator::GenerateCPackPropertiesFile()
{
  cmake::InstalledFilesMap const& installedFiles =
    this->CMakeInstance->GetInstalledFiles();

  cmLocalGenerator* lg = this->LocalGenerators[0];
  cmMakefile* mf = lg->GetMakefile();

  std::vector<std::string> configs;
  std::string config = mf->GetConfigurations(configs, false);

  std::string path = this->CMakeInstance->GetHomeOutputDirectory();
  path += "/CPackProperties.cmake";

  if (!cmSystemTools::FileExists(path.c_str()) && installedFiles.empty()) {
    return true;
  }

  cmGeneratedFileStream file(path.c_str());
  file << "# CPack properties\n";

  for (cmake::InstalledFilesMap::const_iterator i = installedFiles.begin();
       i != installedFiles.end(); ++i) {
    cmInstalledFile const& installedFile = i->second;

    cmCPackPropertiesGenerator cpackPropertiesGenerator(lg, installedFile,
                                                        configs);

    cpackPropertiesGenerator.Generate(file, config, configs);
  }

  return true;
}
