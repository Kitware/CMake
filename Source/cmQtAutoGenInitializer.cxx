/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenInitializer.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenGlobalInitializer.h"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmDuration.h"
#include "cmFilePathChecksum.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmProcessOutput.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmsys/FStream.hxx"
#include "cmsys/SystemInformation.hxx"

#include <algorithm>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

std::string GetQtExecutableTargetName(
  const cmQtAutoGen::IntegerVersion& qtVersion, std::string const& executable)
{
  if (qtVersion.Major == 6) {
    return ("Qt6::" + executable);
  }
  if (qtVersion.Major == 5) {
    return ("Qt5::" + executable);
  }
  if (qtVersion.Major == 4) {
    return ("Qt4::" + executable);
  }
  return ("");
}

static std::size_t GetParallelCPUCount()
{
  static std::size_t count = 0;
  // Detect only on the first call
  if (count == 0) {
    cmsys::SystemInformation info;
    info.RunCPUCheck();
    count = info.GetNumberOfPhysicalCPU();
    count = std::max<std::size_t>(count, 1);
    count = std::min<std::size_t>(count, cmQtAutoGen::ParallelMax);
  }
  return count;
}

static bool AddToSourceGroup(cmMakefile* makefile, std::string const& fileName,
                             cmQtAutoGen::GeneratorT genType)
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
        case cmQtAutoGen::GeneratorT::MOC:
          props[0] = "AUTOMOC_SOURCE_GROUP";
          break;
        case cmQtAutoGen::GeneratorT::RCC:
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
        cmSystemTools::Error(ost.str());
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

