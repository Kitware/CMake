/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmQtAutoGeneratorInitializer.h"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmFilePathChecksum.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cm_sys_stat.h"
#include "cmake.h"
#include "cmsys/FStream.hxx"

#include <algorithm>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

inline static const char* SafeString(const char* value)
{
  return (value != nullptr) ? value : "";
}

inline static std::string GetSafeProperty(cmGeneratorTarget const* target,
                                          const char* key)
{
  return std::string(SafeString(target->GetProperty(key)));
}

inline static std::string GetSafeProperty(cmSourceFile const* sf,
                                          const char* key)
{
  return std::string(SafeString(sf->GetProperty(key)));
}

static cmQtAutoGen::MultiConfig AutogenMultiConfig(
  cmGlobalGenerator* globalGen)
{
  if (!globalGen->IsMultiConfig()) {
    return cmQtAutoGen::SINGLE;
  }

  // FIXME: Xcode does not support per-config sources, yet.
  //        (EXCLUDED_SOURCE_FILE_NAMES)
  // if (globalGen->GetName().find("Xcode") != std::string::npos) {
  //  return cmQtAutoGen::FULL;
  //}

  // FIXME: Visual Studio does not support per-config sources, yet.
  //        (EXCLUDED_SOURCE_FILE_NAMES)
  // if (globalGen->GetName().find("Visual Studio") != std::string::npos) {
  //  return cmQtAutoGen::FULL;
  //}

  return cmQtAutoGen::WRAP;
}

static std::string GetAutogenTargetName(cmGeneratorTarget const* target)
{
  std::string autogenTargetName = target->GetName();
  autogenTargetName += "_autogen";
  return autogenTargetName;
}

static std::string GetAutogenTargetFilesDir(cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string targetDir = makefile->GetCurrentBinaryDirectory();
  targetDir += makefile->GetCMakeInstance()->GetCMakeFilesDirectory();
  targetDir += "/";
  targetDir += GetAutogenTargetName(target);
  targetDir += ".dir";
  return targetDir;
}

static std::string GetAutogenTargetBuildDir(cmGeneratorTarget const* target)
{
  std::string targetDir = GetSafeProperty(target, "AUTOGEN_BUILD_DIR");
  if (targetDir.empty()) {
    cmMakefile* makefile = target->Target->GetMakefile();
    targetDir = makefile->GetCurrentBinaryDirectory();
    targetDir += "/";
    targetDir += GetAutogenTargetName(target);
  }
  return targetDir;
}

std::string cmQtAutoGeneratorInitializer::GetQtMajorVersion(
  cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string qtMajor = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajor.empty()) {
    qtMajor = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
  }
  const char* targetQtVersion =
    target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", "");
  if (targetQtVersion != nullptr) {
    qtMajor = targetQtVersion;
  }
  return qtMajor;
}

std::string cmQtAutoGeneratorInitializer::GetQtMinorVersion(
  cmGeneratorTarget const* target, std::string const& qtVersionMajor)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string qtMinor;
  if (qtVersionMajor == "5") {
    qtMinor = makefile->GetSafeDefinition("Qt5Core_VERSION_MINOR");
  }
  if (qtMinor.empty()) {
    qtMinor = makefile->GetSafeDefinition("QT_VERSION_MINOR");
  }

  const char* targetQtVersion =
    target->GetLinkInterfaceDependentStringProperty("QT_MINOR_VERSION", "");
  if (targetQtVersion != nullptr) {
    qtMinor = targetQtVersion;
  }
  return qtMinor;
}

static bool QtVersionGreaterOrEqual(std::string const& major,
                                    std::string const& minor,
                                    unsigned long requestMajor,
                                    unsigned long requestMinor)
{
  unsigned long majorUL(0);
  unsigned long minorUL(0);
  if (cmSystemTools::StringToULong(major.c_str(), &majorUL) &&
      cmSystemTools::StringToULong(minor.c_str(), &minorUL)) {
    return (majorUL > requestMajor) ||
      (majorUL == requestMajor && minorUL >= requestMinor);
  }
  return false;
}

static void GetConfigs(cmMakefile* makefile, std::string& configDefault,
                       std::vector<std::string>& configsList)
{
  configDefault = makefile->GetConfigurations(configsList);
  if (configsList.empty()) {
    configsList.push_back(configDefault);
  }
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 std::string const& value)
{
  makefile->AddDefinition(key,
                          cmOutputConverter::EscapeForCMake(value).c_str());
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 const std::vector<std::string>& values)
{
  makefile->AddDefinition(
    key, cmOutputConverter::EscapeForCMake(cmJoin(values, ";")).c_str());
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 const std::set<std::string>& values)
{
  makefile->AddDefinition(
    key, cmOutputConverter::EscapeForCMake(cmJoin(values, ";")).c_str());
}

static void AddDefinitionEscaped(
  cmMakefile* makefile, const char* key,
  const std::vector<std::vector<std::string>>& lists)
{
  std::vector<std::string> seplist;
  for (const std::vector<std::string>& list : lists) {
    std::string blist = "{";
    blist += cmJoin(list, ";");
    blist += "}";
    seplist.push_back(std::move(blist));
  }
  makefile->AddDefinition(key, cmOutputConverter::EscapeForCMake(
                                 cmJoin(seplist, cmQtAutoGen::listSep))
                                 .c_str());
}

