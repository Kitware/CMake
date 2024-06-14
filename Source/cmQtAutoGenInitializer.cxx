/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenInitializer.h"

#include <array>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <limits>
#include <map>
#include <set>
#include <sstream> // for basic_ios, istringstream
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/algorithm>
#include <cm/iterator>
#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/SystemInformation.hxx"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmEvaluatedTargetProperty.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenGlobalInitializer.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocationKind.h"
#include "cmSourceGroup.h"
#include "cmStandardLevelResolver.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

unsigned int GetParallelCPUCount()
{
  static unsigned int count = 0;
  // Detect only on the first call
  if (count == 0) {
    cmsys::SystemInformation info;
    info.RunCPUCheck();
    count =
      cm::clamp(info.GetNumberOfPhysicalCPU(), 1u, cmQtAutoGen::ParallelMax);
  }
  return count;
}

std::string FileProjectRelativePath(cmMakefile const* makefile,
                                    std::string const& fileName)
{
  std::string res;
  {
    std::string pSource = cmSystemTools::RelativePath(
      makefile->GetCurrentSourceDirectory(), fileName);
    std::string pBinary = cmSystemTools::RelativePath(
      makefile->GetCurrentBinaryDirectory(), fileName);
    if (pSource.size() < pBinary.size()) {
      res = std::move(pSource);
    } else if (pBinary.size() < fileName.size()) {
      res = std::move(pBinary);
    } else {
      res = fileName;
    }
  }
  return res;
}

/**
 * Tests if targetDepend is a STATIC_LIBRARY and if any of its
 * recursive STATIC_LIBRARY dependencies depends on targetOrigin
 * (STATIC_LIBRARY cycle).
 */
bool StaticLibraryCycle(cmGeneratorTarget const* targetOrigin,
                        cmGeneratorTarget const* targetDepend,
                        std::string const& config)
{
  bool cycle = false;
  if ((targetOrigin->GetType() == cmStateEnums::STATIC_LIBRARY) &&
      (targetDepend->GetType() == cmStateEnums::STATIC_LIBRARY)) {
    std::set<cmGeneratorTarget const*> knownLibs;
    std::deque<cmGeneratorTarget const*> testLibs;

    // Insert initial static_library dependency
    knownLibs.insert(targetDepend);
    testLibs.push_back(targetDepend);

    while (!testLibs.empty()) {
      cmGeneratorTarget const* testTarget = testLibs.front();
      testLibs.pop_front();
      // Check if the test target is the origin target (cycle)
      if (testTarget == targetOrigin) {
        cycle = true;
        break;
      }
      // Collect all static_library dependencies from the test target
      cmLinkImplementationLibraries const* libs =
        testTarget->GetLinkImplementationLibraries(
          config, cmGeneratorTarget::LinkInterfaceFor::Link);
      if (libs) {
        for (cmLinkItem const& item : libs->Libraries) {
          cmGeneratorTarget const* depTarget = item.Target;
          if (depTarget &&
              (depTarget->GetType() == cmStateEnums::STATIC_LIBRARY) &&
              knownLibs.insert(depTarget).second) {
            testLibs.push_back(depTarget);
          }
        }
      }
    }
  }
  return cycle;
}

/** Sanitizes file search paths.  */
class SearchPathSanitizer
{
public:
  SearchPathSanitizer(cmMakefile* makefile)
    : SourcePath_(makefile->GetCurrentSourceDirectory())
  {
  }
  std::vector<std::string> operator()(
    std::vector<std::string> const& paths) const;

private:
  std::string SourcePath_;
};

std::vector<std::string> SearchPathSanitizer::operator()(
  std::vector<std::string> const& paths) const
{
  std::vector<std::string> res;
  res.reserve(paths.size());
  for (std::string const& srcPath : paths) {
    // Collapse relative paths
    std::string path =
      cmSystemTools::CollapseFullPath(srcPath, this->SourcePath_);
    // Remove suffix slashes
    while (cmHasSuffix(path, '/')) {
      path.pop_back();
    }
    // Accept only non empty paths
    if (!path.empty()) {
      res.emplace_back(std::move(path));
    }
  }
  return res;
}

/** @brief Writes a CMake info file.  */
class InfoWriter
{
public:
  // -- Single value
  void Set(std::string const& key, std::string const& value)
  {
    this->Value_[key] = value;
  }
  void SetConfig(std::string const& key,
                 cmQtAutoGenInitializer::ConfigString const& cfgStr);
  void SetBool(std::string const& key, bool value)
  {
    this->Value_[key] = value;
  }
  void SetUInt(std::string const& key, unsigned int value)
  {
    this->Value_[key] = value;
  }

  // -- Array utility
  template <typename CONT>
  static bool MakeArray(Json::Value& jval, CONT const& container);

  template <typename CONT>
  static void MakeStringArray(Json::Value& jval, CONT const& container);

  // -- Array value
  template <typename CONT>
  void SetArray(std::string const& key, CONT const& container);
  template <typename CONT>
  void SetConfigArray(
    std::string const& key,
    cmQtAutoGenInitializer::ConfigStrings<CONT> const& cfgStr);

  // -- Array of arrays
  template <typename CONT, typename FUNC>
  void SetArrayArray(std::string const& key, CONT const& container, FUNC func);

  // -- Save to json file
  bool Save(std::string const& filename);

private:
  Json::Value Value_;
};

void InfoWriter::SetConfig(std::string const& key,
                           cmQtAutoGenInitializer::ConfigString const& cfgStr)
{
  this->Set(key, cfgStr.Default);
  for (auto const& item : cfgStr.Config) {
    this->Set(cmStrCat(key, '_', item.first), item.second);
  }
}

template <typename CONT>
bool InfoWriter::MakeArray(Json::Value& jval, CONT const& container)
{
  jval = Json::arrayValue;
  std::size_t const listSize = cm::size(container);
  if (listSize == 0) {
    return false;
  }
  jval.resize(static_cast<unsigned int>(listSize));
  return true;
}

template <typename CONT>
void InfoWriter::MakeStringArray(Json::Value& jval, CONT const& container)
{
  if (MakeArray(jval, container)) {
    Json::ArrayIndex ii = 0;
    for (std::string const& item : container) {
      jval[ii++] = item;
    }
  }
}

template <typename CONT>
void InfoWriter::SetArray(std::string const& key, CONT const& container)
{
  MakeStringArray(this->Value_[key], container);
}

template <typename CONT, typename FUNC>
void InfoWriter::SetArrayArray(std::string const& key, CONT const& container,
                               FUNC func)
{
  Json::Value& jval = this->Value_[key];
  if (MakeArray(jval, container)) {
    Json::ArrayIndex ii = 0;
    for (auto const& citem : container) {
      Json::Value& aval = jval[ii++];
      aval = Json::arrayValue;
      func(aval, citem);
    }
  }
}

template <typename CONT>
void InfoWriter::SetConfigArray(
  std::string const& key,
  cmQtAutoGenInitializer::ConfigStrings<CONT> const& cfgStr)
{
  this->SetArray(key, cfgStr.Default);
  for (auto const& item : cfgStr.Config) {
    this->SetArray(cmStrCat(key, '_', item.first), item.second);
  }
}

bool InfoWriter::Save(std::string const& filename)
{
  cmGeneratedFileStream fileStream;
  fileStream.SetCopyIfDifferent(true);
  fileStream.Open(filename, false, true);
  if (!fileStream) {
    return false;
  }

  Json::StyledStreamWriter jsonWriter;
  try {
    jsonWriter.write(fileStream, this->Value_);
  } catch (...) {
    return false;
  }

  return fileStream.Close();
}

cmQtAutoGen::ConfigStrings<std::vector<std::string>> generateListOptions(
  cmQtAutoGen::ConfigStrings<cmQtAutoGen::CompilerFeaturesHandle> const&
    executableFeatures,
  bool IsMultiConfig)
{
  cmQtAutoGen::ConfigStrings<std::vector<std::string>> tempListOptions;
  if (IsMultiConfig) {
    for (auto const& executableFeature : executableFeatures.Config) {
      tempListOptions.Config[executableFeature.first] =
        executableFeature.second->ListOptions;
    }
  } else {
    tempListOptions.Default = executableFeatures.Default->ListOptions;
  }

  return tempListOptions;
}

} // End of unnamed namespace

cmQtAutoGenInitializer::cmQtAutoGenInitializer(
  cmQtAutoGenGlobalInitializer* globalInitializer,
  cmGeneratorTarget* genTarget, IntegerVersion const& qtVersion,
  bool mocEnabled, bool uicEnabled, bool rccEnabled, bool globalAutogenTarget,
  bool globalAutoRccTarget)
  : GlobalInitializer(globalInitializer)
  , GenTarget(genTarget)
  , GlobalGen(genTarget->GetGlobalGenerator())
  , LocalGen(genTarget->GetLocalGenerator())
  , Makefile(genTarget->Makefile)
  , PathCheckSum(genTarget->Makefile)
  , QtVersion(qtVersion)
{
  this->AutogenTarget.GlobalTarget = globalAutogenTarget;
  this->Moc.Enabled = mocEnabled;
  this->Uic.Enabled = uicEnabled;
  this->Rcc.Enabled = rccEnabled;
  this->Rcc.GlobalTarget = globalAutoRccTarget;
  this->CrossConfig =
    !this->Makefile->GetSafeDefinition("CMAKE_CROSS_CONFIGS").empty();
  this->UseBetterGraph =
    this->GenTarget->GetProperty("AUTOGEN_BETTER_GRAPH_MULTI_CONFIG").IsSet()
    ? this->GenTarget->GetProperty("AUTOGEN_BETTER_GRAPH_MULTI_CONFIG").IsOn()
    : (this->QtVersion >= IntegerVersion(6, 8));
  // AUTOGEN_BETTER_GRAPH_MULTI_CONFIG is set explicitly because it is read by
  // the qt library
  this->GenTarget->Target->SetProperty("AUTOGEN_BETTER_GRAPH_MULTI_CONFIG",
                                       this->UseBetterGraph ? "ON" : "OFF");
}

void cmQtAutoGenInitializer::AddAutogenExecutableToDependencies(
  cmQtAutoGenInitializer::GenVarsT const& genVars,
  std::vector<std::string>& dependencies) const
{
  if (genVars.ExecutableTarget != nullptr) {
    dependencies.push_back(genVars.ExecutableTarget->Target->GetName());
  } else if (this->MultiConfig && this->UseBetterGraph) {
    cm::string_view const& configGenexWithCommandConfig =
      "$<COMMAND_CONFIG:$<$<CONFIG:";
    cm::string_view const& configGenex = "$<$<CONFIG:";
    cm::string_view const& configGenexEnd = ">";
    cm::string_view const& configGenexEndWithCommandConfig = ">>";
    auto genexBegin =
      this->CrossConfig ? configGenexWithCommandConfig : configGenex;
    auto genexEnd =
      this->CrossConfig ? configGenexEndWithCommandConfig : configGenexEnd;
    for (auto const& config : genVars.Executable.Config) {
      auto executableWithConfig =
        cmStrCat(genexBegin, config.first, ">:", config.second, genexEnd);
      dependencies.emplace_back(std::move(executableWithConfig));
    }
  } else {
    if (!genVars.Executable.Default.empty()) {
      dependencies.push_back(genVars.Executable.Default);
    }
  }
}

