/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGeneratorInitializer.h"
#include "cmQtAutoGeneratorCommon.h"

#include "cmAlgorithms.h"
#include "cmCustomCommandLines.h"
#include "cmFilePathChecksum.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmState.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmake.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#include "cmGlobalVisualStudioGenerator.h"
#endif

#include <algorithm>
#include <cmConfigure.h>
#include <cmsys/FStream.hxx>
#include <cmsys/RegularExpression.hxx>
#include <map>
#include <set>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>

static void utilCopyTargetProperty(cmTarget* destinationTarget,
                                   cmTarget* sourceTarget,
                                   const std::string& propertyName)
{
  const char* propertyValue = sourceTarget->GetProperty(propertyName);
  if (propertyValue) {
    destinationTarget->SetProperty(propertyName, propertyValue);
  }
}

inline static bool PropertyEnabled(cmSourceFile* sourceFile, const char* key)
{
  return cmSystemTools::IsOn(sourceFile->GetPropertyForUser(key));
}

static std::string GetSafeProperty(cmGeneratorTarget const* target,
                                   const char* key)
{
  const char* tmp = target->GetProperty(key);
  return std::string((tmp != CM_NULLPTR) ? tmp : "");
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
  targetDir += ".dir/";
  return targetDir;
}

static std::string GetAutogenTargetBuildDir(cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string targetDir = makefile->GetCurrentBinaryDirectory();
  targetDir += "/";
  targetDir += GetAutogenTargetName(target);
  targetDir += "/";
  return targetDir;
}

static std::string GetQtMajorVersion(cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion.empty()) {
    qtMajorVersion = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
  }
  const char* targetQtVersion =
    target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", "");
  if (targetQtVersion != CM_NULLPTR) {
    qtMajorVersion = targetQtVersion;
  }
  return qtMajorVersion;
}

static void GetCompileDefinitionsAndDirectories(
  cmGeneratorTarget const* target, const std::string& config,
  std::string& incs, std::string& defs)
{
  cmLocalGenerator* localGen = target->GetLocalGenerator();
  {
    std::vector<std::string> includeDirs;
    // Get the include dirs for this target, without stripping the implicit
    // include dirs off, see
    // https://gitlab.kitware.com/cmake/cmake/issues/13667
    localGen->GetIncludeDirectories(includeDirs, target, "CXX", config, false);
    incs = cmJoin(includeDirs, ";");
  }
  {
    std::set<std::string> defines;
    localGen->AddCompileDefinitions(defines, target, config, "CXX");
    defs += cmJoin(defines, ";");
  }
}

static void AddDefinitionEscaped(cmMakefile* makefile, const char* key,
                                 const std::string& value)
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