static bool AddToSourceGroup(cmMakefile* makefile, std::string const& fileName,
                             cmQtAutoGen::Generator genType)
{
  cmSourceGroup* sourceGroup = nullptr;
  // Acquire source group
  {
    std::string property;
    std::string groupName;
    {
      std::array<std::string, 2> props;
      // Use generator specific group name
      switch (genType) {
        case cmQtAutoGen::MOC:
          props[0] = "AUTOMOC_SOURCE_GROUP";
          break;
        case cmQtAutoGen::RCC:
          props[0] = "AUTORCC_SOURCE_GROUP";
          break;
        default:
          props[0] = "AUTOGEN_SOURCE_GROUP";
          break;
      }
      props[1] = "AUTOGEN_SOURCE_GROUP";
      for (std::string& prop : props) {
        const char* propName = makefile->GetState()->GetGlobalProperty(prop);
        if ((propName != nullptr) && (*propName != '\0')) {
          groupName = propName;
          property = std::move(prop);
          break;
        }
      }
    }
    // Generate a source group on demand
    if (!groupName.empty()) {
      sourceGroup = makefile->GetOrCreateSourceGroup(groupName);
      if (sourceGroup == nullptr) {
        std::ostringstream ost;
        ost << cmQtAutoGen::GeneratorNameUpper(genType);
        ost << ": " << property;
        ost << ": Could not find or create the source group ";
        ost << cmQtAutoGen::Quoted(groupName);
        cmSystemTools::Error(ost.str().c_str());
        return false;
      }
    }
  }
  if (sourceGroup != nullptr) {
    sourceGroup->AddGroupFile(fileName);
  }
  return true;
}

static void AddCleanFile(cmMakefile* makefile, std::string const& fileName)
{
  makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES", fileName.c_str(),
                           false);
}

static std::vector<std::string> AddGeneratedSource(
  cmGeneratorTarget* target, std::string const& filename,
  cmQtAutoGen::MultiConfig multiConfig,
  const std::vector<std::string>& configsList, cmQtAutoGen::Generator genType)
{
  std::vector<std::string> genFiles;
  // Register source file in makefile and source group
  if (multiConfig != cmQtAutoGen::FULL) {
    genFiles.push_back(filename);
  } else {
    for (std::string const& cfg : configsList) {
      genFiles.push_back(
        cmQtAutoGen::AppendFilenameSuffix(filename, "_" + cfg));
    }
  }
  {
    cmMakefile* makefile = target->Target->GetMakefile();
    for (std::string const& genFile : genFiles) {
      {
        cmSourceFile* gFile = makefile->GetOrCreateSource(genFile, true);
        gFile->SetProperty("GENERATED", "1");
        gFile->SetProperty("SKIP_AUTOGEN", "On");
      }
      AddToSourceGroup(makefile, genFile, genType);
    }
  }

  // Add source file to target
  if (multiConfig != cmQtAutoGen::FULL) {
    target->AddSource(filename);
  } else {
    for (std::string const& cfg : configsList) {
      std::string src = "$<$<CONFIG:";
      src += cfg;
      src += ">:";
      src += cmQtAutoGen::AppendFilenameSuffix(filename, "_" + cfg);
      src += ">";
      target->AddSource(src);
    }
  }

  return genFiles;
}

/* @brief Tests if targetDepend is a STATIC_LIBRARY and if any of its
 * recursive STATIC_LIBRARY dependencies depends on targetOrigin
 * (STATIC_LIBRARY cycle).
 */
