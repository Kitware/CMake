/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGenerator.h"

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>
#endif

#include "cmAlgorithms.h"
#include "cmCPackPropertiesGenerator.h"
#include "cmComputeTargetDepends.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmDuration.h"
#include "cmExportBuildFileGenerator.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmInstallGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLocalGenerator.h"
#include "cmMSVC60LinkLineComputer.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmVersion.h"
#include "cmWorkingDirectory.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#  include "cmCryptoHash.h"
#  include "cmQtAutoGenGlobalInitializer.h"
#  include "cm_jsoncpp_value.h"
#  include "cm_jsoncpp_writer.h"
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1800
#  define KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#endif

const std::string kCMAKE_PLATFORM_INFO_INITIALIZED =
  "CMAKE_PLATFORM_INFO_INITIALIZED";

class cmInstalledFile;

bool cmTarget::StrictTargetComparison::operator()(cmTarget const* t1,
                                                  cmTarget const* t2) const
{
  int nameResult = strcmp(t1->GetName().c_str(), t2->GetName().c_str());
  if (nameResult == 0) {
    return strcmp(t1->GetMakefile()->GetCurrentBinaryDirectory().c_str(),
                  t2->GetMakefile()->GetCurrentBinaryDirectory().c_str()) < 0;
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
  this->TryCompileTimeout = cmDuration::zero();

  this->ExtraGenerator = nullptr;
  this->CurrentConfigureMakefile = nullptr;
  this->TryCompileOuterMakefile = nullptr;

  this->ConfigureDoneCMP0026AndCMP0024 = false;
  this->FirstTimeProgress = 0.0f;

  this->RecursionDepth = 0;

  cm->GetState()->SetIsGeneratorMultiConfig(false);
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

#if defined(CMAKE_BUILD_WITH_CMAKE)
Json::Value cmGlobalGenerator::GetJson() const
{
  Json::Value generator = Json::objectValue;
  generator["name"] = this->GetName();
  return generator;
}
#endif

bool cmGlobalGenerator::SetGeneratorInstance(std::string const& i,
                                             cmMakefile* mf)
{
  if (i.empty()) {
    return true;
  }

  std::ostringstream e;
  /* clang-format off */
  e <<
    "Generator\n"
    "  " << this->GetName() << "\n"
    "does not support instance specification, but instance\n"
    "  " << i << "\n"
    "was specified.";
  /* clang-format on */
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  return false;
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
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
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
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  return false;
}

std::string cmGlobalGenerator::SelectMakeProgram(
  const std::string& inMakeProgram, const std::string& makeDefault) const
{
  std::string makeProgram = inMakeProgram;
  if (cmSystemTools::IsOff(makeProgram)) {
    const char* makeProgramCSTR =
      this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
    if (cmSystemTools::IsOff(makeProgramCSTR)) {
      makeProgram = makeDefault;
    } else {
      makeProgram = makeProgramCSTR;
    }
    if (cmSystemTools::IsOff(makeProgram) && !makeProgram.empty()) {
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
      cmSystemTools::Error(langComp + " not set, after EnableLanguage");
    }
    return;
  }
  std::string const& name = mf->GetRequiredDefinition(langComp);
  std::string path;
  if (!cmSystemTools::FileIsFullPath(name)) {
    path = cmSystemTools::FindProgram(name);
  } else {
    path = name;
  }
  if (!optional && (path.empty() || !cmSystemTools::FileExists(path))) {
    return;
  }
  const std::string* cname =
    this->GetCMakeInstance()->GetState()->GetInitializedCacheValue(langComp);
  std::string changeVars;
  if (cname && !optional) {
    std::string cnameString;
    if (!cmSystemTools::FileIsFullPath(*cname)) {
      cnameString = cmSystemTools::FindProgram(*cname);
    } else {
      cnameString = *cname;
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
      changeVars += *cname;
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
      for (cmMakefile* m : this->Makefiles) {
        m->RemoveExportBuildFileGeneratorCMP0024(it->second);
      }
    }

    delete it->second;
    it->second = nullptr;
    this->BuildExportSets.erase(it);
    return result;
  }
  return false;
}

void cmGlobalGenerator::ForceLinkerLanguages()
{
}

bool cmGlobalGenerator::CheckTargetsForMissingSources() const
{
  bool failed = false;
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    const std::vector<cmGeneratorTarget*>& targets =
      localGen->GetGeneratorTargets();

    for (cmGeneratorTarget* target : targets) {
      if (target->GetType() == cmStateEnums::TargetType::GLOBAL_TARGET ||
          target->GetType() == cmStateEnums::TargetType::INTERFACE_LIBRARY ||
          target->GetType() == cmStateEnums::TargetType::UTILITY ||
          cmSystemTools::IsOn(target->GetProperty("ghs_integrity_app"))) {
        continue;
      }

      std::vector<std::string> configs;
      target->Makefile->GetConfigurations(configs);
      std::vector<cmSourceFile*> srcs;
      if (configs.empty()) {
        target->GetSourceFiles(srcs, "");
      } else {
        for (std::string const& config : configs) {
          target->GetSourceFiles(srcs, config);
          if (srcs.empty()) {
            break;
          }
        }
      }
      if (srcs.empty()) {
        std::ostringstream e;
        e << "No SOURCES given to target: " << target->GetName();
        this->GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR, e.str(), target->GetBacktrace());
        failed = true;
      }
    }
  }
  return failed;
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
bool cmGlobalGenerator::FindMakeProgram(cmMakefile* mf)
{
  if (this->FindMakeProgramFile.empty()) {
    cmSystemTools::Error(
      "Generator implementation error, "
      "all generators must specify this->FindMakeProgramFile");
    return false;
  }
  if (!mf->GetDefinition("CMAKE_MAKE_PROGRAM") ||
      cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::string setMakeProgram = mf->GetModulesFile(this->FindMakeProgramFile);
    if (!setMakeProgram.empty()) {
      mf->ReadListFile(setMakeProgram);
    }
  }
  if (!mf->GetDefinition("CMAKE_MAKE_PROGRAM") ||
      cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::ostringstream err;
    err << "CMake was unable to find a build program corresponding to \""
        << this->GetName() << "\".  CMAKE_MAKE_PROGRAM is not set.  You "
        << "probably need to select a different build tool.";
    cmSystemTools::Error(err.str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  std::string makeProgram = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  // if there are spaces in the make program use short path
  // but do not short path the actual program name, as
  // this can cause trouble with VSExpress
  if (makeProgram.find(' ') != std::string::npos) {
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
                           "make program", cmStateEnums::FILEPATH);
  }
  return true;
}

bool cmGlobalGenerator::CheckLanguages(
  std::vector<std::string> const& /* languages */, cmMakefile* /* mf */) const
{
  return true;
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
  for (std::string const& li : cur_languages) {
    if (!this->LanguagesInProgress.insert(li).second) {
      std::ostringstream e;
      e << "Language '" << li
        << "' is currently being enabled.  "
           "Recursive call not allowed.";
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      cmSystemTools::SetFatalErrorOccured();
      return;
    }
  }

  if (this->TryCompileOuterMakefile) {
    // In a try-compile we can only enable languages provided by caller.
    for (std::string const& lang : languages) {
      if (lang == "NONE") {
        this->SetLanguageEnabled("NONE", mf);
      } else {
        if (this->LanguagesReady.find(lang) == this->LanguagesReady.end()) {
          std::ostringstream e;
          e << "The test project needs language " << lang
            << " which is not enabled.";
          this->TryCompileOuterMakefile->IssueMessage(MessageType::FATAL_ERROR,
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
  rootBin += "/CMakeFiles";

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

  if (!this->CMakeInstance->GetIsInTryCompile()) {
    // Keep a mark in the cache to indicate that we've initialized the
    // platform information directory.  If the platform information
    // directory exists but the mark is missing then CMakeCache.txt
    // has been removed or replaced without also removing the CMakeFiles/
    // directory.  In this case remove the platform information directory
    // so that it will be re-initialized and the relevant information
    // restored in the cache.
    if (cmSystemTools::FileIsDirectory(rootBin) &&
        !mf->IsOn(kCMAKE_PLATFORM_INFO_INITIALIZED)) {
      cmSystemTools::RemoveADirectory(rootBin);
    }
    this->GetCMakeInstance()->AddCacheEntry(
      kCMAKE_PLATFORM_INFO_INITIALIZED, "1",
      "Platform information initialized", cmStateEnums::INTERNAL);
  }

  // try and load the CMakeSystem.cmake if it is there
  std::string fpath = rootBin;
  bool const readCMakeSystem = !mf->GetDefinition("CMAKE_SYSTEM_LOADED");
  if (readCMakeSystem) {
    fpath += "/CMakeSystem.cmake";
    if (cmSystemTools::FileExists(fpath)) {
      mf->ReadListFile(fpath);
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

#  ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#    pragma warning(push)
#    pragma warning(disable : 4996)
#  endif
    GetVersionExW((OSVERSIONINFOW*)&osviex);
#  ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#    pragma warning(pop)
#  endif
    std::ostringstream windowsVersionString;
    windowsVersionString << osviex.dwMajorVersion << "."
                         << osviex.dwMinorVersion << "."
                         << osviex.dwBuildNumber;
    mf->AddDefinition("CMAKE_HOST_SYSTEM_VERSION",
                      windowsVersionString.str().c_str());
#endif
    // Read the DetermineSystem file
    std::string systemFile = mf->GetModulesFile("CMakeDetermineSystem.cmake");
    mf->ReadListFile(systemFile);
    // load the CMakeSystem.cmake from the binary directory
    // this file is configured by the CMakeDetermineSystem.cmake file
    fpath = rootBin;
    fpath += "/CMakeSystem.cmake";
    mf->ReadListFile(fpath);
  }

  if (readCMakeSystem) {
    // Tell the generator about the instance, if any.
    std::string instance = mf->GetSafeDefinition("CMAKE_GENERATOR_INSTANCE");
    if (!this->SetGeneratorInstance(instance, mf)) {
      cmSystemTools::SetFatalErrorOccured();
      return;
    }

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

    // Find the native build tool for this generator.
    if (!this->FindMakeProgram(mf)) {
      return;
    }
  }

  // Check that the languages are supported by the generator and its
  // native build tool found above.
  if (!this->CheckLanguages(languages, mf)) {
    return;
  }

  // **** Load the system specific initialization if not yet loaded
  if (!mf->GetDefinition("CMAKE_SYSTEM_SPECIFIC_INITIALIZE_LOADED")) {
    fpath = mf->GetModulesFile("CMakeSystemSpecificInitialize.cmake");
    if (!mf->ReadListFile(fpath)) {
      cmSystemTools::Error("Could not find cmake module file: "
                           "CMakeSystemSpecificInitialize.cmake");
    }
  }

  std::map<std::string, bool> needTestLanguage;
  std::map<std::string, bool> needSetLanguageEnabledMaps;
  // foreach language
  // load the CMakeDetermine(LANG)Compiler.cmake file to find
  // the compiler

  for (std::string const& lang : languages) {
    needSetLanguageEnabledMaps[lang] = false;
    if (lang == "NONE") {
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
      if (cmSystemTools::FileExists(fpath)) {
        if (!mf->ReadListFile(fpath)) {
          cmSystemTools::Error("Could not find cmake module file: " + fpath);
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
      std::string determineFile = mf->GetModulesFile(determineCompiler);
      if (!mf->ReadListFile(determineFile)) {
        cmSystemTools::Error("Could not find cmake module file: " +
                             determineCompiler);
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
        const std::string& envVar = mf->GetRequiredDefinition(compilerEnv);
        const std::string& envVarValue =
          mf->GetRequiredDefinition(compilerName);
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
      if (!mf->ReadListFile(fpath)) {
        cmSystemTools::Error("Could not find cmake module file: " + fpath);
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
    if (!mf->ReadListFile(fpath)) {
      cmSystemTools::Error("Could not find cmake module file: "
                           "CMakeSystemSpecificInformation.cmake");
    }
  }
  // loop over languages again loading CMake(LANG)Information.cmake
  //
  for (std::string const& lang : languages) {
    if (lang == "NONE") {
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
    } else if ((lang != "RC") && (lang != "ASM_MASM")) {
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
          mf->IssueMessage(MessageType::FATAL_ERROR, noCompiler.str());
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
      std::string informationFile = mf->GetModulesFile(fpath);
      if (informationFile.empty()) {
        cmSystemTools::Error("Could not find cmake module file: " + fpath);
      } else if (!mf->ReadListFile(informationFile)) {
        cmSystemTools::Error("Could not process cmake module file: " +
                             informationFile);
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
        std::string ifpath = mf->GetModulesFile(testLang);
        if (!mf->ReadListFile(ifpath)) {
          cmSystemTools::Error("Could not find cmake module file: " +
                               testLang);
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
    this->LanguageToOriginalSharedLibFlags[lang] =
      mf->GetSafeDefinition(sharedLibFlagsVar);

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
  if (cmSystemTools::FileExists(projectCompatibility)) {
    mf->ReadListFile(projectCompatibility);
  }
  // Inform any extra generator of the new language.
  if (this->ExtraGenerator) {
    this->ExtraGenerator->EnableLanguage(languages, mf, false);
  }

  if (fatalError) {
    cmSystemTools::SetFatalErrorOccured();
  }

  for (std::string const& lang : cur_languages) {
    this->LanguagesInProgress.erase(lang);
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
          mf->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to convert AppleClang to Clang.
        mf->AddDefinition(compilerIdVar, "Clang");
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        mf->IssueMessage(
          MessageType::FATAL_ERROR,
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
          mf->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
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
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0047));
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior is to keep QCC.
        break;
    }
  }

  if (strcmp(compilerId, "XLClang") == 0) {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0089)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0089")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0089) << "\n"
            "Converting " << lang <<
            " compiler id \"XLClang\" to \"XL\" for compatibility."
            ;
          /* clang-format on */
          mf->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to convert XLClang to XL.
        mf->AddDefinition(compilerIdVar, "XL");
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        mf->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0089));
      case cmPolicies::NEW:
        // NEW behavior is to keep AppleClang.
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
    // output extension for some language.  In that case it should be ignored
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
    cmSystemTools::Message(msg, "Warning");
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
  for (std::string const& i : extensionList) {
    this->IgnoreExtensions[i] = true;
  }
}

void cmGlobalGenerator::FillExtensionToLanguageMap(const std::string& l,
                                                   cmMakefile* mf)
{
  std::string extensionsVar = std::string("CMAKE_") + std::string(l) +
    std::string("_SOURCE_FILE_EXTENSIONS");
  const std::string& exts = mf->GetSafeDefinition(extensionsVar);
  std::vector<std::string> extensionList;
  cmSystemTools::ExpandListArgument(exts, extensionList);
  for (std::string const& i : extensionList) {
    this->ExtensionToLanguage[i] = l;
  }
}

const char* cmGlobalGenerator::GetGlobalSetting(std::string const& name) const
{
  assert(!this->Makefiles.empty());
  return this->Makefiles[0]->GetDefinition(name);
}

bool cmGlobalGenerator::GlobalSettingIsOn(std::string const& name) const
{
  assert(!this->Makefiles.empty());
  return this->Makefiles[0]->IsOn(name);
}

std::string cmGlobalGenerator::GetSafeGlobalSetting(
  std::string const& name) const
{
  assert(!this->Makefiles.empty());
  return this->Makefiles[0]->GetSafeDefinition(name);
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
  this->CMakeInstance->GetState()->ClearEnabledLanguages();
}

void cmGlobalGenerator::CreateLocalGenerators()
{
  this->LocalGeneratorSearchIndex.clear();
  cmDeleteAll(this->LocalGenerators);
  this->LocalGenerators.clear();
  this->LocalGenerators.reserve(this->Makefiles.size());
  for (cmMakefile* m : this->Makefiles) {
    cmLocalGenerator* lg = this->CreateLocalGenerator(m);
    this->LocalGenerators.push_back(lg);
    this->IndexLocalGenerator(lg);
  }
}

void cmGlobalGenerator::Configure()
{
  this->FirstTimeProgress = 0.0f;
  this->ClearGeneratorMembers();

  cmStateSnapshot snapshot = this->CMakeInstance->GetCurrentSnapshot();

  snapshot.GetDirectory().SetCurrentSource(
    this->CMakeInstance->GetHomeDirectory());
  snapshot.GetDirectory().SetCurrentBinary(
    this->CMakeInstance->GetHomeOutputDirectory());

  cmMakefile* dirMf = new cmMakefile(this, snapshot);
  dirMf->SetRecursionDepth(this->RecursionDepth);
  this->Makefiles.push_back(dirMf);
  this->IndexMakefile(dirMf);

  this->BinaryDirectories.insert(
    this->CMakeInstance->GetHomeOutputDirectory());

  // now do it
  this->ConfigureDoneCMP0026AndCMP0024 = false;
  dirMf->Configure();
  dirMf->EnforceDirectoryLevelRules();

  this->ConfigureDoneCMP0026AndCMP0024 = true;

  // Put a copy of each global target in every directory.
  std::vector<GlobalTargetInfo> globalTargets;
  this->CreateDefaultGlobalTargets(globalTargets);

  for (cmMakefile* mf : this->Makefiles) {
    cmTargets* targets = &(mf->GetTargets());
    for (GlobalTargetInfo const& globalTarget : globalTargets) {
      targets->insert(cmTargets::value_type(
        globalTarget.Name, this->CreateGlobalTarget(globalTarget, mf)));
    }
  }

  // update the cache entry for the number of local generators, this is used
  // for progress
  char num[100];
  sprintf(num, "%d", static_cast<int>(this->Makefiles.size()));
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_NUMBER_OF_MAKEFILES", num,
                                          "number of local generators",
                                          cmStateEnums::INTERNAL);

  // check for link libraries and include directories containing "NOTFOUND"
  // and for infinite loops
  this->CheckTargetProperties();

  if (this->CMakeInstance->GetWorkingMode() == cmake::NORMAL_MODE) {
    std::ostringstream msg;
    if (cmSystemTools::GetErrorOccuredFlag()) {
      msg << "Configuring incomplete, errors occurred!";
      const char* logs[] = { "CMakeOutput.log", "CMakeError.log", nullptr };
      for (const char** log = logs; *log; ++log) {
        std::string f = this->CMakeInstance->GetHomeOutputDirectory();
        f += "/CMakeFiles";
        f += "/";
        f += *log;
        if (cmSystemTools::FileExists(f)) {
          msg << "\nSee also \"" << f << "\".";
        }
      }
    } else {
      msg << "Configuring done";
    }
    this->CMakeInstance->UpdateProgress(msg.str(), -1);
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
  for (std::string const& t : targets) {
    cmGeneratorTarget* gt = lg->FindGeneratorTargetToUse(t);
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
  return it == this->BuildExportSets.end() ? nullptr : it->second;
}

void cmGlobalGenerator::AddCMP0042WarnTarget(const std::string& target)
{
  this->CMP0042WarnTargets.insert(target);
}

void cmGlobalGenerator::AddCMP0068WarnTarget(const std::string& target)
{
  this->CMP0068WarnTargets.insert(target);
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
  cmSystemTools::Error(e.str());
  return false;
}

void cmGlobalGenerator::ComputeBuildFileGenerators()
{
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    std::vector<cmExportBuildFileGenerator*> gens =
      this->Makefiles[i]->GetExportBuildFileGenerators();
    for (cmExportBuildFileGenerator* g : gens) {
      g->Compute(this->LocalGenerators[i]);
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
  // clear targets to issue warning CMP0068 for
  this->CMP0068WarnTargets.clear();

  // Check whether this generator is allowed to run.
  if (!this->CheckALLOW_DUPLICATE_CUSTOM_TARGETS()) {
    return false;
  }
  this->FinalizeTargetCompileInfo();

  this->CreateGenerationObjects();

  // at this point this->LocalGenerators has been filled,
  // so create the map from project name to vector of local generators
  this->FillProjectMap();

  // Iterate through all targets and set up AUTOMOC, AUTOUIC and AUTORCC
  if (!this->QtAutoGen()) {
    return false;
  }

  // Add generator specific helper commands
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    localGen->AddHelperCommands();
  }

  // Finalize the set of compile features for each target.
  // FIXME: This turns into calls to cmMakefile::AddRequiredTargetFeature
  // which actually modifies the <lang>_STANDARD target property
  // on the original cmTarget instance.  It accumulates features
  // across all configurations.  Some refactoring is needed to
  // compute a per-config resulta purely during generation.
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    if (!localGen->ComputeTargetCompileFeatures()) {
      return false;
    }
  }

  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    cmMakefile* mf = localGen->GetMakefile();
    for (cmInstallGenerator* g : mf->GetInstallGenerators()) {
      g->Compute(localGen);
    }
  }

  this->AddExtraIDETargets();

  // Trace the dependencies, after that no custom commands should be added
  // because their dependencies might not be handled correctly
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    localGen->TraceDependencies();
  }

  // Make sure that all (non-imported) targets have source files added!
  if (this->CheckTargetsForMissingSources()) {
    return false;
  }

  this->ForceLinkerLanguages();

  // Compute the manifest of main targets generated.
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    localGen->ComputeTargetManifest();
  }

  // Compute the inter-target dependencies.
  if (!this->ComputeTargetDepends()) {
    return false;
  }

  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    localGen->ComputeHomeRelativeOutputPath();
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
      "Generating",
      (static_cast<float>(i) + 1.0f) /
        static_cast<float>(this->LocalGenerators.size()));
  }
  this->SetCurrentMakefile(nullptr);

  if (!this->GenerateCPackPropertiesFile()) {
    this->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR, "Could not write CPack properties file.");
  }

  for (auto& buildExpSet : this->BuildExportSets) {
    if (!buildExpSet.second->GenerateImportFile()) {
      if (!cmSystemTools::GetErrorOccuredFlag()) {
        this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                               "Could not write export file.");
      }
      return;
    }
  }
  // Update rule hashes.
  this->CheckRuleHashes();

  this->WriteSummary();

  if (this->ExtraGenerator != nullptr) {
    this->ExtraGenerator->Generate();
  }

  if (!this->CMP0042WarnTargets.empty()) {
    std::ostringstream w;
    w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0042) << "\n";
    w << "MACOSX_RPATH is not specified for"
         " the following targets:\n";
    for (std::string const& t : this->CMP0042WarnTargets) {
      w << " " << t << "\n";
    }
    this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
  }

  if (!this->CMP0068WarnTargets.empty()) {
    std::ostringstream w;
    /* clang-format off */
    w <<
      cmPolicies::GetPolicyWarning(cmPolicies::CMP0068) << "\n"
      "For compatibility with older versions of CMake, the install_name "
      "fields for the following targets are still affected by RPATH "
      "settings:\n"
      ;
    /* clang-format on */
    for (std::string const& t : this->CMP0068WarnTargets) {
      w << " " << t << "\n";
    }
    this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
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
  for (cmGeneratorTarget const* target : targets) {
    ctd.GetTargetDirectDepends(target, this->TargetDependencies[target]);
  }
  return true;
}

bool cmGlobalGenerator::QtAutoGen()
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmQtAutoGenGlobalInitializer initializer(this->LocalGenerators);
  return initializer.generate();
#else
  return true;
#endif
}

cmLinkLineComputer* cmGlobalGenerator::CreateLinkLineComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir) const
{
  return new cmLinkLineComputer(outputConverter, stateDir);
}

cmLinkLineComputer* cmGlobalGenerator::CreateMSVC60LinkLineComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir) const
{
  return new cmMSVC60LinkLineComputer(outputConverter, stateDir);
}

void cmGlobalGenerator::FinalizeTargetCompileInfo()
{
  std::vector<std::string> const langs =
    this->CMakeInstance->GetState()->GetEnabledLanguages();

  // Construct per-target generator information.
  for (cmMakefile* mf : this->Makefiles) {
    const cmStringRange noconfig_compile_definitions =
      mf->GetCompileDefinitionsEntries();
    const cmBacktraceRange noconfig_compile_definitions_bts =
      mf->GetCompileDefinitionsBacktraces();

    cmTargets& targets = mf->GetTargets();
    for (auto& target : targets) {
      cmTarget* t = &target.second;
      if (t->GetType() == cmStateEnums::GLOBAL_TARGET) {
        continue;
      }

      t->AppendBuildInterfaceIncludes();

      if (t->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }

      cmBacktraceRange::const_iterator btIt =
        noconfig_compile_definitions_bts.begin();
      for (cmStringRange::const_iterator it =
             noconfig_compile_definitions.begin();
           it != noconfig_compile_definitions.end(); ++it, ++btIt) {
        t->InsertCompileDefinition(*it, *btIt);
      }

      cmPolicies::PolicyStatus polSt =
        mf->GetPolicyStatus(cmPolicies::CMP0043);
      if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD) {
        std::vector<std::string> configs;
        mf->GetConfigurations(configs);

        for (std::string const& c : configs) {
          std::string defPropName = "COMPILE_DEFINITIONS_";
          defPropName += cmSystemTools::UpperCase(c);
          t->AppendProperty(defPropName, mf->GetProperty(defPropName));
        }
      }
    }

    // The standard include directories for each language
    // should be treated as system include directories.
    std::set<std::string> standardIncludesSet;
    for (std::string const& li : langs) {
      std::string const standardIncludesVar =
        "CMAKE_" + li + "_STANDARD_INCLUDE_DIRECTORIES";
      std::string const& standardIncludesStr =
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
    for (auto& target : targets) {
      cmTarget* t = &target.second;
      cmGeneratorTarget* gt = new cmGeneratorTarget(t, lg);
      lg->AddGeneratorTarget(gt);
    }
  }

  std::vector<cmTarget*> itgts = mf->GetImportedTargets();

  for (cmTarget* t : itgts) {
    lg->AddImportedGeneratorTarget(importedMap.find(t)->second);
  }
}

void cmGlobalGenerator::CreateGeneratorTargets(TargetTypes targetTypes)
{
  std::map<cmTarget*, cmGeneratorTarget*> importedMap;
  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    cmMakefile* mf = this->Makefiles[i];
    for (cmTarget* ownedImpTgt : mf->GetOwnedImportedTargets()) {
      cmLocalGenerator* lg = this->LocalGenerators[i];
      cmGeneratorTarget* gt = new cmGeneratorTarget(ownedImpTgt, lg);
      lg->AddOwnedImportedGeneratorTarget(gt);
      importedMap[ownedImpTgt] = gt;
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
  this->MakefileSearchIndex.clear();
  this->LocalGeneratorSearchIndex.clear();
  this->ProjectMap.clear();
  this->RuleHashes.clear();
  this->DirectoryContentMap.clear();
  this->BinaryDirectories.clear();
}

void cmGlobalGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* /*unused*/) const
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
    for (auto const& target : targets) {
      if (target.second.GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }
      const cmTarget::LinkLibraryVectorType& libs =
        target.second.GetOriginalLinkLibraries();
      for (auto const& lib : libs) {
        if (lib.first.size() > 9 &&
            cmSystemTools::IsNOTFOUND(lib.first.c_str())) {
          std::string varName = lib.first.substr(0, lib.first.size() - 9);
          if (state->GetCacheEntryPropertyAsBool(varName, "ADVANCED")) {
            varName += " (ADVANCED)";
          }
          std::string text = notFoundMap[varName];
          text += "\n    linked by target \"";
          text += target.second.GetName();
          text += "\" in directory ";
          text += this->Makefiles[i]->GetCurrentSourceDirectory();
          notFoundMap[varName] = text;
        }
      }
      std::vector<std::string> incs;
      const char* incDirProp =
        target.second.GetProperty("INCLUDE_DIRECTORIES");
      if (!incDirProp) {
        continue;
      }

      std::string incDirs = cmGeneratorExpression::Preprocess(
        incDirProp, cmGeneratorExpression::StripAllGeneratorExpressions);

      cmSystemTools::ExpandListArgument(incDirs, incs);

      for (std::string const& incDir : incs) {
        if (incDir.size() > 9 && cmSystemTools::IsNOTFOUND(incDir.c_str())) {
          std::string varName = incDir.substr(0, incDir.size() - 9);
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
      "Configuring",
      0.9f +
        0.1f * (static_cast<float>(i) + 1.0f) /
          static_cast<float>(this->Makefiles.size()));
  }

  if (!notFoundMap.empty()) {
    std::string notFoundVars;
    for (auto const& notFound : notFoundMap) {
      notFoundVars += notFound.first;
      notFoundVars += notFound.second;
      notFoundVars += "\n";
    }
    cmSystemTools::Error("The following variables are used in this project, "
                         "but they are set to NOTFOUND.\n"
                         "Please set them or make sure they are set and "
                         "tested correctly in the CMake files:\n" +
                         notFoundVars);
  }
}

int cmGlobalGenerator::TryCompile(int jobs, const std::string& srcdir,
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

  std::vector<std::string> newTarget = {};
  if (!target.empty()) {
    newTarget = { target };
  }
  std::string config =
    mf->GetSafeDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
  return this->Build(jobs, srcdir, bindir, projectName, newTarget, output, "",
                     config, false, fast, false, this->TryCompileTimeout);
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalGenerator::GenerateBuildCommand(
  const std::string& /*unused*/, const std::string& /*unused*/,
  const std::string& /*unused*/, std::vector<std::string> const& /*unused*/,
  const std::string& /*unused*/, bool /*unused*/, int /*unused*/,
  bool /*unused*/, std::vector<std::string> const& /*unused*/)
{
  GeneratedMakeCommand makeCommand;
  makeCommand.Add("cmGlobalGenerator::GenerateBuildCommand not implemented");
  return { std::move(makeCommand) };
}

void cmGlobalGenerator::PrintBuildCommandAdvice(std::ostream& /*os*/,
                                                int /*jobs*/) const
{
  // Subclasses override this method if they e.g want to give a warning that
  // they do not support certain build command line options
}

int cmGlobalGenerator::Build(
  int jobs, const std::string& /*unused*/, const std::string& bindir,
  const std::string& projectName, const std::vector<std::string>& targets,
  std::string& output, const std::string& makeCommandCSTR,
  const std::string& config, bool clean, bool fast, bool verbose,
  cmDuration timeout, cmSystemTools::OutputOption outputflag,
  std::vector<std::string> const& nativeOptions)
{
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();

  /**
   * Run an executable command and put the stdout in output.
   */
  cmWorkingDirectory workdir(bindir);
  output += "Change Dir: ";
  output += bindir;
  output += "\n";
  if (workdir.Failed()) {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    cmSystemTools::Error("Failed to change directory: ",
                         std::strerror(workdir.GetLastResult()));
    output += "Failed to change directory: ";
    output += std::strerror(workdir.GetLastResult());
    output += "\n";
    return 1;
  }

  int retVal = 0;
  cmSystemTools::SetRunCommandHideConsole(true);
  std::string outputBuffer;
  std::string* outputPtr = &outputBuffer;

  std::vector<GeneratedMakeCommand> makeCommand =
    this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir, targets,
                               config, fast, jobs, verbose, nativeOptions);

  // Workaround to convince some commands to produce output.
  if (outputflag == cmSystemTools::OUTPUT_PASSTHROUGH &&
      makeCommand.back().RequiresOutputForward) {
    outputflag = cmSystemTools::OUTPUT_FORWARD;
  }

  // should we do a clean first?
  if (clean) {
    std::vector<GeneratedMakeCommand> cleanCommand =
      this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir,
                                 { "clean" }, config, fast, jobs, verbose);
    output += "\nRun Clean Command:";
    output += cleanCommand.front().Printable();
    output += "\n";
    if (cleanCommand.size() != 1) {
      this->GetCMakeInstance()->IssueMessage(MessageType::INTERNAL_ERROR,
                                             "The generator did not produce "
                                             "exactly one command for the "
                                             "'clean' target");
      return 1;
    }
    if (!cmSystemTools::RunSingleCommand(cleanCommand.front().PrimaryCommand,
                                         outputPtr, outputPtr, &retVal,
                                         nullptr, outputflag, timeout)) {
      cmSystemTools::SetRunCommandHideConsole(hideconsole);
      cmSystemTools::Error("Generator: execution of make clean failed.");
      output += *outputPtr;
      output += "\nGenerator: execution of make clean failed.\n";

      return 1;
    }
    output += *outputPtr;
  }

  // now build
  std::string makeCommandStr;
  output += "\nRun Build Command(s):";

  for (auto command = makeCommand.begin(); command != makeCommand.end();
       ++command) {
    makeCommandStr = command->Printable();
    if (command != makeCommand.end()) {
      makeCommandStr += " && ";
    }

    output += makeCommandStr;
    if (!cmSystemTools::RunSingleCommand(command->PrimaryCommand, outputPtr,
                                         outputPtr, &retVal, nullptr,
                                         outputflag, timeout)) {
      cmSystemTools::SetRunCommandHideConsole(hideconsole);
      cmSystemTools::Error(
        "Generator: execution of make failed. Make command was: " +
        makeCommandStr);
      output += *outputPtr;
      output += "\nGenerator: execution of make failed. Make command was: " +
        makeCommandStr + "\n";

      return 1;
    }
    output += *outputPtr;
  }
  output += "\n";
  cmSystemTools::SetRunCommandHideConsole(hideconsole);

  // The OpenWatcom tools do not return an error code when a link
  // library is not found!
  if (this->CMakeInstance->GetState()->UseWatcomWMake() && retVal == 0 &&
      output.find("W1008: cannot open") != std::string::npos) {
    retVal = 1;
  }

  return retVal;
}

bool cmGlobalGenerator::Open(const std::string& bindir,
                             const std::string& projectName, bool dryRun)
{
  if (this->ExtraGenerator) {
    return this->ExtraGenerator->Open(bindir, projectName, dryRun);
  }

  return false;
}

std::string cmGlobalGenerator::GenerateCMakeBuildCommand(
  const std::string& target, const std::string& config,
  const std::string& native, bool ignoreErrors)
{
  std::string makeCommand = cmSystemTools::GetCMakeCommand();
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand);
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
  this->IndexMakefile(mf);

  // update progress
  // estimate how many lg there will be
  const std::string* numGenC =
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

  int numGen = atoi(numGenC->c_str());
  float prog = 0.9f * static_cast<float>(this->Makefiles.size()) /
    static_cast<float>(numGen);
  if (prog > 0.9f) {
    prog = 0.9f;
  }
  this->CMakeInstance->UpdateProgress("Configuring", prog);
}

void cmGlobalGenerator::AddInstallComponent(const std::string& component)
{
  if (!component.empty()) {
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
  this->GetCMakeInstance()->AddCacheEntry(
    "CMAKE_MAKE_PROGRAM", make, "make program", cmStateEnums::FILEPATH);
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
    this->ConfiguredFilesPath += "/CMakeFiles";
  }
}

bool cmGlobalGenerator::IsExcluded(cmStateSnapshot const& rootSnp,
                                   cmStateSnapshot const& snp_) const
{
  cmStateSnapshot snp = snp_;
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

  cmStateSnapshot rootSnp = root->GetStateSnapshot();
  cmStateSnapshot snp = gen->GetStateSnapshot();

  return this->IsExcluded(rootSnp, snp);
}

bool cmGlobalGenerator::IsExcluded(cmGeneratorTarget* target) const
{
  return target->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
    target->GetPropertyAsBool("EXCLUDE_FROM_ALL");
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
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    // for each local generator add all projects
    cmStateSnapshot snp = localGen->GetStateSnapshot();
    std::string name;
    do {
      std::string snpProjName = snp.GetProjectName();
      if (name != snpProjName) {
        name = snpProjName;
        this->ProjectMap[name].push_back(localGen);
      }
      snp = snp.GetBuildsystemDirectoryParent();
    } while (snp.IsValid());
  }
}