static void AcquireScanFiles(cmGeneratorTarget const* target,
                             std::vector<std::string>& mocUicSources,
                             std::vector<std::string>& mocUicHeaders,
                             std::vector<std::string>& mocSkipList,
                             std::vector<std::string>& uicSkipList)
{
  const bool mocTarget = target->GetPropertyAsBool("AUTOMOC");
  const bool uicTarget = target->GetPropertyAsBool("AUTOUIC");

  std::vector<cmSourceFile*> srcFiles;
  target->GetConfigCommonSourceFiles(srcFiles);
  for (std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
       fileIt != srcFiles.end(); ++fileIt) {
    cmSourceFile* sf = *fileIt;
    const cmSystemTools::FileFormat fileType =
      cmSystemTools::GetFileFormat(sf->GetExtension().c_str());

    if (!(fileType == cmSystemTools::CXX_FILE_FORMAT) &&
        !(fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
      continue;
    }
    const std::string absFile =
      cmsys::SystemTools::GetRealPath(sf->GetFullPath());
    // Skip flags
    const bool skipAll = PropertyEnabled(sf, "SKIP_AUTOGEN");
    const bool mocSkip = skipAll || PropertyEnabled(sf, "SKIP_AUTOMOC");
    const bool uicSkip = skipAll || PropertyEnabled(sf, "SKIP_AUTOUIC");
    // Add file name to skip lists.
    // Do this even when the file is not added to the sources/headers lists
    // because the file name may be extracted from an other file when
    // processing
    if (mocSkip) {
      mocSkipList.push_back(absFile);
    }
    if (uicSkip) {
      uicSkipList.push_back(absFile);
    }

    if ((mocTarget && !mocSkip) || (uicTarget && !uicSkip)) {
      // Add file name to sources or headers list
      switch (fileType) {
        case cmSystemTools::CXX_FILE_FORMAT:
          mocUicSources.push_back(absFile);
          break;
        case cmSystemTools::HEADER_FILE_FORMAT:
          mocUicHeaders.push_back(absFile);
          break;
        default:
          break;
      }
    }
  }
}

static void MocSetupAutoTarget(
  cmGeneratorTarget const* target, const std::string& autogenTargetName,
  const std::string& qtMajorVersion,
  std::vector<std::string> const& mocSkipList,
  std::map<std::string, std::string>& configIncludes,
  std::map<std::string, std::string>& configDefines)
{
  cmLocalGenerator* lg = target->GetLocalGenerator();
  cmMakefile* makefile = target->Target->GetMakefile();

  AddDefinitionEscaped(makefile, "_moc_options",
                       GetSafeProperty(target, "AUTOMOC_MOC_OPTIONS"));
  AddDefinitionEscaped(makefile, "_moc_skip", mocSkipList);
  AddDefinitionEscaped(makefile, "_moc_relaxed_mode",
                       makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE") ? "TRUE"
                                                                    : "FALSE");
  AddDefinitionEscaped(makefile, "_moc_depend_filters",
                       GetSafeProperty(target, "AUTOMOC_DEPEND_FILTERS"));

  // Moc includes and compile definitions
  {
    std::string _moc_incs;
    std::string _moc_compile_defs;
    std::vector<std::string> configs;
    {
      const std::string& config = makefile->GetConfigurations(configs);
      GetCompileDefinitionsAndDirectories(target, config, _moc_incs,
                                          _moc_compile_defs);
      AddDefinitionEscaped(makefile, "_moc_incs", _moc_incs);
      AddDefinitionEscaped(makefile, "_moc_compile_defs", _moc_compile_defs);
    }
    for (std::vector<std::string>::const_iterator li = configs.begin();
         li != configs.end(); ++li) {
      std::string config_moc_incs;
      std::string config_moc_compile_defs;
      GetCompileDefinitionsAndDirectories(target, *li, config_moc_incs,
                                          config_moc_compile_defs);
      if (config_moc_incs != _moc_incs) {
        configIncludes[*li] =
          cmOutputConverter::EscapeForCMake(config_moc_incs);
        if (_moc_incs.empty()) {
          _moc_incs = config_moc_incs;
        }
      }
      if (config_moc_compile_defs != _moc_compile_defs) {
        configDefines[*li] =
          cmOutputConverter::EscapeForCMake(config_moc_compile_defs);
        if (_moc_compile_defs.empty()) {
          _moc_compile_defs = config_moc_compile_defs;
        }
      }
    }
  }

  // Moc executable
  {
    std::string err;
    const char* mocExec = CM_NULLPTR;
    if (qtMajorVersion == "5") {
      cmGeneratorTarget* qt5Moc = lg->FindGeneratorTargetToUse("Qt5::moc");
      if (qt5Moc != CM_NULLPTR) {
        mocExec = qt5Moc->ImportedGetLocation("");
      } else {
        err = "Qt5::moc target not found " + autogenTargetName;
      }
    } else if (qtMajorVersion == "4") {
      cmGeneratorTarget* qt4Moc = lg->FindGeneratorTargetToUse("Qt4::moc");
      if (qt4Moc != CM_NULLPTR) {
        mocExec = qt4Moc->ImportedGetLocation("");
      } else {
        err = "Qt4::moc target not found " + autogenTargetName;
      }
    } else {
      err = "The CMAKE_AUTOMOC feature supports only Qt 4 and Qt 5 ";
      err += autogenTargetName;
    }
    // Add definition or error
    if (err.empty()) {
      AddDefinitionEscaped(makefile, "_qt_moc_executable",
                           mocExec ? mocExec : "");
    } else {
      cmSystemTools::Error(err.c_str());
    }
  }
}

static void UicGetOpts(cmGeneratorTarget const* target,
                       const std::string& config, std::string& optString)
{
  std::vector<std::string> opts;
  target->GetAutoUicOptions(opts, config);
  optString = cmJoin(opts, ";");
}

static void UicSetupAutoTarget(
  cmGeneratorTarget const* target, const std::string& qtMajorVersion,
  std::vector<std::string> const& uicSkipList,
  std::map<std::string, std::string>& configUicOptions)
{
  cmLocalGenerator* lg = target->GetLocalGenerator();
  cmMakefile* makefile = target->Target->GetMakefile();

  AddDefinitionEscaped(makefile, "_uic_skip", uicSkipList);

  // Uic search paths
  {
    std::vector<std::string> uicSearchPaths;
    cmSystemTools::ExpandListArgument(
      GetSafeProperty(target, "AUTOUIC_SEARCH_PATHS"), uicSearchPaths);
    const std::string srcDir = makefile->GetCurrentSourceDirectory();
    for (std::vector<std::string>::iterator it = uicSearchPaths.begin();
         it != uicSearchPaths.end(); ++it) {
      *it = cmSystemTools::CollapseFullPath(*it, srcDir);
    }
    AddDefinitionEscaped(makefile, "_uic_search_paths", uicSearchPaths);
  }

  // Uic target options
  {
    std::string _uic_opts;
    std::vector<std::string> configs;
    UicGetOpts(target, makefile->GetConfigurations(configs), _uic_opts);

    AddDefinitionEscaped(makefile, "_uic_target_options", _uic_opts);

    for (std::vector<std::string>::const_iterator li = configs.begin();
         li != configs.end(); ++li) {
      std::string config_uic_opts;
      UicGetOpts(target, *li, config_uic_opts);
      if (config_uic_opts != _uic_opts) {
        configUicOptions[*li] =
          cmOutputConverter::EscapeForCMake(config_uic_opts);
        if (_uic_opts.empty()) {
          _uic_opts = config_uic_opts;
        }
      }
    }
  }
  // Uic files options
  {
    std::vector<std::string> uiFileFiles;
    std::vector<std::string> uiFileOptions;
    {
      std::set<std::string> skipped;
      skipped.insert(uicSkipList.begin(), uicSkipList.end());

      const std::vector<cmSourceFile*> uiFilesWithOptions =
        makefile->GetQtUiFilesWithOptions();
      for (std::vector<cmSourceFile*>::const_iterator fileIt =
             uiFilesWithOptions.begin();
           fileIt != uiFilesWithOptions.end(); ++fileIt) {
        cmSourceFile* sf = *fileIt;
        const std::string absFile =
          cmsys::SystemTools::GetRealPath(sf->GetFullPath());
        if (skipped.insert(absFile).second) {
          // The file wasn't skipped
          uiFileFiles.push_back(absFile);
          {
            std::string opts = sf->GetProperty("AUTOUIC_OPTIONS");
            cmSystemTools::ReplaceString(opts, ";",
                                         cmQtAutoGeneratorCommon::listSep);
            uiFileOptions.push_back(opts);
          }
        }
      }
    }
    AddDefinitionEscaped(makefile, "_qt_uic_options_files", uiFileFiles);
    AddDefinitionEscaped(makefile, "_qt_uic_options_options", uiFileOptions);
  }

  // Uic executable
  {
    std::string err;
    const char* uicExec = CM_NULLPTR;
    if (qtMajorVersion == "5") {
      cmGeneratorTarget* qt5Uic = lg->FindGeneratorTargetToUse("Qt5::uic");
      if (qt5Uic != CM_NULLPTR) {
        uicExec = qt5Uic->ImportedGetLocation("");
      } else {
        // Project does not use Qt5Widgets, but has AUTOUIC ON anyway
      }
    } else if (qtMajorVersion == "4") {
      cmGeneratorTarget* qt4Uic = lg->FindGeneratorTargetToUse("Qt4::uic");
      if (qt4Uic != CM_NULLPTR) {
        uicExec = qt4Uic->ImportedGetLocation("");
      } else {
        err = "Qt4::uic target not found " + target->GetName();
      }
    } else {
      err = "The CMAKE_AUTOUIC feature supports only Qt 4 and Qt 5 ";
      err += target->GetName();
    }
    // Add definition or error
    if (err.empty()) {
      AddDefinitionEscaped(makefile, "_qt_uic_executable",
                           uicExec ? uicExec : "");
    } else {
      cmSystemTools::Error(err.c_str());
    }
  }
}

static std::string RccGetExecutable(cmGeneratorTarget const* target,
                                    const std::string& qtMajorVersion)
{
  std::string rccExec;
  cmLocalGenerator* lg = target->GetLocalGenerator();
  if (qtMajorVersion == "5") {
    cmGeneratorTarget* qt5Rcc = lg->FindGeneratorTargetToUse("Qt5::rcc");
    if (qt5Rcc != CM_NULLPTR) {
      rccExec = qt5Rcc->ImportedGetLocation("");
    } else {
      cmSystemTools::Error("Qt5::rcc target not found ",
                           target->GetName().c_str());
    }
  } else if (qtMajorVersion == "4") {
    cmGeneratorTarget* qt4Rcc = lg->FindGeneratorTargetToUse("Qt4::rcc");
    if (qt4Rcc != CM_NULLPTR) {
      rccExec = qt4Rcc->ImportedGetLocation("");
    } else {
      cmSystemTools::Error("Qt4::rcc target not found ",
                           target->GetName().c_str());
    }
  } else {
    cmSystemTools::Error(
      "The CMAKE_AUTORCC feature supports only Qt 4 and Qt 5 ",
      target->GetName().c_str());
  }
  return rccExec;
}

static void RccMergeOptions(std::vector<std::string>& opts,
                            const std::vector<std::string>& fileOpts,
                            bool isQt5)
{
  static const char* valueOptions[] = { "name", "root", "compress",
                                        "threshold" };
  std::vector<std::string> extraOpts;
  for (std::vector<std::string>::const_iterator fit = fileOpts.begin();
       fit != fileOpts.end(); ++fit) {
    std::vector<std::string>::iterator existingIt =
      std::find(opts.begin(), opts.end(), *fit);
    if (existingIt != opts.end()) {
      const char* optName = fit->c_str();
      if (*optName == '-') {
        ++optName;
        if (isQt5 && *optName == '-') {
          ++optName;
        }
      }
      // Test if this is a value option and change the existing value
      if ((optName != fit->c_str()) &&
          std::find_if(cmArrayBegin(valueOptions), cmArrayEnd(valueOptions),
                       cmStrCmp(optName)) != cmArrayEnd(valueOptions)) {
        const std::vector<std::string>::iterator existValueIt(existingIt + 1);
        const std::vector<std::string>::const_iterator fileValueIt(fit + 1);
        if ((existValueIt != opts.end()) && (fileValueIt != fileOpts.end())) {
          *existValueIt = *fileValueIt;
          ++fit;
        }
      }
    } else {
      extraOpts.push_back(*fit);
    }
  }
  opts.insert(opts.end(), extraOpts.begin(), extraOpts.end());
}

static void RccSetupAutoTarget(cmGeneratorTarget const* target,
                               const std::string& qtMajorVersion)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  const bool qtMajorVersion5 = (qtMajorVersion == "5");
  const std::string rccCommand = RccGetExecutable(target, qtMajorVersion);
  std::vector<std::string> _rcc_files;
  std::vector<std::string> _rcc_inputs;
  std::vector<std::string> rccFileFiles;
  std::vector<std::string> rccFileOptions;
  std::vector<std::string> rccOptionsTarget;
  if (const char* opts = target->GetProperty("AUTORCC_OPTIONS")) {
    cmSystemTools::ExpandListArgument(opts, rccOptionsTarget);
  }

  std::vector<cmSourceFile*> srcFiles;
  target->GetConfigCommonSourceFiles(srcFiles);
  for (std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
       fileIt != srcFiles.end(); ++fileIt) {
    cmSourceFile* sf = *fileIt;
    if ((sf->GetExtension() == "qrc") &&
        !PropertyEnabled(sf, "SKIP_AUTOGEN") &&
        !PropertyEnabled(sf, "SKIP_AUTORCC")) {
      const std::string absFile =
        cmsys::SystemTools::GetRealPath(sf->GetFullPath());
      // qrc file
      _rcc_files.push_back(absFile);
      // qrc file entries
      {
        std::string entriesList = "{";
        // Read input file list only for non generated .qrc files.
        if (!PropertyEnabled(sf, "GENERATED")) {
          std::string error;
          std::vector<std::string> files;
          if (cmQtAutoGeneratorCommon::RccListInputs(
                qtMajorVersion, rccCommand, absFile, files, &error)) {
            entriesList += cmJoin(files, cmQtAutoGeneratorCommon::listSep);
          } else {
            cmSystemTools::Error(error.c_str());
          }
        }
        entriesList += "}";
        _rcc_inputs.push_back(entriesList);
      }
      // rcc options for this qrc file
      {
        // Merged target and file options
        std::vector<std::string> rccOptions(rccOptionsTarget);
        if (const char* prop = sf->GetProperty("AUTORCC_OPTIONS")) {
          std::vector<std::string> optsVec;
          cmSystemTools::ExpandListArgument(prop, optsVec);
          RccMergeOptions(rccOptions, optsVec, qtMajorVersion5);
        }
        // Only store non empty options lists
        if (!rccOptions.empty()) {
          rccFileFiles.push_back(absFile);
          rccFileOptions.push_back(
            cmJoin(rccOptions, cmQtAutoGeneratorCommon::listSep));
        }
      }
    }
  }

  AddDefinitionEscaped(makefile, "_qt_rcc_executable", rccCommand);
  AddDefinitionEscaped(makefile, "_rcc_files", _rcc_files);
  AddDefinitionEscaped(makefile, "_rcc_inputs", _rcc_inputs);
  AddDefinitionEscaped(makefile, "_rcc_options_files", rccFileFiles);
  AddDefinitionEscaped(makefile, "_rcc_options_options", rccFileOptions);
}