static bool StaticLibraryCycle(cmGeneratorTarget const* targetOrigin,
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
        testTarget->GetLinkImplementationLibraries(config);
      if (libs != nullptr) {
        for (cmLinkItem const& item : libs->Libraries) {
          cmGeneratorTarget const* depTarget = item.Target;
          if ((depTarget != nullptr) &&
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

struct cmQtAutoGenSetup
{
  std::set<std::string> MocSkip;
  std::set<std::string> UicSkip;

  std::map<std::string, std::string> ConfigMocIncludes;
  std::map<std::string, std::string> ConfigMocDefines;
  std::map<std::string, std::string> ConfigUicOptions;
};

static void SetupAcquireSkipFiles(cmQtAutoGenDigest const& digest,
                                  cmQtAutoGenSetup& setup)
{
  // Read skip files from makefile sources
  {
    std::string pathError;
    for (cmSourceFile* sf : digest.Target->Makefile->GetSourceFiles()) {
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      // Since we're iterating over source files that might be not in the
      // target we need to check for path errors (not existing files).
      std::string const& fPath = sf->GetFullPath(&pathError);
      if (!pathError.empty()) {
        pathError.clear();
        continue;
      }
      cmSystemTools::FileFormat const fileType =
        cmSystemTools::GetFileFormat(sf->GetExtension().c_str());
      if (!(fileType == cmSystemTools::CXX_FILE_FORMAT) &&
          !(fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
        continue;
      }
      const bool skipAll = sf->GetPropertyAsBool("SKIP_AUTOGEN");
      const bool mocSkip = digest.MocEnabled &&
        (skipAll || sf->GetPropertyAsBool("SKIP_AUTOMOC"));
      const bool uicSkip = digest.UicEnabled &&
        (skipAll || sf->GetPropertyAsBool("SKIP_AUTOUIC"));
      if (mocSkip || uicSkip) {
        std::string const absFile = cmSystemTools::GetRealPath(fPath);
        if (mocSkip) {
          setup.MocSkip.insert(absFile);
        }
        if (uicSkip) {
          setup.UicSkip.insert(absFile);
        }
      }
    }
  }
}

static void SetupAutoTargetMoc(cmQtAutoGenDigest const& digest,
                               std::string const& configDefault,
                               std::vector<std::string> const& configsList,
                               cmQtAutoGenSetup& setup)
{
  cmGeneratorTarget const* target = digest.Target;
  cmLocalGenerator* localGen = target->GetLocalGenerator();
  cmMakefile* makefile = target->Target->GetMakefile();

  AddDefinitionEscaped(makefile, "_moc_skip", setup.MocSkip);
  AddDefinitionEscaped(makefile, "_moc_options",
                       GetSafeProperty(target, "AUTOMOC_MOC_OPTIONS"));
  AddDefinitionEscaped(makefile, "_moc_relaxed_mode",
                       makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE") ? "TRUE"
                                                                    : "FALSE");
  AddDefinitionEscaped(makefile, "_moc_macro_names",
                       GetSafeProperty(target, "AUTOMOC_MACRO_NAMES"));
  AddDefinitionEscaped(makefile, "_moc_depend_filters",
                       GetSafeProperty(target, "AUTOMOC_DEPEND_FILTERS"));

  // Compiler predefines
  if (target->GetPropertyAsBool("AUTOMOC_COMPILER_PREDEFINES")) {
    if (QtVersionGreaterOrEqual(digest.QtVersionMajor, digest.QtVersionMinor,
                                5, 8)) {
      AddDefinitionEscaped(
        makefile, "_moc_predefs_cmd",
        makefile->GetSafeDefinition("CMAKE_CXX_COMPILER_PREDEFINES_COMMAND"));
    }
  }
  // Moc includes and compile definitions
  {
    auto GetIncludeDirs = [target,
                           localGen](std::string const& cfg) -> std::string {
      // Get the include dirs for this target, without stripping the implicit
      // include dirs off, see
      // https://gitlab.kitware.com/cmake/cmake/issues/13667
      std::vector<std::string> includeDirs;
      localGen->GetIncludeDirectories(includeDirs, target, "CXX", cfg, false);
      return cmJoin(includeDirs, ";");
    };
    auto GetCompileDefinitions =
      [target, localGen](std::string const& cfg) -> std::string {
      std::set<std::string> defines;
      localGen->AddCompileDefinitions(defines, target, cfg, "CXX");
      return cmJoin(defines, ";");
    };

    // Default configuration settings
    std::string const includeDirs = GetIncludeDirs(configDefault);
    std::string const compileDefs = GetCompileDefinitions(configDefault);
    // Other configuration settings
    for (std::string const& cfg : configsList) {
      {
        std::string const configIncludeDirs = GetIncludeDirs(cfg);
        if (configIncludeDirs != includeDirs) {
          setup.ConfigMocIncludes[cfg] = configIncludeDirs;
        }
      }
      {
        std::string const configCompileDefs = GetCompileDefinitions(cfg);
        if (configCompileDefs != compileDefs) {
          setup.ConfigMocDefines[cfg] = configCompileDefs;
        }
      }
    }
    AddDefinitionEscaped(makefile, "_moc_include_dirs", includeDirs);
    AddDefinitionEscaped(makefile, "_moc_compile_defs", compileDefs);
  }

  // Moc executable
  {
    std::string mocExec;
    std::string err;

    if (digest.QtVersionMajor == "5") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt5::moc");
      if (tgt != nullptr) {
        mocExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        err = "AUTOMOC: Qt5::moc target not found";
      }
    } else if (digest.QtVersionMajor == "4") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt4::moc");
      if (tgt != nullptr) {
        mocExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        err = "AUTOMOC: Qt4::moc target not found";
      }
    } else {
      err = "The AUTOMOC feature supports only Qt 4 and Qt 5";
    }

    if (err.empty()) {
      AddDefinitionEscaped(makefile, "_qt_moc_executable", mocExec);
    } else {
      err += " (" + target->GetName() + ")";
      cmSystemTools::Error(err.c_str());
    }
  }
}

static void SetupAutoTargetUic(cmQtAutoGenDigest const& digest,
                               std::string const& config,
                               std::vector<std::string> const& configs,
                               cmQtAutoGenSetup& setup)
{
  cmGeneratorTarget const* target = digest.Target;
  cmMakefile* makefile = target->Target->GetMakefile();

  // Uic search paths
  {
    std::vector<std::string> uicSearchPaths;
    {
      std::string const usp = GetSafeProperty(target, "AUTOUIC_SEARCH_PATHS");
      if (!usp.empty()) {
        cmSystemTools::ExpandListArgument(usp, uicSearchPaths);
        std::string const srcDir = makefile->GetCurrentSourceDirectory();
        for (std::string& path : uicSearchPaths) {
          path = cmSystemTools::CollapseFullPath(path, srcDir);
        }
      }
    }
    AddDefinitionEscaped(makefile, "_uic_search_paths", uicSearchPaths);
  }
  // Uic target options
  {
    auto UicGetOpts = [target](std::string const& cfg) -> std::string {
      std::vector<std::string> opts;
      target->GetAutoUicOptions(opts, cfg);
      return cmJoin(opts, ";");
    };

    // Default settings
    std::string const uicOpts = UicGetOpts(config);
    AddDefinitionEscaped(makefile, "_uic_target_options", uicOpts);

    // Configuration specific settings
    for (std::string const& cfg : configs) {
      std::string const configUicOpts = UicGetOpts(cfg);
      if (configUicOpts != uicOpts) {
        setup.ConfigUicOptions[cfg] = configUicOpts;
      }
    }
  }
  // .ui files skip and options
  {
    std::vector<std::string> uiFileFiles;
    std::vector<std::vector<std::string>> uiFileOptions;
    {
      std::string const uiExt = "ui";
      std::string pathError;
      for (cmSourceFile* sf : makefile->GetSourceFiles()) {
        // sf->GetExtension() is only valid after sf->GetFullPath() ...
        // Since we're iterating over source files that might be not in the
        // target we need to check for path errors (not existing files).
        std::string const& fPath = sf->GetFullPath(&pathError);
        if (!pathError.empty()) {
          pathError.clear();
          continue;
        }
        if (sf->GetExtension() == uiExt) {
          std::string const absFile = cmSystemTools::GetRealPath(fPath);
          // Check if the file should be skipped
          if (sf->GetPropertyAsBool("SKIP_AUTOUIC") ||
              sf->GetPropertyAsBool("SKIP_AUTOGEN")) {
            setup.UicSkip.insert(absFile);
          }
          // Check if the files has uic options
          std::string const uicOpts = GetSafeProperty(sf, "AUTOUIC_OPTIONS");
          if (!uicOpts.empty()) {
            // Check if file isn't skipped
            if (setup.UicSkip.count(absFile) == 0) {
              uiFileFiles.push_back(absFile);
              std::vector<std::string> optsVec;
              cmSystemTools::ExpandListArgument(uicOpts, optsVec);
              uiFileOptions.push_back(std::move(optsVec));
            }
          }
        }
      }
    }
    AddDefinitionEscaped(makefile, "_qt_uic_options_files", uiFileFiles);
    AddDefinitionEscaped(makefile, "_qt_uic_options_options", uiFileOptions);
  }

  AddDefinitionEscaped(makefile, "_uic_skip", setup.UicSkip);

  // Uic executable
  {
    std::string err;
    std::string uicExec;

    cmLocalGenerator* localGen = target->GetLocalGenerator();
    if (digest.QtVersionMajor == "5") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt5::uic");
      if (tgt != nullptr) {
        uicExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        // Project does not use Qt5Widgets, but has AUTOUIC ON anyway
      }
    } else if (digest.QtVersionMajor == "4") {
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt4::uic");
      if (tgt != nullptr) {
        uicExec = SafeString(tgt->ImportedGetLocation(""));
      } else {
        err = "AUTOUIC: Qt4::uic target not found";
      }
    } else {
      err = "The AUTOUIC feature supports only Qt 4 and Qt 5";
    }

    if (err.empty()) {
      AddDefinitionEscaped(makefile, "_qt_uic_executable", uicExec);
    } else {
      err += " (" + target->GetName() + ")";
      cmSystemTools::Error(err.c_str());
    }
  }
}

static std::string RccGetExecutable(cmGeneratorTarget const* target,
                                    std::string const& qtMajorVersion)
{
  std::string rccExec;
  std::string err;

  cmLocalGenerator* localGen = target->GetLocalGenerator();
  if (qtMajorVersion == "5") {
    cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt5::rcc");
    if (tgt != nullptr) {
      rccExec = SafeString(tgt->ImportedGetLocation(""));
    } else {
      err = "AUTORCC: Qt5::rcc target not found";
    }
  } else if (qtMajorVersion == "4") {
    cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse("Qt4::rcc");
    if (tgt != nullptr) {
      rccExec = SafeString(tgt->ImportedGetLocation(""));
    } else {
      err = "AUTORCC: Qt4::rcc target not found";
    }
  } else {
    err = "The AUTORCC feature supports only Qt 4 and Qt 5";
  }

  if (!err.empty()) {
    err += " (" + target->GetName() + ")";
    cmSystemTools::Error(err.c_str());
  }
  return rccExec;
}

static void SetupAutoTargetRcc(cmQtAutoGenDigest const& digest)
{
  std::vector<std::string> rccFiles;
  std::vector<std::string> rccBuilds;
  std::vector<std::vector<std::string>> rccOptions;
  std::vector<std::vector<std::string>> rccInputs;

  for (cmQtAutoGenDigestQrc const& qrcDigest : digest.Qrcs) {
    rccFiles.push_back(qrcDigest.QrcFile);
    rccBuilds.push_back(qrcDigest.RccFile);
    rccOptions.push_back(qrcDigest.Options);
    rccInputs.push_back(qrcDigest.Resources);
  }

  cmMakefile* makefile = digest.Target->Target->GetMakefile();
  AddDefinitionEscaped(makefile, "_qt_rcc_executable",
                       RccGetExecutable(digest.Target, digest.QtVersionMajor));
  AddDefinitionEscaped(makefile, "_rcc_files", rccFiles);
  AddDefinitionEscaped(makefile, "_rcc_builds", rccBuilds);
  AddDefinitionEscaped(makefile, "_rcc_options", rccOptions);
  AddDefinitionEscaped(makefile, "_rcc_inputs", rccInputs);
}

void cmQtAutoGeneratorInitializer::InitializeAutogenTarget(
  cmQtAutoGenDigest& digest)
{
  cmGeneratorTarget* target = digest.Target;
  cmMakefile* makefile = target->Target->GetMakefile();
  cmLocalGenerator* localGen = target->GetLocalGenerator();
  cmGlobalGenerator* globalGen = localGen->GetGlobalGenerator();

  std::string const autogenTargetName = GetAutogenTargetName(target);
  std::string const autogenInfoDir = GetAutogenTargetFilesDir(target);
  std::string const autogenBuildDir = GetAutogenTargetBuildDir(target);
  std::string const workingDirectory =
    cmSystemTools::CollapseFullPath("", makefile->GetCurrentBinaryDirectory());

  cmQtAutoGen::MultiConfig const multiConfig = AutogenMultiConfig(globalGen);
  std::string configDefault;
  std::vector<std::string> configsList;
  GetConfigs(makefile, configDefault, configsList);

  std::set<std::string> autogenDependFiles;
  std::set<cmTarget*> autogenDependTargets;
  std::vector<std::string> autogenProvides;

  // Remove build directories on cleanup
  AddCleanFile(makefile, autogenBuildDir);
  // Remove old settings on cleanup
  {
    std::string base = autogenInfoDir + "/AutogenOldSettings";
    if (multiConfig == cmQtAutoGen::SINGLE) {
      AddCleanFile(makefile, base.append(".cmake"));
    } else {
      for (std::string const& cfg : configsList) {
        std::string filename = base;
        filename += "_";
        filename += cfg;
        filename += ".cmake";
        AddCleanFile(makefile, filename);
      }
    }
  }

  // Compose command lines
  cmCustomCommandLines commandLines;
  {
    cmCustomCommandLine currentLine;
    currentLine.push_back(cmSystemTools::GetCMakeCommand());
    currentLine.push_back("-E");
    currentLine.push_back("cmake_autogen");
    currentLine.push_back(autogenInfoDir);
    currentLine.push_back("$<CONFIGURATION>");
    commandLines.push_back(currentLine);
  }

  // Compose target comment
  std::string autogenComment;
  {
    std::vector<std::string> toolNames;
    if (digest.MocEnabled) {
      toolNames.emplace_back("MOC");
    }
    if (digest.UicEnabled) {
      toolNames.emplace_back("UIC");
    }
    if (digest.RccEnabled) {
      toolNames.emplace_back("RCC");
    }

    std::string tools = toolNames.front();
    toolNames.erase(toolNames.begin());
    if (!toolNames.empty()) {
      while (toolNames.size() > 1) {
        tools += ", ";
        tools += toolNames.front();
        toolNames.erase(toolNames.begin());
      }
      tools += " and " + toolNames.front();
    }
    autogenComment = "Automatic " + tools + " for target " + target->GetName();
  }

  // Add moc compilation to generated files list
  if (digest.MocEnabled) {
    std::string const mocsComp = autogenBuildDir + "/mocs_compilation.cpp";
    auto files = AddGeneratedSource(target, mocsComp, multiConfig, configsList,
                                    cmQtAutoGen::MOC);
    for (std::string& file : files) {
      autogenProvides.push_back(std::move(file));
    }
  }

  // Add autogen includes directory to the origin target INCLUDE_DIRECTORIES
  if (digest.MocEnabled || digest.UicEnabled) {
    std::string includeDir = autogenBuildDir + "/include";
    if (multiConfig != cmQtAutoGen::SINGLE) {
      includeDir += "_$<CONFIG>";
    }
    target->AddIncludeDirectory(includeDir, true);
  }

  // Extract relevant source files
  std::vector<std::string> generatedSources;
  std::vector<std::string> generatedHeaders;
  {
    std::string const qrcExt = "qrc";
    std::vector<cmSourceFile*> srcFiles;
    target->GetConfigCommonSourceFiles(srcFiles);
    for (cmSourceFile* sf : srcFiles) {
      if (sf->GetPropertyAsBool("SKIP_AUTOGEN")) {
        continue;
      }
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      std::string const& fPath = sf->GetFullPath();
      std::string const& ext = sf->GetExtension();
      // Register generated files that will be scanned by moc or uic
      if (digest.MocEnabled || digest.UicEnabled) {
        cmSystemTools::FileFormat const fileType =
          cmSystemTools::GetFileFormat(ext.c_str());
        if ((fileType == cmSystemTools::CXX_FILE_FORMAT) ||
            (fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
          std::string const absPath = cmSystemTools::GetRealPath(fPath);
          if ((digest.MocEnabled && !sf->GetPropertyAsBool("SKIP_AUTOMOC")) ||
              (digest.UicEnabled && !sf->GetPropertyAsBool("SKIP_AUTOUIC"))) {
            // Register source
            const bool generated = sf->GetPropertyAsBool("GENERATED");
            if (fileType == cmSystemTools::HEADER_FILE_FORMAT) {
              if (generated) {
                generatedHeaders.push_back(absPath);
              } else {
                digest.Headers.push_back(absPath);
              }
            } else {
              if (generated) {
                generatedSources.push_back(absPath);
              } else {
                digest.Sources.push_back(absPath);
              }
            }
          }
        }
      }
      // Register rcc enabled files
      if (digest.RccEnabled && (ext == qrcExt) &&
          !sf->GetPropertyAsBool("SKIP_AUTORCC")) {
        // Register qrc file
        {
          cmQtAutoGenDigestQrc qrcDigest;
          qrcDigest.QrcFile = cmSystemTools::GetRealPath(fPath);
          qrcDigest.QrcName =
            cmSystemTools::GetFilenameWithoutLastExtension(qrcDigest.QrcFile);
          qrcDigest.Generated = sf->GetPropertyAsBool("GENERATED");
          // RCC options
          {
            std::string const opts = GetSafeProperty(sf, "AUTORCC_OPTIONS");
            if (!opts.empty()) {
              cmSystemTools::ExpandListArgument(opts, qrcDigest.Options);
            }
          }
          digest.Qrcs.push_back(std::move(qrcDigest));
        }
      }
    }
    // cmGeneratorTarget::GetConfigCommonSourceFiles computes the target's
    // sources meta data cache. Clear it so that OBJECT library targets that
    // are AUTOGEN initialized after this target get their added
    // mocs_compilation.cpp source acknowledged by this target.
    target->ClearSourcesCache();
  }

  // Process GENERATED sources and headers
  if (!generatedSources.empty() || !generatedHeaders.empty()) {
    // Check status of policy CMP0071
    bool policyAccept = false;
    bool policyWarn = false;
    cmPolicies::PolicyStatus const CMP0071_status =
      target->Makefile->GetPolicyStatus(cmPolicies::CMP0071);
    switch (CMP0071_status) {
      case cmPolicies::WARN:
        policyWarn = true;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // Ignore GENERATED file
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        // Process GENERATED file
        policyAccept = true;
        break;
    }

    if (policyAccept) {
      // Accept GENERATED sources
      for (std::string const& absFile : generatedHeaders) {
        digest.Headers.push_back(absFile);
        autogenDependFiles.insert(absFile);
      }
      for (std::string const& absFile : generatedSources) {
        digest.Sources.push_back(absFile);
        autogenDependFiles.insert(absFile);
      }
    } else {
      if (policyWarn) {
        std::string msg;
        msg += cmPolicies::GetPolicyWarning(cmPolicies::CMP0071);
        msg += "\n";
        std::string tools;
        std::string property;
        if (digest.MocEnabled && digest.UicEnabled) {
          tools = "AUTOMOC and AUTOUIC";
          property = "SKIP_AUTOGEN";
        } else if (digest.MocEnabled) {
          tools = "AUTOMOC";
          property = "SKIP_AUTOMOC";
        } else if (digest.UicEnabled) {
          tools = "AUTOUIC";
          property = "SKIP_AUTOUIC";
        }
        msg += "For compatibility, CMake is excluding the GENERATED source "
               "file(s):\n";
        for (const std::string& absFile : generatedHeaders) {
          msg.append("  ").append(cmQtAutoGen::Quoted(absFile)).append("\n");
        }
        for (const std::string& absFile : generatedSources) {
          msg.append("  ").append(cmQtAutoGen::Quoted(absFile)).append("\n");
        }
        msg += "from processing by ";
        msg += tools;
        msg +=
          ". If any of the files should be processed, set CMP0071 to NEW. "
          "If any of the files should not be processed, "
          "explicitly exclude them by setting the source file property ";
        msg += property;
        msg += ":\n  set_property(SOURCE file.h PROPERTY ";
        msg += property;
        msg += " ON)\n";
        makefile->IssueMessage(cmake::AUTHOR_WARNING, msg);
      }
    }
  }
  // Sort headers and sources
  std::sort(digest.Headers.begin(), digest.Headers.end());
  std::sort(digest.Sources.begin(), digest.Sources.end());

  // Process qrc files
  if (!digest.Qrcs.empty()) {
    const bool QtV5 = (digest.QtVersionMajor == "5");
    std::string const rcc = RccGetExecutable(target, digest.QtVersionMajor);
    // Target rcc options
    std::vector<std::string> optionsTarget;
    cmSystemTools::ExpandListArgument(
      GetSafeProperty(target, "AUTORCC_OPTIONS"), optionsTarget);

    // Check if file name is unique
    for (cmQtAutoGenDigestQrc& qrcDigest : digest.Qrcs) {
      qrcDigest.Unique = true;
      for (cmQtAutoGenDigestQrc const& qrcDig2 : digest.Qrcs) {
        if ((&qrcDigest != &qrcDig2) &&
            (qrcDigest.QrcName == qrcDig2.QrcName)) {
          qrcDigest.Unique = false;
          break;
        }
      }
    }
    // Path checksum
    {
      cmFilePathChecksum const fpathCheckSum(makefile);
      for (cmQtAutoGenDigestQrc& qrcDigest : digest.Qrcs) {
        qrcDigest.PathChecksum = fpathCheckSum.getPart(qrcDigest.QrcFile);
        // RCC output file name
        std::string rccFile = autogenBuildDir + "/";
        rccFile += qrcDigest.PathChecksum;
        rccFile += "/qrc_";
        rccFile += qrcDigest.QrcName;
        rccFile += ".cpp";
        qrcDigest.RccFile = std::move(rccFile);
      }
    }
    // RCC options
    for (cmQtAutoGenDigestQrc& qrcDigest : digest.Qrcs) {
      // Target options
      std::vector<std::string> opts = optionsTarget;
      // Merge computed "-name XYZ" option
      {
        std::string name = qrcDigest.QrcName;
        // Replace '-' with '_'. The former is not valid for symbol names.
        std::replace(name.begin(), name.end(), '-', '_');
        if (!qrcDigest.Unique) {
          name += "_";
          name += qrcDigest.PathChecksum;
        }
        std::vector<std::string> nameOpts;
        nameOpts.emplace_back("-name");
        nameOpts.emplace_back(std::move(name));
        cmQtAutoGen::RccMergeOptions(opts, nameOpts, QtV5);
      }
      // Merge file option
      cmQtAutoGen::RccMergeOptions(opts, qrcDigest.Options, QtV5);
      qrcDigest.Options = std::move(opts);
    }
    for (cmQtAutoGenDigestQrc& qrcDigest : digest.Qrcs) {
      // Register file at target
      {
        auto files = AddGeneratedSource(target, qrcDigest.RccFile, multiConfig,
                                        configsList, cmQtAutoGen::RCC);
        for (std::string& file : files) {
          autogenProvides.push_back(std::move(file));
        }
      }
      // Dependencies
      if (qrcDigest.Generated) {
        // Add the GENERATED .qrc file to the dependencies
        autogenDependFiles.insert(qrcDigest.QrcFile);
      } else {
        // Add the resource files to the dependencies
        {
          std::string error;
          if (cmQtAutoGen::RccListInputs(digest.QtVersionMajor, rcc,
                                         qrcDigest.QrcFile,
                                         qrcDigest.Resources, &error)) {
            for (std::string const& fileName : qrcDigest.Resources) {
              autogenDependFiles.insert(fileName);
            }
          } else {
            cmSystemTools::Error(error.c_str());
          }
        }
        // Run cmake again when .qrc file changes
        makefile->AddCMakeDependFile(qrcDigest.QrcFile);
      }
    }
  }

  // Add user defined autogen target dependencies
  {
    std::string const deps = GetSafeProperty(target, "AUTOGEN_TARGET_DEPENDS");
    if (!deps.empty()) {
      std::vector<std::string> extraDeps;
      cmSystemTools::ExpandListArgument(deps, extraDeps);
      for (std::string const& depName : extraDeps) {
        // Allow target and file dependencies
        auto* depTarget = makefile->FindTargetToUse(depName);
        if (depTarget != nullptr) {
          autogenDependTargets.insert(depTarget);
        } else {
          autogenDependFiles.insert(depName);
        }
      }
    }
  }

  // Use PRE_BUILD on demand
  bool usePRE_BUILD = false;
  if (globalGen->GetName().find("Visual Studio") != std::string::npos) {
    // Under VS use a PRE_BUILD event instead of a separate target to
    // reduce the number of targets loaded into the IDE.
    // This also works around a VS 11 bug that may skip updating the target:
    //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
    usePRE_BUILD = true;
  }
  // Disable PRE_BUILD in some cases
  if (usePRE_BUILD) {
    // Cannot use PRE_BUILD with file depends
    if (!autogenDependFiles.empty()) {
      usePRE_BUILD = false;
    }
  }
  // Create the autogen target/command
  if (usePRE_BUILD) {
    // Add additional autogen target dependencies to origin target
    for (cmTarget* depTarget : autogenDependTargets) {
      target->Target->AddUtility(depTarget->GetName(), makefile);
    }

    // Add the pre-build command directly to bypass the OBJECT_LIBRARY
    // rejection in cmMakefile::AddCustomCommandToTarget because we know
    // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
    //
    // PRE_BUILD does not support file dependencies!
    const std::vector<std::string> no_output;
    const std::vector<std::string> no_deps;
    cmCustomCommand cc(makefile, no_output, autogenProvides, no_deps,
                       commandLines, autogenComment.c_str(),
                       workingDirectory.c_str());
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    target->Target->AddPreBuildCommand(cc);
  } else {

    // Add link library target dependencies to the autogen target dependencies
    {
      // add_dependencies/addUtility do not support generator expressions.
      // We depend only on the libraries found in all configs therefore.
      std::map<cmGeneratorTarget const*, std::size_t> commonTargets;
      for (std::string const& config : configsList) {
        cmLinkImplementationLibraries const* libs =
          target->GetLinkImplementationLibraries(config);
        if (libs != nullptr) {
          for (cmLinkItem const& item : libs->Libraries) {
            cmGeneratorTarget const* libTarget = item.Target;
            if ((libTarget != nullptr) &&
                !StaticLibraryCycle(target, libTarget, config)) {
              // Increment target config count
              commonTargets[libTarget]++;
            }
          }
        }
      }
      for (auto const& item : commonTargets) {
        if (item.second == configsList.size()) {
          autogenDependTargets.insert(item.first->Target);
        }
      }
    }

    // Create autogen target
    cmTarget* autogenTarget = makefile->AddUtilityCommand(
      autogenTargetName, true, workingDirectory.c_str(),
      /*byproducts=*/autogenProvides,
      std::vector<std::string>(autogenDependFiles.begin(),
                               autogenDependFiles.end()),
      commandLines, false, autogenComment.c_str());
    // Create autogen generator target
    localGen->AddGeneratorTarget(
      new cmGeneratorTarget(autogenTarget, localGen));

    // Forward origin utilities to autogen target
    for (std::string const& depName : target->Target->GetUtilities()) {
      autogenTarget->AddUtility(depName, makefile);
    }
    // Add additional autogen target dependencies to autogen target
    for (cmTarget* depTarget : autogenDependTargets) {
      autogenTarget->AddUtility(depTarget->GetName(), makefile);
    }

    // Set FOLDER property in autogen target
    {
      const char* autogenFolder =
        makefile->GetState()->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
      if (autogenFolder == nullptr) {
        autogenFolder =
          makefile->GetState()->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
      }
      // Inherit FOLDER property from target (#13688)
      if (autogenFolder == nullptr) {
        autogenFolder = SafeString(target->Target->GetProperty("FOLDER"));
      }
      if ((autogenFolder != nullptr) && (*autogenFolder != '\0')) {
        autogenTarget->SetProperty("FOLDER", autogenFolder);
      }
    }

    // Add autogen target to the origin target dependencies
    target->Target->AddUtility(autogenTargetName, makefile);
  }
}

void cmQtAutoGeneratorInitializer::SetupAutoGenerateTarget(
  cmQtAutoGenDigest const& digest)
{
  cmGeneratorTarget const* target = digest.Target;
  cmMakefile* makefile = target->Target->GetMakefile();
  cmQtAutoGen::MultiConfig const multiConfig =
    AutogenMultiConfig(target->GetGlobalGenerator());

  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  // Configurations
  std::string configDefault;
  std::vector<std::string> configsList;
  std::map<std::string, std::string> configSuffixes;
  {
    configDefault = makefile->GetConfigurations(configsList);
    if (configsList.empty()) {
      configsList.push_back("");
    }
  }
  for (std::string const& cfg : configsList) {
    configSuffixes[cfg] = "_" + cfg;
  }

  // Configurations settings buffers
  cmQtAutoGenSetup setup;

  // Basic setup
  AddDefinitionEscaped(makefile, "_multi_config",
                       cmQtAutoGen::MultiConfigName(multiConfig));
  AddDefinitionEscaped(makefile, "_build_dir",
                       GetAutogenTargetBuildDir(target));
  AddDefinitionEscaped(makefile, "_sources", digest.Sources);
  AddDefinitionEscaped(makefile, "_headers", digest.Headers);
  AddDefinitionEscaped(makefile, "_qt_version_major", digest.QtVersionMajor);
  AddDefinitionEscaped(makefile, "_qt_version_minor", digest.QtVersionMinor);
  {
    if (digest.MocEnabled || digest.UicEnabled) {
      SetupAcquireSkipFiles(digest, setup);
      if (digest.MocEnabled) {
        SetupAutoTargetMoc(digest, configDefault, configsList, setup);
      }
      if (digest.UicEnabled) {
        SetupAutoTargetUic(digest, configDefault, configsList, setup);
      }
    }
    if (digest.RccEnabled) {
      SetupAutoTargetRcc(digest);
    }
  }

  // Generate info file
  {
    std::string const infoDir = GetAutogenTargetFilesDir(target);
    if (!cmSystemTools::MakeDirectory(infoDir)) {
      std::string emsg = ("Could not create directory: ");
      emsg += cmQtAutoGen::Quoted(infoDir);
      cmSystemTools::Error(emsg.c_str());
    }
    std::string const infoFile = infoDir + "/AutogenInfo.cmake";
    {
      std::string infoFileIn = cmSystemTools::GetCMakeRoot();
      infoFileIn += "/Modules/AutogenInfo.cmake.in";
      makefile->ConfigureFile(infoFileIn.c_str(), infoFile.c_str(), false,
                              true, false);
    }

    // Append custom definitions to info file
    // --------------------------------------

    // Ensure we have write permission in case .in was read-only.
    mode_t perm = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
    mode_t mode_write = S_IWRITE;
#else
    mode_t mode_write = S_IWUSR;
#endif
    cmSystemTools::GetPermissions(infoFile, perm);
    if (!(perm & mode_write)) {
      cmSystemTools::SetPermissions(infoFile, perm | mode_write);
    }

    // Open and write file
    cmsys::ofstream ofs(infoFile.c_str(), std::ios::app);
    if (ofs) {
      auto OfsWriteMap = [&ofs](
        const char* key, std::map<std::string, std::string> const& map) {
        for (auto const& item : map) {
          ofs << "set(" << key << "_" << item.first << " "
              << cmOutputConverter::EscapeForCMake(item.second) << ")\n";
        }
      };
      ofs << "# Configurations options\n";
      OfsWriteMap("AM_CONFIG_SUFFIX", configSuffixes);
      OfsWriteMap("AM_MOC_DEFINITIONS", setup.ConfigMocDefines);
      OfsWriteMap("AM_MOC_INCLUDES", setup.ConfigMocIncludes);
      OfsWriteMap("AM_UIC_TARGET_OPTIONS", setup.ConfigUicOptions);
    } else {
      // File open error
      std::string error = "Internal CMake error when trying to open file: ";
      error += cmQtAutoGen::Quoted(infoFile);
      error += " for writing.";
      cmSystemTools::Error(error.c_str());
    }
  }
}