static std::string FileProjectRelativePath(cmMakefile* makefile,
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

cmQtAutoGenInitializer::InfoWriter::InfoWriter(std::string const& filename)
{
  Ofs_.SetCopyIfDifferent(true);
  Ofs_.Open(filename, false, true);
}

template <class IT>
std::string cmQtAutoGenInitializer::InfoWriter::ListJoin(IT it_begin,
                                                         IT it_end)
{
  std::string res;
  for (IT it = it_begin; it != it_end; ++it) {
    if (it != it_begin) {
      res += ';';
    }
    for (const char* c = it->c_str(); *c; ++c) {
      if (*c == '"') {
        // Escape the double quote to avoid ending the argument.
        res += "\\\"";
      } else if (*c == '$') {
        // Escape the dollar to avoid expanding variables.
        res += "\\$";
      } else if (*c == '\\') {
        // Escape the backslash to avoid other escapes.
        res += "\\\\";
      } else if (*c == ';') {
        // Escape the semicolon to avoid list expansion.
        res += "\\;";
      } else {
        // Other characters will be parsed correctly.
        res += *c;
      }
    }
  }
  return res;
}

std::string cmQtAutoGenInitializer::InfoWriter::ConfigKey(
  const char* key, std::string const& config)
{
  std::string ckey = key;
  ckey += '_';
  ckey += config;
  return ckey;
}

void cmQtAutoGenInitializer::InfoWriter::Write(const char* key,
                                               std::string const& value)
{
  Ofs_ << "set(" << key << " " << cmOutputConverter::EscapeForCMake(value)
       << ")\n";
};

void cmQtAutoGenInitializer::InfoWriter::WriteUInt(const char* key,
                                                   unsigned int value)
{
  Ofs_ << "set(" << key << " " << value << ")\n";
};

template <class C>
void cmQtAutoGenInitializer::InfoWriter::WriteStrings(const char* key,
                                                      C const& container)
{
  Ofs_ << "set(" << key << " \""
       << ListJoin(container.begin(), container.end()) << "\")\n";
}

void cmQtAutoGenInitializer::InfoWriter::WriteConfig(
  const char* key, std::map<std::string, std::string> const& map)
{
  for (auto const& item : map) {
    Write(ConfigKey(key, item.first).c_str(), item.second);
  }
};

template <class C>
void cmQtAutoGenInitializer::InfoWriter::WriteConfigStrings(
  const char* key, std::map<std::string, C> const& map)
{
  for (auto const& item : map) {
    WriteStrings(ConfigKey(key, item.first).c_str(), item.second);
  }
}

void cmQtAutoGenInitializer::InfoWriter::WriteNestedLists(
  const char* key, std::vector<std::vector<std::string>> const& lists)
{
  std::vector<std::string> seplist;
  for (const std::vector<std::string>& list : lists) {
    std::string blist = "{";
    blist += ListJoin(list.begin(), list.end());
    blist += "}";
    seplist.push_back(std::move(blist));
  }
  Write(key, cmJoin(seplist, cmQtAutoGen::ListSep));
};

cmQtAutoGenInitializer::cmQtAutoGenInitializer(
  cmQtAutoGenGlobalInitializer* globalInitializer, cmGeneratorTarget* target,
  IntegerVersion const& qtVersion, bool mocEnabled, bool uicEnabled,
  bool rccEnabled, bool globalAutogenTarget, bool globalAutoRccTarget)
  : GlobalInitializer(globalInitializer)
  , Target(target)
  , QtVersion(qtVersion)
{
  AutogenTarget.GlobalTarget = globalAutogenTarget;
  Moc.Enabled = mocEnabled;
  Uic.Enabled = uicEnabled;
  Rcc.Enabled = rccEnabled;
  Rcc.GlobalTarget = globalAutoRccTarget;
}

bool cmQtAutoGenInitializer::InitCustomTargets()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
  cmGlobalGenerator* globalGen = localGen->GetGlobalGenerator();

  // Configurations
  this->MultiConfig = globalGen->IsMultiConfig();
  this->ConfigDefault = makefile->GetConfigurations(this->ConfigsList);
  if (this->ConfigsList.empty()) {
    this->ConfigsList.push_back(this->ConfigDefault);
  }

  // Verbosity
  this->Verbosity = makefile->GetSafeDefinition("CMAKE_AUTOGEN_VERBOSE");
  if (!this->Verbosity.empty()) {
    unsigned long iVerb = 0;
    if (!cmSystemTools::StringToULong(this->Verbosity.c_str(), &iVerb)) {
      // Non numeric verbosity
      this->Verbosity = cmSystemTools::IsOn(this->Verbosity) ? "1" : "0";
    }
  }

  // Targets FOLDER
  {
    const char* folder =
      makefile->GetState()->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
    if (folder == nullptr) {
      folder =
        makefile->GetState()->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
    }
    // Inherit FOLDER property from target (#13688)
    if (folder == nullptr) {
      folder = this->Target->GetProperty("FOLDER");
    }
    if (folder != nullptr) {
      this->TargetsFolder = folder;
    }
  }

  // Common directories
  {
    // Collapsed current binary directory
    std::string const cbd = cmSystemTools::CollapseFullPath(
      std::string(), makefile->GetCurrentBinaryDirectory());

    // Info directory
    this->Dir.Info = cbd;
    this->Dir.Info += "/CMakeFiles";
    this->Dir.Info += '/';
    this->Dir.Info += this->Target->GetName();
    this->Dir.Info += "_autogen";
    this->Dir.Info += ".dir";
    cmSystemTools::ConvertToUnixSlashes(this->Dir.Info);

    // Build directory
    this->Dir.Build = this->Target->GetSafeProperty("AUTOGEN_BUILD_DIR");
    if (this->Dir.Build.empty()) {
      this->Dir.Build = cbd;
      this->Dir.Build += '/';
      this->Dir.Build += this->Target->GetName();
      this->Dir.Build += "_autogen";
    }
    cmSystemTools::ConvertToUnixSlashes(this->Dir.Build);
    // Cleanup build directory
    AddCleanFile(makefile, this->Dir.Build);

    // Working directory
    this->Dir.Work = cbd;
    cmSystemTools::ConvertToUnixSlashes(this->Dir.Work);

    // Include directory
    this->Dir.Include = this->Dir.Build;
    this->Dir.Include += "/include";
    if (this->MultiConfig) {
      this->Dir.Include += "_$<CONFIG>";
    }
    // Per config include directories
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::string& dir = this->Dir.ConfigInclude[cfg];
        dir = this->Dir.Build;
        dir += "/include_";
        dir += cfg;
      }
    }
  }

  // Moc, Uic and _autogen target settings
  if (this->Moc.Enabled || this->Uic.Enabled) {
    // Init moc specific settings
    if (this->Moc.Enabled && !InitMoc()) {
      return false;
    }

    // Init uic specific settings
    if (this->Uic.Enabled) {
      if (InitUic()) {
        auto* uicTarget = makefile->FindTargetToUse(
          GetQtExecutableTargetName(this->QtVersion, "uic"));
        if (uicTarget != nullptr) {
          this->AutogenTarget.DependTargets.insert(uicTarget);
        }
      } else {
        return false;
      }
    }

    // Autogen target name
    this->AutogenTarget.Name = this->Target->GetName();
    this->AutogenTarget.Name += "_autogen";

    // Autogen target parallel processing
    this->AutogenTarget.Parallel =
      this->Target->GetSafeProperty("AUTOGEN_PARALLEL");
    if (this->AutogenTarget.Parallel.empty() ||
        (this->AutogenTarget.Parallel == "AUTO")) {
      // Autodetect number of CPUs
      this->AutogenTarget.Parallel = std::to_string(GetParallelCPUCount());
    }

    // Autogen target info and settings files
    {
      this->AutogenTarget.InfoFile = this->Dir.Info;
      this->AutogenTarget.InfoFile += "/AutogenInfo.cmake";

      this->AutogenTarget.SettingsFile = this->Dir.Info;
      this->AutogenTarget.SettingsFile += "/AutogenOldSettings.txt";

      if (this->MultiConfig) {
        for (std::string const& cfg : this->ConfigsList) {
          std::string& filename = this->AutogenTarget.ConfigSettingsFile[cfg];
          filename =
            AppendFilenameSuffix(this->AutogenTarget.SettingsFile, "_" + cfg);
          AddCleanFile(makefile, filename);
        }
      } else {
        AddCleanFile(makefile, this->AutogenTarget.SettingsFile);
      }
    }

    // Autogen target: Compute user defined dependencies
    {
      this->AutogenTarget.DependOrigin =
        this->Target->GetPropertyAsBool("AUTOGEN_ORIGIN_DEPENDS");

      auto* mocTarget = makefile->FindTargetToUse(
        GetQtExecutableTargetName(this->QtVersion, "moc"));
      if (mocTarget != nullptr) {
        this->AutogenTarget.DependTargets.insert(mocTarget);
      }

      std::string const deps =
        this->Target->GetSafeProperty("AUTOGEN_TARGET_DEPENDS");
      if (!deps.empty()) {
        std::vector<std::string> extraDeps;
        cmSystemTools::ExpandListArgument(deps, extraDeps);
        for (std::string const& depName : extraDeps) {
          // Allow target and file dependencies
          auto* depTarget = makefile->FindTargetToUse(depName);
          if (depTarget != nullptr) {
            this->AutogenTarget.DependTargets.insert(depTarget);
          } else {
            this->AutogenTarget.DependFiles.insert(depName);
          }
        }
      }
    }
  }

  // Init rcc specific settings
  if (this->Rcc.Enabled && !InitRcc()) {
    return false;
  }

  // Add autogen include directory to the origin target INCLUDE_DIRECTORIES
  if (this->Moc.Enabled || this->Uic.Enabled ||
      (this->Rcc.Enabled && this->MultiConfig)) {
    this->Target->AddIncludeDirectory(this->Dir.Include, true);
  }

  // Scan files
  if (!this->InitScanFiles()) {
    return false;
  }

  // Create autogen target
  if ((this->Moc.Enabled || this->Uic.Enabled) && !this->InitAutogenTarget()) {
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
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmLocalGenerator* localGen = this->Target->GetLocalGenerator();

  // Mocs compilation file
  this->Moc.MocsCompilation = this->Dir.Build;
  this->Moc.MocsCompilation += "/mocs_compilation.cpp";

  // Moc predefs command
  if (this->Target->GetPropertyAsBool("AUTOMOC_COMPILER_PREDEFINES") &&
      (this->QtVersion >= IntegerVersion(5, 8))) {
    this->Moc.PredefsCmd =
      makefile->GetSafeDefinition("CMAKE_CXX_COMPILER_PREDEFINES_COMMAND");
  }

  // Moc includes
  {
    bool const appendImplicit = (this->QtVersion.Major >= 5);
    auto GetIncludeDirs =
      [this, localGen,
       appendImplicit](std::string const& cfg) -> std::vector<std::string> {
      // Get the include dirs for this target, without stripping the implicit
      // include dirs off, see
      // https://gitlab.kitware.com/cmake/cmake/issues/13667
      std::vector<std::string> dirs;
      localGen->GetIncludeDirectoriesImplicit(dirs, this->Target, "CXX", cfg,
                                              false, appendImplicit);
      return dirs;
    };

    // Default configuration include directories
    this->Moc.Includes = GetIncludeDirs(this->ConfigDefault);
    // Other configuration settings
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::vector<std::string> dirs = GetIncludeDirs(cfg);
        if (dirs != this->Moc.Includes) {
          this->Moc.ConfigIncludes[cfg] = std::move(dirs);
        }
      }
    }
  }

  // Moc compile definitions
  {
    auto GetCompileDefinitions =
      [this, localGen](std::string const& cfg) -> std::set<std::string> {
      std::set<std::string> defines;
      localGen->GetTargetDefines(this->Target, cfg, "CXX", defines);
#ifdef _WIN32
      if (this->Moc.PredefsCmd.empty()) {
        // Add WIN32 definition if we don't have a moc_predefs.h
        defines.insert("WIN32");
      }
#endif
      return defines;
    };

    // Default configuration defines
    this->Moc.Defines = GetCompileDefinitions(this->ConfigDefault);
    // Other configuration defines
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::set<std::string> defines = GetCompileDefinitions(cfg);
        if (defines != this->Moc.Defines) {
          this->Moc.ConfigDefines[cfg] = std::move(defines);
        }
      }
    }
  }

  // Moc executable
  return GetMocExecutable();
}