cmMakefile* cmGlobalGenerator::FindMakefile(const std::string& start_dir) const
{
  MakefileMap::const_iterator i = this->MakefileSearchIndex.find(start_dir);
  if (i != this->MakefileSearchIndex.end()) {
    return i->second;
  }
  return nullptr;
}

cmLocalGenerator* cmGlobalGenerator::FindLocalGenerator(
  cmDirectoryId const& id) const
{
  LocalGeneratorMap::const_iterator i =
    this->LocalGeneratorSearchIndex.find(id.String);
  if (i != this->LocalGeneratorSearchIndex.end()) {
    return i->second;
  }
  return nullptr;
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

static char const hexDigits[] = "0123456789abcdef";

std::string cmGlobalGenerator::IndexGeneratorTargetUniquely(
  cmGeneratorTarget const* gt)
{
  // Use the pointer value to uniquely identify the target instance.
  // Use a ":" prefix to avoid conflict with project-defined targets.
  // We must satisfy cmGeneratorExpression::IsValidTargetName so use no
  // other special characters.
  char buf[1 + sizeof(gt) * 2];
  char* b = buf;
  *b++ = ':';
  for (size_t i = 0; i < sizeof(gt); ++i) {
    unsigned char const c = reinterpret_cast<unsigned char const*>(&gt)[i];
    *b++ = hexDigits[(c & 0xf0) >> 4];
    *b++ = hexDigits[(c & 0x0f)];
  }
  std::string id(buf, sizeof(buf));
  // We internally index pointers to non-const generator targets
  // but our callers only have pointers to const generator targets.
  // They will give up non-const privileges when looking up anyway.
  this->GeneratorTargetSearchIndex[id] = const_cast<cmGeneratorTarget*>(gt);
  return id;
}

void cmGlobalGenerator::IndexMakefile(cmMakefile* mf)
{
  // FIXME: add_subdirectory supports multiple build directories
  // sharing the same source directory.  We currently index only the
  // first one, because that is what FindMakefile has always returned.
  // All of its callers will need to be modified to support looking
  // up directories by build directory path.
  this->MakefileSearchIndex.insert(
    MakefileMap::value_type(mf->GetCurrentSourceDirectory(), mf));
}

void cmGlobalGenerator::IndexLocalGenerator(cmLocalGenerator* lg)
{
  cmDirectoryId id = lg->GetMakefile()->GetDirectoryId();
  this->LocalGeneratorSearchIndex[id.String] = lg;
}

cmTarget* cmGlobalGenerator::FindTargetImpl(std::string const& name) const
{
  TargetMap::const_iterator i = this->TargetSearchIndex.find(name);
  if (i != this->TargetSearchIndex.end()) {
    return i->second;
  }
  return nullptr;
}

cmGeneratorTarget* cmGlobalGenerator::FindGeneratorTargetImpl(
  std::string const& name) const
{
  GeneratorTargetMap::const_iterator i =
    this->GeneratorTargetSearchIndex.find(name);
  if (i != this->GeneratorTargetSearchIndex.end()) {
    return i->second;
  }
  return nullptr;
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
  if (cmSystemTools::IsPathToFramework(libname)) {
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
  if (s.front() == '\"' && s.back() == '\"') {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

bool cmGlobalGenerator::CheckCMP0037(std::string const& targetName,
                                     std::string const& reason) const
{
  cmTarget* tgt = this->FindTarget(targetName);
  if (!tgt) {
    return true;
  }
  MessageType messageType = MessageType::AUTHOR_WARNING;
  std::ostringstream e;
  bool issueMessage = false;
  switch (tgt->GetPolicyStatusCMP0037()) {
    case cmPolicies::WARN:
      e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0037) << "\n";
      issueMessage = true;
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      break;
    case cmPolicies::NEW:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      issueMessage = true;
      messageType = MessageType::FATAL_ERROR;
      break;
  }
  if (issueMessage) {
    e << "The target name \"" << targetName << "\" is reserved " << reason
      << ".";
    if (messageType == MessageType::AUTHOR_WARNING) {
      e << "  It may result in undefined behavior.";
    }
    this->GetCMakeInstance()->IssueMessage(messageType, e.str(),
                                           tgt->GetBacktrace());
    if (messageType == MessageType::FATAL_ERROR) {
      return false;
    }
  }
  return true;
}

void cmGlobalGenerator::CreateDefaultGlobalTargets(
  std::vector<GlobalTargetInfo>& targets)
{
  this->AddGlobalTarget_Package(targets);
  this->AddGlobalTarget_PackageSource(targets);
  this->AddGlobalTarget_Test(targets);
  this->AddGlobalTarget_EditCache(targets);
  this->AddGlobalTarget_RebuildCache(targets);
  this->AddGlobalTarget_Install(targets);
}

void cmGlobalGenerator::AddGlobalTarget_Package(
  std::vector<GlobalTargetInfo>& targets)
{
  cmMakefile* mf = this->Makefiles[0];
  std::string configFile = mf->GetCurrentBinaryDirectory();
  configFile += "/CPackConfig.cmake";
  if (!cmSystemTools::FileExists(configFile)) {
    return;
  }

  static const auto reservedTargets = { "package", "PACKAGE" };
  for (auto const& target : reservedTargets) {
    if (!this->CheckCMP0037(target, "when CPack packaging is enabled")) {
      return;
    }
  }

  const char* cmakeCfgIntDir = this->GetCMakeCFGIntDir();
  GlobalTargetInfo gti;
  gti.Name = this->GetPackageTargetName();
  gti.Message = "Run CPack packaging tool...";
  gti.UsesTerminal = true;
  gti.WorkingDir = mf->GetCurrentBinaryDirectory();
  cmCustomCommandLine singleLine;
  singleLine.push_back(cmSystemTools::GetCPackCommand());
  if (cmakeCfgIntDir && *cmakeCfgIntDir && cmakeCfgIntDir[0] != '.') {
    singleLine.push_back("-C");
    singleLine.push_back(cmakeCfgIntDir);
  }
  singleLine.push_back("--config");
  singleLine.push_back("./CPackConfig.cmake");
  gti.CommandLines.push_back(std::move(singleLine));
  if (this->GetPreinstallTargetName()) {
    gti.Depends.emplace_back(this->GetPreinstallTargetName());
  } else {
    const char* noPackageAll =
      mf->GetDefinition("CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY");
    if (!noPackageAll || cmSystemTools::IsOff(noPackageAll)) {
      gti.Depends.emplace_back(this->GetAllTargetName());
    }
  }
  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_PackageSource(
  std::vector<GlobalTargetInfo>& targets)
{
  const char* packageSourceTargetName = this->GetPackageSourceTargetName();
  if (!packageSourceTargetName) {
    return;
  }

  cmMakefile* mf = this->Makefiles[0];
  std::string configFile = mf->GetCurrentBinaryDirectory();
  configFile += "/CPackSourceConfig.cmake";
  if (!cmSystemTools::FileExists(configFile)) {
    return;
  }

  static const auto reservedTargets = { "package_source" };
  for (auto const& target : reservedTargets) {
    if (!this->CheckCMP0037(target,
                            "when CPack source packaging is enabled")) {
      return;
    }
  }

  GlobalTargetInfo gti;
  gti.Name = packageSourceTargetName;
  gti.Message = "Run CPack packaging tool for source...";
  gti.WorkingDir = mf->GetCurrentBinaryDirectory();
  gti.UsesTerminal = true;
  cmCustomCommandLine singleLine;
  singleLine.push_back(cmSystemTools::GetCPackCommand());
  singleLine.push_back("--config");
  singleLine.push_back("./CPackSourceConfig.cmake");
  singleLine.push_back(std::move(configFile));
  gti.CommandLines.push_back(std::move(singleLine));
  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_Test(
  std::vector<GlobalTargetInfo>& targets)
{
  cmMakefile* mf = this->Makefiles[0];
  if (!mf->IsOn("CMAKE_TESTING_ENABLED")) {
    return;
  }

  static const auto reservedTargets = { "test", "RUN_TESTS" };
  for (auto const& target : reservedTargets) {
    if (!this->CheckCMP0037(target, "when CTest testing is enabled")) {
      return;
    }
  }

  const char* cmakeCfgIntDir = this->GetCMakeCFGIntDir();
  GlobalTargetInfo gti;
  gti.Name = this->GetTestTargetName();
  gti.Message = "Running tests...";
  gti.UsesTerminal = true;
  cmCustomCommandLine singleLine;
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
  gti.CommandLines.push_back(std::move(singleLine));
  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_EditCache(
  std::vector<GlobalTargetInfo>& targets)
{
  const char* editCacheTargetName = this->GetEditCacheTargetName();
  if (!editCacheTargetName) {
    return;
  }
  GlobalTargetInfo gti;
  gti.Name = editCacheTargetName;
  cmCustomCommandLine singleLine;

  // Use generator preference for the edit_cache rule if it is defined.
  std::string edit_cmd = this->GetEditCacheCommand();
  if (!edit_cmd.empty()) {
    singleLine.push_back(std::move(edit_cmd));
    singleLine.push_back("-S$(CMAKE_SOURCE_DIR)");
    singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
    gti.Message = "Running CMake cache editor...";
    gti.UsesTerminal = true;
  } else {
    singleLine.push_back(cmSystemTools::GetCMakeCommand());
    singleLine.push_back("-E");
    singleLine.push_back("echo");
    singleLine.push_back("No interactive CMake dialog available.");
    gti.Message = "No interactive CMake dialog available...";
    gti.UsesTerminal = false;
  }
  gti.CommandLines.push_back(std::move(singleLine));

  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_RebuildCache(
  std::vector<GlobalTargetInfo>& targets)
{
  const char* rebuildCacheTargetName = this->GetRebuildCacheTargetName();
  if (!rebuildCacheTargetName) {
    return;
  }
  GlobalTargetInfo gti;
  gti.Name = rebuildCacheTargetName;
  gti.Message = "Running CMake to regenerate build system...";
  gti.UsesTerminal = true;
  cmCustomCommandLine singleLine;
  singleLine.push_back(cmSystemTools::GetCMakeCommand());
  singleLine.push_back("-S$(CMAKE_SOURCE_DIR)");
  singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
  gti.CommandLines.push_back(std::move(singleLine));
  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_Install(
  std::vector<GlobalTargetInfo>& targets)
{
  cmMakefile* mf = this->Makefiles[0];
  const char* cmakeCfgIntDir = this->GetCMakeCFGIntDir();
  bool skipInstallRules = mf->IsOn("CMAKE_SKIP_INSTALL_RULES");
  if (this->InstallTargetEnabled && skipInstallRules) {
    this->CMakeInstance->IssueMessage(
      MessageType::WARNING,
      "CMAKE_SKIP_INSTALL_RULES was enabled even though "
      "installation rules have been specified",
      mf->GetBacktrace());
  } else if (this->InstallTargetEnabled && !skipInstallRules) {
    if (!cmakeCfgIntDir || !*cmakeCfgIntDir || cmakeCfgIntDir[0] == '.') {
      std::set<std::string>* componentsSet = &this->InstallComponents;
      std::ostringstream ostr;
      if (!componentsSet->empty()) {
        ostr << "Available install components are: ";
        ostr << cmWrap('"', *componentsSet, '"', " ");
      } else {
        ostr << "Only default component available";
      }
      GlobalTargetInfo gti;
      gti.Name = "list_install_components";
      gti.Message = ostr.str();
      gti.UsesTerminal = false;
      targets.push_back(std::move(gti));
    }
    std::string cmd = cmSystemTools::GetCMakeCommand();
    GlobalTargetInfo gti;
    gti.Name = this->GetInstallTargetName();
    gti.Message = "Install the project...";
    gti.UsesTerminal = true;
    cmCustomCommandLine singleLine;
    if (this->GetPreinstallTargetName()) {
      gti.Depends.emplace_back(this->GetPreinstallTargetName());
    } else {
      const char* noall =
        mf->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
      if (!noall || cmSystemTools::IsOff(noall)) {
        gti.Depends.emplace_back(this->GetAllTargetName());
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
      bool useEPN = this->UseEffectivePlatformName(mf);
      if (useEPN) {
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
    gti.CommandLines.push_back(singleLine);
    targets.push_back(gti);

    // install_local
    if (const char* install_local = this->GetInstallLocalTargetName()) {
      gti.Name = install_local;
      gti.Message = "Installing only the local directory...";
      gti.UsesTerminal = true;
      gti.CommandLines.clear();

      cmCustomCommandLine localCmdLine = singleLine;

      localCmdLine.insert(localCmdLine.begin() + 1,
                          "-DCMAKE_INSTALL_LOCAL_ONLY=1");

      gti.CommandLines.push_back(std::move(localCmdLine));
      targets.push_back(gti);
    }

    // install_strip
    const char* install_strip = this->GetInstallStripTargetName();
    if ((install_strip != nullptr) && (mf->IsSet("CMAKE_STRIP"))) {
      gti.Name = install_strip;
      gti.Message = "Installing the project stripped...";
      gti.UsesTerminal = true;
      gti.CommandLines.clear();

      cmCustomCommandLine stripCmdLine = singleLine;

      stripCmdLine.insert(stripCmdLine.begin() + 1,
                          "-DCMAKE_INSTALL_DO_STRIP=1");
      gti.CommandLines.push_back(std::move(stripCmdLine));
      targets.push_back(gti);
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

bool cmGlobalGenerator::UseFolderProperty() const
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

cmTarget cmGlobalGenerator::CreateGlobalTarget(GlobalTargetInfo const& gti,
                                               cmMakefile* mf)
{
  // Package
  cmTarget target(gti.Name, cmStateEnums::GLOBAL_TARGET,
                  cmTarget::VisibilityNormal, mf);
  target.SetProperty("EXCLUDE_FROM_ALL", "TRUE");

  std::vector<std::string> no_outputs;
  std::vector<std::string> no_byproducts;
  std::vector<std::string> no_depends;
  // Store the custom command in the target.
  cmCustomCommand cc(nullptr, no_outputs, no_byproducts, no_depends,
                     gti.CommandLines, nullptr, gti.WorkingDir.c_str());
  cc.SetUsesTerminal(gti.UsesTerminal);
  target.AddPostBuildCommand(cc);
  if (!gti.Message.empty()) {
    target.SetProperty("EchoString", gti.Message.c_str());
  }
  for (std::string const& d : gti.Depends) {
    target.AddUtility(d);
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
    cmSystemTools::ReplaceString(ruleFile, dir, "/CMakeFiles");
  }
  return ruleFile;
}

bool cmGlobalGenerator::ShouldStripResourcePath(cmMakefile* mf) const
{
  return mf->PlatformIsAppleEmbedded();
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

void cmGlobalGenerator::AppendDirectoryForConfig(const std::string& /*unused*/,
                                                 const std::string& /*unused*/,
                                                 const std::string& /*unused*/,
                                                 std::string& /*unused*/)
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
  const char* reservedTargets[] = { "all",       "ALL_BUILD",  "help",
                                    "install",   "INSTALL",    "preinstall",
                                    "clean",     "edit_cache", "rebuild_cache",
                                    "ZERO_CHECK" };

  return std::find(cm::cbegin(reservedTargets), cm::cend(reservedTargets),
                   name) != cm::cend(reservedTargets);
}

void cmGlobalGenerator::SetExternalMakefileProjectGenerator(
  cmExternalMakefileProjectGenerator* extraGenerator)
{
  this->ExtraGenerator = extraGenerator;
  if (this->ExtraGenerator != nullptr) {
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
  for (cmLocalGenerator* generator : generators) {
    // check to make sure generator is not excluded
    if (this->IsExcluded(root, generator)) {
      continue;
    }
    // Get the targets in the makefile
    const std::vector<cmGeneratorTarget*>& tgts =
      generator->GetGeneratorTargets();
    // loop over all the targets
    for (cmGeneratorTarget* target : tgts) {
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
  return (target->GetType() == cmStateEnums::GLOBAL_TARGET ||
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
    for (auto const& t : ts) {
      this->AddTargetDepends(t, projectTargets);
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
    cmCryptoHash md5(cmCryptoHash::AlgoMD5);
    std::string const md5_hex = md5.HashString(content);
    memcpy(hash.Data, md5_hex.c_str(), 32);
  }

  // Shorten the output name (in expected use case).
  cmStateDirectory cmDir =
    this->GetMakefiles()[0]->GetStateSnapshot().GetDirectory();
  std::string fname = cmDir.ConvertToRelPathIfNotContained(
    this->GetMakefiles()[0]->GetState()->GetBinaryDirectory(), outputs[0]);

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
  pfile += "/CMakeFiles";
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
  cmsys::ifstream fin(pfile.c_str());
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
    fname = line.substr(33);

    // Look for a hash for this file's rule.
    std::map<std::string, RuleHash>::const_iterator rhi =
      this->RuleHashes.find(fname);
    if (rhi != this->RuleHashes.end()) {
      // Compare the rule hash in the file to that we were given.
      if (strncmp(line.c_str(), rhi->second.Data, 32) != 0) {
        // The rule has changed.  Delete the output so it will be
        // built again.
        fname = cmSystemTools::CollapseFullPath(fname, home);
        cmSystemTools::RemoveFile(fname);
      }
    } else {
      // We have no hash for a rule previously listed.  This may be a
      // case where a user has turned off a build option and might
      // want to turn it back on later, so do not delete the file.
      // Instead, we keep the rule hash as long as the file exists so
      // that if the feature is turned back on and the rule has
      // changed the file is still rebuilt.
      std::string fpath = cmSystemTools::CollapseFullPath(fname, home);
      if (cmSystemTools::FileExists(fpath)) {
        RuleHash hash;
        memcpy(hash.Data, line.c_str(), 32);
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
    cmGeneratedFileStream fout(pfile);
    fout << "# Hashes of file build rules.\n";
    for (auto const& rh : this->RuleHashes) {
      fout.write(rh.second.Data, 32);
      fout << " " << rh.first << "\n";
    }
  }
}

void cmGlobalGenerator::WriteSummary()
{
  // Record all target directories in a central location.
  std::string fname = this->CMakeInstance->GetHomeOutputDirectory();
  fname += "/CMakeFiles";
  fname += "/TargetDirectories.txt";
  cmGeneratedFileStream fout(fname);

  for (cmLocalGenerator* lg : this->LocalGenerators) {
    const std::vector<cmGeneratorTarget*>& tgts = lg->GetGeneratorTargets();
    for (cmGeneratorTarget* tgt : tgts) {
      if (tgt->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }
      this->WriteSummary(tgt);
      fout << tgt->GetSupportDirectory() << "\n";
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
  const char* targetLabels = target->GetProperty("LABELS");
  const char* directoryLabels =
    target->Target->GetMakefile()->GetProperty("LABELS");
  const char* cmakeDirectoryLabels =
    target->Target->GetMakefile()->GetDefinition("CMAKE_DIRECTORY_LABELS");
  if (targetLabels || directoryLabels || cmakeDirectoryLabels) {
    Json::Value lj_root(Json::objectValue);
    Json::Value& lj_target = lj_root["target"] = Json::objectValue;
    lj_target["name"] = target->GetName();
    Json::Value& lj_target_labels = lj_target["labels"] = Json::arrayValue;
    Json::Value& lj_sources = lj_root["sources"] = Json::arrayValue;

    cmSystemTools::MakeDirectory(dir);
    cmGeneratedFileStream fout(file);

    std::vector<std::string> labels;

    // List the target-wide labels.  All sources in the target get
    // these labels.
    if (targetLabels) {
      cmSystemTools::ExpandListArgument(targetLabels, labels);
      if (!labels.empty()) {
        fout << "# Target labels\n";
        for (std::string const& l : labels) {
          fout << " " << l << "\n";
          lj_target_labels.append(l);
        }
      }
    }

    // List directory labels
    std::vector<std::string> directoryLabelsList;
    std::vector<std::string> cmakeDirectoryLabelsList;

    if (directoryLabels) {
      cmSystemTools::ExpandListArgument(directoryLabels, directoryLabelsList);
    }

    if (cmakeDirectoryLabels) {
      cmSystemTools::ExpandListArgument(cmakeDirectoryLabels,
                                        cmakeDirectoryLabelsList);
    }

    if (!directoryLabelsList.empty() || !cmakeDirectoryLabelsList.empty()) {
      fout << "# Directory labels\n";
    }

    for (std::string const& li : directoryLabelsList) {
      fout << " " << li << "\n";
      lj_target_labels.append(li);
    }

    for (std::string const& li : cmakeDirectoryLabelsList) {
      fout << " " << li << "\n";
      lj_target_labels.append(li);
    }

    // List the source files with any per-source labels.
    fout << "# Source files and their labels\n";
    std::vector<cmSourceFile*> sources;
    std::vector<std::string> configs;
    target->Target->GetMakefile()->GetConfigurations(configs);
    if (configs.empty()) {
      configs.emplace_back();
    }
    for (std::string const& c : configs) {
      target->GetSourceFiles(sources, c);
    }
    std::vector<cmSourceFile*>::const_iterator sourcesEnd =
      cmRemoveDuplicates(sources);
    for (cmSourceFile* sf : cmMakeRange(sources.cbegin(), sourcesEnd)) {
      Json::Value& lj_source = lj_sources.append(Json::objectValue);
      std::string const& sfp = sf->GetFullPath();
      fout << sfp << "\n";
      lj_source["file"] = sfp;
      if (const char* svalue = sf->GetProperty("LABELS")) {
        labels.clear();
        Json::Value& lj_source_labels = lj_source["labels"] = Json::arrayValue;
        cmSystemTools::ExpandListArgument(svalue, labels);
        for (std::string const& label : labels) {
          fout << " " << label << "\n";
          lj_source_labels.append(label);
        }
      }
    }
    cmGeneratedFileStream json_fout(json_file);
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
  result.reserve(s.size());
  for (char i : s) {
    switch (i) {
      case '"':
      case '\\':
        result += '\\';
        result += i;
        break;
      case '\n':
        result += "\\n";
        break;
      case '\t':
        result += "\\t";
        break;
      default:
        result += i;
    }
  }
  return result;
}

void cmGlobalGenerator::SetFilenameTargetDepends(
  cmSourceFile* sf, std::set<cmGeneratorTarget const*> const& tgts)
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
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    localGen->CreateEvaluationFileOutputs(config);
  }
}

void cmGlobalGenerator::ProcessEvaluationFiles()
{
  std::vector<std::string> generatedFiles;
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    localGen->ProcessEvaluationFiles(generatedFiles);
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

  if (!cmSystemTools::FileExists(path) && installedFiles.empty()) {
    return true;
  }

  cmGeneratedFileStream file(path);
  file << "# CPack properties\n";

  for (auto const& i : installedFiles) {
    cmInstalledFile const& installedFile = i.second;

    cmCPackPropertiesGenerator cpackPropertiesGenerator(lg, installedFile,
                                                        configs);

    cpackPropertiesGenerator.Generate(file, config, configs);
  }

  return true;
}