void cmQtAutoGeneratorInitializer::InitializeAutogenSources(
  cmGeneratorTarget* target)
{
  if (target->GetPropertyAsBool("AUTOMOC")) {
    cmMakefile* makefile = target->Target->GetMakefile();
    const std::string mocCppFile =
      GetAutogenTargetBuildDir(target) + "moc_compilation.cpp";
    cmSourceFile* gf = makefile->GetOrCreateSource(mocCppFile, true);
    gf->SetProperty("SKIP_AUTOGEN", "On");
    target->AddSource(mocCppFile);
  }
}

void cmQtAutoGeneratorInitializer::InitializeAutogenTarget(
  cmLocalGenerator* lg, cmGeneratorTarget* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();

  // Create a custom target for running generators at buildtime
  const bool mocEnabled = target->GetPropertyAsBool("AUTOMOC");
  const bool uicEnabled = target->GetPropertyAsBool("AUTOUIC");
  const bool rccEnabled = target->GetPropertyAsBool("AUTORCC");
  const std::string autogenTargetName = GetAutogenTargetName(target);
  const std::string autogenBuildDir = GetAutogenTargetBuildDir(target);
  const std::string workingDirectory =
    cmSystemTools::CollapseFullPath("", makefile->GetCurrentBinaryDirectory());
  const std::string qtMajorVersion = GetQtMajorVersion(target);
  const std::string rccCommand = RccGetExecutable(target, qtMajorVersion);
  std::vector<std::string> autogenDepends;
  std::vector<std::string> autogenProvides;

  // Remove old settings on cleanup
  {
    std::string fname = GetAutogenTargetFilesDir(target);
    fname += "/AutogenOldSettings.cmake";
    makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES", fname.c_str(),
                             false);
  }

  // Compose command lines
  cmCustomCommandLines commandLines;
  {
    cmCustomCommandLine currentLine;
    currentLine.push_back(cmSystemTools::GetCMakeCommand());
    currentLine.push_back("-E");
    currentLine.push_back("cmake_autogen");
    currentLine.push_back(GetAutogenTargetFilesDir(target));
    currentLine.push_back("$<CONFIGURATION>");
    commandLines.push_back(currentLine);
  }

  // Compose target comment
  std::string autogenComment;
  {
    std::vector<std::string> toolNames;
    if (mocEnabled) {
      toolNames.push_back("MOC");
    }
    if (uicEnabled) {
      toolNames.push_back("UIC");
    }
    if (rccEnabled) {
      toolNames.push_back("RCC");
    }

    std::string tools = toolNames[0];
    toolNames.erase(toolNames.begin());
    while (toolNames.size() > 1) {
      tools += ", " + toolNames[0];
      toolNames.erase(toolNames.begin());
    }
    if (toolNames.size() == 1) {
      tools += " and " + toolNames[0];
    }
    autogenComment = "Automatic " + tools + " for target " + target->GetName();
  }

  // Create autogen target build directory and add it to the clean files
  cmSystemTools::MakeDirectory(autogenBuildDir);
  makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                           autogenBuildDir.c_str(), false);

  // Create autogen target includes directory and
  // add it to the origin target INCLUDE_DIRECTORIES
  if (mocEnabled || uicEnabled) {
    const std::string incsDir = autogenBuildDir + "include";
    cmSystemTools::MakeDirectory(incsDir);
    target->AddIncludeDirectory(incsDir, true);
  }

  // Register moc compilation file as generated
  if (mocEnabled) {
    autogenProvides.push_back(autogenBuildDir + "moc_compilation.cpp");
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  bool usePRE_BUILD = false;
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();
  if (gg->GetName().find("Visual Studio") != std::string::npos) {
    cmGlobalVisualStudioGenerator* vsgg =
      static_cast<cmGlobalVisualStudioGenerator*>(gg);
    // Under VS >= 7 use a PRE_BUILD event instead of a separate target to
    // reduce the number of targets loaded into the IDE.
    // This also works around a VS 11 bug that may skip updating the target:
    //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
    usePRE_BUILD = vsgg->GetVersion() >= cmGlobalVisualStudioGenerator::VS7;
  }
#endif

  // Initialize autogen target dependencies
  if (const char* deps = target->GetProperty("AUTOGEN_TARGET_DEPENDS")) {
    cmSystemTools::ExpandListArgument(deps, autogenDepends);
  }
  // Add link library targets to the autogen dependencies
  {
    const cmTarget::LinkLibraryVectorType& libVec =
      target->Target->GetOriginalLinkLibraries();
    for (cmTarget::LinkLibraryVectorType::const_iterator it = libVec.begin();
         it != libVec.end(); ++it) {
      const std::string& libName = it->first;
      if (makefile->FindTargetToUse(libName) != CM_NULLPTR) {
        autogenDepends.push_back(libName);
      }
    }
  }
  {
    cmFilePathChecksum fpathCheckSum(makefile);
    // Iterate over all source files
    std::vector<cmSourceFile*> srcFiles;
    target->GetConfigCommonSourceFiles(srcFiles);
    for (std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
         fileIt != srcFiles.end(); ++fileIt) {
      cmSourceFile* sf = *fileIt;
      if (!PropertyEnabled(sf, "SKIP_AUTOGEN")) {
        const std::string ext = sf->GetExtension();
        // Add generated file that will be scanned by moc or uic to
        // the dependencies
        if (mocEnabled || uicEnabled) {
          const cmSystemTools::FileFormat fileType =
            cmSystemTools::GetFileFormat(ext.c_str());
          if ((fileType == cmSystemTools::CXX_FILE_FORMAT) ||
              (fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
            if (PropertyEnabled(sf, "GENERATED")) {
              if ((mocEnabled && !PropertyEnabled(sf, "SKIP_AUTOMOC")) ||
                  (uicEnabled && !PropertyEnabled(sf, "SKIP_AUTOUIC"))) {
                autogenDepends.push_back(
                  cmsys::SystemTools::GetRealPath(sf->GetFullPath()));
#if defined(_WIN32) && !defined(__CYGWIN__)
                // Cannot use PRE_BUILD with generated files
                usePRE_BUILD = false;
#endif
              }
            }
          }
        }
        // Process rcc enabled files
        if (rccEnabled && (ext == "qrc") &&
            !PropertyEnabled(sf, "SKIP_AUTORCC")) {
          const std::string absFile =
            cmsys::SystemTools::GetRealPath(sf->GetFullPath());

          // Compose rcc output file name
          {
            std::string rccOut = autogenBuildDir;
            rccOut += fpathCheckSum.getPart(absFile);
            rccOut += "/qrc_";
            rccOut +=
              cmsys::SystemTools::GetFilenameWithoutLastExtension(absFile);
            rccOut += ".cpp";

            // Register rcc output file as generated
            autogenProvides.push_back(rccOut);

            // Add rcc output file to origin target sources
            cmSourceFile* gf = makefile->GetOrCreateSource(rccOut, true);
            gf->SetProperty("SKIP_AUTOGEN", "On");
            target->AddSource(rccOut);
          }

          if (PropertyEnabled(sf, "GENERATED")) {
            // Add generated qrc file to the dependencies
            autogenDepends.push_back(absFile);
          } else {
            // Run cmake again when .qrc file changes
            makefile->AddCMakeDependFile(absFile);

            // Add the qrc input files to the dependencies
            std::string error;
            if (!cmQtAutoGeneratorCommon::RccListInputs(
                  qtMajorVersion, rccCommand, absFile, autogenDepends,
                  &error)) {
              cmSystemTools::Error(error.c_str());
            }
          }
#if defined(_WIN32) && !defined(__CYGWIN__)
          // Cannot use PRE_BUILD because the resource files themselves
          // may not be sources within the target so VS may not know the
          // target needs to re-build at all.
          usePRE_BUILD = false;
#endif
        }
      }
    }
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  if (usePRE_BUILD) {
    // If the autogen target depends on an other target don't use PRE_BUILD
    for (std::vector<std::string>::iterator it = autogenDepends.begin();
         it != autogenDepends.end(); ++it) {
      if (makefile->FindTargetToUse(*it) != CM_NULLPTR) {
        usePRE_BUILD = false;
        break;
      }
    }
  }
  if (usePRE_BUILD) {
    // Add the pre-build command directly to bypass the OBJECT_LIBRARY
    // rejection in cmMakefile::AddCustomCommandToTarget because we know
    // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
    std::vector<std::string> no_output;
    cmCustomCommand cc(makefile, no_output, autogenProvides, autogenDepends,
                       commandLines, autogenComment.c_str(),
                       workingDirectory.c_str());
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    target->Target->AddPreBuildCommand(cc);
  } else
#endif
  {
    cmTarget* autogenTarget = makefile->AddUtilityCommand(
      autogenTargetName, true, workingDirectory.c_str(),
      /*byproducts=*/autogenProvides, autogenDepends, commandLines, false,
      autogenComment.c_str());

    cmGeneratorTarget* gt = new cmGeneratorTarget(autogenTarget, lg);
    lg->AddGeneratorTarget(gt);

    // Set target folder
    const char* autogenFolder =
      makefile->GetState()->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
    if (!autogenFolder) {
      autogenFolder =
        makefile->GetState()->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
    }
    if (autogenFolder && *autogenFolder) {
      autogenTarget->SetProperty("FOLDER", autogenFolder);
    } else {
      // inherit FOLDER property from target (#13688)
      utilCopyTargetProperty(gt->Target, target->Target, "FOLDER");
    }

    target->Target->AddUtility(autogenTargetName);
  }
}

void cmQtAutoGeneratorInitializer::SetupAutoGenerateTarget(
  cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();

  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  std::map<std::string, std::string> configMocIncludes;
  std::map<std::string, std::string> configMocDefines;
  std::map<std::string, std::string> configUicOptions;
  {
    const bool mocEnabled = target->GetPropertyAsBool("AUTOMOC");
    const bool uicEnabled = target->GetPropertyAsBool("AUTOUIC");
    const bool rccEnabled = target->GetPropertyAsBool("AUTORCC");
    const std::string autogenTargetName = GetAutogenTargetName(target);
    const std::string qtMajorVersion = GetQtMajorVersion(target);

    std::vector<std::string> _sources;
    std::vector<std::string> _headers;

    if (mocEnabled || uicEnabled || rccEnabled) {
      std::vector<std::string> mocSkipList;
      std::vector<std::string> uicSkipList;
      AcquireScanFiles(target, _sources, _headers, mocSkipList, uicSkipList);
      if (mocEnabled) {
        MocSetupAutoTarget(target, autogenTargetName, qtMajorVersion,
                           mocSkipList, configMocIncludes, configMocDefines);
      }
      if (uicEnabled) {
        UicSetupAutoTarget(target, qtMajorVersion, uicSkipList,
                           configUicOptions);
      }
      if (rccEnabled) {
        RccSetupAutoTarget(target, qtMajorVersion);
      }
    }

    AddDefinitionEscaped(makefile, "_autogen_target_name", autogenTargetName);
    AddDefinitionEscaped(makefile, "_origin_target_name", target->GetName());
    AddDefinitionEscaped(makefile, "_qt_version_major", qtMajorVersion);
    AddDefinitionEscaped(makefile, "_sources", _sources);
    AddDefinitionEscaped(makefile, "_headers", _headers);
  }

  // Generate config file
  std::string inputFile = cmSystemTools::GetCMakeRoot();
  inputFile += "/Modules/AutogenInfo.cmake.in";
  std::string outputFile = GetAutogenTargetFilesDir(target);
  outputFile += "/AutogenInfo.cmake";

  makefile->ConfigureFile(inputFile.c_str(), outputFile.c_str(), false, true,
                          false);

  // Append custom config definitions to info file
  if (!configMocDefines.empty() || !configMocIncludes.empty() ||
      !configUicOptions.empty()) {

    // Ensure we have write permission in case .in was read-only.
    mode_t perm = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
    mode_t mode_write = S_IWRITE;
#else
    mode_t mode_write = S_IWUSR;
#endif
    cmSystemTools::GetPermissions(outputFile, perm);
    if (!(perm & mode_write)) {
      cmSystemTools::SetPermissions(outputFile, perm | mode_write);
    }

    cmsys::ofstream infoFile(outputFile.c_str(), std::ios::app);
    if (!infoFile) {
      std::string error = "Internal CMake error when trying to open file: ";
      error += outputFile;
      error += " for writing.";
      cmSystemTools::Error(error.c_str());
    } else {
      infoFile << "# Configuration specific options\n";
      if (!configMocDefines.empty()) {
        for (std::map<std::string, std::string>::iterator
               it = configMocDefines.begin(),
               end = configMocDefines.end();
             it != end; ++it) {
          infoFile << "set(AM_MOC_DEFINITIONS_" << it->first << " "
                   << it->second << ")\n";
        }
      }
      if (!configMocIncludes.empty()) {
        for (std::map<std::string, std::string>::iterator
               it = configMocIncludes.begin(),
               end = configMocIncludes.end();
             it != end; ++it) {
          infoFile << "set(AM_MOC_INCLUDES_" << it->first << " " << it->second
                   << ")\n";
        }
      }
      if (!configUicOptions.empty()) {
        for (std::map<std::string, std::string>::iterator
               it = configUicOptions.begin(),
               end = configUicOptions.end();
             it != end; ++it) {
          infoFile << "set(AM_UIC_TARGET_OPTIONS_" << it->first << " "
                   << it->second << ")\n";
        }
      }
    }
  }
}