bool cmQtAutoGenInitializer::InitUic()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();

  // Uic search paths
  {
    std::string const usp =
      this->Target->GetSafeProperty("AUTOUIC_SEARCH_PATHS");
    if (!usp.empty()) {
      cmSystemTools::ExpandListArgument(usp, this->Uic.SearchPaths);
      std::string const& srcDir = makefile->GetCurrentSourceDirectory();
      for (std::string& path : this->Uic.SearchPaths) {
        path = cmSystemTools::CollapseFullPath(path, srcDir);
      }
    }
  }
  // Uic target options
  {
    auto UicGetOpts =
      [this](std::string const& cfg) -> std::vector<std::string> {
      std::vector<std::string> opts;
      this->Target->GetAutoUicOptions(opts, cfg);
      return opts;
    };

    // Default settings
    this->Uic.Options = UicGetOpts(this->ConfigDefault);

    // Configuration specific settings
    if (this->MultiConfig) {
      for (std::string const& cfg : this->ConfigsList) {
        std::vector<std::string> options = UicGetOpts(cfg);
        if (options != this->Uic.Options) {
          this->Uic.ConfigOptions[cfg] = std::move(options);
        }
      }
    }
  }

  // Uic executable
  return GetUicExecutable();
}

bool cmQtAutoGenInitializer::InitRcc()
{
  return GetRccExecutable();
}