bool cmQtAutoGenInitializer::InitCustomTargets()
{
  // Configurations
  this->MultiConfig = this->GlobalGen->IsMultiConfig();
  this->ConfigDefault = this->Makefile->GetDefaultConfiguration();
  this->ConfigsList =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  // Verbosity
  {
    std::string const def =
      this->Makefile->GetSafeDefinition("CMAKE_AUTOGEN_VERBOSE");
    if (!def.empty()) {
      unsigned long iVerb = 0;
      if (cmStrToULong(def, &iVerb)) {
        // Numeric verbosity
        this->Verbosity = static_cast<unsigned int>(iVerb);
      } else {
        // Non numeric verbosity
        if (cmIsOn(def)) {
          this->Verbosity = 1;
        }
      }
    }
  }

  // Targets FOLDER
  {
    cmValue folder =
      this->Makefile->GetState()->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
    if (!folder) {
      folder = this->Makefile->GetState()->GetGlobalProperty(
        "AUTOGEN_TARGETS_FOLDER");
    }
    // Inherit FOLDER property from target (#13688)
    if (!folder) {
      folder = this->GenTarget->GetProperty("FOLDER");
    }
    if (folder) {
      this->TargetsFolder = *folder;
    }
  }

  // Check status of policy CMP0071 regarding handling of GENERATED files
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0071)) {
    case cmPolicies::WARN:
      // Ignore GENERATED files but warn
      this->CMP0071Warn = true;
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // Ignore GENERATED files
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Process GENERATED files
      this->CMP0071Accept = true;
      break;
  }

  // Check status of policy CMP0100 regarding handling of .hh headers
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0100)) {
    case cmPolicies::WARN:
      // Ignore but .hh files but warn
      this->CMP0100Warn = true;
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // Ignore .hh files
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Process .hh file
      this->CMP0100Accept = true;
      break;
  }

  // Common directories
  std::string relativeBuildDir;
  {
    // Collapsed current binary directory
    std::string const cbd = cmSystemTools::CollapseFullPath(
      std::string(), this->Makefile->GetCurrentBinaryDirectory());

    // Info directory
    this->Dir.Info = cmStrCat(cbd, "/CMakeFiles/", this->GenTarget->GetName(),
                              "_autogen.dir");
    cmSystemTools::ConvertToUnixSlashes(this->Dir.Info);

    // Build directory
    this->Dir.Build = this->GenTarget->GetSafeProperty("AUTOGEN_BUILD_DIR");
    if (this->Dir.Build.empty()) {
      this->Dir.Build =
        cmStrCat(cbd, '/', this->GenTarget->GetName(), "_autogen");
    }
    cmSystemTools::ConvertToUnixSlashes(this->Dir.Build);
    this->Dir.RelativeBuild =
      cmSystemTools::RelativePath(cbd, this->Dir.Build);
    // Cleanup build directory
    this->AddCleanFile(this->Dir.Build);

    // Working directory
    this->Dir.Work = cbd;
    cmSystemTools::ConvertToUnixSlashes(this->Dir.Work);

    // Include directory
    this->ConfigFileNamesAndGenex(this->Dir.Include, this->Dir.IncludeGenExp,
                                  cmStrCat(this->Dir.Build, "/include"), "");
  }

  // Moc, Uic and _autogen target settings
  if (this->MocOrUicEnabled()) {
    // Init moc specific settings
    if (this->Moc.Enabled && !this->InitMoc()) {
      return false;
    }

    // Init uic specific settings
    if (this->Uic.Enabled && !this->InitUic()) {
      return false;
    }

    // Autogen target name
    this->AutogenTarget.Name =
      cmStrCat(this->GenTarget->GetName(), "_autogen");

    // Autogen target parallel processing
    {
      using ParallelType = decltype(this->AutogenTarget.Parallel);
      unsigned long propInt = 0;
      std::string const& prop =
        this->GenTarget->GetSafeProperty("AUTOGEN_PARALLEL");
      if (prop.empty() || (prop == "AUTO")) {
        // Autodetect number of CPUs
        this->AutogenTarget.Parallel = GetParallelCPUCount();
      } else if (cmStrToULong(prop, &propInt) && propInt > 0 &&
                 propInt <= std::numeric_limits<ParallelType>::max()) {
        this->AutogenTarget.Parallel = static_cast<ParallelType>(propInt);
      } else {
        // Warn the project author that AUTOGEN_PARALLEL is not valid.
        this->Makefile->IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat("AUTOGEN_PARALLEL=\"", prop, "\" for target \"",
                   this->GenTarget->GetName(),
                   "\" is not valid. Using AUTOGEN_PARALLEL=1"));
        this->AutogenTarget.Parallel = 1;
      }
    }

#ifdef _WIN32
    {
      const auto& value =
        this->GenTarget->GetProperty("AUTOGEN_COMMAND_LINE_LENGTH_MAX");
      if (value.IsSet()) {
        using maxCommandLineLengthType =
          decltype(this->AutogenTarget.MaxCommandLineLength);
        unsigned long propInt = 0;
        if (cmStrToULong(value, &propInt) && propInt > 0 &&
            propInt <= std::numeric_limits<maxCommandLineLengthType>::max()) {
          this->AutogenTarget.MaxCommandLineLength =
            static_cast<maxCommandLineLengthType>(propInt);
        } else {
          // Warn the project author that AUTOGEN_PARALLEL is not valid.
          this->Makefile->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmStrCat("AUTOGEN_COMMAND_LINE_LENGTH_MAX=\"", *value,
                     "\" for target \"", this->GenTarget->GetName(),
                     "\" is not valid. Using no limit for "
                     "AUTOGEN_COMMAND_LINE_LENGTH_MAX"));
          this->AutogenTarget.MaxCommandLineLength =
            std::numeric_limits<maxCommandLineLengthType>::max();
        }
      } else {
        // Actually 32767 (see
        // https://devblogs.microsoft.com/oldnewthing/20031210-00/?p=41553) but
        // we allow for a small margin
        this->AutogenTarget.MaxCommandLineLength = 32000;
      }
    }
#endif

    // Autogen target info and settings files
    {
      // Info file
      this->AutogenTarget.InfoFile =
        cmStrCat(this->Dir.Info, "/AutogenInfo.json");

      // Used settings file
      this->ConfigFileNames(this->AutogenTarget.SettingsFile,
                            cmStrCat(this->Dir.Info, "/AutogenUsed"), ".txt");
      this->ConfigFileClean(this->AutogenTarget.SettingsFile);

      // Parse cache file
      this->ConfigFileNames(this->AutogenTarget.ParseCacheFile,
                            cmStrCat(this->Dir.Info, "/ParseCache"), ".txt");
      this->ConfigFileClean(this->AutogenTarget.ParseCacheFile);
    }

    // Autogen target: Compute user defined dependencies
    {
      this->AutogenTarget.DependOrigin =
        this->GenTarget->GetPropertyAsBool("AUTOGEN_ORIGIN_DEPENDS");

      std::string const& deps =
        this->GenTarget->GetSafeProperty("AUTOGEN_TARGET_DEPENDS");
      if (!deps.empty()) {
        for (auto const& depName : cmList{ deps }) {
          // Allow target and file dependencies
          auto* depTarget = this->Makefile->FindTargetToUse(depName);
          if (depTarget) {
            this->AutogenTarget.DependTargets.insert(depTarget);
          } else {
            this->AutogenTarget.DependFiles.insert(depName);
          }
        }
      }
    }

    if (this->Moc.Enabled) {
      // Path prefix
      if (cmIsOn(this->GenTarget->GetProperty("AUTOMOC_PATH_PREFIX"))) {
        this->Moc.PathPrefix = true;
      }

      // CMAKE_AUTOMOC_RELAXED_MODE
      if (this->Makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE")) {
        this->Moc.RelaxedMode = true;
        this->Makefile->IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat("AUTOMOC: CMAKE_AUTOMOC_RELAXED_MODE is "
                   "deprecated an will be removed in the future.  Consider "
                   "disabling it and converting the target ",
                   this->GenTarget->GetName(), " to regular mode."));
      }

      // Options
      cmExpandList(this->GenTarget->GetSafeProperty("AUTOMOC_MOC_OPTIONS"),
                   this->Moc.Options);
      // Filters
      cmExpandList(this->GenTarget->GetSafeProperty("AUTOMOC_MACRO_NAMES"),
                   this->Moc.MacroNames);
      this->Moc.MacroNames.erase(cmRemoveDuplicates(this->Moc.MacroNames),
                                 this->Moc.MacroNames.end());
      {
        cmList const filterList = { this->GenTarget->GetSafeProperty(
          "AUTOMOC_DEPEND_FILTERS") };
        if ((filterList.size() % 2) != 0) {
          cmSystemTools::Error(
            cmStrCat("AutoMoc: AUTOMOC_DEPEND_FILTERS predefs size ",
                     filterList.size(), " is not a multiple of 2."));
          return false;
        }
        this->Moc.DependFilters.reserve(1 + (filterList.size() / 2));
        this->Moc.DependFilters.emplace_back(
          "Q_PLUGIN_METADATA",
          "[\n][ \t]*Q_PLUGIN_METADATA[ \t]*\\("
          "[^\\)]*FILE[ \t]*\"([^\"]+)\"");
        for (cmList::size_type ii = 0; ii != filterList.size(); ii += 2) {
          this->Moc.DependFilters.emplace_back(filterList[ii],
                                               filterList[ii + 1]);
        }
      }
    }
  }

  // Init rcc specific settings
  if (this->Rcc.Enabled && !this->InitRcc()) {
    return false;
  }

  // Add autogen include directory to the origin target INCLUDE_DIRECTORIES
  if (this->MocOrUicEnabled() || (this->Rcc.Enabled && this->MultiConfig)) {
    auto addBefore = false;
    auto const& value =
      this->GenTarget->GetProperty("AUTOGEN_USE_SYSTEM_INCLUDE");
    if (value.IsSet()) {
      if (cmIsOn(value)) {
        this->GenTarget->AddSystemIncludeDirectory(this->Dir.IncludeGenExp,
                                                   "CXX");
      } else {
        addBefore = true;
      }
    } else {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0151)) {
        case cmPolicies::WARN:
        case cmPolicies::OLD:
          addBefore = true;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          this->GenTarget->AddSystemIncludeDirectory(this->Dir.IncludeGenExp,
                                                     "CXX");
          break;
      }
    }
    this->GenTarget->AddIncludeDirectory(this->Dir.IncludeGenExp, addBefore);
  }

  // Scan files
  if (!this->InitScanFiles()) {
    return false;
  }

  // Create autogen target
  if (this->MocOrUicEnabled() && !this->InitAutogenTarget()) {
    return false;
  }

  // Create rcc targets
  if (this->Rcc.Enabled && !this->InitRccTargets()) {
    return false;
  }

  return true;
}

