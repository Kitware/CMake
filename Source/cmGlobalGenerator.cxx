/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGenerator.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

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
#include "cmInstallRuntimeDependencySet.h"
#include "cmLinkLineComputer.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMSVC60LinkLineComputer.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmWorkingDirectory.h"
#include "cmake.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cm3p/json/value.h>
#  include <cm3p/json/writer.h>

#  include "cmCryptoHash.h"
#  include "cmQtAutoGenGlobalInitializer.h"
#endif

const std::string kCMAKE_PLATFORM_INFO_INITIALIZED =
  "CMAKE_PLATFORM_INFO_INITIALIZED";

class cmInstalledFile;

namespace detail {
std::string GeneratedMakeCommand::QuotedPrintable() const
{
  std::string output;
  const char* sep = "";
  int flags = 0;
#if !defined(_WIN32)
  flags |= cmOutputConverter::Shell_Flag_IsUnix;
#endif
  for (auto const& arg : this->PrimaryCommand) {
    output += cmStrCat(sep, cmOutputConverter::EscapeForShell(arg, flags));
    sep = " ";
  }
  return output;
}
}

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
    cmValue makeProgramCSTR =
      this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
    if (cmIsOff(makeProgramCSTR)) {
      makeProgram = makeDefault;
    } else {
      makeProgram = *makeProgramCSTR;
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
  cmValue cname =
    this->GetCMakeInstance()->GetState()->GetInitializedCacheValue(langComp);

  // Split compiler from arguments
  cmList cnameArgList;
  if (cname && !cname->empty()) {
    cnameArgList.assign(*cname);
    cname = cmValue(cnameArgList.front());
  }

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
      cmValue cvars = this->GetCMakeInstance()->GetState()->GetGlobalProperty(
        "__CMAKE_DELETE_CACHE_CHANGE_VARS_");
      if (cvars) {
        changeVars += *cvars;
        changeVars += ";";
      }
      changeVars += langComp;
      changeVars += ";";
      changeVars += *cname;
      this->GetCMakeInstance()->GetState()->SetGlobalProperty(
        "__CMAKE_DELETE_CACHE_CHANGE_VARS_", changeVars);
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
      if (!target->CanCompileSources() ||
          cmIsOn(target->GetProperty("ghs_integrity_app"))) {
        continue;
      }

      if (target->GetAllConfigSources().empty()) {
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

void cmGlobalGenerator::CheckTargetLinkLibraries() const
{
  for (const auto& generator : this->LocalGenerators) {
    for (const auto& gt : generator->GetGeneratorTargets()) {
      gt->CheckLinkLibraries();
    }
    for (const auto& gt : generator->GetOwnedImportedGeneratorTargets()) {
      gt->CheckLinkLibraries();
    }
  }
}

bool cmGlobalGenerator::CheckTargetsForType() const
{
  if (!this->GetLanguageEnabled("Swift")) {
    return false;
  }
  bool failed = false;
  for (const auto& generator : this->LocalGenerators) {
    for (const auto& target : generator->GetGeneratorTargets()) {
      std::string systemName =
        target->Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME");
      if (systemName.find("Windows") == std::string::npos) {
        continue;
      }

      if (target->GetType() == cmStateEnums::EXECUTABLE) {
        std::vector<std::string> const& configs =
          target->Makefile->GetGeneratorConfigs(
            cmMakefile::IncludeEmptyConfig);
        for (std::string const& config : configs) {
          if (target->IsWin32Executable(config) &&
              target->GetLinkerLanguage(config) == "Swift") {
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
      if (!target->CanCompileSources() ||
          cmIsOn(target->GetProperty("ghs_integrity_app"))) {
        continue;
      }

      std::string const& reuseFrom =
        target->GetSafeProperty("PRECOMPILE_HEADERS_REUSE_FROM");
      std::string const& compilePdb =
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
  return !cm::contains(this->BuildExportExportSets, filename);
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
  if (cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::string setMakeProgram = mf->GetModulesFile(this->FindMakeProgramFile);
    if (!setMakeProgram.empty()) {
      mf->ReadListFile(setMakeProgram);
    }
  }
  if (cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    std::ostringstream err;
    err << "CMake was unable to find a build program corresponding to \""
        << this->GetName() << "\".  CMAKE_MAKE_PROGRAM is not set.  You "
        << "probably need to select a different build tool.";
    cmSystemTools::Error(err.str());
    cmSystemTools::SetFatalErrorOccurred();
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
    mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", makeProgram, "make program",
                           cmStateEnums::FILEPATH);
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
  if (!this->IsMultiConfig() &&
      !this->GetCMakeInstance()->GetIsInTryCompile()) {
    std::string envBuildType;
    if (!mf->GetDefinition("CMAKE_BUILD_TYPE") &&
        cmSystemTools::GetEnv("CMAKE_BUILD_TYPE", envBuildType)) {
      mf->AddCacheDefinition(
        "CMAKE_BUILD_TYPE", envBuildType,
        "Choose the type of build.  Options include: empty, "
        "Debug, Release, RelWithDebInfo, MinSizeRel.",
        cmStateEnums::STRING);
    }
  }

  if (languages.empty()) {
    cmSystemTools::Error("EnableLanguage must have a lang specified!");
    cmSystemTools::SetFatalErrorOccurred();
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
      cmSystemTools::SetFatalErrorOccurred();
      return;
    }
  }

  if (this->TryCompileOuterMakefile) {
    // In a try-compile we can only enable languages provided by caller.
    for (std::string const& lang : languages) {
      if (lang == "NONE") {
        this->SetLanguageEnabled("NONE", mf);
      } else {
        if (!cm::contains(this->LanguagesReady, lang)) {
          std::ostringstream e;
          e << "The test project needs language " << lang
            << " which is not enabled.";
          this->TryCompileOuterMakefile->IssueMessage(MessageType::FATAL_ERROR,
                                                      e.str());
          cmSystemTools::SetFatalErrorOccurred();
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
    cmSystemTools::WindowsVersion windowsVersion =
      cmSystemTools::GetWindowsVersion();
    std::ostringstream windowsVersionString;
    windowsVersionString << windowsVersion.dwMajorVersion << "."
                         << windowsVersion.dwMinorVersion << "."
                         << windowsVersion.dwBuildNumber;
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
      cmSystemTools::SetFatalErrorOccurred();
      return;
    }

    // Tell the generator about the target system.
    std::string system = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (!this->SetSystemName(system, mf)) {
      cmSystemTools::SetFatalErrorOccurred();
      return;
    }

    // Tell the generator about the platform, if any.
    std::string platform = mf->GetSafeDefinition("CMAKE_GENERATOR_PLATFORM");
    if (!this->SetGeneratorPlatform(platform, mf)) {
      cmSystemTools::SetFatalErrorOccurred();
      return;
    }

    // Tell the generator about the toolset, if any.
    std::string toolset = mf->GetSafeDefinition("CMAKE_GENERATOR_TOOLSET");
    if (!this->SetGeneratorToolset(toolset, false, mf)) {
      cmSystemTools::SetFatalErrorOccurred();
      return;
    }

    // Find the native build tool for this generator.
    if (!this->FindMakeProgram(mf)) {
      return;
    }

    // One-time includes of user-provided project setup files
    mf->GetState()->SetInTopLevelIncludes(true);
    std::string includes =
      mf->GetSafeDefinition("CMAKE_PROJECT_TOP_LEVEL_INCLUDES");
    cmList includesList{ includes };
    for (std::string const& setupFile : includesList) {
      std::string absSetupFile = cmSystemTools::CollapseFullPath(
        setupFile, mf->GetCurrentSourceDirectory());
      if (!cmSystemTools::FileExists(absSetupFile)) {
        cmSystemTools::Error(
          "CMAKE_PROJECT_TOP_LEVEL_INCLUDES file does not exist: " +
          setupFile);
        mf->GetState()->SetInTopLevelIncludes(false);
        return;
      }
      if (cmSystemTools::FileIsDirectory(absSetupFile)) {
        cmSystemTools::Error(
          "CMAKE_PROJECT_TOP_LEVEL_INCLUDES file is a directory: " +
          setupFile);
        mf->GetState()->SetInTopLevelIncludes(false);
        return;
      }
      if (!mf->ReadListFile(absSetupFile)) {
        cmSystemTools::Error(
          "Failed reading CMAKE_PROJECT_TOP_LEVEL_INCLUDES file: " +
          setupFile);
        mf->GetState()->SetInTopLevelIncludes(false);
        return;
      }
    }
  }
  mf->GetState()->SetInTopLevelIncludes(false);

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
      if (cmSystemTools::GetFatalErrorOccurred()) {
        return;
      }
      needTestLanguage[lang] = true;
      // Some generators like visual studio should not use the env variables
      // So the global generator can specify that in this variable
      if ((mf->GetPolicyStatus(cmPolicies::CMP0132) == cmPolicies::OLD ||
           mf->GetPolicyStatus(cmPolicies::CMP0132) == cmPolicies::WARN) &&
          !mf->GetDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV")) {
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
    cmValue compilerFile = mf->GetDefinition(compilerName);
    if (!cmNonempty(compilerFile) || cmIsNOTFOUND(*compilerFile)) {
      /* clang-format off */
      noCompiler <<
        "No " << compilerName << " could be found.\n"
        ;
      /* clang-format on */
    } else if ((lang != "RC") && (lang != "ASM_MARMASM") &&
               (lang != "ASM_MASM")) {
      if (!cmSystemTools::FileIsFullPath(*compilerFile)) {
        /* clang-format off */
        noCompiler <<
          "The " << compilerName << ":\n"
          "  " << *compilerFile << "\n"
          "is not a full path and was not found in the PATH.\n"
          ;
        /* clang-format on */
      } else if (!cmSystemTools::FileExists(*compilerFile)) {
        /* clang-format off */
        noCompiler <<
          "The " << compilerName << ":\n"
          "  " << *compilerFile << "\n"
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
    cmSystemTools::SetFatalErrorOccurred();
  }

  for (std::string const& lang : cur_languages) {
    this->LanguagesInProgress.erase(lang);
  }
}

void cmGlobalGenerator::PrintCompilerAdvice(std::ostream& os,
                                            std::string const& lang,
                                            cmValue envVar) const
{
  // Subclasses override this method if they do not support this advice.
  os << "Tell CMake where to find the compiler by setting ";
  if (envVar) {
    os << "either the environment variable \"" << *envVar << "\" or ";
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
        break;
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
        break;
      case cmPolicies::NEW:
        // NEW behavior is to keep AppleClang.
        break;
    }
  }

  if (compilerId == "LCC") {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0129)) {
      case cmPolicies::WARN:
        if (!this->CMakeInstance->GetIsInTryCompile() &&
            mf->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0129")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0129) << "\n"
            "Converting " << lang <<
            R"( compiler id "LCC" to "GNU" for compatibility.)"
            ;
          /* clang-format on */
          mf->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to convert LCC to GNU.
        mf->AddDefinition(compilerIdVar, "GNU");
        if (lang == "C") {
          mf->AddDefinition("CMAKE_COMPILER_IS_GNUCC", "1");
        } else if (lang == "CXX") {
          mf->AddDefinition("CMAKE_COMPILER_IS_GNUCXX", "1");
        } else if (lang == "Fortran") {
          mf->AddDefinition("CMAKE_COMPILER_IS_GNUG77", "1");
        }
        {
          // Fix compiler versions.
          std::string version = "CMAKE_" + lang + "_COMPILER_VERSION";
          std::string emulated = "CMAKE_" + lang + "_SIMULATE_VERSION";
          std::string emulatedId = "CMAKE_" + lang + "_SIMULATE_ID";
          std::string const& actual = mf->GetRequiredDefinition(emulated);
          mf->AddDefinition(version, actual);
          mf->RemoveDefinition(emulatedId);
          mf->RemoveDefinition(emulated);
        }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        mf->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0129));
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior is to keep LCC.
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
  if (!ext) {
    return "";
  }
  if (*ext == '.') {
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
  if (cm::contains(this->LanguageToLinkerPreference, l)) {
    return;
  }

  std::string linkerPrefVar = "CMAKE_" + l + "_LINKER_PREFERENCE";
  cmValue linkerPref = mf->GetDefinition(linkerPrefVar);
  int preference = 0;
  if (cmNonempty(linkerPref)) {
    if (sscanf(linkerPref->c_str(), "%d", &preference) != 1) {
      // backward compatibility: before 2.6 LINKER_PREFERENCE
      // was either "None" or "Preferred", and only the first character was
      // tested. So if there is a custom language out there and it is
      // "Preferred", set its preference high
      if ((*linkerPref)[0] == 'P') {
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

  std::string outputExtensionVar = "CMAKE_" + l + "_OUTPUT_EXTENSION";
  if (cmValue p = mf->GetDefinition(outputExtensionVar)) {
    std::string outputExtension = *p;
    this->LanguageToOutputExtension[l] = outputExtension;
    this->OutputExtensions[outputExtension] = outputExtension;
    if (cmHasPrefix(outputExtension, ".")) {
      outputExtension = outputExtension.substr(1);
      this->OutputExtensions[outputExtension] = outputExtension;
    }
  }

  // The map was originally filled by SetLanguageEnabledFlag, but
  // since then the compiler- and platform-specific files have been
  // loaded which might have added more entries.
  this->FillExtensionToLanguageMap(l, mf);

  std::string ignoreExtensionsVar =
    std::string("CMAKE_") + std::string(l) + std::string("_IGNORE_EXTENSIONS");
  std::string ignoreExts = mf->GetSafeDefinition(ignoreExtensionsVar);
  cmList extensionList{ ignoreExts };
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
  cmList extensionList{ exts };
  for (std::string const& i : extensionList) {
    this->ExtensionToLanguage[i] = l;
  }
}

cmValue cmGlobalGenerator::GetGlobalSetting(std::string const& name) const
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
  return this->Makefiles[0]->GetDefinition(name);
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
  auto startTime = std::chrono::steady_clock::now();

  this->FirstTimeProgress = 0.0f;
  this->ClearGeneratorMembers();
  this->NextDeferId = 0;

  cmStateSnapshot snapshot = this->CMakeInstance->GetCurrentSnapshot();

  snapshot.GetDirectory().SetCurrentSource(
    this->CMakeInstance->GetHomeDirectory());
  snapshot.GetDirectory().SetCurrentBinary(
    this->CMakeInstance->GetHomeOutputDirectory());

  auto dirMfu = cm::make_unique<cmMakefile>(this, snapshot);
  auto* dirMf = dirMfu.get();
  this->Makefiles.push_back(std::move(dirMfu));
  dirMf->SetRecursionDepth(this->RecursionDepth);
  this->IndexMakefile(dirMf);

  this->BinaryDirectories.insert(
    this->CMakeInstance->GetHomeOutputDirectory());

  if (this->ExtraGenerator && !this->CMakeInstance->GetIsInTryCompile()) {
    this->CMakeInstance->IssueMessage(
      MessageType::DEPRECATION_WARNING,
      cmStrCat("Support for \"Extra Generators\" like\n  ",
               this->ExtraGenerator->GetName(),
               "\nis deprecated and will be removed from a future version "
               "of CMake.  IDEs may use the cmake-file-api(7) to view "
               "CMake-generated project build trees."));
  }

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
      for (GlobalTargetInfo const& globalTarget : globalTargets) {
        this->CreateGlobalTarget(globalTarget, mf.get());
      }
    }
  }

  // update the cache entry for the number of local generators, this is used
  // for progress
  this->GetCMakeInstance()->AddCacheEntry(
    "CMAKE_NUMBER_OF_MAKEFILES", std::to_string(this->Makefiles.size()),
    "number of local generators", cmStateEnums::INTERNAL);

  auto endTime = std::chrono::steady_clock::now();

  if (this->CMakeInstance->GetWorkingMode() == cmake::NORMAL_MODE) {
    std::ostringstream msg;
    if (cmSystemTools::GetErrorOccurredFlag()) {
      msg << "Configuring incomplete, errors occurred!";
    } else {
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
      msg << "Configuring done (" << std::fixed << std::setprecision(1)
          << ms.count() / 1000.0L << "s)";
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
  if (targetTypes == TargetTypes::AllTargets) {
    this->ComputeBuildFileGenerators();
  }
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
  if (!this->InspectConfigTypeVariables()) {
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
  this->FinalizeTargetConfiguration();

  this->CreateGenerationObjects();

  // at this point this->LocalGenerators has been filled,
  // so create the map from project name to vector of local generators
  this->FillProjectMap();

  // Add automatically generated sources (e.g. unity build).
  if (!this->AddAutomaticSources()) {
    return false;
  }

  // Iterate through all targets and add verification targets for header sets
  if (!this->AddHeaderSetVerification()) {
    return false;
  }

#ifndef CMAKE_BOOTSTRAP
  this->QtAutoGen =
    cm::make_unique<cmQtAutoGenGlobalInitializer>(this->LocalGenerators);
  if (!this->QtAutoGen->InitializeCustomTargets()) {
    return false;
  }
#endif

  // Add generator specific helper commands
  for (const auto& localGen : this->LocalGenerators) {
    localGen->AddHelperCommands();
  }

  // Perform up-front computation in order to handle errors (such as unknown
  // features) at this point. While processing the compile features we also
  // calculate and cache the language standard required by the compile
  // features.
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
  this->ComputeTargetOrder();

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
  auto startTime = std::chrono::steady_clock::now();

  // Create a map from local generator to the complete set of targets
  // it builds by default.
  this->InitializeProgressMarks();

  this->ProcessEvaluationFiles();

  this->CMakeInstance->UpdateProgress("Generating", 0.1f);

#ifndef CMAKE_BOOTSTRAP
  if (!this->QtAutoGen->SetupCustomTargets()) {
    if (!cmSystemTools::GetErrorOccurredFlag()) {
      this->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR,
        "Problem setting up custom targets for QtAutoGen");
    }
    return;
  }
#endif

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
      if (!cmSystemTools::GetErrorOccurredFlag()) {
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

  // Perform validation checks on memoized link structures.
  this->CheckTargetLinkLibraries();

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

  auto endTime = std::chrono::steady_clock::now();
  auto ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
  std::ostringstream msg;
  msg << "Generating done (" << std::fixed << std::setprecision(1)
      << ms.count() / 1000.0L << "s)";
  this->CMakeInstance->UpdateProgress(msg.str(), -1);
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

std::vector<cmGeneratorTarget*>
cmGlobalGenerator::GetLocalGeneratorTargetsInOrder(cmLocalGenerator* lg) const
{
  std::vector<cmGeneratorTarget*> gts;
  cm::append(gts, lg->GetGeneratorTargets());
  std::sort(gts.begin(), gts.end(),
            [this](cmGeneratorTarget const* l, cmGeneratorTarget const* r) {
              return this->TargetOrderIndexLess(l, r);
            });
  return gts;
}

void cmGlobalGenerator::ComputeTargetOrder()
{
  size_t index = 0;
  auto const& lgens = this->GetLocalGenerators();
  for (auto const& lgen : lgens) {
    const auto& targets = lgen->GetGeneratorTargets();
    for (const auto& gt : targets) {
      this->ComputeTargetOrder(gt.get(), index);
    }
  }
  assert(index == this->TargetOrderIndex.size());
}

void cmGlobalGenerator::ComputeTargetOrder(cmGeneratorTarget const* gt,
                                           size_t& index)
{
  std::map<cmGeneratorTarget const*, size_t>::value_type value(gt, 0);
  auto insertion = this->TargetOrderIndex.insert(value);
  if (!insertion.second) {
    return;
  }
  auto entry = insertion.first;

  const auto& deps = this->GetTargetDirectDepends(gt);
  for (const auto& d : deps) {
    this->ComputeTargetOrder(d, index);
  }

  entry->second = index++;
}

bool cmGlobalGenerator::AddHeaderSetVerification()
{
  for (auto const& gen : this->LocalGenerators) {
    // Because AddHeaderSetVerification() adds generator targets, we need to
    // cache the existing list of generator targets before starting.
    std::vector<cmGeneratorTarget*> genTargets;
    genTargets.reserve(gen->GetGeneratorTargets().size());
    for (auto const& tgt : gen->GetGeneratorTargets()) {
      genTargets.push_back(tgt.get());
    }

    for (auto* tgt : genTargets) {
      if (!tgt->AddHeaderSetVerification()) {
        return false;
      }
    }
  }

  cmTarget* allVerifyTarget = this->Makefiles.front()->FindTargetToUse(
    "all_verify_interface_header_sets", true);
  if (allVerifyTarget) {
    this->LocalGenerators.front()->AddGeneratorTarget(
      cm::make_unique<cmGeneratorTarget>(allVerifyTarget,
                                         this->LocalGenerators.front().get()));
  }

  return true;
}

bool cmGlobalGenerator::AddAutomaticSources()
{
  for (const auto& lg : this->LocalGenerators) {
    lg->CreateEvaluationFileOutputs();
  }
  for (const auto& lg : this->LocalGenerators) {
    for (const auto& gt : lg->GetGeneratorTargets()) {
      if (!gt->CanCompileSources()) {
        continue;
      }
      lg->AddUnityBuild(gt.get());
      lg->AddISPCDependencies(gt.get());
      // Targets that re-use a PCH are handled below.
      if (!gt->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM")) {
        lg->AddPchDependencies(gt.get());
      }
      lg->AddXCConfigSources(gt.get());
    }
  }
  for (const auto& lg : this->LocalGenerators) {
    for (const auto& gt : lg->GetGeneratorTargets()) {
      if (!gt->CanCompileSources()) {
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

void cmGlobalGenerator::FinalizeTargetConfiguration()
{
  std::vector<std::string> const langs =
    this->CMakeInstance->GetState()->GetEnabledLanguages();

  // Construct per-target generator information.
  for (const auto& mf : this->Makefiles) {
    const cmBTStringRange noConfigCompileDefinitions =
      mf->GetCompileDefinitionsEntries();
    cm::optional<std::map<std::string, cmValue>> perConfigCompileDefinitions;

    for (auto& target : mf->GetTargets()) {
      cmTarget* t = &target.second;
      t->FinalizeTargetConfiguration(noConfigCompileDefinitions,
                                     perConfigCompileDefinitions);
    }

    // The standard include directories for each language
    // should be treated as system include directories.
    std::set<std::string> standardIncludesSet;
    for (std::string const& li : langs) {
      std::string const standardIncludesVar =
        "CMAKE_" + li + "_STANDARD_INCLUDE_DIRECTORIES";
      std::string const& standardIncludesStr =
        mf->GetSafeDefinition(standardIncludesVar);
      cmList standardIncludesList{ standardIncludesStr };
      standardIncludesSet.insert(standardIncludesList.begin(),
                                 standardIncludesList.end());
    }
    mf->AddSystemIncludeDirectories(standardIncludesSet);
  }
}

void cmGlobalGenerator::CreateGeneratorTargets(
  TargetTypes targetTypes, cmMakefile* mf, cmLocalGenerator* lg,
  std::map<cmTarget*, cmGeneratorTarget*> const& importedMap)
{
  if (targetTypes == AllTargets) {
    for (cmTarget* target : mf->GetOrderedTargets()) {
      lg->AddGeneratorTarget(cm::make_unique<cmGeneratorTarget>(target, lg));
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
  this->InstallComponents.clear();
  this->TargetDependencies.clear();
  this->TargetSearchIndex.clear();
  this->GeneratorTargetSearchIndex.clear();
  this->MakefileSearchIndex.clear();
  this->LocalGeneratorSearchIndex.clear();
  this->TargetOrderIndex.clear();
  this->ProjectMap.clear();
  this->RuleHashes.clear();
  this->DirectoryContentMap.clear();
  this->BinaryDirectories.clear();
  this->GeneratedFiles.clear();
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
      cmValue incDirProp = target.second.GetProperty("INCLUDE_DIRECTORIES");
      if (!incDirProp) {
        continue;
      }

      std::string incDirs = cmGeneratorExpression::Preprocess(
        *incDirProp, cmGeneratorExpression::StripAllGeneratorExpressions);

      cmList incs(incDirs);

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
  cmBuildOptions defaultBuildOptions(false, fast, PackageResolveMode::Disable);

  std::stringstream ostr;
  auto ret =
    this->Build(jobs, srcdir, bindir, projectName, newTarget, ostr, "", config,
                defaultBuildOptions, true, this->TryCompileTimeout);
  output = ostr.str();
  return ret;
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalGenerator::GenerateBuildCommand(
  const std::string& /*unused*/, const std::string& /*unused*/,
  const std::string& /*unused*/, std::vector<std::string> const& /*unused*/,
  const std::string& /*unused*/, int /*unused*/, bool /*unused*/,
  const cmBuildOptions& /*unused*/, std::vector<std::string> const& /*unused*/)
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
  std::ostream& ostr, const std::string& makeCommandCSTR,
  const std::string& config, const cmBuildOptions& buildOptions, bool verbose,
  cmDuration timeout, cmSystemTools::OutputOption outputflag,
  std::vector<std::string> const& nativeOptions)
{
  bool hideconsole = cmSystemTools::GetRunCommandHideConsole();

  /**
   * Run an executable command and put the stdout in output.
   */
  cmWorkingDirectory workdir(bindir);
  ostr << "Change Dir: '" << bindir << '\'' << std::endl;
  if (workdir.Failed()) {
    cmSystemTools::SetRunCommandHideConsole(hideconsole);
    std::string err = cmStrCat("Failed to change directory: ",
                               std::strerror(workdir.GetLastResult()));
    cmSystemTools::Error(err);
    ostr << err << std::endl;
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

  std::vector<GeneratedMakeCommand> makeCommand = this->GenerateBuildCommand(
    makeCommandCSTR, projectName, bindir, targets, realConfig, jobs, verbose,
    buildOptions, nativeOptions);

  // Workaround to convince some commands to produce output.
  if (outputflag == cmSystemTools::OUTPUT_PASSTHROUGH &&
      makeCommand.back().RequiresOutputForward) {
    outputflag = cmSystemTools::OUTPUT_FORWARD;
  }

  // should we do a clean first?
  if (buildOptions.Clean) {
    std::vector<GeneratedMakeCommand> cleanCommand =
      this->GenerateBuildCommand(makeCommandCSTR, projectName, bindir,
                                 { "clean" }, realConfig, jobs, verbose,
                                 buildOptions);
    ostr << "\nRun Clean Command: " << cleanCommand.front().QuotedPrintable()
         << std::endl;
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
      ostr << *outputPtr << "\nGenerator: execution of make clean failed."
           << std::endl;

      return 1;
    }
    ostr << *outputPtr;
  }

  // now build
  std::string makeCommandStr;
  std::string outputMakeCommandStr;
  bool isWatcomWMake = this->CMakeInstance->GetState()->UseWatcomWMake();
  bool needBuildOutput = isWatcomWMake;
  std::string buildOutput;
  ostr << "\nRun Build Command(s): ";

  retVal = 0;
  for (auto command = makeCommand.begin();
       command != makeCommand.end() && retVal == 0; ++command) {
    makeCommandStr = command->Printable();
    outputMakeCommandStr = command->QuotedPrintable();
    if ((command + 1) != makeCommand.end()) {
      makeCommandStr += " && ";
      outputMakeCommandStr += " && ";
    }

    ostr << outputMakeCommandStr << std::endl;
    if (!cmSystemTools::RunSingleCommand(command->PrimaryCommand, outputPtr,
                                         outputPtr, &retVal, nullptr,
                                         outputflag, timeout)) {
      cmSystemTools::SetRunCommandHideConsole(hideconsole);
      cmSystemTools::Error(
        "Generator: execution of make failed. Make command was: " +
        makeCommandStr);
      ostr << *outputPtr
           << "\nGenerator: execution of make failed. Make command was: "
           << outputMakeCommandStr << std::endl;

      return 1;
    }
    ostr << *outputPtr << std::flush;
    if (needBuildOutput) {
      buildOutput += *outputPtr;
    }
  }
  ostr << std::endl;
  cmSystemTools::SetRunCommandHideConsole(hideconsole);

  // The OpenWatcom tools do not return an error code when a link
  // library is not found!
  if (isWatcomWMake && retVal == 0 &&
      buildOutput.find("W1008: cannot open") != std::string::npos) {
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
  const std::string& parallel, const std::string& native, bool ignoreErrors)
{
  std::string makeCommand = cmSystemTools::GetCMakeCommand();
  makeCommand =
    cmStrCat(cmSystemTools::ConvertToOutputPath(makeCommand), " --build .");
  if (!config.empty()) {
    makeCommand += " --config \"";
    makeCommand += config;
    makeCommand += "\"";
  }
  if (!parallel.empty()) {
    makeCommand += " --parallel \"";
    makeCommand += parallel;
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
  cmValue numGenC = this->CMakeInstance->GetState()->GetInitializedCacheValue(
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

void cmGlobalGenerator::MarkAsGeneratedFile(const std::string& filepath)
{
  this->GeneratedFiles.insert(filepath);
}

bool cmGlobalGenerator::IsGeneratedFile(const std::string& filepath)
{
  return this->GeneratedFiles.find(filepath) != this->GeneratedFiles.end();
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
  cmValue make =
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
                                   const cmGeneratorTarget* target) const
{
  if (!target->IsInBuildSystem()) {
    return true;
  }
  cmMakefile* mf = root->GetMakefile();
  const std::string EXCLUDE_FROM_ALL = "EXCLUDE_FROM_ALL";
  if (cmValue exclude = target->GetProperty(EXCLUDE_FROM_ALL)) {
    // Expand the property value per configuration.
    unsigned int trueCount = 0;
    unsigned int falseCount = 0;
    const std::vector<std::string>& configs =
      mf->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
    for (const std::string& config : configs) {
      cmGeneratorExpressionInterpreter genexInterpreter(root, config, target);
      if (cmIsOn(genexInterpreter.Evaluate(*exclude, EXCLUDE_FROM_ALL))) {
        ++trueCount;
      } else {
        ++falseCount;
      }
    }

    // Check whether the genex expansion of the property agrees in all
    // configurations.
    if (trueCount > 0 && falseCount > 0) {
      std::ostringstream e;
      e << "The EXCLUDE_FROM_ALL property of target \"" << target->GetName()
        << "\" varies by configuration. This is not supported by the \""
        << root->GetGlobalGenerator()->GetName() << "\" generator.";
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    }
    return trueCount;
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
  return cm::contains(this->AliasTargets, name);
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
  constexpr size_t sizeof_ptr =
    sizeof(gt); // NOLINT(bugprone-sizeof-expression)
  char buf[1 + sizeof_ptr * 2];
  char* b = buf;
  *b++ = ':';
  for (size_t i = 0; i < sizeof_ptr; ++i) {
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
  // We index by both source and binary directory.  add_subdirectory
  // supports multiple build directories sharing the same source directory.
  // The source directory index will reference only the first time it is used.
  this->MakefileSearchIndex.insert(
    MakefileMap::value_type(mf->GetCurrentSourceDirectory(), mf));
  this->MakefileSearchIndex.insert(
    MakefileMap::value_type(mf->GetCurrentBinaryDirectory(), mf));
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

// If the file has no extension it's either a raw executable or might
// be a direct reference to a binary within a framework (bad practice!).
// This is where we change the path to point to the framework directory.
// .tbd files also can be located in SDK frameworks (they are
// placeholders for actual libraries shipped with the OS)
cm::optional<cmGlobalGenerator::FrameworkDescriptor>
cmGlobalGenerator::SplitFrameworkPath(const std::string& path,
                                      FrameworkFormat format) const
{
  // Check for framework structure:
  //    (/path/to/)?FwName.framework
  // or (/path/to/)?FwName.framework/FwName(.tbd)?
  // or (/path/to/)?FwName.framework/Versions/*/FwName(.tbd)?
  static cmsys::RegularExpression frameworkPath(
    "((.+)/)?([^/]+)\\.framework(/Versions/([^/]+))?(/(.+))?$");

  auto ext = cmSystemTools::GetFilenameLastExtension(path);
  if ((ext.empty() || ext == ".tbd" || ext == ".framework") &&
      frameworkPath.find(path)) {
    auto name = frameworkPath.match(3);
    auto libname =
      cmSystemTools::GetFilenameWithoutExtension(frameworkPath.match(7));
    if (format == FrameworkFormat::Strict && libname.empty()) {
      return cm::nullopt;
    }
    if (!libname.empty() && !cmHasPrefix(libname, name)) {
      return cm::nullopt;
    }

    if (libname.empty() || name.size() == libname.size()) {
      return FrameworkDescriptor{ frameworkPath.match(2),
                                  frameworkPath.match(5), name };
    }

    return FrameworkDescriptor{ frameworkPath.match(2), frameworkPath.match(5),
                                name, libname.substr(name.size()) };
  }

  if (format == FrameworkFormat::Extended) {
    // path format can be more flexible: (/path/to/)?fwName(.framework)?
    auto fwDir = cmSystemTools::GetParentDirectory(path);
    auto name = cmSystemTools::GetFilenameLastExtension(path) == ".framework"
      ? cmSystemTools::GetFilenameWithoutExtension(path)
      : cmSystemTools::GetFilenameName(path);

    return FrameworkDescriptor{ fwDir, name };
  }

  return cm::nullopt;
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
  if (cmNonempty(cmakeCfgIntDir) && cmakeCfgIntDir[0] != '.') {
    singleLine.push_back("-C");
    singleLine.push_back(cmakeCfgIntDir);
  }
  singleLine.push_back("--config");
  singleLine.push_back("./CPackConfig.cmake");
  gti.CommandLines.push_back(std::move(singleLine));
  if (this->GetPreinstallTargetName()) {
    gti.Depends.emplace_back(this->GetPreinstallTargetName());
  } else {
    cmValue noPackageAll =
      mf->GetDefinition("CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY");
    if (cmIsOff(noPackageAll)) {
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
  cmList args(mf->GetDefinition("CMAKE_CTEST_ARGUMENTS"));
  for (auto const& arg : args) {
    singleLine.push_back(arg);
  }
  if (cmNonempty(cmakeCfgIntDir) && cmakeCfgIntDir[0] != '.') {
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
  std::vector<GlobalTargetInfo>& targets) const
{
  const char* editCacheTargetName = this->GetEditCacheTargetName();
  if (!editCacheTargetName) {
    return;
  }
  GlobalTargetInfo gti;
  gti.Name = editCacheTargetName;
  gti.PerConfig = cmTarget::PerConfig::No;
  cmCustomCommandLine singleLine;

  // Use generator preference for the edit_cache rule if it is defined.
  std::string edit_cmd = this->GetEditCacheCommand();
  if (!edit_cmd.empty()) {
    singleLine.push_back(std::move(edit_cmd));
    if (this->GetCMakeInstance()->GetIgnoreWarningAsError()) {
      singleLine.push_back("--compile-no-warning-as-error");
    }
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
    gti.StdPipesUTF8 = true;
  }
  gti.CommandLines.push_back(std::move(singleLine));

  targets.push_back(std::move(gti));
}

void cmGlobalGenerator::AddGlobalTarget_RebuildCache(
  std::vector<GlobalTargetInfo>& targets) const
{
  const char* rebuildCacheTargetName = this->GetRebuildCacheTargetName();
  if (!rebuildCacheTargetName) {
    return;
  }
  GlobalTargetInfo gti;
  gti.Name = rebuildCacheTargetName;
  gti.Message = "Running CMake to regenerate build system...";
  gti.UsesTerminal = true;
  gti.PerConfig = cmTarget::PerConfig::No;
  cmCustomCommandLine singleLine;
  singleLine.push_back(cmSystemTools::GetCMakeCommand());
  singleLine.push_back("--regenerate-during-build");
  if (this->GetCMakeInstance()->GetIgnoreWarningAsError()) {
    singleLine.push_back("--compile-no-warning-as-error");
  }
  singleLine.push_back("-S$(CMAKE_SOURCE_DIR)");
  singleLine.push_back("-B$(CMAKE_BINARY_DIR)");
  gti.CommandLines.push_back(std::move(singleLine));
  gti.StdPipesUTF8 = true;
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
    if (!(cmNonempty(cmakeCfgIntDir) && cmakeCfgIntDir[0] != '.')) {
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
    gti.StdPipesUTF8 = true;
    cmCustomCommandLine singleLine;
    if (this->GetPreinstallTargetName()) {
      gti.Depends.emplace_back(this->GetPreinstallTargetName());
    } else {
      cmValue noall = mf->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
      if (cmIsOff(noall)) {
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
    if (cmNonempty(cmakeCfgIntDir) && cmakeCfgIntDir[0] != '.') {
      std::string cfgArg = "-DBUILD_TYPE=";
      bool useEPN = this->UseEffectivePlatformName(mf.get());
      if (useEPN) {
        cfgArg += "$(CONFIGURATION)";
        singleLine.push_back(cfgArg);
        cfgArg = "-DEFFECTIVE_PLATFORM_NAME=$(EFFECTIVE_PLATFORM_NAME)";
      } else {
        cfgArg += this->GetCMakeCFGIntDir();
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

std::string cmGlobalGenerator::GetPredefinedTargetsFolder() const
{
  cmValue prop = this->GetCMakeInstance()->GetState()->GetGlobalProperty(
    "PREDEFINED_TARGETS_FOLDER");

  if (prop) {
    return *prop;
  }

  return "CMakePredefinedTargets";
}

bool cmGlobalGenerator::UseFolderProperty() const
{
  const cmValue prop =
    this->GetCMakeInstance()->GetState()->GetGlobalProperty("USE_FOLDERS");

  // If this property is defined, let the setter turn this on or off.
  if (prop) {
    return cmIsOn(*prop);
  }

  // If CMP0143 is NEW `treat` "USE_FOLDERS" as ON. Otherwise `treat` it as OFF
  assert(!this->Makefiles.empty());
  return (this->Makefiles[0]->GetPolicyStatus(cmPolicies::CMP0143) ==
          cmPolicies::NEW);
}

void cmGlobalGenerator::CreateGlobalTarget(GlobalTargetInfo const& gti,
                                           cmMakefile* mf)
{
  // Package
  auto tb =
    mf->CreateNewTarget(gti.Name, cmStateEnums::GLOBAL_TARGET, gti.PerConfig);

  // Do nothing if gti.Name is already used
  if (!tb.second) {
    return;
  }

  cmTarget& target = tb.first;
  target.SetProperty("EXCLUDE_FROM_ALL", "TRUE");

  // Store the custom command in the target.
  cmCustomCommand cc;
  cc.SetCommandLines(gti.CommandLines);
  cc.SetWorkingDirectory(gti.WorkingDir.c_str());
  cc.SetStdPipesUTF8(gti.StdPipesUTF8);
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

bool cmGlobalGenerator::TargetOrderIndexLess(cmGeneratorTarget const* l,
                                             cmGeneratorTarget const* r) const
{
  return this->TargetOrderIndex.at(l) < this->TargetOrderIndex.at(r);
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

  return cm::contains(reservedTargets, name);
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
  for (auto* generator : generators) {
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
  std::string fname =
    this->LocalGenerators[0]->MaybeRelativeToTopBinDir(outputs[0]);

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
      if (!tgt->IsInBuildSystem()) {
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
  cmValue targetLabels = target->GetProperty("LABELS");
  cmValue directoryLabels =
    target->Target->GetMakefile()->GetProperty("LABELS");
  cmValue cmakeDirectoryLabels =
    target->Target->GetMakefile()->GetDefinition("CMAKE_DIRECTORY_LABELS");
  if (targetLabels || directoryLabels || cmakeDirectoryLabels) {
    Json::Value lj_root(Json::objectValue);
    Json::Value& lj_target = lj_root["target"] = Json::objectValue;
    lj_target["name"] = target->GetName();
    Json::Value& lj_target_labels = lj_target["labels"] = Json::arrayValue;
    Json::Value& lj_sources = lj_root["sources"] = Json::arrayValue;

    cmSystemTools::MakeDirectory(dir);
    cmGeneratedFileStream fout(file);

    cmList labels;

    // List the target-wide labels.  All sources in the target get
    // these labels.
    if (targetLabels) {
      labels.assign(*targetLabels);
      if (!labels.empty()) {
        fout << "# Target labels\n";
        for (std::string const& l : labels) {
          fout << " " << l << "\n";
          lj_target_labels.append(l);
        }
      }
    }

    // List directory labels
    cmList directoryLabelsList;
    cmList cmakeDirectoryLabelsList;

    if (directoryLabels) {
      directoryLabelsList.assign(*directoryLabels);
    }

    if (cmakeDirectoryLabels) {
      cmakeDirectoryLabelsList.assign(*cmakeDirectoryLabels);
    }

    if (!directoryLabelsList.empty() || !cmakeDirectoryLabelsList.empty()) {
      fout << "# Directory labels\n";
    }

    for (auto const& li : directoryLabelsList) {
      fout << " " << li << "\n";
      lj_target_labels.append(li);
    }

    for (auto const& li : cmakeDirectoryLabelsList) {
      fout << " " << li << "\n";
      lj_target_labels.append(li);
    }

    // List the source files with any per-source labels.
    fout << "# Source files and their labels\n";
    std::vector<cmSourceFile*> sources;
    std::vector<std::string> const& configs =
      target->Target->GetMakefile()->GetGeneratorConfigs(
        cmMakefile::IncludeEmptyConfig);
    for (std::string const& c : configs) {
      target->GetSourceFiles(sources, c);
    }
    auto const sourcesEnd = cmRemoveDuplicates(sources);
    for (cmSourceFile* sf : cmMakeRange(sources.cbegin(), sourcesEnd)) {
      Json::Value& lj_source = lj_sources.append(Json::objectValue);
      std::string const& sfp = sf->ResolveFullPath();
      fout << sfp << "\n";
      lj_source["file"] = sfp;
      if (cmValue svalue = sf->GetProperty("LABELS")) {
        Json::Value& lj_source_labels = lj_source["labels"] = Json::arrayValue;
        labels.assign(*svalue);
        for (auto const& label : labels) {
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

std::string cmGlobalGenerator::NewDeferId()
{
  return cmStrCat("__"_s, std::to_string(this->NextDeferId++));
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

  std::vector<std::string> configs =
    mf->GetGeneratorConfigs(cmMakefile::OnlyMultiConfig);
  std::string config = mf->GetDefaultConfiguration();

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

cmInstallRuntimeDependencySet*
cmGlobalGenerator::CreateAnonymousRuntimeDependencySet()
{
  auto set = cm::make_unique<cmInstallRuntimeDependencySet>();
  auto* retval = set.get();
  this->RuntimeDependencySets.push_back(std::move(set));
  return retval;
}

cmInstallRuntimeDependencySet* cmGlobalGenerator::GetNamedRuntimeDependencySet(
  const std::string& name)
{
  auto it = this->RuntimeDependencySetsByName.find(name);
  if (it == this->RuntimeDependencySetsByName.end()) {
    auto set = cm::make_unique<cmInstallRuntimeDependencySet>(name);
    it =
      this->RuntimeDependencySetsByName.insert(std::make_pair(name, set.get()))
        .first;
    this->RuntimeDependencySets.push_back(std::move(set));
  }
  return it->second;
}

cmGlobalGenerator::StripCommandStyle cmGlobalGenerator::GetStripCommandStyle(
  std::string const& strip)
{
#ifdef __APPLE__
  auto i = this->StripCommandStyleMap.find(strip);
  if (i == this->StripCommandStyleMap.end()) {
    StripCommandStyle style = StripCommandStyle::Default;

    // Try running strip tool with Apple-specific options.
    std::vector<std::string> cmd{ strip, "-u", "-r" };
    std::string out;
    std::string err;
    int ret;
    if (cmSystemTools::RunSingleCommand(cmd, &out, &err, &ret, nullptr,
                                        cmSystemTools::OUTPUT_NONE) &&
        // Check for Apple-specific output.
        ret != 0 && cmHasLiteralPrefix(err, "fatal error: /") &&
        err.find("/usr/bin/strip: no files specified") != std::string::npos) {
      style = StripCommandStyle::Apple;
    }
    i = this->StripCommandStyleMap.emplace(strip, style).first;
  }
  return i->second;
#else
  static_cast<void>(strip);
  return StripCommandStyle::Default;
#endif
}