bool cmQtAutoGenInitializer::InitScanFiles()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();

  // String constants
  std::string const SKIP_AUTOGEN_str = "SKIP_AUTOGEN";
  std::string const SKIP_AUTOMOC_str = "SKIP_AUTOMOC";
  std::string const SKIP_AUTOUIC_str = "SKIP_AUTOUIC";

  // Scan through target files
  {
    // String constants
    std::string const qrc_str = "qrc";
    std::string const SKIP_AUTORCC_str = "SKIP_AUTORCC";
    std::string const AUTORCC_OPTIONS_str = "AUTORCC_OPTIONS";

    // Scan through target files
    std::vector<cmSourceFile*> srcFiles;
    this->Target->GetConfigCommonSourceFiles(srcFiles);
    for (cmSourceFile* sf : srcFiles) {
      if (sf->GetPropertyAsBool(SKIP_AUTOGEN_str)) {
        continue;
      }

      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      std::string const& fPath = sf->GetFullPath();
      std::string const& ext = sf->GetExtension();

      // Register generated files that will be scanned by moc or uic
      if (this->Moc.Enabled || this->Uic.Enabled) {
        cmSystemTools::FileFormat const fileType =
          cmSystemTools::GetFileFormat(ext);
        if ((fileType == cmSystemTools::CXX_FILE_FORMAT) ||
            (fileType == cmSystemTools::HEADER_FILE_FORMAT)) {
          std::string const absPath = cmSystemTools::GetRealPath(fPath);
          if ((this->Moc.Enabled &&
               !sf->GetPropertyAsBool(SKIP_AUTOMOC_str)) ||
              (this->Uic.Enabled &&
               !sf->GetPropertyAsBool(SKIP_AUTOUIC_str))) {
            // Register source
            const bool generated = sf->GetIsGenerated();
            if (fileType == cmSystemTools::HEADER_FILE_FORMAT) {
              if (generated) {
                this->AutogenTarget.HeadersGenerated.push_back(absPath);
              } else {
                this->AutogenTarget.Headers.push_back(absPath);
              }
            } else {
              if (generated) {
                this->AutogenTarget.SourcesGenerated.push_back(absPath);
              } else {
                this->AutogenTarget.Sources.push_back(absPath);
              }
            }
          }
        }
      }
      // Register rcc enabled files
      if (this->Rcc.Enabled) {
        if ((ext == qrc_str) && !sf->GetPropertyAsBool(SKIP_AUTORCC_str)) {
          // Register qrc file
          Qrc qrc;
          qrc.QrcFile = cmSystemTools::GetRealPath(fPath);
          qrc.QrcName =
            cmSystemTools::GetFilenameWithoutLastExtension(qrc.QrcFile);
          qrc.Generated = sf->GetIsGenerated();
          // RCC options
          {
            std::string const opts = sf->GetSafeProperty(AUTORCC_OPTIONS_str);
            if (!opts.empty()) {
              cmSystemTools::ExpandListArgument(opts, qrc.Options);
            }
          }
          this->Rcc.Qrcs.push_back(std::move(qrc));
        }
      }
    }
  }
  // cmGeneratorTarget::GetConfigCommonSourceFiles computes the target's
  // sources meta data cache. Clear it so that OBJECT library targets that
  // are AUTOGEN initialized after this target get their added
  // mocs_compilation.cpp source acknowledged by this target.
  this->Target->ClearSourcesCache();

  // Scan through all source files in the makefile to extract moc and uic
  // parameters.  Historically we support non target source file parameters.
  // The reason is that their file names might be discovered from source files
  // at generation time.
  if (this->Moc.Enabled || this->Uic.Enabled) {
    // String constants
    std::string const ui_str = "ui";
    std::string const AUTOUIC_OPTIONS_str = "AUTOUIC_OPTIONS";

    for (cmSourceFile* sf : makefile->GetSourceFiles()) {
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      // Since we're iterating over source files that might be not in the
      // target we need to check for path errors (not existing files).
      std::string pathError;
      std::string const& fullPath = sf->GetFullPath(&pathError);
      if (!pathError.empty() || fullPath.empty()) {
        continue;
      }

      // Check file type
      auto const fileType = cmSystemTools::GetFileFormat(sf->GetExtension());
      bool const isSource = (fileType == cmSystemTools::CXX_FILE_FORMAT) ||
        (fileType == cmSystemTools::HEADER_FILE_FORMAT);
      bool const isUi = (this->Moc.Enabled && sf->GetExtension() == ui_str);

      // Process only certain file types
      if (isSource || isUi) {
        std::string const absFile = cmSystemTools::GetRealPath(fullPath);
        // Acquire file properties
        bool const skipAUTOGEN = sf->GetPropertyAsBool(SKIP_AUTOGEN_str);
        bool const skipMoc = (this->Moc.Enabled && isSource) &&
          (skipAUTOGEN || sf->GetPropertyAsBool(SKIP_AUTOMOC_str));
        bool const skipUic = this->Uic.Enabled &&
          (skipAUTOGEN || sf->GetPropertyAsBool(SKIP_AUTOUIC_str));

        // Register moc and uic skipped file
        if (skipMoc) {
          this->Moc.Skip.insert(absFile);
        }
        if (skipUic) {
          this->Uic.Skip.insert(absFile);
        }

        // Check if the .ui file has uic options
        if (isUi && !skipUic) {
          std::string const uicOpts = sf->GetSafeProperty(AUTOUIC_OPTIONS_str);
          if (!uicOpts.empty()) {
            this->Uic.FileFiles.push_back(absFile);
            std::vector<std::string> optsVec;
            cmSystemTools::ExpandListArgument(uicOpts, optsVec);
            this->Uic.FileOptions.push_back(std::move(optsVec));
          }
        }
      }
    }
  }

  // Process GENERATED sources and headers
  if (this->Moc.Enabled || this->Uic.Enabled) {
    if (!this->AutogenTarget.SourcesGenerated.empty() ||
        !this->AutogenTarget.HeadersGenerated.empty()) {
      // Check status of policy CMP0071
      bool policyAccept = false;
      bool policyWarn = false;
      cmPolicies::PolicyStatus const CMP0071_status =
        makefile->GetPolicyStatus(cmPolicies::CMP0071);
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
        for (std::string const& absFile :
             this->AutogenTarget.HeadersGenerated) {
          this->AutogenTarget.Headers.push_back(absFile);
          this->AutogenTarget.DependFiles.insert(absFile);
        }
        for (std::string const& absFile :
             this->AutogenTarget.SourcesGenerated) {
          this->AutogenTarget.Sources.push_back(absFile);
          this->AutogenTarget.DependFiles.insert(absFile);
        }
      } else {
        if (policyWarn) {
          std::string msg;
          msg += cmPolicies::GetPolicyWarning(cmPolicies::CMP0071);
          msg += "\n";
          std::string tools;
          std::string property;
          if (this->Moc.Enabled && this->Uic.Enabled) {
            tools = "AUTOMOC and AUTOUIC";
            property = "SKIP_AUTOGEN";
          } else if (this->Moc.Enabled) {
            tools = "AUTOMOC";
            property = "SKIP_AUTOMOC";
          } else if (this->Uic.Enabled) {
            tools = "AUTOUIC";
            property = "SKIP_AUTOUIC";
          }
          msg += "For compatibility, CMake is excluding the GENERATED source "
                 "file(s):\n";
          for (const std::string& absFile :
               this->AutogenTarget.HeadersGenerated) {
            msg.append("  ").append(Quoted(absFile)).append("\n");
          }
          for (const std::string& absFile :
               this->AutogenTarget.SourcesGenerated) {
            msg.append("  ").append(Quoted(absFile)).append("\n");
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
          makefile->IssueMessage(MessageType::AUTHOR_WARNING, msg);
        }
      }
    }
  }

  // Sort headers and sources
  if (this->Moc.Enabled || this->Uic.Enabled) {
    std::sort(this->AutogenTarget.Headers.begin(),
              this->AutogenTarget.Headers.end());
    std::sort(this->AutogenTarget.Sources.begin(),
              this->AutogenTarget.Sources.end());
  }

  // Process qrc files
  if (!this->Rcc.Qrcs.empty()) {
    const bool modernQt = (this->QtVersion.Major >= 5);
    // Target rcc options
    std::vector<std::string> optionsTarget;
    cmSystemTools::ExpandListArgument(
      this->Target->GetSafeProperty("AUTORCC_OPTIONS"), optionsTarget);

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
    {
      cmFilePathChecksum const fpathCheckSum(makefile);
      for (Qrc& qrc : this->Rcc.Qrcs) {
        qrc.PathChecksum = fpathCheckSum.getPart(qrc.QrcFile);
        // RCC output file name
        {
          std::string rccFile = this->Dir.Build + "/";
          rccFile += qrc.PathChecksum;
          rccFile += "/qrc_";
          rccFile += qrc.QrcName;
          rccFile += ".cpp";
          qrc.RccFile = std::move(rccFile);
        }
        {
          std::string base = this->Dir.Info;
          base += "/RCC";
          base += qrc.QrcName;
          if (!qrc.Unique) {
            base += qrc.PathChecksum;
          }

          qrc.LockFile = base;
          qrc.LockFile += ".lock";

          qrc.InfoFile = base;
          qrc.InfoFile += "Info.cmake";

          qrc.SettingsFile = base;
          qrc.SettingsFile += "Settings.txt";

          if (this->MultiConfig) {
            for (std::string const& cfg : this->ConfigsList) {
              qrc.ConfigSettingsFile[cfg] =
                AppendFilenameSuffix(qrc.SettingsFile, "_" + cfg);
            }
          }
        }
      }
    }
    // RCC options
    for (Qrc& qrc : this->Rcc.Qrcs) {
      // Target options
      std::vector<std::string> opts = optionsTarget;
      // Merge computed "-name XYZ" option
      {
        std::string name = qrc.QrcName;
        // Replace '-' with '_'. The former is not valid for symbol names.
        std::replace(name.begin(), name.end(), '-', '_');
        if (!qrc.Unique) {
          name += "_";
          name += qrc.PathChecksum;
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
    // RCC resources
    for (Qrc& qrc : this->Rcc.Qrcs) {
      if (!qrc.Generated) {
        std::string error;
        if (!RccListInputs(qrc.QrcFile, qrc.Resources, error)) {
          cmSystemTools::Error(error);
          return false;
        }
      }
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitAutogenTarget()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
  cmGlobalGenerator* globalGen = localGen->GetGlobalGenerator();

  // Register info file as generated by CMake
  makefile->AddCMakeOutputFile(this->AutogenTarget.InfoFile);

  // Files provided by the autogen target
  std::vector<std::string> autogenProvides;
  if (this->Moc.Enabled) {
    this->AddGeneratedSource(this->Moc.MocsCompilation, GeneratorT::MOC, true);
    autogenProvides.push_back(this->Moc.MocsCompilation);
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
    autogenComment = "Automatic ";
    autogenComment += tools;
    autogenComment += " for target ";
    autogenComment += this->Target->GetName();
  }

  // Compose command lines
  cmCustomCommandLines commandLines;
  {
    cmCustomCommandLine currentLine;
    currentLine.push_back(cmSystemTools::GetCMakeCommand());
    currentLine.push_back("-E");
    currentLine.push_back("cmake_autogen");
    currentLine.push_back(this->AutogenTarget.InfoFile);
    currentLine.push_back("$<CONFIGURATION>");
    commandLines.push_back(std::move(currentLine));
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
    if (!this->AutogenTarget.DependFiles.empty()) {
      usePRE_BUILD = false;
    }
    // Cannot use PRE_BUILD when a global autogen target is in place
    if (AutogenTarget.GlobalTarget) {
      usePRE_BUILD = false;
    }
  }
  // Create the autogen target/command
  if (usePRE_BUILD) {
    // Add additional autogen target dependencies to origin target
    for (cmTarget* depTarget : this->AutogenTarget.DependTargets) {
      this->Target->Target->AddUtility(depTarget->GetName(), makefile);
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
                       this->Dir.Work.c_str());
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    this->Target->Target->AddPreBuildCommand(cc);
  } else {

    // Add link library target dependencies to the autogen target
    // dependencies
    if (this->AutogenTarget.DependOrigin) {
      // add_dependencies/addUtility do not support generator expressions.
      // We depend only on the libraries found in all configs therefore.
      std::map<cmGeneratorTarget const*, std::size_t> commonTargets;
      for (std::string const& config : this->ConfigsList) {
        cmLinkImplementationLibraries const* libs =
          this->Target->GetLinkImplementationLibraries(config);
        if (libs != nullptr) {
          for (cmLinkItem const& item : libs->Libraries) {
            cmGeneratorTarget const* libTarget = item.Target;
            if ((libTarget != nullptr) &&
                !StaticLibraryCycle(this->Target, libTarget, config)) {
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

    // Create autogen target
    cmTarget* autogenTarget = makefile->AddUtilityCommand(
      this->AutogenTarget.Name, cmMakefile::TargetOrigin::Generator, true,
      this->Dir.Work.c_str(), /*byproducts=*/autogenProvides,
      std::vector<std::string>(this->AutogenTarget.DependFiles.begin(),
                               this->AutogenTarget.DependFiles.end()),
      commandLines, false, autogenComment.c_str());
    // Create autogen generator target
    localGen->AddGeneratorTarget(
      new cmGeneratorTarget(autogenTarget, localGen));

    // Forward origin utilities to autogen target
    if (this->AutogenTarget.DependOrigin) {
      for (BT<std::string> const& depName : this->Target->GetUtilities()) {
        autogenTarget->AddUtility(depName.Value, makefile);
      }
    }
    // Add additional autogen target dependencies to autogen target
    for (cmTarget* depTarget : this->AutogenTarget.DependTargets) {
      autogenTarget->AddUtility(depTarget->GetName(), makefile);
    }

    // Set FOLDER property in autogen target
    if (!this->TargetsFolder.empty()) {
      autogenTarget->SetProperty("FOLDER", this->TargetsFolder.c_str());
    }

    // Add autogen target to the origin target dependencies
    this->Target->Target->AddUtility(this->AutogenTarget.Name, makefile);

    // Add autogen target to the global autogen target dependencies
    if (this->AutogenTarget.GlobalTarget) {
      this->GlobalInitializer->AddToGlobalAutoGen(localGen,
                                                  this->AutogenTarget.Name);
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitRccTargets()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
  auto rccTargetName = GetQtExecutableTargetName(this->QtVersion, "rcc");

  for (Qrc const& qrc : this->Rcc.Qrcs) {
    // Register info file as generated by CMake
    makefile->AddCMakeOutputFile(qrc.InfoFile);
    // Register file at target
    this->AddGeneratedSource(qrc.RccFile, GeneratorT::RCC);

    std::vector<std::string> ccOutput;
    ccOutput.push_back(qrc.RccFile);

    std::vector<std::string> ccDepends;
    // Add the .qrc and info file to the custom command dependencies
    ccDepends.push_back(qrc.QrcFile);
    ccDepends.push_back(qrc.InfoFile);

    cmCustomCommandLines commandLines;
    if (this->MultiConfig) {
      // Build for all configurations
      for (std::string const& config : this->ConfigsList) {
        cmCustomCommandLine currentLine;
        currentLine.push_back(cmSystemTools::GetCMakeCommand());
        currentLine.push_back("-E");
        currentLine.push_back("cmake_autorcc");
        currentLine.push_back(qrc.InfoFile);
        currentLine.push_back(config);
        commandLines.push_back(std::move(currentLine));
      }
    } else {
      cmCustomCommandLine currentLine;
      currentLine.push_back(cmSystemTools::GetCMakeCommand());
      currentLine.push_back("-E");
      currentLine.push_back("cmake_autorcc");
      currentLine.push_back(qrc.InfoFile);
      currentLine.push_back("$<CONFIG>");
      commandLines.push_back(std::move(currentLine));
    }
    std::string ccComment = "Automatic RCC for ";
    ccComment += FileProjectRelativePath(makefile, qrc.QrcFile);

    if (qrc.Generated || this->Rcc.GlobalTarget) {
      // Create custom rcc target
      std::string ccName;
      {
        ccName = this->Target->GetName();
        ccName += "_arcc_";
        ccName += qrc.QrcName;
        if (!qrc.Unique) {
          ccName += "_";
          ccName += qrc.PathChecksum;
        }

        cmTarget* autoRccTarget = makefile->AddUtilityCommand(
          ccName, cmMakefile::TargetOrigin::Generator, true,
          this->Dir.Work.c_str(), ccOutput, ccDepends, commandLines, false,
          ccComment.c_str());

        // Create autogen generator target
        localGen->AddGeneratorTarget(
          new cmGeneratorTarget(autoRccTarget, localGen));

        // Set FOLDER property in autogen target
        if (!this->TargetsFolder.empty()) {
          autoRccTarget->SetProperty("FOLDER", this->TargetsFolder.c_str());
        }
        if (!rccTargetName.empty()) {
          autoRccTarget->AddUtility(rccTargetName, makefile);
        }
      }
      // Add autogen target to the origin target dependencies
      this->Target->Target->AddUtility(ccName, makefile);

      // Add autogen target to the global autogen target dependencies
      if (this->Rcc.GlobalTarget) {
        this->GlobalInitializer->AddToGlobalAutoRcc(localGen, ccName);
      }
    } else {
      // Create custom rcc command
      {
        std::vector<std::string> ccByproducts;

        // Add the resource files to the dependencies
        for (std::string const& fileName : qrc.Resources) {
          // Add resource file to the custom command dependencies
          ccDepends.push_back(fileName);
        }
        if (!rccTargetName.empty()) {
          ccDepends.push_back(rccTargetName);
        }
        makefile->AddCustomCommandToOutput(ccOutput, ccByproducts, ccDepends,
                                           /*main_dependency*/ std::string(),
                                           commandLines, ccComment.c_str(),
                                           this->Dir.Work.c_str());
      }
      // Reconfigure when .qrc file changes
      makefile->AddCMakeDependFile(qrc.QrcFile);
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::SetupCustomTargets()
{
  // Create info directory on demand
  if (!cmSystemTools::MakeDirectory(this->Dir.Info)) {
    std::string emsg = ("AutoGen: Could not create directory: ");
    emsg += Quoted(this->Dir.Info);
    cmSystemTools::Error(emsg);
    return false;
  }

  // Generate autogen target info file
  if (this->Moc.Enabled || this->Uic.Enabled) {
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
  InfoWriter ofs(this->AutogenTarget.InfoFile);
  if (ofs) {
    // Utility lambdas
    cmMakefile* makefile = this->Target->Target->GetMakefile();
    auto MfDef = [makefile](const char* key) {
      return makefile->GetSafeDefinition(key);
    };

    // Write common settings
    ofs.Write("# Meta\n");
    ofs.Write("AM_MULTI_CONFIG", this->MultiConfig ? "TRUE" : "FALSE");
    ofs.Write("AM_PARALLEL", this->AutogenTarget.Parallel);
    ofs.Write("AM_VERBOSITY", this->Verbosity);

    ofs.Write("# Directories\n");
    ofs.Write("AM_CMAKE_SOURCE_DIR", MfDef("CMAKE_SOURCE_DIR"));
    ofs.Write("AM_CMAKE_BINARY_DIR", MfDef("CMAKE_BINARY_DIR"));
    ofs.Write("AM_CMAKE_CURRENT_SOURCE_DIR",
              MfDef("CMAKE_CURRENT_SOURCE_DIR"));
    ofs.Write("AM_CMAKE_CURRENT_BINARY_DIR",
              MfDef("CMAKE_CURRENT_BINARY_DIR"));
    ofs.Write("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE",
              MfDef("CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE"));
    ofs.Write("AM_BUILD_DIR", this->Dir.Build);
    ofs.Write("AM_INCLUDE_DIR", this->Dir.Include);
    ofs.WriteConfig("AM_INCLUDE_DIR", this->Dir.ConfigInclude);

    ofs.Write("# Files\n");
    ofs.WriteStrings("AM_SOURCES", this->AutogenTarget.Sources);
    ofs.WriteStrings("AM_HEADERS", this->AutogenTarget.Headers);
    ofs.Write("AM_SETTINGS_FILE", this->AutogenTarget.SettingsFile);
    ofs.WriteConfig("AM_SETTINGS_FILE",
                    this->AutogenTarget.ConfigSettingsFile);

    ofs.Write("# Qt\n");
    ofs.WriteUInt("AM_QT_VERSION_MAJOR", this->QtVersion.Major);
    ofs.Write("AM_QT_MOC_EXECUTABLE", this->Moc.Executable);
    ofs.Write("AM_QT_UIC_EXECUTABLE", this->Uic.Executable);

    // Write moc settings
    if (this->Moc.Enabled) {
      ofs.Write("# MOC settings\n");
      ofs.WriteStrings("AM_MOC_SKIP", this->Moc.Skip);
      ofs.WriteStrings("AM_MOC_DEFINITIONS", this->Moc.Defines);
      ofs.WriteConfigStrings("AM_MOC_DEFINITIONS", this->Moc.ConfigDefines);
      ofs.WriteStrings("AM_MOC_INCLUDES", this->Moc.Includes);
      ofs.WriteConfigStrings("AM_MOC_INCLUDES", this->Moc.ConfigIncludes);
      ofs.Write("AM_MOC_OPTIONS",
                this->Target->GetSafeProperty("AUTOMOC_MOC_OPTIONS"));
      ofs.Write("AM_MOC_RELAXED_MODE", MfDef("CMAKE_AUTOMOC_RELAXED_MODE"));
      ofs.Write("AM_MOC_MACRO_NAMES",
                this->Target->GetSafeProperty("AUTOMOC_MACRO_NAMES"));
      ofs.Write("AM_MOC_DEPEND_FILTERS",
                this->Target->GetSafeProperty("AUTOMOC_DEPEND_FILTERS"));
      ofs.Write("AM_MOC_PREDEFS_CMD", this->Moc.PredefsCmd);
    }

    // Write uic settings
    if (this->Uic.Enabled) {
      ofs.Write("# UIC settings\n");
      ofs.WriteStrings("AM_UIC_SKIP", this->Uic.Skip);
      ofs.WriteStrings("AM_UIC_TARGET_OPTIONS", this->Uic.Options);
      ofs.WriteConfigStrings("AM_UIC_TARGET_OPTIONS", this->Uic.ConfigOptions);
      ofs.WriteStrings("AM_UIC_OPTIONS_FILES", this->Uic.FileFiles);
      ofs.WriteNestedLists("AM_UIC_OPTIONS_OPTIONS", this->Uic.FileOptions);
      ofs.WriteStrings("AM_UIC_SEARCH_PATHS", this->Uic.SearchPaths);
    }
  } else {
    std::string err = "AutoGen: Could not write file ";
    err += this->AutogenTarget.InfoFile;
    cmSystemTools::Error(err);
    return false;
  }

  return true;
}

bool cmQtAutoGenInitializer::SetupWriteRccInfo()
{
  for (Qrc const& qrc : this->Rcc.Qrcs) {
    InfoWriter ofs(qrc.InfoFile);
    if (ofs) {
      // Write
      ofs.Write("# Configurations\n");
      ofs.Write("ARCC_MULTI_CONFIG", this->MultiConfig ? "TRUE" : "FALSE");
      ofs.Write("ARCC_VERBOSITY", this->Verbosity);
      ofs.Write("# Settings file\n");
      ofs.Write("ARCC_SETTINGS_FILE", qrc.SettingsFile);
      ofs.WriteConfig("ARCC_SETTINGS_FILE", qrc.ConfigSettingsFile);

      ofs.Write("# Directories\n");
      ofs.Write("ARCC_BUILD_DIR", this->Dir.Build);
      ofs.Write("ARCC_INCLUDE_DIR", this->Dir.Include);
      ofs.WriteConfig("ARCC_INCLUDE_DIR", this->Dir.ConfigInclude);

      ofs.Write("# Rcc executable\n");
      ofs.Write("ARCC_RCC_EXECUTABLE", this->Rcc.Executable);
      ofs.WriteStrings("ARCC_RCC_LIST_OPTIONS", this->Rcc.ListOptions);

      ofs.Write("# Rcc job\n");
      ofs.Write("ARCC_LOCK_FILE", qrc.LockFile);
      ofs.Write("ARCC_SOURCE", qrc.QrcFile);
      ofs.Write("ARCC_OUTPUT_CHECKSUM", qrc.PathChecksum);
      ofs.Write("ARCC_OUTPUT_NAME",
                cmSystemTools::GetFilenameName(qrc.RccFile));
      ofs.WriteStrings("ARCC_OPTIONS", qrc.Options);
      ofs.WriteStrings("ARCC_INPUTS", qrc.Resources);
    } else {
      std::string err = "AutoRcc: Could not write file ";
      err += qrc.InfoFile;
      cmSystemTools::Error(err);
      return false;
    }
  }

  return true;
}

void cmQtAutoGenInitializer::AddGeneratedSource(std::string const& filename,
                                                GeneratorT genType,
                                                bool prepend)
{
  // Register source file in makefile
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  {
    cmSourceFile* gFile = makefile->GetOrCreateSource(filename, true);
    gFile->SetProperty("GENERATED", "1");
    gFile->SetProperty("SKIP_AUTOGEN", "On");
  }

  // Add source file to source group
  AddToSourceGroup(makefile, filename, genType);

  // Add source file to target
  this->Target->AddSource(filename, prepend);
}

static unsigned int CharPtrToUInt(const char* const input)
{
  unsigned long tmp = 0;
  if (input != nullptr && cmSystemTools::StringToULong(input, &tmp)) {
    return static_cast<unsigned int>(tmp);
  }
  return 0;
}

static std::vector<cmQtAutoGen::IntegerVersion> GetKnownQtVersions(
  cmGeneratorTarget const* target)
{
  cmMakefile* makefile = target->Target->GetMakefile();
  std::vector<cmQtAutoGen::IntegerVersion> result;
  // Adds a version to the result (nullptr safe)
  auto addVersion = [&result](const char* major, const char* minor) {
    cmQtAutoGen::IntegerVersion ver(CharPtrToUInt(major),
                                    CharPtrToUInt(minor));
    if (ver.Major != 0) {
      result.emplace_back(ver);
    }
  };
  // Qt version variable prefixes
  std::array<std::string, 3> const prefixes{ { "Qt6Core", "Qt5Core", "QT" } };

  // Read versions from variables
  for (const std::string& prefix : prefixes) {
    addVersion(makefile->GetDefinition(prefix + "_VERSION_MAJOR"),
               makefile->GetDefinition(prefix + "_VERSION_MINOR"));
  }

  // Read versions from directory properties
  for (const std::string& prefix : prefixes) {
    addVersion(makefile->GetProperty(prefix + "_VERSION_MAJOR"),
               makefile->GetProperty(prefix + "_VERSION_MINOR"));
  }

  return result;
}

std::pair<cmQtAutoGen::IntegerVersion, unsigned int>
cmQtAutoGenInitializer::GetQtVersion(cmGeneratorTarget const* target)
{
  std::pair<IntegerVersion, unsigned int> res(
    IntegerVersion(),
    CharPtrToUInt(target->GetLinkInterfaceDependentStringProperty(
      "QT_MAJOR_VERSION", "")));

  auto knownQtVersions = GetKnownQtVersions(target);
  if (!knownQtVersions.empty()) {
    if (res.second == 0) {
      // No specific version was requested by the target:
      // Use highest known Qt version.
      res.first = knownQtVersions.at(0);
    } else {
      // Pick a version from the known versions:
      for (auto it : knownQtVersions) {
        if (it.Major == res.second) {
          res.first = it;
          break;
        }
      }
    }
  }
  return res;
}

std::pair<bool, std::string> cmQtAutoGenInitializer::GetQtExecutable(
  const std::string& executable, bool ignoreMissingTarget, std::string* output)
{
  const std::string upperExecutable = cmSystemTools::UpperCase(executable);
  std::string result = this->Target->Target->GetSafeProperty(
    "AUTO" + upperExecutable + "_EXECUTABLE");
  if (!result.empty()) {
    cmListFileBacktrace lfbt =
      this->Target->Target->GetMakefile()->GetBacktrace();
    cmGeneratorExpression ge(lfbt);
    std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(result);
    result = cge->Evaluate(this->Target->GetLocalGenerator(), "");

    return std::make_pair(true, result);
  }

  std::string err;

  // Find executable
  {
    const std::string targetName =
      GetQtExecutableTargetName(this->QtVersion, executable);
    if (targetName.empty()) {
      err = "The AUTO" + upperExecutable + " feature ";
      err += "supports only Qt 4, Qt 5 and Qt 6.";
    } else {
      cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
      cmGeneratorTarget* tgt = localGen->FindGeneratorTargetToUse(targetName);
      if (tgt != nullptr) {
        if (tgt->IsImported()) {
          result = tgt->ImportedGetLocation("");
        } else {
          result = tgt->GetLocation("");
        }
      } else {
        if (ignoreMissingTarget) {
          return std::make_pair(true, "");
        }

        err = "Could not find target " + targetName;
      }
    }
  }

  // Test executable
  if (err.empty()) {
    this->GlobalInitializer->GetExecutableTestOutput(executable, result, err,
                                                     output);
  }

  // Print error
  if (!err.empty()) {
    std::string msg = "AutoGen (";
    msg += this->Target->GetName();
    msg += "): ";
    msg += err;
    cmSystemTools::Error(msg);
    return std::make_pair(false, "");
  }

  return std::make_pair(true, result);
}

bool cmQtAutoGenInitializer::GetMocExecutable()
{
  const auto result = this->GetQtExecutable("moc", false, nullptr);
  this->Moc.Executable = result.second;
  return result.first;
}

bool cmQtAutoGenInitializer::GetUicExecutable()
{
  const auto result = this->GetQtExecutable("uic", true, nullptr);
  this->Uic.Executable = result.second;
  return result.first;
}

bool cmQtAutoGenInitializer::GetRccExecutable()
{
  std::string stdOut;
  const auto result = this->GetQtExecutable("rcc", false, &stdOut);
  this->Rcc.Executable = result.second;
  if (!result.first) {
    return false;
  }

  if (this->QtVersion.Major == 5 || this->QtVersion.Major == 6) {
    if (stdOut.find("--list") != std::string::npos) {
      this->Rcc.ListOptions.emplace_back("--list");
    } else {
      this->Rcc.ListOptions.emplace_back("-list");
    }
  }
  return true;
}

/// @brief Reads the resource files list from from a .qrc file
/// @arg fileName Must be the absolute path of the .qrc file
/// @return True if the rcc file was successfully read
bool cmQtAutoGenInitializer::RccListInputs(std::string const& fileName,
                                           std::vector<std::string>& files,
                                           std::string& error)
{
  if (!cmSystemTools::FileExists(fileName)) {
    error = "rcc resource file does not exist:\n  ";
    error += Quoted(fileName);
    error += "\n";
    return false;
  }
  if (!this->Rcc.ListOptions.empty()) {
    // Use rcc for file listing
    if (this->Rcc.Executable.empty()) {
      error = "rcc executable not available";
      return false;
    }

    // Run rcc list command in the directory of the qrc file with the
    // pathless
    // qrc file name argument. This way rcc prints relative paths.
    // This avoids issues on Windows when the qrc file is in a path that
    // contains non-ASCII characters.
    std::string const fileDir = cmSystemTools::GetFilenamePath(fileName);
    std::string const fileNameName = cmSystemTools::GetFilenameName(fileName);

    bool result = false;
    int retVal = 0;
    std::string rccStdOut;
    std::string rccStdErr;
    {
      std::vector<std::string> cmd;
      cmd.push_back(this->Rcc.Executable);
      cmd.insert(cmd.end(), this->Rcc.ListOptions.begin(),
                 this->Rcc.ListOptions.end());
      cmd.push_back(fileNameName);
      result = cmSystemTools::RunSingleCommand(
        cmd, &rccStdOut, &rccStdErr, &retVal, fileDir.c_str(),
        cmSystemTools::OUTPUT_NONE, cmDuration::zero(), cmProcessOutput::Auto);
    }
    if (!result || retVal) {
      error = "rcc list process failed for:\n  ";
      error += Quoted(fileName);
      error += "\n";
      error += rccStdOut;
      error += "\n";
      error += rccStdErr;
      error += "\n";
      return false;
    }
    if (!RccListParseOutput(rccStdOut, rccStdErr, files, error)) {
      return false;
    }
  } else {
    // We can't use rcc for the file listing.
    // Read the qrc file content into string and parse it.
    {
      std::string qrcContents;
      {
        cmsys::ifstream ifs(fileName.c_str());
        if (ifs) {
          std::ostringstream osst;
          osst << ifs.rdbuf();
          qrcContents = osst.str();
        } else {
          error = "rcc file not readable:\n  ";
          error += Quoted(fileName);
          error += "\n";
          return false;
        }
      }
      // Parse string content
      RccListParseContent(qrcContents, files);
    }
  }

  // Convert relative paths to absolute paths
  RccListConvertFullPath(cmSystemTools::GetFilenamePath(fileName), files);
  return true;
}