bool cmQtAutoGenInitializer::InitMoc()
{
  // Mocs compilation file
  if (this->GlobalGen->IsXcode()) {
    // XXX(xcode-per-cfg-src): Drop this Xcode-specific code path
    // when the Xcode generator supports per-config sources.
    this->Moc.CompilationFile.Default =
      cmStrCat(this->Dir.Build, "/mocs_compilation.cpp");
    this->Moc.CompilationFileGenex = this->Moc.CompilationFile.Default;
  } else {
    this->ConfigFileNamesAndGenex(
      this->Moc.CompilationFile, this->Moc.CompilationFileGenex,
      cmStrCat(this->Dir.Build, "/mocs_compilation"_s), ".cpp"_s);
  }

  // Moc predefs
  if (this->GenTarget->GetPropertyAsBool("AUTOMOC_COMPILER_PREDEFINES") &&
      (this->QtVersion >= IntegerVersion(5, 8))) {
    // Command
    cmList::assign(
      this->Moc.PredefsCmd,
      this->Makefile->GetDefinition("CMAKE_CXX_COMPILER_PREDEFINES_COMMAND"));
    // Header
    if (!this->Moc.PredefsCmd.empty()) {
      this->ConfigFileNames(this->Moc.PredefsFile,
                            cmStrCat(this->Dir.Build, "/moc_predefs"), ".h");
    }
  }

  // Moc includes
  {
    SearchPathSanitizer const sanitizer(this->Makefile);
    auto getDirs =
      [this, &sanitizer](std::string const& cfg) -> std::vector<std::string> {
      // Get the include dirs for this target, without stripping the implicit
      // include dirs off, see issue #13667.
      std::vector<std::string> dirs;
      bool const appendImplicit = (this->QtVersion.Major >= 5);
      this->LocalGen->GetIncludeDirectoriesImplicit(
        dirs, this->GenTarget, "CXX", cfg, false, appendImplicit);
      return sanitizer(dirs);
    };

    // Other configuration settings
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::vector<std::string> dirs = getDirs(cfg);
        if (dirs == this->Moc.Includes.Default) {
          continue;
        }
        this->Moc.Includes.Config[cfg] = std::move(dirs);
      }
    } else {
      // Default configuration include directories
      this->Moc.Includes.Default = getDirs(this->ConfigDefault);
    }
  }

  // Moc compile definitions
  {
    auto getDefs = [this](std::string const& cfg) -> std::set<std::string> {
      std::set<std::string> defines;
      this->LocalGen->GetTargetDefines(this->GenTarget, cfg, "CXX", defines);
      if (this->Moc.PredefsCmd.empty() &&
          this->Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME") ==
            "Windows") {
        // Add WIN32 definition if we don't have a moc_predefs.h
        defines.insert("WIN32");
      }
      return defines;
    };

    // Other configuration defines
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::set<std::string> defines = getDefs(cfg);
        if (defines == this->Moc.Defines.Default) {
          continue;
        }
        this->Moc.Defines.Config[cfg] = std::move(defines);
      }
    } else {
      // Default configuration defines
      this->Moc.Defines.Default = getDefs(this->ConfigDefault);
    }
  }

  // Moc executable
  {
    if (!this->GetQtExecutable(this->Moc, "moc", false)) {
      return false;
    }
    // Let the _autogen target depend on the moc executable
    if (this->Moc.ExecutableTarget) {
      this->AutogenTarget.DependTargets.insert(
        this->Moc.ExecutableTarget->Target);
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitUic()
{
  // Uic search paths
  {
    std::string const& usp =
      this->GenTarget->GetSafeProperty("AUTOUIC_SEARCH_PATHS");
    if (!usp.empty()) {
      this->Uic.SearchPaths =
        SearchPathSanitizer(this->Makefile)(cmList{ usp });
    }
  }
  // Uic target options
  {
    auto getOpts = [this](std::string const& cfg) -> std::vector<std::string> {
      std::vector<std::string> opts;
      this->GenTarget->GetAutoUicOptions(opts, cfg);
      return opts;
    };

    // Default options
    this->Uic.Options.Default = getOpts(this->ConfigDefault);
    // Configuration specific options
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::vector<std::string> options = getOpts(cfg);
        if (options == this->Uic.Options.Default) {
          continue;
        }
        this->Uic.Options.Config[cfg] = std::move(options);
      }
    }
  }

  // Uic executable
  {
    if (!this->GetQtExecutable(this->Uic, "uic", true)) {
      return false;
    }
    // Let the _autogen target depend on the uic executable
    if (this->Uic.ExecutableTarget) {
      this->AutogenTarget.DependTargets.insert(
        this->Uic.ExecutableTarget->Target);
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitRcc()
{
  // Rcc executable
  {
    if (!this->GetQtExecutable(this->Rcc, "rcc", false)) {
      return false;
    }
    // Evaluate test output on demand
    auto& features = this->Rcc.ExecutableFeatures;
    auto checkAndAddOptions = [this](CompilerFeaturesHandle& feature) {
      if (!feature->Evaluated) {
        // Look for list options
        if (this->QtVersion.Major == 5 || this->QtVersion.Major == 6) {
          static std::array<std::string, 2> const listOptions{ { "--list",
                                                                 "-list" } };
          for (std::string const& opt : listOptions) {
            if (feature->HelpOutput.find(opt) != std::string::npos) {
              feature->ListOptions.emplace_back(opt);
              break;
            }
          }
        }
        // Evaluation finished
        feature->Evaluated = true;
      }
    };
    if (this->MultiConfig && this->UseBetterGraph) {
      for (auto const& config : this->ConfigsList) {
        checkAndAddOptions(features.Config[config]);
      }
    } else {
      checkAndAddOptions(features.Default);
    }
  }

  // Disable zstd if it is not supported
  {
    std::string const qtFeatureZSTD = "QT_FEATURE_zstd";
    if (this->GenTarget->Target->GetMakefile()->IsDefinitionSet(
          qtFeatureZSTD)) {
      const auto zstdDef =
        this->GenTarget->Target->GetMakefile()->GetSafeDefinition(
          qtFeatureZSTD);
      const auto zstdVal = cmValue(zstdDef);
      if (zstdVal.IsOff()) {
        auto const& kw = this->GlobalInitializer->kw();
        auto rccOptions = this->GenTarget->GetSafeProperty(kw.AUTORCC_OPTIONS);
        std::string const nozstd = "--no-zstd";
        if (rccOptions.find(nozstd) == std::string::npos) {
          rccOptions.append(";" + nozstd + ";");
        }
        this->GenTarget->Target->SetProperty(kw.AUTORCC_OPTIONS, rccOptions);
      }
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitScanFiles()
{
  cmake const* cm = this->Makefile->GetCMakeInstance();
  auto const& kw = this->GlobalInitializer->kw();

  auto makeMUFile = [this, &kw](cmSourceFile* sf, std::string const& fullPath,
                                std::vector<size_t> const& configs,
                                bool muIt) -> MUFileHandle {
    MUFileHandle muf = cm::make_unique<MUFile>();
    muf->FullPath = fullPath;
    muf->SF = sf;
    if (!configs.empty() && configs.size() != this->ConfigsList.size()) {
      muf->Configs = configs;
    }
    muf->Generated = sf->GetIsGenerated();
    bool const skipAutogen = sf->GetPropertyAsBool(kw.SKIP_AUTOGEN);
    muf->SkipMoc = this->Moc.Enabled &&
      (skipAutogen || sf->GetPropertyAsBool(kw.SKIP_AUTOMOC));
    muf->SkipUic = this->Uic.Enabled &&
      (skipAutogen || sf->GetPropertyAsBool(kw.SKIP_AUTOUIC));
    if (muIt) {
      muf->MocIt = this->Moc.Enabled && !muf->SkipMoc;
      muf->UicIt = this->Uic.Enabled && !muf->SkipUic;
    }
    return muf;
  };

  auto addMUHeader = [this](MUFileHandle&& muf, cm::string_view extension) {
    cmSourceFile* sf = muf->SF;
    const bool muIt = (muf->MocIt || muf->UicIt);
    if (this->CMP0100Accept || (extension != "hh")) {
      // Accept
      if (muIt && muf->Generated) {
        this->AutogenTarget.FilesGenerated.emplace_back(muf.get());
      }
      this->AutogenTarget.Headers.emplace(sf, std::move(muf));
    } else if (muIt && this->CMP0100Warn) {
      // Store file for warning message
      this->AutogenTarget.CMP0100HeadersWarn.push_back(sf);
    }
  };

  auto addMUSource = [this](MUFileHandle&& muf) {
    if ((muf->MocIt || muf->UicIt) && muf->Generated) {
      this->AutogenTarget.FilesGenerated.emplace_back(muf.get());
    }
    this->AutogenTarget.Sources.emplace(muf->SF, std::move(muf));
  };

  // Scan through target files
  {
    // Scan through target files
    for (cmGeneratorTarget::AllConfigSource const& acs :
         this->GenTarget->GetAllConfigSources()) {
      std::string const& fullPath = acs.Source->GetFullPath();
      std::string const& extLower =
        cmSystemTools::LowerCase(acs.Source->GetExtension());

      // Register files that will be scanned by moc or uic
      if (this->MocOrUicEnabled()) {
        if (cm->IsAHeaderExtension(extLower)) {
          addMUHeader(makeMUFile(acs.Source, fullPath, acs.Configs, true),
                      extLower);
        } else if (cm->IsACLikeSourceExtension(extLower)) {
          addMUSource(makeMUFile(acs.Source, fullPath, acs.Configs, true));
        }
      }

      // Register rcc enabled files
      if (this->Rcc.Enabled) {
        if ((extLower == kw.qrc) &&
            !acs.Source->GetPropertyAsBool(kw.SKIP_AUTOGEN) &&
            !acs.Source->GetPropertyAsBool(kw.SKIP_AUTORCC)) {
          // Register qrc file
          Qrc qrc;
          qrc.QrcFile = fullPath;
          qrc.QrcName =
            cmSystemTools::GetFilenameWithoutLastExtension(qrc.QrcFile);
          qrc.Generated = acs.Source->GetIsGenerated();
          // RCC options
          {
            std::string const& opts =
              acs.Source->GetSafeProperty(kw.AUTORCC_OPTIONS);
            if (!opts.empty()) {
              cmExpandList(opts, qrc.Options);
            }
          }
          this->Rcc.Qrcs.push_back(std::move(qrc));
        }
      }
    }
  }
  // cmGeneratorTarget::GetAllConfigSources computes the target's
  // sources meta data cache. Clear it so that OBJECT library targets that
  // are AUTOGEN initialized after this target get their added
  // mocs_compilation.cpp source acknowledged by this target.
  this->GenTarget->ClearSourcesCache();

  // For source files find additional headers and private headers
  if (this->MocOrUicEnabled()) {
    // Header search suffixes and extensions
    static std::initializer_list<cm::string_view> const suffixes{ "", "_p" };
    auto const& exts = cm->GetHeaderExtensions();
    // Scan through sources
    for (auto const& pair : this->AutogenTarget.Sources) {
      MUFile const& muf = *pair.second;
      if (muf.MocIt || muf.UicIt) {
        // Search for the default header file and a private header
        std::string const& srcFullPath = muf.SF->ResolveFullPath();
        std::string const basePath = cmStrCat(
          cmQtAutoGen::SubDirPrefix(srcFullPath),
          cmSystemTools::GetFilenameWithoutLastExtension(srcFullPath));
        for (auto const& suffix : suffixes) {
          std::string const suffixedPath = cmStrCat(basePath, suffix);
          for (auto const& ext : exts) {
            std::string const fullPath = cmStrCat(suffixedPath, '.', ext);

            auto constexpr locationKind = cmSourceFileLocationKind::Known;
            cmSourceFile* sf =
              this->Makefile->GetSource(fullPath, locationKind);
            if (sf) {
              // Check if we know about this header already
              if (cm::contains(this->AutogenTarget.Headers, sf)) {
                continue;
              }
              // We only accept not-GENERATED files that do exist.
              if (!sf->GetIsGenerated() &&
                  !cmSystemTools::FileExists(fullPath)) {
                continue;
              }
            } else if (cmSystemTools::FileExists(fullPath)) {
              // Create a new source file for the existing file
              sf = this->Makefile->CreateSource(fullPath, false, locationKind);
            }

            if (sf) {
              auto eMuf = makeMUFile(sf, fullPath, muf.Configs, true);
              // Only process moc/uic when the parent is processed as well
              if (!muf.MocIt) {
                eMuf->MocIt = false;
              }
              if (!muf.UicIt) {
                eMuf->UicIt = false;
              }
              addMUHeader(std::move(eMuf), ext);
            }
          }
        }
      }
    }
  }

  // Scan through all source files in the makefile to extract moc and uic
  // parameters.  Historically we support non target source file parameters.
  // The reason is that their file names might be discovered from source files
  // at generation time.
  if (this->MocOrUicEnabled()) {
    for (const auto& sf : this->Makefile->GetSourceFiles()) {
      // sf->GetExtension() is only valid after sf->ResolveFullPath() ...
      // Since we're iterating over source files that might be not in the
      // target we need to check for path errors (not existing files).
      std::string pathError;
      std::string const& fullPath = sf->ResolveFullPath(&pathError);
      if (!pathError.empty() || fullPath.empty()) {
        continue;
      }
      std::string const& extLower =
        cmSystemTools::LowerCase(sf->GetExtension());

      if (cm->IsAHeaderExtension(extLower)) {
        if (!cm::contains(this->AutogenTarget.Headers, sf.get())) {
          auto muf = makeMUFile(sf.get(), fullPath, {}, false);
          if (muf->SkipMoc || muf->SkipUic) {
            addMUHeader(std::move(muf), extLower);
          }
        }
      } else if (cm->IsACLikeSourceExtension(extLower)) {
        if (!cm::contains(this->AutogenTarget.Sources, sf.get())) {
          auto muf = makeMUFile(sf.get(), fullPath, {}, false);
          if (muf->SkipMoc || muf->SkipUic) {
            addMUSource(std::move(muf));
          }
        }
      } else if (this->Uic.Enabled && (extLower == kw.ui)) {
        // .ui file
        bool const skipAutogen = sf->GetPropertyAsBool(kw.SKIP_AUTOGEN);
        bool const skipUic =
          (skipAutogen || sf->GetPropertyAsBool(kw.SKIP_AUTOUIC));
        if (!skipUic) {
          // Check if the .ui file has uic options
          std::string const uicOpts = sf->GetSafeProperty(kw.AUTOUIC_OPTIONS);
          if (uicOpts.empty()) {
            this->Uic.UiFilesNoOptions.emplace_back(fullPath);
          } else {
            this->Uic.UiFilesWithOptions.emplace_back(
              fullPath, std::move(cmList{ uicOpts }.data()));
          }

          auto uiHeaderRelativePath = cmSystemTools::RelativePath(
            this->LocalGen->GetCurrentSourceDirectory(),
            cmSystemTools::GetFilenamePath(fullPath));

          // Avoid creating a path containing adjacent slashes
          if (!uiHeaderRelativePath.empty() &&
              uiHeaderRelativePath.back() != '/') {
            uiHeaderRelativePath += '/';
          }

          auto uiHeaderFilePath = cmStrCat(
            '/', uiHeaderRelativePath, "ui_"_s,
            cmSystemTools::GetFilenameWithoutLastExtension(fullPath), ".h"_s);

          ConfigString uiHeader;
          std::string uiHeaderGenex;
          this->ConfigFileNamesAndGenex(
            uiHeader, uiHeaderGenex, cmStrCat(this->Dir.Build, "/include"_s),
            uiHeaderFilePath);

          this->Uic.UiHeaders.emplace_back(uiHeader, uiHeaderGenex);
        } else {
          // Register skipped .ui file
          this->Uic.SkipUi.insert(fullPath);
        }
      }
    }
  }

  // Process GENERATED sources and headers
  if (this->MocOrUicEnabled() && !this->AutogenTarget.FilesGenerated.empty()) {
    if (this->CMP0071Accept) {
      // Let the autogen target depend on the GENERATED files
      if (this->MultiConfig && !this->CrossConfig) {
        for (MUFile const* muf : this->AutogenTarget.FilesGenerated) {
          if (muf->Configs.empty()) {
            this->AutogenTarget.DependFiles.insert(muf->FullPath);
          } else {
            for (size_t ci : muf->Configs) {
              std::string const& config = this->ConfigsList[ci];
              std::string const& pathWithConfig =
                cmStrCat("$<$<CONFIG:", config, ">:", muf->FullPath, '>');
              this->AutogenTarget.DependFiles.insert(pathWithConfig);
            }
          }
        }
      } else {
        for (MUFile const* muf : this->AutogenTarget.FilesGenerated) {
          this->AutogenTarget.DependFiles.insert(muf->FullPath);
        }
      }
    } else if (this->CMP0071Warn) {
      cm::string_view property;
      if (this->Moc.Enabled && this->Uic.Enabled) {
        property = "SKIP_AUTOGEN";
      } else if (this->Moc.Enabled) {
        property = "SKIP_AUTOMOC";
      } else if (this->Uic.Enabled) {
        property = "SKIP_AUTOUIC";
      }
      std::string files;
      for (MUFile const* muf : this->AutogenTarget.FilesGenerated) {
        files += cmStrCat("  ", Quoted(muf->FullPath), '\n');
      }
      this->Makefile->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(
          cmPolicies::GetPolicyWarning(cmPolicies::CMP0071), '\n',
          "For compatibility, CMake is excluding the GENERATED source "
          "file(s):\n",
          files, "from processing by ",
          cmQtAutoGen::Tools(this->Moc.Enabled, this->Uic.Enabled, false),
          ".  If any of the files should be processed, set CMP0071 to NEW.  "
          "If any of the files should not be processed, "
          "explicitly exclude them by setting the source file property ",
          property, ":\n  set_property(SOURCE file.h PROPERTY ", property,
          " ON)\n"));
    }
  }

  // Generate CMP0100 warning
  if (this->MocOrUicEnabled() &&
      !this->AutogenTarget.CMP0100HeadersWarn.empty()) {
    cm::string_view property;
    if (this->Moc.Enabled && this->Uic.Enabled) {
      property = "SKIP_AUTOGEN";
    } else if (this->Moc.Enabled) {
      property = "SKIP_AUTOMOC";
    } else if (this->Uic.Enabled) {
      property = "SKIP_AUTOUIC";
    }
    std::string files;
    for (cmSourceFile const* sf : this->AutogenTarget.CMP0100HeadersWarn) {
      files += cmStrCat("  ", Quoted(sf->GetFullPath()), '\n');
    }
    this->Makefile->IssueMessage(
      MessageType::AUTHOR_WARNING,
      cmStrCat(
        cmPolicies::GetPolicyWarning(cmPolicies::CMP0100), '\n',
        "For compatibility, CMake is excluding the header file(s):\n", files,
        "from processing by ",
        cmQtAutoGen::Tools(this->Moc.Enabled, this->Uic.Enabled, false),
        ".  If any of the files should be processed, set CMP0100 to NEW.  "
        "If any of the files should not be processed, "
        "explicitly exclude them by setting the source file property ",
        property, ":\n  set_property(SOURCE file.hh PROPERTY ", property,
        " ON)\n"));
  }

  // Process qrc files
  if (!this->Rcc.Qrcs.empty()) {
    const bool modernQt = (this->QtVersion.Major >= 5);
    // Target rcc options
    cmList const optionsTarget{ this->GenTarget->GetSafeProperty(
      kw.AUTORCC_OPTIONS) };

    // Check if file name is unique
    for (Qrc& qrc : this->Rcc.Qrcs) {
      qrc.Unique = true;
      for (Qrc const& qrc2 : this->Rcc.Qrcs) {
        if ((&qrc != &qrc2) && (qrc.QrcName == qrc2.QrcName)) {
          qrc.Unique = false;
          break;
        }
      }
    }
    // Path checksum and file names
    for (Qrc& qrc : this->Rcc.Qrcs) {
      // Path checksum
      qrc.QrcPathChecksum = this->PathCheckSum.getPart(qrc.QrcFile);
      // Output file name
      if (this->MultiConfig && !this->GlobalGen->IsXcode() &&
          this->UseBetterGraph) {
        qrc.OutputFile = cmStrCat(this->Dir.Build, '/', qrc.QrcPathChecksum,
                                  "_$<CONFIG>", "/qrc_", qrc.QrcName, ".cpp");
      } else {
        qrc.OutputFile = cmStrCat(this->Dir.Build, '/', qrc.QrcPathChecksum,
                                  "/qrc_", qrc.QrcName, ".cpp");
      }
      std::string const base = cmStrCat(this->Dir.Info, "/AutoRcc_",
                                        qrc.QrcName, '_', qrc.QrcPathChecksum);
      qrc.LockFile = cmStrCat(base, "_Lock.lock");
      qrc.InfoFile = cmStrCat(base, "_Info.json");
      this->ConfigFileNames(qrc.SettingsFile, cmStrCat(base, "_Used"), ".txt");
    }
    // rcc options
    for (Qrc& qrc : this->Rcc.Qrcs) {
      // Target options
      std::vector<std::string> opts = optionsTarget;
      // Merge computed "-name XYZ" option
      {
        std::string name = qrc.QrcName;
        // Replace '-' with '_'. The former is not valid for symbol names.
        std::replace(name.begin(), name.end(), '-', '_');
        if (!qrc.Unique) {
          name += cmStrCat('_', qrc.QrcPathChecksum);
        }
        std::vector<std::string> nameOpts;
        nameOpts.emplace_back("-name");
        nameOpts.emplace_back(std::move(name));
        RccMergeOptions(opts, nameOpts, modernQt);
      }
      // Merge file option
      RccMergeOptions(opts, qrc.Options, modernQt);
      qrc.Options = std::move(opts);
    }
    // rcc resources
    for (Qrc& qrc : this->Rcc.Qrcs) {
      if (!qrc.Generated) {
        std::string error;
        if (this->MultiConfig && this->UseBetterGraph) {
          for (auto const& config : this->ConfigsList) {
            RccLister const lister(
              this->Rcc.Executable.Config[config],
              this->Rcc.ExecutableFeatures.Config[config]->ListOptions);
            if (!lister.list(qrc.QrcFile, qrc.Resources.Config[config],
                             error)) {
              cmSystemTools::Error(error);
              return false;
            }
          }
        } else {
          RccLister const lister(
            this->Rcc.Executable.Default,
            this->Rcc.ExecutableFeatures.Default->ListOptions);
          if (!lister.list(qrc.QrcFile, qrc.Resources.Default, error)) {
            cmSystemTools::Error(error);
            return false;
          }
        }
      }
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitAutogenTarget()
{
  // Register info file as generated by CMake
  this->Makefile->AddCMakeOutputFile(this->AutogenTarget.InfoFile);

  // Determine whether to use a depfile for the AUTOGEN target.
  bool const useDepfile = [this]() -> bool {
    auto const& gen = this->GlobalGen->GetName();
    return this->QtVersion >= IntegerVersion(5, 15) &&
      (gen.find("Ninja") != std::string::npos ||
       gen.find("Make") != std::string::npos);
  }();

  // Files provided by the autogen target
  std::vector<std::string> autogenByproducts;
  std::vector<std::string> timestampByproducts;
  if (this->Moc.Enabled) {
    this->AddGeneratedSource(this->Moc.CompilationFile, this->Moc, true);
    if (useDepfile) {
      if (this->CrossConfig &&
          this->GlobalGen->GetName().find("Ninja") != std::string::npos &&
          !this->UseBetterGraph) {
        // Make all mocs_compilation_<CONFIG>.cpp files byproducts of the
        // ${target}_autogen/timestamp custom command.
        // We cannot just use Moc.CompilationFileGenex here, because that
        // custom command runs cmake_autogen for each configuration.
        for (const auto& p : this->Moc.CompilationFile.Config) {
          timestampByproducts.push_back(p.second);
        }
      } else {
        timestampByproducts.push_back(this->Moc.CompilationFileGenex);
      }
    } else {
      autogenByproducts.push_back(this->Moc.CompilationFileGenex);
    }
  }

  if (this->Uic.Enabled) {
    for (const auto& file : this->Uic.UiHeaders) {
      this->AddGeneratedSource(file.first, this->Uic);
      autogenByproducts.push_back(file.second);
    }
  }

  // Compose target comment
  std::string autogenComment;
  {
    std::string tools;
    if (this->Moc.Enabled) {
      tools += "MOC";
    }
    if (this->Uic.Enabled) {
      if (!tools.empty()) {
        tools += " and ";
      }
      tools += "UIC";
    }
    autogenComment = cmStrCat("Automatic ", tools, " for target ",
                              this->GenTarget->GetName());
  }

  // Compose command lines
  // FIXME: Take advantage of our per-config mocs_compilation_$<CONFIG>.cpp
  // instead of fiddling with the include directories

  bool constexpr stdPipesUTF8 = true;
  cmCustomCommandLines commandLines;
  AddCMakeProcessToCommandLines(this->AutogenTarget.InfoFile, "cmake_autogen",
                                commandLines);

  // Use PRE_BUILD on demand
  bool usePRE_BUILD = false;
  if (this->GlobalGen->GetName().find("Visual Studio") != std::string::npos) {
    // Under VS use a PRE_BUILD event instead of a separate target to
    // reduce the number of targets loaded into the IDE.
    // This also works around a VS 11 bug that may skip updating the target:
    //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
    usePRE_BUILD = true;
  }
  // Disable PRE_BUILD in some cases
  if (usePRE_BUILD) {
    // Cannot use PRE_BUILD with file depends
    if (!this->AutogenTarget.DependFiles.empty()) {
      usePRE_BUILD = false;
    }
    // Cannot use PRE_BUILD when a global autogen target is in place
    if (this->AutogenTarget.GlobalTarget) {
      usePRE_BUILD = false;
    }
  }
  // Create the autogen target/command
  if (usePRE_BUILD) {
    // Add additional autogen target dependencies to origin target
    for (cmTarget const* depTarget : this->AutogenTarget.DependTargets) {
      this->GenTarget->Target->AddUtility(depTarget->GetName(), false,
                                          this->Makefile);
    }

    if (!this->Uic.UiFilesNoOptions.empty() ||
        !this->Uic.UiFilesWithOptions.empty()) {
      // Add a generated timestamp file
      ConfigString timestampFile;
      std::string timestampFileGenex;
      ConfigFileNamesAndGenex(timestampFile, timestampFileGenex,
                              cmStrCat(this->Dir.Build, "/autouic"_s),
                              ".stamp"_s);
      this->AddGeneratedSource(timestampFile, this->Uic);

      // Add a step in the pre-build command to touch the timestamp file
      commandLines.push_back(
        cmMakeCommandLine({ cmSystemTools::GetCMakeCommand(), "-E", "touch",
                            timestampFileGenex }));

      // UIC needs to be re-run if any of the known UI files change or the
      // executable itself has been updated
      auto uicDependencies = this->Uic.UiFilesNoOptions;
      for (auto const& uiFile : this->Uic.UiFilesWithOptions) {
        uicDependencies.push_back(uiFile.first);
      }
      AddAutogenExecutableToDependencies(this->Uic, uicDependencies);

      // Add a rule file to cause the target to build if a dependency has
      // changed, which will trigger the pre-build command to run autogen
      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(timestampFileGenex);
      cc->SetDepends(uicDependencies);
      cc->SetComment("");
      cc->SetWorkingDirectory(this->Dir.Work.c_str());
      cc->SetEscapeOldStyle(false);
      cc->SetStdPipesUTF8(stdPipesUTF8);
      this->LocalGen->AddCustomCommandToOutput(std::move(cc));
    }

    // Add the pre-build command directly to bypass the OBJECT_LIBRARY
    // rejection in cmMakefile::AddCustomCommandToTarget because we know
    // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
    //
    // PRE_BUILD does not support file dependencies!
    cmCustomCommand cc;
    cc.SetByproducts(autogenByproducts);
    cc.SetCommandLines(commandLines);
    cc.SetComment(autogenComment.c_str());
    cc.SetBacktrace(this->Makefile->GetBacktrace());
    cc.SetWorkingDirectory(this->Dir.Work.c_str());
    cc.SetStdPipesUTF8(stdPipesUTF8);
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    this->GenTarget->Target->AddPreBuildCommand(std::move(cc));
  } else {

    // Add link library target dependencies to the autogen target
    // dependencies
    if (this->AutogenTarget.DependOrigin) {
      // add_dependencies/addUtility do not support generator expressions.
      // We depend only on the libraries found in all configs therefore.
      std::map<cmGeneratorTarget const*, std::size_t> commonTargets;
      for (std::string const& config : this->ConfigsList) {
        cmLinkImplementationLibraries const* libs =
          this->GenTarget->GetLinkImplementationLibraries(
            config, cmGeneratorTarget::LinkInterfaceFor::Link);
        if (libs) {
          for (cmLinkItem const& item : libs->Libraries) {
            cmGeneratorTarget const* libTarget = item.Target;
            if (libTarget &&
                !StaticLibraryCycle(this->GenTarget, libTarget, config)) {
              // Increment target config count
              commonTargets[libTarget]++;
            }
          }
        }
      }
      for (auto const& item : commonTargets) {
        if (item.second == this->ConfigsList.size()) {
          this->AutogenTarget.DependTargets.insert(item.first->Target);
        }
      }
    }

    cmTarget* timestampTarget = nullptr;
    std::vector<std::string> dependencies(
      this->AutogenTarget.DependFiles.begin(),
      this->AutogenTarget.DependFiles.end());
    if (useDepfile) {
      // Create a custom command that generates a timestamp file and
      // has a depfile assigned. The depfile is created by JobDepFilesMergeT.
      //
      // Also create an additional '_autogen_timestamp_deps' that the custom
      // command will depend on. It will have no sources or commands to
      // execute, but it will have dependencies that would originally be
      // assigned to the pre-Qt 5.15 'autogen' target. These dependencies will
      // serve as a list of order-only dependencies for the custom command,
      // without forcing the custom command to re-execute.
      //
      // The dependency tree would then look like
      // '_autogen_timestamp_deps (order-only)' <- '/timestamp' file <-
      // '_autogen' target.
      const auto timestampTargetName =
        cmStrCat(this->GenTarget->GetName(), "_autogen_timestamp_deps");

      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetWorkingDirectory(this->Dir.Work.c_str());
      cc->SetDepends(dependencies);
      cc->SetEscapeOldStyle(false);
      timestampTarget = this->LocalGen->AddUtilityCommand(timestampTargetName,
                                                          true, std::move(cc));

      this->LocalGen->AddGeneratorTarget(
        cm::make_unique<cmGeneratorTarget>(timestampTarget, this->LocalGen));

      // Set FOLDER property on the timestamp target, so it appears in the
      // appropriate folder in an IDE or in the file api.
      if (!this->TargetsFolder.empty()) {
        timestampTarget->SetProperty("FOLDER", this->TargetsFolder);
      }

      // Make '/timestamp' file depend on '_autogen_timestamp_deps' and on the
      // moc and uic executables (whichever are enabled).
      dependencies.clear();
      dependencies.push_back(timestampTargetName);

      AddAutogenExecutableToDependencies(this->Moc, dependencies);
      AddAutogenExecutableToDependencies(this->Uic, dependencies);
      std::string outputFile;
      std::string depFile;
      // Create the custom command that outputs the timestamp file.
      if (this->MultiConfig && this->UseBetterGraph) {
        // create timestamp file with $<CONFIG> in the name so that
        // every cmake_autogen target has its own timestamp file
        std::string const configView = "$<CONFIG>";
        std::string const timestampFileWithoutConfig = "timestamp_";
        std::string const depFileWithoutConfig =
          cmStrCat(this->Dir.Build, "/deps_");
        std::string const timestampFileName =
          timestampFileWithoutConfig + configView;
        outputFile = cmStrCat(this->Dir.Build, "/", timestampFileName);
        auto const depFileWithConfig =
          cmStrCat(depFileWithoutConfig, configView);
        depFile = depFileWithConfig;
        commandLines.push_back(cmMakeCommandLine(
          { cmSystemTools::GetCMakeCommand(), "-E", "touch", outputFile }));

        ConfigString outputFileWithConfig;
        for (std::string const& config : this->ConfigsList) {
          auto tempTimestampFileName = timestampFileWithoutConfig + config;
          auto tempDepFile = depFileWithoutConfig + config;
          outputFileWithConfig.Config[config] = tempTimestampFileName;
          this->AutogenTarget.DepFileRuleName.Config[config] =
            cmStrCat(this->Dir.RelativeBuild, "/", tempTimestampFileName);
          this->AutogenTarget.DepFile.Config[config] = tempDepFile;
        }
        this->AddGeneratedSource(outputFileWithConfig, this->Moc);
      } else {
        cm::string_view const timestampFileName = "timestamp";
        outputFile = cmStrCat(this->Dir.Build, "/", timestampFileName);
        this->AutogenTarget.DepFile.Default =
          cmStrCat(this->Dir.Build, "/deps");
        depFile = this->AutogenTarget.DepFile.Default;
        this->AutogenTarget.DepFileRuleName.Default =
          cmStrCat(this->Dir.RelativeBuild, "/", timestampFileName);
        commandLines.push_back(cmMakeCommandLine(
          { cmSystemTools::GetCMakeCommand(), "-E", "touch", outputFile }));
        this->AddGeneratedSource(outputFile, this->Moc);
      }
      cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(outputFile);
      cc->SetByproducts(timestampByproducts);
      cc->SetDepends(dependencies);
      cc->SetCommandLines(commandLines);
      cc->SetComment(autogenComment.c_str());
      cc->SetWorkingDirectory(this->Dir.Work.c_str());
      cc->SetEscapeOldStyle(false);
      cc->SetDepfile(depFile);
      cc->SetStdPipesUTF8(stdPipesUTF8);
      this->LocalGen->AddCustomCommandToOutput(std::move(cc));
      dependencies.clear();
      dependencies.emplace_back(std::move(outputFile));
      commandLines.clear();
      autogenComment.clear();
    }

    // Create autogen target
    auto cc = cm::make_unique<cmCustomCommand>();
    cc->SetWorkingDirectory(this->Dir.Work.c_str());
    cc->SetByproducts(autogenByproducts);
    cc->SetDepends(dependencies);
    cc->SetCommandLines(commandLines);
    cc->SetEscapeOldStyle(false);
    cc->SetComment(autogenComment.c_str());
    cmTarget* autogenTarget = this->LocalGen->AddUtilityCommand(
      this->AutogenTarget.Name, true, std::move(cc));
    // Create autogen generator target
    this->LocalGen->AddGeneratorTarget(
      cm::make_unique<cmGeneratorTarget>(autogenTarget, this->LocalGen));

    // Order the autogen target(s) just before the original target.
    cmTarget* orderTarget = timestampTarget ? timestampTarget : autogenTarget;
    // Forward origin utilities to autogen target
    if (this->AutogenTarget.DependOrigin) {
      for (BT<std::pair<std::string, bool>> const& depName :
           this->GenTarget->GetUtilities()) {
        orderTarget->AddUtility(depName.Value.first, false, this->Makefile);
      }
    }

    // Add additional autogen target dependencies to autogen target
    for (cmTarget const* depTarget : this->AutogenTarget.DependTargets) {
      orderTarget->AddUtility(depTarget->GetName(), false, this->Makefile);
    }

    // Set FOLDER property in autogen target
    if (!this->TargetsFolder.empty()) {
      autogenTarget->SetProperty("FOLDER", this->TargetsFolder);
    }

    // Add autogen target to the origin target dependencies
    this->GenTarget->Target->AddUtility(this->AutogenTarget.Name, false,
                                        this->Makefile);

    // Add autogen target to the global autogen target dependencies
    if (this->AutogenTarget.GlobalTarget) {
      this->GlobalInitializer->AddToGlobalAutoGen(this->LocalGen,
                                                  this->AutogenTarget.Name);
    }
  }

  return true;
}

void cmQtAutoGenInitializer::AddCMakeProcessToCommandLines(
  std::string const& infoFile, std::string const& processName,
  cmCustomCommandLines& commandLines)
{
  if (this->CrossConfig && this->UseBetterGraph) {
    commandLines.push_back(cmMakeCommandLine(
      { cmSystemTools::GetCMakeCommand(), "-E", processName, infoFile,
        "$<CONFIG>", "$<COMMAND_CONFIG:$<CONFIG>>" }));
  } else if ((this->MultiConfig && this->GlobalGen->IsXcode()) ||
             this->CrossConfig) {
    for (std::string const& config : this->ConfigsList) {
      commandLines.push_back(
        cmMakeCommandLine({ cmSystemTools::GetCMakeCommand(), "-E",
                            processName, infoFile, config }));
    }
  } else {
    std::string autoInfoFileConfig;
    if (this->MultiConfig) {
      autoInfoFileConfig = "$<CONFIG>";
    } else {
      std::vector<std::string> configs;
      this->GlobalGen->GetQtAutoGenConfigs(configs);
      autoInfoFileConfig = configs[0];
    }
    commandLines.push_back(
      cmMakeCommandLine({ cmSystemTools::GetCMakeCommand(), "-E", processName,
                          infoFile, autoInfoFileConfig }));
  }
}

bool cmQtAutoGenInitializer::InitRccTargets()
{
  for (Qrc const& qrc : this->Rcc.Qrcs) {
    // Register info file as generated by CMake
    this->Makefile->AddCMakeOutputFile(qrc.InfoFile);
    // Register file at target
    {
      cmSourceFile* sf = this->AddGeneratedSource(qrc.OutputFile, this->Rcc);
      sf->SetProperty("SKIP_UNITY_BUILD_INCLUSION", "On");
    }

    std::vector<std::string> ccOutput;
    ccOutput.push_back(qrc.OutputFile);

    std::vector<std::string> ccDepends;
    // Add the .qrc and info file to the custom command dependencies
    ccDepends.push_back(qrc.QrcFile);
    ccDepends.push_back(qrc.InfoFile);

    cmCustomCommandLines commandLines;
    AddCMakeProcessToCommandLines(qrc.InfoFile, "cmake_autorcc", commandLines);

    std::string const ccComment =
      cmStrCat("Automatic RCC for ",
               FileProjectRelativePath(this->Makefile, qrc.QrcFile));

    auto cc = cm::make_unique<cmCustomCommand>();
    cc->SetWorkingDirectory(this->Dir.Work.c_str());
    cc->SetCommandLines(commandLines);
    cc->SetComment(ccComment.c_str());
    cc->SetStdPipesUTF8(true);

    if (qrc.Generated || this->Rcc.GlobalTarget) {
      // Create custom rcc target
      std::string ccName;
      {
        ccName = cmStrCat(this->GenTarget->GetName(), "_arcc_", qrc.QrcName);
        if (!qrc.Unique) {
          ccName += cmStrCat('_', qrc.QrcPathChecksum);
        }

        cc->SetByproducts(ccOutput);
        cc->SetDepends(ccDepends);
        cc->SetEscapeOldStyle(false);
        cmTarget* autoRccTarget =
          this->LocalGen->AddUtilityCommand(ccName, true, std::move(cc));

        // Create autogen generator target
        this->LocalGen->AddGeneratorTarget(
          cm::make_unique<cmGeneratorTarget>(autoRccTarget, this->LocalGen));

        // Set FOLDER property in autogen target
        if (!this->TargetsFolder.empty()) {
          autoRccTarget->SetProperty("FOLDER", this->TargetsFolder);
        }
        if (!this->Rcc.ExecutableTargetName.empty()) {
          autoRccTarget->AddUtility(this->Rcc.ExecutableTargetName, false,
                                    this->Makefile);
        }
      }
      // Add autogen target to the origin target dependencies
      this->GenTarget->Target->AddUtility(ccName, false, this->Makefile);

      // Add autogen target to the global autogen target dependencies
      if (this->Rcc.GlobalTarget) {
        this->GlobalInitializer->AddToGlobalAutoRcc(this->LocalGen, ccName);
      }
    } else {
      // Create custom rcc command
      {
        // Add the resource files to the dependencies
        if (this->MultiConfig && this->UseBetterGraph) {
          for (auto const& config : this->ConfigsList) {
            // Add resource file to the custom command dependencies
            auto resourceFilesWithConfig = cmStrCat(
              "$<$<CONFIG:", config,
              ">:", cmList{ qrc.Resources.Config.at(config) }.to_string(),
              ">");
            ccDepends.emplace_back(std::move(resourceFilesWithConfig));
          }
        } else {
          for (std::string const& fileName : qrc.Resources.Default) {
            // Add resource file to the custom command dependencies
            ccDepends.push_back(fileName);
          }
        }

        if (!this->Rcc.ExecutableTargetName.empty()) {
          ccDepends.push_back(this->Rcc.ExecutableTargetName);
        }

        AddAutogenExecutableToDependencies(this->Rcc, ccDepends);

        cc->SetOutputs(ccOutput);
        cc->SetDepends(ccDepends);
        this->LocalGen->AddCustomCommandToOutput(std::move(cc));
      }
      // Reconfigure when .qrc file changes
      this->Makefile->AddCMakeDependFile(qrc.QrcFile);
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::SetupCustomTargets()
{
  // Create info directory on demand
  if (!cmSystemTools::MakeDirectory(this->Dir.Info)) {
    cmSystemTools::Error(cmStrCat("AutoGen: Could not create directory: ",
                                  Quoted(this->Dir.Info)));
    return false;
  }

  // Generate autogen target info file
  if (this->MocOrUicEnabled()) {
    // Write autogen target info files
    if (!this->SetupWriteAutogenInfo()) {
      return false;
    }
  }

  // Write AUTORCC info files
  return !this->Rcc.Enabled || this->SetupWriteRccInfo();
}

bool cmQtAutoGenInitializer::SetupWriteAutogenInfo()
{
  // Utility lambdas
  auto MfDef = [this](std::string const& key) {
    return this->Makefile->GetSafeDefinition(key);
  };

  // Filtered headers and sources
  std::set<std::string> moc_skip;
  std::set<std::string> uic_skip;
  std::vector<MUFile const*> headers;
  std::vector<MUFile const*> sources;

  // Filter headers
  {
    headers.reserve(this->AutogenTarget.Headers.size());
    for (auto const& pair : this->AutogenTarget.Headers) {
      MUFile const* const muf = pair.second.get();
      if (muf->SkipMoc) {
        moc_skip.insert(muf->FullPath);
      }
      if (muf->SkipUic) {
        uic_skip.insert(muf->FullPath);
      }
      if (muf->Generated && !this->CMP0071Accept) {
        continue;
      }
      if (muf->MocIt || muf->UicIt) {
        headers.emplace_back(muf);
      }
    }
    std::sort(headers.begin(), headers.end(),
              [](MUFile const* a, MUFile const* b) {
                return (a->FullPath < b->FullPath);
              });
  }

  // Filter sources
  {
    sources.reserve(this->AutogenTarget.Sources.size());
    for (auto const& pair : this->AutogenTarget.Sources) {
      MUFile const* const muf = pair.second.get();
      if (muf->Generated && !this->CMP0071Accept) {
        continue;
      }
      if (muf->SkipMoc) {
        moc_skip.insert(muf->FullPath);
      }
      if (muf->SkipUic) {
        uic_skip.insert(muf->FullPath);
      }
      if (muf->MocIt || muf->UicIt) {
        sources.emplace_back(muf);
      }
    }
    std::sort(sources.begin(), sources.end(),
              [](MUFile const* a, MUFile const* b) {
                return (a->FullPath < b->FullPath);
              });
  }

  // Info writer
  InfoWriter info;

  // General
  info.SetBool("MULTI_CONFIG", this->MultiConfig);
  info.SetBool("CROSS_CONFIG", this->CrossConfig);
  info.SetBool("USE_BETTER_GRAPH", this->UseBetterGraph);
  info.SetUInt("PARALLEL", this->AutogenTarget.Parallel);
#ifdef _WIN32
  info.SetUInt("AUTOGEN_COMMAND_LINE_LENGTH_MAX",
               this->AutogenTarget.MaxCommandLineLength);
#endif
  info.SetUInt("VERBOSITY", this->Verbosity);

  // Directories
  info.Set("CMAKE_SOURCE_DIR", MfDef("CMAKE_SOURCE_DIR"));
  info.Set("CMAKE_BINARY_DIR", MfDef("CMAKE_BINARY_DIR"));
  info.Set("CMAKE_CURRENT_SOURCE_DIR", MfDef("CMAKE_CURRENT_SOURCE_DIR"));
  info.Set("CMAKE_CURRENT_BINARY_DIR", MfDef("CMAKE_CURRENT_BINARY_DIR"));
  info.Set("BUILD_DIR", this->Dir.Build);
  info.SetConfig("INCLUDE_DIR", this->Dir.Include);

  info.SetUInt("QT_VERSION_MAJOR", this->QtVersion.Major);
  info.SetUInt("QT_VERSION_MINOR", this->QtVersion.Minor);
  info.SetConfig("QT_MOC_EXECUTABLE", this->Moc.Executable);
  info.SetConfig("QT_UIC_EXECUTABLE", this->Uic.Executable);

  info.Set("CMAKE_EXECUTABLE", cmSystemTools::GetCMakeCommand());
  info.SetConfig("SETTINGS_FILE", this->AutogenTarget.SettingsFile);
  info.SetConfig("PARSE_CACHE_FILE", this->AutogenTarget.ParseCacheFile);
  info.SetConfig("DEP_FILE", this->AutogenTarget.DepFile);
  info.SetConfig("DEP_FILE_RULE_NAME", this->AutogenTarget.DepFileRuleName);
  info.SetArray("CMAKE_LIST_FILES", this->Makefile->GetListFiles());
  info.SetArray("HEADER_EXTENSIONS",
                this->Makefile->GetCMakeInstance()->GetHeaderExtensions());
  auto cfgArray = [this](std::vector<size_t> const& configs) -> Json::Value {
    Json::Value value;
    if (!configs.empty()) {
      value = Json::arrayValue;
      for (size_t ci : configs) {
        value.append(this->ConfigsList[ci]);
      }
    }
    return value;
  };
  info.SetArrayArray("HEADERS", headers,
                     [this, &cfgArray](Json::Value& jval, MUFile const* muf) {
                       jval.resize(4u);
                       jval[0u] = muf->FullPath;
                       jval[1u] = cmStrCat(muf->MocIt ? 'M' : 'm',
                                           muf->UicIt ? 'U' : 'u');
                       jval[2u] = this->GetMocBuildPath(*muf);
                       jval[3u] = cfgArray(muf->Configs);
                     });
  info.SetArrayArray(
    "SOURCES", sources, [&cfgArray](Json::Value& jval, MUFile const* muf) {
      jval.resize(3u);
      jval[0u] = muf->FullPath;
      jval[1u] = cmStrCat(muf->MocIt ? 'M' : 'm', muf->UicIt ? 'U' : 'u');
      jval[2u] = cfgArray(muf->Configs);
    });

  // Write moc settings
  if (this->Moc.Enabled) {
    info.SetArray("MOC_SKIP", moc_skip);
    info.SetConfigArray("MOC_DEFINITIONS", this->Moc.Defines);
    info.SetConfigArray("MOC_INCLUDES", this->Moc.Includes);
    info.SetArray("MOC_OPTIONS", this->Moc.Options);
    info.SetBool("MOC_RELAXED_MODE", this->Moc.RelaxedMode);
    info.SetBool("MOC_PATH_PREFIX", this->Moc.PathPrefix);

    cmGeneratorExpressionDAGChecker dagChecker(
      this->GenTarget, "AUTOMOC_MACRO_NAMES", nullptr, nullptr);
    EvaluatedTargetPropertyEntries InterfaceAutoMocMacroNamesEntries;

    if (this->MultiConfig) {
      for (auto const& cfg : this->ConfigsList) {
        if (!cfg.empty()) {
          AddInterfaceEntries(this->GenTarget, cfg,
                              "INTERFACE_AUTOMOC_MACRO_NAMES", "CXX",
                              &dagChecker, InterfaceAutoMocMacroNamesEntries,
                              IncludeRuntimeInterface::Yes);
        }
      }
    } else {
      AddInterfaceEntries(this->GenTarget, this->ConfigDefault,
                          "INTERFACE_AUTOMOC_MACRO_NAMES", "CXX", &dagChecker,
                          InterfaceAutoMocMacroNamesEntries,
                          IncludeRuntimeInterface::Yes);
    }

    for (auto const& entry : InterfaceAutoMocMacroNamesEntries.Entries) {
      this->Moc.MacroNames.insert(this->Moc.MacroNames.end(),
                                  entry.Values.begin(), entry.Values.end());
    }
    this->Moc.MacroNames.erase(cmRemoveDuplicates(this->Moc.MacroNames),
                               this->Moc.MacroNames.end());

    info.SetArray("MOC_MACRO_NAMES", this->Moc.MacroNames);
    info.SetArrayArray(
      "MOC_DEPEND_FILTERS", this->Moc.DependFilters,
      [](Json::Value& jval, std::pair<std::string, std::string> const& pair) {
        jval.resize(2u);
        jval[0u] = pair.first;
        jval[1u] = pair.second;
      });
    info.SetConfig("MOC_COMPILATION_FILE", this->Moc.CompilationFile);
    info.SetConfig("MOC_PREDEFS_FILE", this->Moc.PredefsFile);

    cmStandardLevelResolver const resolver{ this->Makefile };
    auto const CompileOptionFlag =
      resolver.GetCompileOptionDef(this->GenTarget, "CXX", "");

    auto const CompileOptionValue =
      this->GenTarget->Makefile->GetSafeDefinition(CompileOptionFlag);

    if (!CompileOptionValue.empty()) {
      if (this->Moc.PredefsCmd.size() >= 3) {
        this->Moc.PredefsCmd.insert(this->Moc.PredefsCmd.begin() + 1,
                                    CompileOptionValue);
      }
    }
    info.SetArray("MOC_PREDEFS_CMD", this->Moc.PredefsCmd);
  }

  // Write uic settings
  if (this->Uic.Enabled) {
    // Add skipped .ui files
    uic_skip.insert(this->Uic.SkipUi.begin(), this->Uic.SkipUi.end());

    info.SetArray("UIC_SKIP", uic_skip);
    info.SetArrayArray("UIC_UI_FILES", this->Uic.UiFilesWithOptions,
                       [](Json::Value& jval, UicT::UiFileT const& uiFile) {
                         jval.resize(2u);
                         jval[0u] = uiFile.first;
                         InfoWriter::MakeStringArray(jval[1u], uiFile.second);
                       });
    info.SetConfigArray("UIC_OPTIONS", this->Uic.Options);
    info.SetArray("UIC_SEARCH_PATHS", this->Uic.SearchPaths);
  }

  info.Save(this->AutogenTarget.InfoFile);

  return true;
}

bool cmQtAutoGenInitializer::SetupWriteRccInfo()
{
  for (Qrc const& qrc : this->Rcc.Qrcs) {
    // Utility lambdas
    auto MfDef = [this](std::string const& key) {
      return this->Makefile->GetSafeDefinition(key);
    };

    InfoWriter info;

    // General
    info.SetBool("MULTI_CONFIG", this->MultiConfig);
    info.SetBool("CROSS_CONFIG", this->CrossConfig);
    info.SetBool("USE_BETTER_GRAPH", this->UseBetterGraph);
    info.SetUInt("VERBOSITY", this->Verbosity);
    info.Set("GENERATOR", this->GlobalGen->GetName());

    // Files
    info.Set("LOCK_FILE", qrc.LockFile);
    info.SetConfig("SETTINGS_FILE", qrc.SettingsFile);

    // Directories
    info.Set("CMAKE_SOURCE_DIR", MfDef("CMAKE_SOURCE_DIR"));
    info.Set("CMAKE_BINARY_DIR", MfDef("CMAKE_BINARY_DIR"));
    info.Set("CMAKE_CURRENT_SOURCE_DIR", MfDef("CMAKE_CURRENT_SOURCE_DIR"));
    info.Set("CMAKE_CURRENT_BINARY_DIR", MfDef("CMAKE_CURRENT_BINARY_DIR"));
    info.Set("BUILD_DIR", this->Dir.Build);
    info.SetConfig("INCLUDE_DIR", this->Dir.Include);

    // rcc executable
    info.SetConfig("RCC_EXECUTABLE", this->Rcc.Executable);
    info.SetConfigArray(
      "RCC_LIST_OPTIONS",
      generateListOptions(this->Rcc.ExecutableFeatures, this->MultiConfig));

    // qrc file
    info.Set("SOURCE", qrc.QrcFile);
    info.Set("OUTPUT_CHECKSUM", qrc.QrcPathChecksum);
    info.Set("OUTPUT_NAME", cmSystemTools::GetFilenameName(qrc.OutputFile));
    info.SetArray("OPTIONS", qrc.Options);
    info.SetConfigArray("INPUTS", qrc.Resources);

    info.Save(qrc.InfoFile);
  }

  return true;
}

cmSourceFile* cmQtAutoGenInitializer::RegisterGeneratedSource(
  std::string const& filename)
{
  cmSourceFile* gFile = this->Makefile->GetOrCreateSource(filename, true);
  gFile->MarkAsGenerated();
  gFile->SetProperty("SKIP_AUTOGEN", "1");
  gFile->SetProperty("SKIP_LINTING", "ON");
  gFile->SetProperty("CXX_SCAN_FOR_MODULES", "0");
  return gFile;
}

cmSourceFile* cmQtAutoGenInitializer::AddGeneratedSource(
  std::string const& filename, GenVarsT const& genVars, bool prepend)
{
  // Register source at makefile
  cmSourceFile* gFile = this->RegisterGeneratedSource(filename);
  // Add source file to target
  this->GenTarget->AddSource(filename, prepend);

  // Add source file to source group
  this->AddToSourceGroup(filename, genVars.GenNameUpper);

  return gFile;
}

void cmQtAutoGenInitializer::AddGeneratedSource(ConfigString const& filename,
                                                GenVarsT const& genVars,
                                                bool prepend)
{
  // XXX(xcode-per-cfg-src): Drop the Xcode-specific part of the condition
  // when the Xcode generator supports per-config sources.
  if (!this->MultiConfig || this->GlobalGen->IsXcode()) {
    cmSourceFile* sf =
      this->AddGeneratedSource(filename.Default, genVars, prepend);
    handleSkipPch(sf);
    return;
  }
  for (auto const& cfg : this->ConfigsList) {
    std::string const& filenameCfg = filename.Config.at(cfg);
    // Register source at makefile
    cmSourceFile* sf = this->RegisterGeneratedSource(filenameCfg);
    handleSkipPch(sf);
    // Add source file to target for this configuration.
    this->GenTarget->AddSource(
      cmStrCat("$<$<CONFIG:"_s, cfg, ">:"_s, filenameCfg, ">"_s), prepend);
    // Add source file to source group
    this->AddToSourceGroup(filenameCfg, genVars.GenNameUpper);
  }
}

void cmQtAutoGenInitializer::AddToSourceGroup(std::string const& fileName,
                                              cm::string_view genNameUpper)
{
  cmSourceGroup* sourceGroup = nullptr;
  // Acquire source group
  {
    std::string property;
    std::string groupName;
    {
      // Prefer generator specific source group name
      std::initializer_list<std::string> const props{
        cmStrCat(genNameUpper, "_SOURCE_GROUP"), "AUTOGEN_SOURCE_GROUP"
      };
      for (std::string const& prop : props) {
        cmValue propName = this->Makefile->GetState()->GetGlobalProperty(prop);
        if (cmNonempty(propName)) {
          groupName = *propName;
          property = prop;
          break;
        }
      }
    }
    // Generate a source group on demand
    if (!groupName.empty()) {
      sourceGroup = this->Makefile->GetOrCreateSourceGroup(groupName);
      if (!sourceGroup) {
        cmSystemTools::Error(
          cmStrCat(genNameUpper, " error in ", property,
                   ": Could not find or create the source group ",
                   cmQtAutoGen::Quoted(groupName)));
      }
    }
  }
  if (sourceGroup) {
    sourceGroup->AddGroupFile(fileName);
  }
}

void cmQtAutoGenInitializer::AddCleanFile(std::string const& fileName)
{
  this->GenTarget->Target->AppendProperty("ADDITIONAL_CLEAN_FILES", fileName);
}

void cmQtAutoGenInitializer::ConfigFileNames(ConfigString& configString,
                                             cm::string_view prefix,
                                             cm::string_view suffix)
{
  configString.Default = cmStrCat(prefix, suffix);
  if (this->MultiConfig) {
    for (auto const& cfg : this->ConfigsList) {
      configString.Config[cfg] = cmStrCat(prefix, '_', cfg, suffix);
    }
  }
}

void cmQtAutoGenInitializer::ConfigFileNamesAndGenex(
  ConfigString& configString, std::string& genex, cm::string_view const prefix,
  cm::string_view const suffix)
{
  this->ConfigFileNames(configString, prefix, suffix);
  if (this->MultiConfig) {
    genex = cmStrCat(prefix, "_$<CONFIG>"_s, suffix);
  } else {
    genex = configString.Default;
  }
}

void cmQtAutoGenInitializer::ConfigFileClean(ConfigString& configString)
{
  this->AddCleanFile(configString.Default);
  if (this->MultiConfig) {
    for (auto const& pair : configString.Config) {
      this->AddCleanFile(pair.second);
    }
  }
}

static cmQtAutoGen::IntegerVersion parseMocVersion(std::string str)
{
  cmQtAutoGen::IntegerVersion result;

  static const std::string prelude = "moc ";
  size_t const pos = str.find(prelude);
  if (pos == std::string::npos) {
    return result;
  }

  str.erase(0, prelude.size() + pos);
  std::istringstream iss(str);
  std::string major;
  std::string minor;
  if (!std::getline(iss, major, '.') || !std::getline(iss, minor, '.')) {
    return result;
  }

  result.Major = static_cast<unsigned int>(std::stoi(major));
  result.Minor = static_cast<unsigned int>(std::stoi(minor));
  return result;
}

static cmQtAutoGen::IntegerVersion GetMocVersion(
  const std::string& mocExecutablePath)
{
  std::string capturedStdOut;
  int exitCode;
  if (!cmSystemTools::RunSingleCommand({ mocExecutablePath, "--version" },
                                       &capturedStdOut, nullptr, &exitCode,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    return {};
  }

  if (exitCode != 0) {
    return {};
  }

  return parseMocVersion(capturedStdOut);
}

static std::string FindMocExecutableFromMocTarget(cmMakefile const* makefile,
                                                  unsigned int qtMajorVersion)
{
  std::string result;
  const std::string mocTargetName =
    "Qt" + std::to_string(qtMajorVersion) + "::moc";
  cmTarget const* mocTarget = makefile->FindTargetToUse(mocTargetName);
  if (mocTarget) {
    result = mocTarget->GetSafeProperty("IMPORTED_LOCATION");
  }
  return result;
}

std::pair<cmQtAutoGen::IntegerVersion, unsigned int>
cmQtAutoGenInitializer::GetQtVersion(cmGeneratorTarget const* target,
                                     std::string mocExecutable)
{
  // Converts a char ptr to an unsigned int value
  auto toUInt = [](const char* const input) -> unsigned int {
    unsigned long tmp = 0;
    if (input && cmStrToULong(input, &tmp)) {
      return static_cast<unsigned int>(tmp);
    }
    return 0u;
  };
  auto toUInt2 = [](cmValue input) -> unsigned int {
    unsigned long tmp = 0;
    if (input && cmStrToULong(*input, &tmp)) {
      return static_cast<unsigned int>(tmp);
    }
    return 0u;
  };

  // Initialize return value to a default
  std::pair<IntegerVersion, unsigned int> res(
    IntegerVersion(),
    toUInt(target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION",
                                                           "")));

  // Acquire known Qt versions
  std::vector<cmQtAutoGen::IntegerVersion> knownQtVersions;
  {
    // Qt version variable prefixes
    static std::initializer_list<
      std::pair<cm::string_view, cm::string_view>> const keys{
      { "Qt6Core_VERSION_MAJOR", "Qt6Core_VERSION_MINOR" },
      { "Qt5Core_VERSION_MAJOR", "Qt5Core_VERSION_MINOR" },
      { "QT_VERSION_MAJOR", "QT_VERSION_MINOR" },
    };

    knownQtVersions.reserve(keys.size() * 2);

    // Adds a version to the result (nullptr safe)
    auto addVersion = [&knownQtVersions, &toUInt2](cmValue major,
                                                   cmValue minor) {
      cmQtAutoGen::IntegerVersion ver(toUInt2(major), toUInt2(minor));
      if (ver.Major != 0) {
        knownQtVersions.emplace_back(ver);
      }
    };

    // Read versions from variables
    for (auto const& keyPair : keys) {
      addVersion(target->Makefile->GetDefinition(std::string(keyPair.first)),
                 target->Makefile->GetDefinition(std::string(keyPair.second)));
    }

    // Read versions from directory properties
    for (auto const& keyPair : keys) {
      addVersion(target->Makefile->GetProperty(std::string(keyPair.first)),
                 target->Makefile->GetProperty(std::string(keyPair.second)));
    }
  }

  // Evaluate known Qt versions
  if (!knownQtVersions.empty()) {
    if (res.second == 0) {
      // No specific version was requested by the target:
      // Use highest known Qt version.
      res.first = knownQtVersions.at(0);
    } else {
      // Pick a version from the known versions:
      for (auto const& it : knownQtVersions) {
        if (it.Major == res.second) {
          res.first = it;
          break;
        }
      }
    }
  }

  if (res.first.Major == 0) {
    // We could not get the version number from variables or directory
    // properties. This might happen if the find_package call for Qt is wrapped
    // in a function. Try to find the moc executable path from the available
    // targets and call "moc --version" to get the Qt version.
    if (mocExecutable.empty()) {
      mocExecutable =
        FindMocExecutableFromMocTarget(target->Makefile, res.second);
    }
    if (!mocExecutable.empty()) {
      res.first = GetMocVersion(mocExecutable);
    }
  }

  return res;
}

std::string cmQtAutoGenInitializer::GetMocBuildPath(MUFile const& muf)
{
  std::string res;
  if (!muf.MocIt) {
    return res;
  }

  std::string basePath =
    cmStrCat(this->PathCheckSum.getPart(muf.FullPath), "/moc_",
             FileNameWithoutLastExtension(muf.FullPath));

  res = cmStrCat(basePath, ".cpp");
  if (this->Moc.EmittedBuildPaths.emplace(res).second) {
    return res;
  }

  // File name already emitted.
  // Try appending the header suffix to the base path.
  basePath = cmStrCat(basePath, '_', muf.SF->GetExtension());
  res = cmStrCat(basePath, ".cpp");
  if (this->Moc.EmittedBuildPaths.emplace(res).second) {
    return res;
  }

  // File name with header extension already emitted.
  // Try adding a number to the base path.
  constexpr std::size_t number_begin = 2;
  constexpr std::size_t number_end = 256;
  for (std::size_t ii = number_begin; ii != number_end; ++ii) {
    res = cmStrCat(basePath, '_', ii, ".cpp");
    if (this->Moc.EmittedBuildPaths.emplace(res).second) {
      return res;
    }
  }

  // Output file name conflict (unlikely, but still...)
  cmSystemTools::Error(
    cmStrCat("moc output file name conflict for ", muf.FullPath));

  return res;
}

bool cmQtAutoGenInitializer::GetQtExecutable(GenVarsT& genVars,
                                             const std::string& executable,
                                             bool ignoreMissingTarget) const
{
  auto print_err = [this, &genVars](std::string const& err) {
    cmSystemTools::Error(cmStrCat(genVars.GenNameUpper, " for target ",
                                  this->GenTarget->GetName(), ": ", err));
  };

  // Custom executable
  {
    std::string const prop = cmStrCat(genVars.GenNameUpper, "_EXECUTABLE");
    std::string const& val = this->GenTarget->Target->GetSafeProperty(prop);
    if (!val.empty()) {
      // Evaluate generator expression
      {
        cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
        cmGeneratorExpression ge(*this->Makefile->GetCMakeInstance(), lfbt);
        std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(val);
        if (this->MultiConfig && this->UseBetterGraph) {
          for (auto const& config : this->ConfigsList) {
            genVars.Executable.Config[config] =
              cge->Evaluate(this->LocalGen, config);
          }
        } else {
          genVars.Executable.Default = cge->Evaluate(this->LocalGen, "");
        }
      }

      if (genVars.Executable.Default.empty() &&
          genVars.Executable.Config.empty() && !ignoreMissingTarget) {
        print_err(prop + " evaluates to an empty value");
        return false;
      }

      // Create empty compiler features.
      if (this->MultiConfig && this->UseBetterGraph) {
        for (auto const& config : this->ConfigsList) {
          genVars.ExecutableFeatures.Config[config] =
            std::make_shared<cmQtAutoGen::CompilerFeatures>();
        }
      } else {
        genVars.ExecutableFeatures.Default =
          std::make_shared<cmQtAutoGen::CompilerFeatures>();
      }
      return true;
    }
  }

  // Find executable target
  {
    // Find executable target name
    cm::string_view prefix;
    if (this->QtVersion.Major == 4) {
      prefix = "Qt4::";
    } else if (this->QtVersion.Major == 5) {
      prefix = "Qt5::";
    } else if (this->QtVersion.Major == 6) {
      prefix = "Qt6::";
    }
    std::string const targetName = cmStrCat(prefix, executable);

    // Find target
    cmGeneratorTarget* genTarget =
      this->LocalGen->FindGeneratorTargetToUse(targetName);
    if (genTarget) {
      genVars.ExecutableTargetName = targetName;
      genVars.ExecutableTarget = genTarget;
      if (genTarget->IsImported()) {
        if (this->MultiConfig && this->UseBetterGraph) {
          for (auto const& config : this->ConfigsList) {
            genVars.Executable.Config[config] =
              genTarget->ImportedGetLocation(config);
          }
        } else {
          genVars.Executable.Default =
            genTarget->ImportedGetLocation(this->ConfigDefault);
        }

      } else {
        if (this->MultiConfig && this->UseBetterGraph) {
          for (auto const& config : this->ConfigsList) {
            genVars.Executable.Config[config] = genTarget->GetLocation(config);
          }
        } else {
          genVars.Executable.Default =
            genTarget->GetLocation(this->ConfigDefault);
        }
      }
    } else {
      if (ignoreMissingTarget) {
        // Create empty compiler features.
        if (this->MultiConfig && this->UseBetterGraph) {
          for (auto const& config : this->ConfigsList) {
            genVars.ExecutableFeatures.Config[config] =
              std::make_shared<cmQtAutoGen::CompilerFeatures>();
          }
        } else {
          genVars.ExecutableFeatures.Default =
            std::make_shared<cmQtAutoGen::CompilerFeatures>();
        }

        return true;
      }
      print_err(cmStrCat("Could not find ", executable, " executable target ",
                         targetName));
      return false;
    }
  }

  // Get executable features
  {
    std::string err;
    genVars.ExecutableFeatures = this->GlobalInitializer->GetCompilerFeatures(
      executable, genVars.Executable, err, this->MultiConfig,
      this->UseBetterGraph);
    if (this->MultiConfig && this->UseBetterGraph) {
      for (auto const& config : this->ConfigsList) {
        if (!genVars.ExecutableFeatures.Config[config]) {
          if (!genVars.ExecutableFeatures.Config[config]) {
            print_err(err);
            return false;
          }
        }
      }
    } else {
      if (!genVars.ExecutableFeatures.Default) {
        print_err(err);
        return false;
      }
    }
  }

  return true;
}

void cmQtAutoGenInitializer::handleSkipPch(cmSourceFile* sf)
{
  bool skipPch = true;
  for (auto const& pair : this->AutogenTarget.Sources) {
    if (!pair.first->GetIsGenerated() &&
        !pair.first->GetProperty("SKIP_PRECOMPILE_HEADERS")) {
      skipPch = false;
    }
  }

  if (skipPch) {
    sf->SetProperty("SKIP_PRECOMPILE_HEADERS", "ON");
  }
}
