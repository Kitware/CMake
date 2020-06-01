/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGenerator.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <utility>

#include <cm/memory>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"

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
#include "cmListFileCache.h"
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

#if !defined(CMAKE_BOOTSTRAP)
#  include "cm_jsoncpp_value.h"
#  include "cm_jsoncpp_writer.h"

#  include "cmCryptoHash.h"
#  include "cmQtAutoGenGlobalInitializer.h"
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
}

#if !defined(CMAKE_BOOTSTRAP)
Json::Value cmGlobalGenerator::GetJson() const
{
  Json::Value generator = Json::objectValue;
  generator["name"] = this->GetName();
  generator["multiConfig"] = this->IsMultiConfig();
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

bool cmGlobalGenerator::SetGeneratorToolset(std::string const& ts, bool,
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
  if (cmIsOff(makeProgram)) {
    const char* makeProgramCSTR =
      this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
    if (cmIsOff(makeProgramCSTR)) {
      makeProgram = makeDefault;
    } else {
      makeProgram = makeProgramCSTR;
    }
    if (cmIsOff(makeProgram) && !makeProgram.empty()) {
      makeProgram = "CMAKE_MAKE_PROGRAM-NOTFOUND";
    }
  }
  return makeProgram;
}

void cmGlobalGenerator::ResolveLanguageCompiler(const std::string& lang,
                                                cmMakefile* mf,
                                                bool optional) const
{
  std::string langComp = cmStrCat("CMAKE_", lang, "_COMPILER");

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
  this->BuildExportExportSets[gen->GetMainExportFileName()] = gen;
  this->AddBuildExportSet(gen);
}

bool cmGlobalGenerator::GenerateImportFile(const std::string& file)
{
  auto const it = this->BuildExportSets.find(file);
  if (it != this->BuildExportSets.end()) {
    bool result = it->second->GenerateImportFile();

    if (!this->ConfigureDoneCMP0026AndCMP0024) {
      for (const auto& m : this->Makefiles) {
        m->RemoveExportBuildFileGeneratorCMP0024(it->second);
      }
    }

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
  for (const auto& localGen : this->LocalGenerators) {
    for (const auto& target : localGen->GetGeneratorTargets()) {
      if (target->GetType() == cmStateEnums::TargetType::GLOBAL_TARGET ||
          target->GetType() == cmStateEnums::TargetType::INTERFACE_LIBRARY ||
          target->GetType() == cmStateEnums::TargetType::UTILITY ||
          cmIsOn(target->GetProperty("ghs_integrity_app"))) {
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

bool cmGlobalGenerator::CheckTargetsForType() const
{
  if (!this->GetLanguageEnabled("Swift")) {
    return false;
  }
  bool failed = false;
  for (const auto& generator : this->LocalGenerators) {
    for (const auto& target : generator->GetGeneratorTargets()) {
      if (target->GetType() == cmStateEnums::EXECUTABLE &&
          target->GetPropertyAsBool("WIN32_EXECUTABLE")) {
        std::vector<std::string> const& configs =
          target->Makefile->GetGeneratorConfigs();
        for (std::string const& config : configs) {
          if (target->GetLinkerLanguage(config) == "Swift") {
            this->GetCMakeInstance()->IssueMessage(
              MessageType::FATAL_ERROR,
              "WIN32_EXECUTABLE property is not supported on Swift "
              "executables",
              target->GetBacktrace());
            failed = true;
          }
        }
      }
    }
  }
  return failed;
}

bool cmGlobalGenerator::CheckTargetsForPchCompilePdb() const
{
  if (!this->GetLanguageEnabled("C") && !this->GetLanguageEnabled("CXX")) {
    return false;
  }
  bool failed = false;
  for (const auto& generator : this->LocalGenerators) {
    for (const auto& target : generator->GetGeneratorTargets()) {
      if (target->GetType() == cmStateEnums::TargetType::GLOBAL_TARGET ||
          target->GetType() == cmStateEnums::TargetType::INTERFACE_LIBRARY ||
          target->GetType() == cmStateEnums::TargetType::UTILITY ||
          cmIsOn(target->GetProperty("ghs_integrity_app"))) {
        continue;
      }

      const std::string reuseFrom =
        target->GetSafeProperty("PRECOMPILE_HEADERS_REUSE_FROM");
      const std::string compilePdb =
        target->GetSafeProperty("COMPILE_PDB_NAME");

      if (!reuseFrom.empty() && reuseFrom != compilePdb) {
        const std::string e = cmStrCat(
          "PRECOMPILE_HEADERS_REUSE_FROM property is set on target (\"",
          target->GetName(),
          "\"). Reusable precompile headers requires the COMPILE_PDB_NAME"
          " property to have the value \"",
          reuseFrom, "\"\n");
        this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e,
                                               target->GetBacktrace());
        failed = true;
      }
    }
  }
  return failed;
}

bool cmGlobalGenerator::IsExportedTargetsFile(
  const std::string& filename) const
{
  auto const it = this->BuildExportSets.find(filename);
  if (it == this->BuildExportSets.end()) {
    return false;
  }
  return !cmContains(this->BuildExportExportSets, filename);
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
      cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::string setMakeProgram = mf->GetModulesFile(this->FindMakeProgramFile);
    if (!setMakeProgram.empty()) {
      mf->ReadListFile(setMakeProgram);
    }
  }
  if (!mf->GetDefinition("CMAKE_MAKE_PROGRAM") ||
      cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
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
    makeProgram = cmStrCat(dir, '/', saveFile);
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
        if (!cmContains(this->LanguagesReady, lang)) {
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

  mf->AddDefinitionBool("RUN_CONFIGURE", true);
  std::string rootBin =
    cmStrCat(this->CMakeInstance->GetHomeOutputDirectory(), "/CMakeFiles");

  // If the configuration files path has been set,
  // then we are in a try compile and need to copy the enable language
  // files from the parent cmake bin dir, into the try compile bin dir
  if (!this->ConfiguredFilesPath.empty()) {
    rootBin = this->ConfiguredFilesPath;
  }
  rootBin += '/';
  rootBin += cmVersion::GetCMakeVersion();

  // set the dir for parent files so they can be used by modules
  mf->AddDefinition("CMAKE_PLATFORM_INFO_DIR", rootBin);

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
#    ifdef __INTEL_COMPILER
#      pragma warning(disable : 1478)
#    elif defined __clang__
#      pragma clang diagnostic push
#      pragma clang diagnostic ignored "-Wdeprecated-declarations"
#    else
#      pragma warning(disable : 4996)
#    endif
#  endif
    GetVersionExW((OSVERSIONINFOW*)&osviex);
#  ifdef KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#    ifdef __clang__
#      pragma clang diagnostic pop
#    else
#      pragma warning(pop)
#    endif
#  endif
    std::ostringstream windowsVersionString;
    windowsVersionString << osviex.dwMajorVersion << "."
                         << osviex.dwMinorVersion << "."
                         << osviex.dwBuildNumber;
    mf->AddDefinition("CMAKE_HOST_SYSTEM_VERSION", windowsVersionString.str());
#endif
    // Read the DetermineSystem file
    std::string systemFile = mf->GetModulesFile("CMakeDetermineSystem.cmake");
    mf->ReadListFile(systemFile);
    // load the CMakeSystem.cmake from the binary directory
    // this file is configured by the CMakeDetermineSystem.cmake file
    fpath = cmStrCat(rootBin, "/CMakeSystem.cmake");
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
    if (!this->SetGeneratorToolset(toolset, false, mf)) {
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
    std::string loadedLang = cmStrCat("CMAKE_", lang, "_COMPILER_LOADED");
    if (!mf->GetDefinition(loadedLang)) {
      fpath = cmStrCat(rootBin, "/CMake", lang, "Compiler.cmake");

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
      std::string determineCompiler =
        cmStrCat("CMakeDetermine", lang, "Compiler.cmake");
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
        std::string compilerName = cmStrCat("CMAKE_", lang, "_COMPILER");
        std::string compilerEnv =
          cmStrCat("CMAKE_", lang, "_COMPILER_ENV_VAR");
        const std::string& envVar = mf->GetRequiredDefinition(compilerEnv);
        const std::string& envVarValue =
          mf->GetRequiredDefinition(compilerName);
        std::string env = cmStrCat(envVar, '=', envVarValue);
        cmSystemTools::PutEnv(env);
      }

      // if determineLanguage was called then load the file it
      // configures CMake(LANG)Compiler.cmake
      fpath = cmStrCat(rootBin, "/CMake", lang, "Compiler.cmake");
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
    std::string compilerName = cmStrCat("CMAKE_", lang, "_COMPILER");
    std::string compilerEnv = cmStrCat("CMAKE_", lang, "_COMPILER_ENV_VAR");
    std::ostringstream noCompiler;
    const char* compilerFile = mf->GetDefinition(compilerName);
    if (!compilerFile || !*compilerFile || cmIsNOTFOUND(compilerFile)) {
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
        std::string compilerLangFile =
          cmStrCat(rootBin, "/CMake", lang, "Compiler.cmake");
        cmSystemTools::RemoveFile(compilerLangFile);
        if (!this->CMakeInstance->GetIsInTryCompile()) {
          this->PrintCompilerAdvice(noCompiler, lang,
                                    mf->GetDefinition(compilerEnv));
          mf->IssueMessage(MessageType::FATAL_ERROR, noCompiler.str());
          fatalError = true;
        }
      }
    }

    std::string langLoadedVar =
      cmStrCat("CMAKE_", lang, "_INFORMATION_LOADED");
    if (!mf->GetDefinition(langLoadedVar)) {
      fpath = cmStrCat("CMake", lang, "Information.cmake");
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
        std::string testLang = cmStrCat("CMakeTest", lang, "Compiler.cmake");
        std::string ifpath = mf->GetModulesFile(testLang);
        if (!mf->ReadListFile(ifpath)) {
          cmSystemTools::Error("Could not find cmake module file: " +
                               testLang);
        }
        std::string compilerWorks =
          cmStrCat("CMAKE_", lang, "_COMPILER_WORKS");
        // if the compiler did not work, then remove the
        // CMake(LANG)Compiler.cmake file so that it will get tested the
        // next time cmake is run
        if (!mf->IsOn(compilerWorks)) {
          std::string compilerLangFile =
            cmStrCat(rootBin, "/CMake", lang, "Compiler.cmake");
          cmSystemTools::RemoveFile(compilerLangFile);
        }
      } // end if in try compile
    }   // end need test language
    // Store the shared library flags so that we can satisfy CMP0018
    std::string sharedLibFlagsVar =
      cmStrCat("CMAKE_SHARED_LIBRARY_", lang, "_FLAGS");
    this->LanguageToOriginalSharedLibFlags[lang] =
      mf->GetSafeDefinition(sharedLibFlagsVar);

    // Translate compiler ids for compatibility.
    this->CheckCompilerIdCompatibility(mf, lang);
  } // end for each language

  // Now load files that can override any settings on the platform or for
  // the project First load the project compatibility file if it is in
  // cmake
  std::string projectCompatibility =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Modules/",
             mf->GetSafeDefinition("PROJECT_NAME"), "Compatibility.cmake");
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
  std::string const compilerId = mf->GetSafeDefinition(compilerIdVar);

  if (compilerId == "AppleClang") {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0025)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0025")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0025) << "\n"
            "Converting " << lang <<
            R"( compiler id "AppleClang" to "Clang" for compatibility.)"
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

  if (compilerId == "QCC") {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0047)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0047")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0047) << "\n"
            "Converting " << lang <<
            R"( compiler id "QCC" to "GNU" for compatibility.)"
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

  if (compilerId == "XLClang") {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0089)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0089")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0089) << "\n"
            "Converting " << lang <<
            R"( compiler id "XLClang" to "XL" for compatibility.)"
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
    auto const it = this->LanguageToOutputExtension.find(lang);
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
  auto const it = this->ExtensionToLanguage.find(ext);
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
  if (cmContains(this->LanguageToLinkerPreference, l)) {
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
    std::string msg =
      cmStrCat(linkerPrefVar, " is negative, adjusting it to 0");
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
  std::vector<std::string> extensionList = cmExpandedList(ignoreExts);
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
  std::vector<std::string> extensionList = cmExpandedList(exts);
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
  this->LocalGenerators.clear();
  this->LocalGenerators.reserve(this->Makefiles.size());
  for (const auto& m : this->Makefiles) {
    auto lg = this->CreateLocalGenerator(m.get());
    this->IndexLocalGenerator(lg.get());
    this->LocalGenerators.push_back(std::move(lg));
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

  auto dirMfu = cm::make_unique<cmMakefile>(this, snapshot);
  auto dirMf = dirMfu.get();
  this->Makefiles.push_back(std::move(dirMfu));
  dirMf->SetRecursionDepth(this->RecursionDepth);
  this->IndexMakefile(dirMf);

  this->BinaryDirectories.insert(
    this->CMakeInstance->GetHomeOutputDirectory());

  // now do it
  this->ConfigureDoneCMP0026AndCMP0024 = false;
  dirMf->Configure();
  dirMf->EnforceDirectoryLevelRules();

  this->ConfigureDoneCMP0026AndCMP0024 = true;

  // Put a copy of each global target in every directory.
  {
    std::vector<GlobalTargetInfo> globalTargets;
    this->CreateDefaultGlobalTargets(globalTargets);

    for (const auto& mf : this->Makefiles) {
      auto& targets = mf->GetTargets();
      for (GlobalTargetInfo const& globalTarget : globalTargets) {
        targets.emplace(globalTarget.Name,
                        this->CreateGlobalTarget(globalTarget, mf.get()));
      }
    }
  }

  // update the cache entry for the number of local generators, this is used
  // for progress
  char num[100];
  sprintf(num, "%d", static_cast<int>(this->Makefiles.size()));
  this->GetCMakeInstance()->AddCacheEntry("CMAKE_NUMBER_OF_MAKEFILES", num,
                                          "number of local generators",
                                          cmStateEnums::INTERNAL);

  if (this->CMakeInstance->GetWorkingMode() == cmake::NORMAL_MODE) {
    std::ostringstream msg;
    if (cmSystemTools::GetErrorOccuredFlag()) {
      msg << "Configuring incomplete, errors occurred!";
      const char* logs[] = { "CMakeOutput.log", "CMakeError.log", nullptr };
      for (const char** log = logs; *log; ++log) {
        std::string f = cmStrCat(this->CMakeInstance->GetHomeOutputDirectory(),
                                 "/CMakeFiles/", *log);
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
  // Commit side effects only if we are actually generating
  if (this->GetConfigureDoneCMP0026()) {
    this->CheckTargetProperties();
  }
  this->CreateGeneratorTargets(targetTypes);
  this->ComputeBuildFileGenerators();
}

void cmGlobalGenerator::CreateImportedGenerationObjects(
  cmMakefile* mf, const std::vector<std::string>& targets,
  std::vector<const cmGeneratorTarget*>& exports)
{
  this->CreateGenerationObjects(ImportedOnly);
  auto const mfit =
    std::find_if(this->Makefiles.begin(), this->Makefiles.end(),
                 [mf](const std::unique_ptr<cmMakefile>& item) {
                   return item.get() == mf;
                 });
  auto& lg =
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
  auto const it = this->BuildExportSets.find(filename);
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
    std::vector<std::unique_ptr<cmExportBuildFileGenerator>> const& gens =
      this->Makefiles[i]->GetExportBuildFileGenerators();
    for (std::unique_ptr<cmExportBuildFileGenerator> const& g : gens) {
      g->Compute(this->LocalGenerators[i].get());
    }
  }
}

bool cmGlobalGenerator::UnsupportedVariableIsDefined(const std::string& name,
                                                     bool supported) const
{
  if (!supported && this->Makefiles.front()->GetDefinition(name)) {
    std::ostringstream e;
    /* clang-format off */
    e <<
      "Generator\n"
      "  " << this->GetName() << "\n"
      "does not support variable\n"
      "  " << name << "\n"
      "but it has been specified."
      ;
    /* clang-format on */
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return true;
  }

  return false;
}

bool cmGlobalGenerator::Compute()
{
  // Make sure unsupported variables are not used.
  if (this->UnsupportedVariableIsDefined("CMAKE_DEFAULT_BUILD_TYPE",
                                         this->SupportsDefaultBuildType())) {
    return false;
  }
  if (this->UnsupportedVariableIsDefined("CMAKE_CROSS_CONFIGS",
                                         this->SupportsCrossConfigs())) {
    return false;
  }
  if (this->UnsupportedVariableIsDefined("CMAKE_DEFAULT_CONFIGS",
                                         this->SupportsDefaultConfigs())) {
    return false;
  }

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

  // Add automatically generated sources (e.g. unity build).
  if (!this->AddAutomaticSources()) {
    return false;
  }

  // Add generator specific helper commands
  for (const auto& localGen : this->LocalGenerators) {
    localGen->AddHelperCommands();
  }

  // Finalize the set of compile features for each target.
  // FIXME: This turns into calls to cmMakefile::AddRequiredTargetFeature
  // which actually modifies the <lang>_STANDARD target property
  // on the original cmTarget instance.  It accumulates features
  // across all configurations.  Some refactoring is needed to
  // compute a per-config resulta purely during generation.
  for (const auto& localGen : this->LocalGenerators) {
    if (!localGen->ComputeTargetCompileFeatures()) {
      return false;
    }
  }

  for (const auto& localGen : this->LocalGenerators) {
    cmMakefile* mf = localGen->GetMakefile();
    for (const auto& g : mf->GetInstallGenerators()) {
      if (!g->Compute(localGen.get())) {
        return false;
      }
    }
  }

  this->AddExtraIDETargets();

  // Trace the dependencies, after that no custom commands should be added
  // because their dependencies might not be handled correctly
  for (const auto& localGen : this->LocalGenerators) {
    localGen->TraceDependencies();
  }

  // Make sure that all (non-imported) targets have source files added!
  if (this->CheckTargetsForMissingSources()) {
    return false;
  }

  this->ForceLinkerLanguages();

  // Compute the manifest of main targets generated.
  for (const auto& localGen : this->LocalGenerators) {
    localGen->ComputeTargetManifest();
  }

  // Compute the inter-target dependencies.
  if (!this->ComputeTargetDepends()) {
    return false;
  }

  if (this->CheckTargetsForType()) {
    return false;
  }

  if (this->CheckTargetsForPchCompilePdb()) {
    return false;
  }

  for (const auto& localGen : this->LocalGenerators) {
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

  this->CMakeInstance->UpdateProgress("Generating", 0.1f);

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
      0.1f +
        0.9f * (static_cast<float>(i) + 1.0f) /
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

  if (this->ExtraGenerator) {
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
  for (cmGeneratorTarget const* target : ctd.GetTargets()) {
    ctd.GetTargetDirectDepends(target, this->TargetDependencies[target]);
  }
  return true;
}

bool cmGlobalGenerator::QtAutoGen()
{
#ifndef CMAKE_BOOTSTRAP
  cmQtAutoGenGlobalInitializer initializer(this->LocalGenerators);
  return initializer.generate();
#else
  return true;
#endif
}

bool cmGlobalGenerator::AddAutomaticSources()
{
  for (const auto& lg : this->LocalGenerators) {
    lg->CreateEvaluationFileOutputs();
    for (const auto& gt : lg->GetGeneratorTargets()) {
      if (gt->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
          gt->GetType() == cmStateEnums::UTILITY ||
          gt->GetType() == cmStateEnums::GLOBAL_TARGET) {
        continue;
      }
      lg->AddUnityBuild(gt.get());
      // Targets that re-use a PCH are handled below.
      if (!gt->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM")) {
        lg->AddPchDependencies(gt.get());
      }
    }
  }
  for (const auto& lg : this->LocalGenerators) {
    for (const auto& gt : lg->GetGeneratorTargets()) {
      if (gt->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
          gt->GetType() == cmStateEnums::UTILITY ||
          gt->GetType() == cmStateEnums::GLOBAL_TARGET) {
        continue;
      }
      // Handle targets that re-use a PCH from an above-handled target.
      if (gt->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM")) {
        lg->AddPchDependencies(gt.get());
      }
    }
  }
  // The above transformations may have changed the classification of sources.
  // Clear the source list and classification cache (KindedSources) of all
  // targets so that it will be recomputed correctly by the generators later
  // now that the above transformations are done for all targets.
  for (const auto& lg : this->LocalGenerators) {
    for (const auto& gt : lg->GetGeneratorTargets()) {
      gt->ClearSourcesCache();
    }
  }
  return true;
}

std::unique_ptr<cmLinkLineComputer> cmGlobalGenerator::CreateLinkLineComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir) const
{
  return cm::make_unique<cmLinkLineComputer>(outputConverter, stateDir);
}

std::unique_ptr<cmLinkLineComputer>
cmGlobalGenerator::CreateMSVC60LinkLineComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir) const
{
  return std::unique_ptr<cmLinkLineComputer>(
    cm::make_unique<cmMSVC60LinkLineComputer>(outputConverter, stateDir));
}

void cmGlobalGenerator::FinalizeTargetCompileInfo()
{
  std::vector<std::string> const langs =
    this->CMakeInstance->GetState()->GetEnabledLanguages();

  // Construct per-target generator information.
  for (const auto& mf : this->Makefiles) {
    const cmStringRange noconfig_compile_definitions =
      mf->GetCompileDefinitionsEntries();
    const cmBacktraceRange noconfig_compile_definitions_bts =
      mf->GetCompileDefinitionsBacktraces();

    for (auto& target : mf->GetTargets()) {
      cmTarget* t = &target.second;
      if (t->GetType() == cmStateEnums::GLOBAL_TARGET) {
        continue;
      }

      t->AppendBuildInterfaceIncludes();

      if (t->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }

      {
        auto btIt = noconfig_compile_definitions_bts.begin();
        auto it = noconfig_compile_definitions.begin();
        for (; it != noconfig_compile_definitions.end(); ++it, ++btIt) {
          t->InsertCompileDefinition(*it, *btIt);
        }
      }

      cmPolicies::PolicyStatus polSt =
        mf->GetPolicyStatus(cmPolicies::CMP0043);
      if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD) {
        std::vector<std::string> configs;
        mf->GetConfigurations(configs);

        for (std::string const& c : configs) {
          std::string defPropName =
            cmStrCat("COMPILE_DEFINITIONS_", cmSystemTools::UpperCase(c));
          if (const char* val = mf->GetProperty(defPropName)) {
            t->AppendProperty(defPropName, val);
          }
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
      std::vector<std::string> standardIncludesVec =
        cmExpandedList(standardIncludesStr);
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
    for (auto& target : mf->GetTargets()) {
      cmTarget* t = &target.second;
      lg->AddGeneratorTarget(cm::make_unique<cmGeneratorTarget>(t, lg));
    }
  }

  for (cmTarget* t : mf->GetImportedTargets()) {
    lg->AddImportedGeneratorTarget(importedMap.find(t)->second);
  }
}

void cmGlobalGenerator::CreateGeneratorTargets(TargetTypes targetTypes)
{
  std::map<cmTarget*, cmGeneratorTarget*> importedMap;
  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    auto& mf = this->Makefiles[i];
    for (const auto& ownedImpTgt : mf->GetOwnedImportedTargets()) {
      cmLocalGenerator* lg = this->LocalGenerators[i].get();
      auto gt = cm::make_unique<cmGeneratorTarget>(ownedImpTgt.get(), lg);
      importedMap[ownedImpTgt.get()] = gt.get();
      lg->AddOwnedImportedGeneratorTarget(std::move(gt));
    }
  }

  // Construct per-target generator information.
  for (unsigned int i = 0; i < this->LocalGenerators.size(); ++i) {
    this->CreateGeneratorTargets(targetTypes, this->Makefiles[i].get(),
                                 this->LocalGenerators[i].get(), importedMap);
  }
}

void cmGlobalGenerator::ClearGeneratorMembers()
{
  this->BuildExportSets.clear();

  this->Makefiles.clear();

  this->LocalGenerators.clear();

  this->AliasTargets.clear();
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
  // check for link libraries and include directories containing "NOTFOUND"
  // and for infinite loops
  std::map<std::string, std::string> notFoundMap;
  cmState* state = this->GetCMakeInstance()->GetState();
  for (unsigned int i = 0; i < this->Makefiles.size(); ++i) {
    this->Makefiles[i]->Generate(*this->LocalGenerators[i]);
    for (auto const& target : this->Makefiles[i]->GetTargets()) {
      if (target.second.GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }
      for (auto const& lib : target.second.GetOriginalLinkLibraries()) {
        if (lib.first.size() > 9 && cmIsNOTFOUND(lib.first)) {
          std::string varName = lib.first.substr(0, lib.first.size() - 9);
          if (state->GetCacheEntryPropertyAsBool(varName, "ADVANCED")) {
            varName += " (ADVANCED)";
          }
          std::string text =
            cmStrCat(notFoundMap[varName], "\n    linked by target \"",
                     target.second.GetName(), "\" in directory ",
                     this->Makefiles[i]->GetCurrentSourceDirectory());
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

      cmExpandList(incDirs, incs);

      for (std::string const& incDir : incs) {
        if (incDir.size() > 9 && cmIsNOTFOUND(incDir)) {
          std::string varName = incDir.substr(0, incDir.size() - 9);
          if (state->GetCacheEntryPropertyAsBool(varName, "ADVANCED")) {
            varName += " (ADVANCED)";
          }
          std::string text =
            cmStrCat(notFoundMap[varName],
                     "\n   used as include directory in directory ",
                     this->Makefiles[i]->GetCurrentSourceDirectory());
          notFoundMap[varName] = text;
        }
      }
    }
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
    std::string err = cmStrCat("Failed to change directory: ",
                               std::strerror(workdir.GetLastResult()));
    cmSystemTools::Error(err);
    output += err;
    output += "\n";
    return 1;
  }
  std::string realConfig = config;
  if (realConfig.empty()) {
    realConfig = this->GetDefaultBuildConfig();
  }

  int retVal = 0;
  cmSystemTools::SetRunCommandHideConsole(true);
  std::string outputBuffer;
  std::string* outputPtr = &outputBuffer;

  std::vector<GeneratedMakeCommand> makeCommand =
    this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir, targets,
                               realConfig, fast, jobs, verbose, nativeOptions);

  // Workaround to convince some commands to produce output.
  if (outputflag == cmSystemTools::OUTPUT_PASSTHROUGH &&
      makeCommand.back().RequiresOutputForward) {
    outputflag = cmSystemTools::OUTPUT_FORWARD;
  }

  // should we do a clean first?
  if (clean) {
    std::vector<GeneratedMakeCommand> cleanCommand =
      this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir,
                                 { "clean" }, realConfig, fast, jobs, verbose);
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
  makeCommand =
    cmStrCat(cmSystemTools::ConvertToOutputPath(makeCommand), " --build .");
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

void cmGlobalGenerator::AddMakefile(std::unique_ptr<cmMakefile> mf)
{
  this->IndexMakefile(mf.get());
  this->Makefiles.push_back(std::move(mf));

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
  float prog =
    static_cast<float>(this->Makefiles.size()) / static_cast<float>(numGen);
  if (prog > 1.0f) {
    prog = 1.0f;
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

std::unique_ptr<cmLocalGenerator> cmGlobalGenerator::CreateLocalGenerator(
  cmMakefile* mf)
{
  return cm::make_unique<cmLocalGenerator>(this, mf);
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
    this->ConfiguredFilesPath =
      cmStrCat(gen->CMakeInstance->GetHomeOutputDirectory(), "/CMakeFiles");
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

bool cmGlobalGenerator::IsExcluded(cmLocalGenerator* root,
                                   cmGeneratorTarget* target) const
{
  if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return true;
  }
  if (const char* exclude = target->GetProperty("EXCLUDE_FROM_ALL")) {
    return cmIsOn(exclude);
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
  auto const it = this->LanguageToLinkerPreference.find(lang);
  if (it != this->LanguageToLinkerPreference.end()) {
    return it->second;
  }
  return 0;
}

void cmGlobalGenerator::FillProjectMap()
{
  this->ProjectMap.clear(); // make sure we start with a clean map
  for (const auto& localGen : this->LocalGenerators) {
    // for each local generator add all projects
    cmStateSnapshot snp = localGen->GetStateSnapshot();
    std::string name;
    do {
      std::string snpProjName = snp.GetProjectName();
      if (name != snpProjName) {
        name = snpProjName;
        this->ProjectMap[name].push_back(localGen.get());
      }
      snp = snp.GetBuildsystemDirectoryParent();
    } while (snp.IsValid());
  }
}

cmMakefile* cmGlobalGenerator::FindMakefile(const std::string& start_dir) const
{
  auto const it = this->MakefileSearchIndex.find(start_dir);
  if (it != this->MakefileSearchIndex.end()) {
    return it->second;
  }
  return nullptr;
}

cmLocalGenerator* cmGlobalGenerator::FindLocalGenerator(
  cmDirectoryId const& id) const
{
  auto const it = this->LocalGeneratorSearchIndex.find(id.String);
  if (it != this->LocalGeneratorSearchIndex.end()) {
    return it->second;
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
  return cmContains(this->AliasTargets, name);
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
  auto const it = this->TargetSearchIndex.find(name);
  if (it != this->TargetSearchIndex.end()) {
    return it->second;
  }
  return nullptr;
}

cmGeneratorTarget* cmGlobalGenerator::FindGeneratorTargetImpl(
  std::string const& name) const
{
  auto const it = this->GeneratorTargetSearchIndex.find(name);
  if (it != this->GeneratorTargetSearchIndex.end()) {
    return it->second;
  }
  return nullptr;
}

cmTarget* cmGlobalGenerator::FindTarget(const std::string& name,
                                        bool excludeAliases) const
{
  if (!excludeAliases) {
    auto const ai = this->AliasTargets.find(name);
    if (ai != this->AliasTargets.end()) {
      return this->FindTargetImpl(ai->second);
    }
  }
  return this->FindTargetImpl(name);
}

cmGeneratorTarget* cmGlobalGenerator::FindGeneratorTarget(
  const std::string& name) const
{
  auto const ai = this->AliasTargets.find(name);
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
  auto& mf = this->Makefiles[0];
  std::string configFile =
    cmStrCat(mf->GetCurrentBinaryDirectory(), "/CPackConfig.cmake");
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
    if (!noPackageAll || cmIsOff(noPackageAll)) {
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

  auto& mf = this->Makefiles[0];
  std::string configFile =
    cmStrCat(mf->GetCurrentBinaryDirectory(), "/CPackSourceConfig.cmake");
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
  auto& mf = this->Makefiles[0];
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
  if (auto testArgs = mf->GetDefinition("CMAKE_CTEST_ARGUMENTS")) {
    std::vector<std::string> args;
    cmExpandList(testArgs, args);
    for (auto const& arg : args) {
      singleLine.push_back(arg);
    }
  }
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
  gti.PerConfig = false;
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
  gti.PerConfig = false;
  cmCustomCommandLine singleLine;
  singleLine.push_back(cmSystemTools::GetCMakeCommand());
  singleLine.push_back("--regenerate-during-build");
  singleLine.push_back("-S$(CMAKE_SOURCE_DIR)");
  singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
  gti.CommandLines.push_back(std::move(singleLine));
  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_Install(
  std::vector<GlobalTargetInfo>& targets)
{
  auto& mf = this->Makefiles[0];
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
      if (!noall || cmIsOff(noall)) {
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
      bool useEPN = this->UseEffectivePlatformName(mf.get());
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
    return cmIsOn(prop);
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
                  cmTarget::VisibilityNormal, mf, gti.PerConfig);
  target.SetProperty("EXCLUDE_FROM_ALL", "TRUE");

  std::vector<std::string> no_outputs;
  std::vector<std::string> no_byproducts;
  std::vector<std::string> no_depends;
  // Store the custom command in the target.
  cmCustomCommand cc(no_outputs, no_byproducts, no_depends, gti.CommandLines,
                     cmListFileBacktrace(), nullptr, gti.WorkingDir.c_str());
  cc.SetUsesTerminal(gti.UsesTerminal);
  target.AddPostBuildCommand(std::move(cc));
  if (!gti.Message.empty()) {
    target.SetProperty("EchoString", gti.Message);
  }
  for (std::string const& d : gti.Depends) {
    target.AddUtility(d, false);
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
  std::string ruleFile = cmStrCat(output, ".rule");
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
  auto const it = this->LanguageToOriginalSharedLibFlags.find(l);
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

  return cmContains(reservedTargets, name);
}

void cmGlobalGenerator::SetExternalMakefileProjectGenerator(
  std::unique_ptr<cmExternalMakefileProjectGenerator> extraGenerator)
{
  this->ExtraGenerator = std::move(extraGenerator);
  if (this->ExtraGenerator) {
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

void cmGlobalGenerator::GetTargetSets(
  TargetDependSet& projectTargets, TargetDependSet& originalTargets,
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  // loop over all local generators
  for (auto generator : generators) {
    // check to make sure generator is not excluded
    if (this->IsExcluded(root, generator)) {
      continue;
    }
    // loop over all the generator targets in the makefile
    for (const auto& target : generator->GetGeneratorTargets()) {
      if (this->IsRootOnlyTarget(target.get()) &&
          target->GetLocalGenerator() != root) {
        continue;
      }
      // put the target in the set of original targets
      originalTargets.insert(target.get());
      // Get the set of targets that depend on target
      this->AddTargetDepends(target.get(), projectTargets);
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
    for (auto const& t : this->GetTargetDirectDepends(target)) {
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
#if !defined(CMAKE_BOOTSTRAP)
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
#if !defined(CMAKE_BOOTSTRAP)
  std::string home = this->GetCMakeInstance()->GetHomeOutputDirectory();
  std::string pfile = cmStrCat(home, "/CMakeFiles/CMakeRuleHashes.txt");
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
    auto const rhi = this->RuleHashes.find(fname);
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
  std::string fname = cmStrCat(this->CMakeInstance->GetHomeOutputDirectory(),
                               "/CMakeFiles/TargetDirectories.txt");
  cmGeneratedFileStream fout(fname);

  for (const auto& lg : this->LocalGenerators) {
    for (const auto& tgt : lg->GetGeneratorTargets()) {
      if (tgt->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }
      this->WriteSummary(tgt.get());
      fout << tgt->GetSupportDirectory() << "\n";
    }
  }
}

void cmGlobalGenerator::WriteSummary(cmGeneratorTarget* target)
{
  // Place the labels file in a per-target support directory.
  std::string dir = target->GetSupportDirectory();
  std::string file = cmStrCat(dir, "/Labels.txt");
  std::string json_file = dir + "/Labels.json";

#ifndef CMAKE_BOOTSTRAP
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
      cmExpandList(targetLabels, labels);
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
      cmExpandList(directoryLabels, directoryLabelsList);
    }

    if (cmakeDirectoryLabels) {
      cmExpandList(cmakeDirectoryLabels, cmakeDirectoryLabelsList);
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
    std::vector<std::string> const& configs =
      target->Target->GetMakefile()->GetGeneratorConfigs();
    for (std::string const& c : configs) {
      target->GetSourceFiles(sources, c);
    }
    auto const sourcesEnd = cmRemoveDuplicates(sources);
    for (cmSourceFile* sf : cmMakeRange(sources.cbegin(), sourcesEnd)) {
      Json::Value& lj_source = lj_sources.append(Json::objectValue);
      std::string const& sfp = sf->ResolveFullPath();
      fout << sfp << "\n";
      lj_source["file"] = sfp;
      if (const char* svalue = sf->GetProperty("LABELS")) {
        labels.clear();
        Json::Value& lj_source_labels = lj_source["labels"] = Json::arrayValue;
        cmExpandList(svalue, labels);
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

const std::string& cmGlobalGenerator::GetRealPath(const std::string& dir)
{
  auto i = this->RealPaths.lower_bound(dir);
  if (i == this->RealPaths.end() ||
      this->RealPaths.key_comp()(dir, i->first)) {
    i = this->RealPaths.emplace_hint(i, dir, cmSystemTools::GetRealPath(dir));
  }
  return i->second;
}

void cmGlobalGenerator::ProcessEvaluationFiles()
{
  std::vector<std::string> generatedFiles;
  for (auto& localGen : this->LocalGenerators) {
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

  const auto& lg = this->LocalGenerators[0];
  cmMakefile* mf = lg->GetMakefile();

  std::vector<std::string> configs;
  std::string config = mf->GetConfigurations(configs, false);

  std::string path = cmStrCat(this->CMakeInstance->GetHomeOutputDirectory(),
                              "/CPackProperties.cmake");

  if (!cmSystemTools::FileExists(path) && installedFiles.empty()) {
    return true;
  }

  cmGeneratedFileStream file(path);
  file << "# CPack properties\n";

  for (auto const& i : installedFiles) {
    cmInstalledFile const& installedFile = i.second;

    cmCPackPropertiesGenerator cpackPropertiesGenerator(
      lg.get(), installedFile, configs);

    cpackPropertiesGenerator.Generate(file, config, configs);
  }

  return true;
}
