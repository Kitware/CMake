/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenInitializer.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenGlobalInitializer.h"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
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
#include "cmSourceFile.h"
#include "cmSourceFileLocationKind.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmake.h"
#include "cmsys/SystemInformation.hxx"

#include <algorithm>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

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

/**
 * Tests if targetDepend is a STATIC_LIBRARY and if any of its
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

  // Check status of policy CMP0071
  {
    cmPolicies::PolicyStatus const CMP0071_status =
      makefile->GetPolicyStatus(cmPolicies::CMP0071);
    switch (CMP0071_status) {
      case cmPolicies::WARN:
        this->CMP0071Warn = true;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // Ignore GENERATED file
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        // Process GENERATED file
        this->CMP0071Accept = true;
        break;
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
    this->AddCleanFile(this->Dir.Build);

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
  if (this->MocOrUicEnabled()) {
    // Init moc specific settings
    if (this->Moc.Enabled && !InitMoc()) {
      return false;
    }

    // Init uic specific settings
    if (this->Uic.Enabled && !InitUic()) {
      return false;
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
          this->AddCleanFile(filename);
        }
      } else {
        this->AddCleanFile(this->AutogenTarget.SettingsFile);
      }

      this->AutogenTarget.ParseCacheFile = this->Dir.Info;
      this->AutogenTarget.ParseCacheFile += "/ParseCache.txt";
      this->AddCleanFile(this->AutogenTarget.ParseCacheFile);
    }

    // Autogen target: Compute user defined dependencies
    {
      this->AutogenTarget.DependOrigin =
        this->Target->GetPropertyAsBool("AUTOGEN_ORIGIN_DEPENDS");

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

    // CMAKE_AUTOMOC_RELAXED_MODE deprecation warning
    if (this->Moc.Enabled) {
      if (cmSystemTools::IsOn(
            makefile->GetDefinition("CMAKE_AUTOMOC_RELAXED_MODE"))) {
        std::string msg = "AUTOMOC: CMAKE_AUTOMOC_RELAXED_MODE is "
                          "deprecated an will be removed in the future.  ";
        msg += "Consider disabling it and converting the target ";
        msg += this->Target->GetName();
        msg += " to regular mode.";
        makefile->IssueMessage(MessageType::AUTHOR_WARNING, msg);
      }
    }
  }

  // Init rcc specific settings
  if (this->Rcc.Enabled && !InitRcc()) {
    return false;
  }

  // Add autogen include directory to the origin target INCLUDE_DIRECTORIES
  if (this->MocOrUicEnabled() || (this->Rcc.Enabled && this->MultiConfig)) {
    this->Target->AddIncludeDirectory(this->Dir.Include, true);
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
  {
    if (!this->GetQtExecutable(this->Moc, "moc", false)) {
      return false;
    }
    // Let the _autogen target depend on the moc executable
    if (this->Moc.ExecutableTarget != nullptr) {
      this->AutogenTarget.DependTargets.insert(
        this->Moc.ExecutableTarget->Target);
    }
  }

  return true;
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
  {
    if (!this->GetQtExecutable(this->Uic, "uic", true)) {
      return false;
    }
    // Let the _autogen target depend on the uic executable
    if (this->Uic.ExecutableTarget != nullptr) {
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
    CompilerFeatures& features = *this->Rcc.ExecutableFeatures;
    if (!features.Evaluated) {
      // Look for list options
      if (this->QtVersion.Major == 5 || this->QtVersion.Major == 6) {
        if (features.HelpOutput.find("--list") != std::string::npos) {
          features.ListOptions.emplace_back("--list");
        } else if (features.HelpOutput.find("-list") != std::string::npos) {
          features.ListOptions.emplace_back("-list");
        }
      }
      // Evaluation finished
      features.Evaluated = true;
    }
  }

  return true;
}

bool cmQtAutoGenInitializer::InitScanFiles()
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  auto const& kw = this->GlobalInitializer->kw();

  auto makeMUFile = [this, &kw](cmSourceFile* sf, std::string const& fullPath,
                                bool muIt) -> MUFileHandle {
    MUFileHandle muf = cm::make_unique<MUFile>();
    muf->RealPath = cmSystemTools::GetRealPath(fullPath);
    muf->SF = sf;
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

  auto addMUFile = [&](MUFileHandle&& muf, bool isHeader) {
    if ((muf->MocIt || muf->UicIt) && muf->Generated) {
      this->AutogenTarget.FilesGenerated.emplace_back(muf.get());
    }
    if (isHeader) {
      this->AutogenTarget.Headers.emplace(muf->SF, std::move(muf));
    } else {
      this->AutogenTarget.Sources.emplace(muf->SF, std::move(muf));
    }
  };

  // Scan through target files
  {
    // Scan through target files
    std::vector<cmSourceFile*> srcFiles;
    this->Target->GetConfigCommonSourceFiles(srcFiles);
    for (cmSourceFile* sf : srcFiles) {
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      // Since we're iterating over source files that might be not in the
      // target we need to check for path errors (not existing files).
      std::string pathError;
      std::string const& fullPath = sf->GetFullPath(&pathError);
      if (!pathError.empty() || fullPath.empty()) {
        continue;
      }
      std::string const& ext = sf->GetExtension();

      // Register files that will be scanned by moc or uic
      if (this->MocOrUicEnabled()) {
        switch (cmSystemTools::GetFileFormat(ext)) {
          case cmSystemTools::HEADER_FILE_FORMAT:
            addMUFile(makeMUFile(sf, fullPath, true), true);
            break;
          case cmSystemTools::CXX_FILE_FORMAT:
            addMUFile(makeMUFile(sf, fullPath, true), false);
            break;
          default:
            break;
        }
      }

      // Register rcc enabled files
      if (this->Rcc.Enabled) {
        if ((ext == kw.qrc) && !sf->GetPropertyAsBool(kw.SKIP_AUTOGEN) &&
            !sf->GetPropertyAsBool(kw.SKIP_AUTORCC)) {
          // Register qrc file
          Qrc qrc;
          qrc.QrcFile = cmSystemTools::GetRealPath(fullPath);
          qrc.QrcName =
            cmSystemTools::GetFilenameWithoutLastExtension(qrc.QrcFile);
          qrc.Generated = sf->GetIsGenerated();
          // RCC options
          {
            std::string const opts = sf->GetSafeProperty(kw.AUTORCC_OPTIONS);
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

  // For source files find additional headers and private headers
  if (this->MocOrUicEnabled()) {
    std::vector<MUFileHandle> extraHeaders;
    extraHeaders.reserve(this->AutogenTarget.Sources.size() * 2);
    // Header search suffixes and extensions
    std::array<std::string, 2> const suffixes{ { "", "_p" } };
    auto const& exts = makefile->GetCMakeInstance()->GetHeaderExtensions();
    // Scan through sources
    for (auto const& pair : this->AutogenTarget.Sources) {
      MUFile const& muf = *pair.second;
      if (muf.MocIt || muf.UicIt) {
        // Search for the default header file and a private header
        std::string const& srcPath = muf.SF->GetFullPath();
        std::string basePath = cmQtAutoGen::SubDirPrefix(srcPath);
        basePath += cmSystemTools::GetFilenameWithoutLastExtension(srcPath);
        for (auto const& suffix : suffixes) {
          std::string const suffixedPath = basePath + suffix;
          for (auto const& ext : exts) {
            std::string fullPath = suffixedPath;
            fullPath += '.';
            fullPath += ext;

            auto constexpr locationKind = cmSourceFileLocationKind::Known;
            cmSourceFile* sf = makefile->GetSource(fullPath, locationKind);
            if (sf != nullptr) {
              // Check if we know about this header already
              if (this->AutogenTarget.Headers.find(sf) !=
                  this->AutogenTarget.Headers.end()) {
                continue;
              }
              // We only accept not-GENERATED files that do exist.
              if (!sf->GetIsGenerated() &&
                  !cmSystemTools::FileExists(fullPath)) {
                continue;
              }
            } else if (cmSystemTools::FileExists(fullPath)) {
              // Create a new source file for the existing file
              sf = makefile->CreateSource(fullPath, false, locationKind);
            }

            if (sf != nullptr) {
              auto eMuf = makeMUFile(sf, fullPath, true);
              // Ony process moc/uic when the parent is processed as well
              if (!muf.MocIt) {
                eMuf->MocIt = false;
              }
              if (!muf.UicIt) {
                eMuf->UicIt = false;
              }
              extraHeaders.emplace_back(std::move(eMuf));
            }
          }
        }
      }
    }
    // Move generated files to main headers list
    for (auto& eMuf : extraHeaders) {
      addMUFile(std::move(eMuf), true);
    }
  }

  // Scan through all source files in the makefile to extract moc and uic
  // parameters.  Historically we support non target source file parameters.
  // The reason is that their file names might be discovered from source files
  // at generation time.
  if (this->MocOrUicEnabled()) {
    for (cmSourceFile* sf : makefile->GetSourceFiles()) {
      // sf->GetExtension() is only valid after sf->GetFullPath() ...
      // Since we're iterating over source files that might be not in the
      // target we need to check for path errors (not existing files).
      std::string pathError;
      std::string const& fullPath = sf->GetFullPath(&pathError);
      if (!pathError.empty() || fullPath.empty()) {
        continue;
      }
      std::string const& ext = sf->GetExtension();

      auto const fileFormat = cmSystemTools::GetFileFormat(ext);
      if (fileFormat == cmSystemTools::HEADER_FILE_FORMAT) {
        if (this->AutogenTarget.Headers.find(sf) ==
            this->AutogenTarget.Headers.end()) {
          auto muf = makeMUFile(sf, fullPath, false);
          if (muf->SkipMoc || muf->SkipUic) {
            this->AutogenTarget.Headers.emplace(sf, std::move(muf));
          }
        }
      } else if (fileFormat == cmSystemTools::CXX_FILE_FORMAT) {
        if (this->AutogenTarget.Sources.find(sf) ==
            this->AutogenTarget.Sources.end()) {
          auto muf = makeMUFile(sf, fullPath, false);
          if (muf->SkipMoc || muf->SkipUic) {
            this->AutogenTarget.Sources.emplace(sf, std::move(muf));
          }
        }
      } else if (this->Uic.Enabled && (ext == kw.ui)) {
        // .ui file
        std::string realPath = cmSystemTools::GetRealPath(fullPath);
        bool const skipAutogen = sf->GetPropertyAsBool(kw.SKIP_AUTOGEN);
        bool const skipUic =
          (skipAutogen || sf->GetPropertyAsBool(kw.SKIP_AUTOUIC));
        if (!skipUic) {
          // Check if the .ui file has uic options
          std::string const uicOpts = sf->GetSafeProperty(kw.AUTOUIC_OPTIONS);
          if (!uicOpts.empty()) {
            this->Uic.FileFiles.push_back(std::move(realPath));
            std::vector<std::string> optsVec;
            cmSystemTools::ExpandListArgument(uicOpts, optsVec);
            this->Uic.FileOptions.push_back(std::move(optsVec));
          }
        } else {
          // Register skipped .ui file
          this->Uic.SkipUi.insert(std::move(realPath));
        }
      }
    }
  }

  // Process GENERATED sources and headers
  if (this->MocOrUicEnabled() && !this->AutogenTarget.FilesGenerated.empty()) {
    if (this->CMP0071Accept) {
      // Let the autogen target depend on the GENERATED files
      for (MUFile* muf : this->AutogenTarget.FilesGenerated) {
        this->AutogenTarget.DependFiles.insert(muf->RealPath);
      }
    } else if (this->CMP0071Warn) {
      std::string msg;
      msg += cmPolicies::GetPolicyWarning(cmPolicies::CMP0071);
      msg += '\n';
      std::string property;
      if (this->Moc.Enabled && this->Uic.Enabled) {
        property = kw.SKIP_AUTOGEN;
      } else if (this->Moc.Enabled) {
        property = kw.SKIP_AUTOMOC;
      } else if (this->Uic.Enabled) {
        property = kw.SKIP_AUTOUIC;
      }
      msg += "For compatibility, CMake is excluding the GENERATED source "
             "file(s):\n";
      for (MUFile* muf : this->AutogenTarget.FilesGenerated) {
        msg += "  ";
        msg += Quoted(muf->RealPath);
        msg += '\n';
      }
      msg += "from processing by ";
      msg += cmQtAutoGen::Tools(this->Moc.Enabled, this->Uic.Enabled, false);
      msg += ". If any of the files should be processed, set CMP0071 to NEW. "
             "If any of the files should not be processed, "
             "explicitly exclude them by setting the source file property ";
      msg += property;
      msg += ":\n  set_property(SOURCE file.h PROPERTY ";
      msg += property;
      msg += " ON)\n";
      makefile->IssueMessage(MessageType::AUTHOR_WARNING, msg);
    }
  }

  // Process qrc files
  if (!this->Rcc.Qrcs.empty()) {
    const bool modernQt = (this->QtVersion.Major >= 5);
    // Target rcc options
    std::vector<std::string> optionsTarget;
    cmSystemTools::ExpandListArgument(
      this->Target->GetSafeProperty(kw.AUTORCC_OPTIONS), optionsTarget);

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
        RccLister const lister(this->Rcc.Executable,
                               this->Rcc.ExecutableFeatures->ListOptions);
        if (!lister.list(qrc.QrcFile, qrc.Resources, error)) {
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
    this->AddGeneratedSource(this->Moc.MocsCompilation, this->Moc, true);
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

  for (Qrc const& qrc : this->Rcc.Qrcs) {
    // Register info file as generated by CMake
    makefile->AddCMakeOutputFile(qrc.InfoFile);
    // Register file at target
    this->AddGeneratedSource(qrc.RccFile, this->Rcc);

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
        if (!this->Rcc.ExecutableTargetName.empty()) {
          autoRccTarget->AddUtility(this->Rcc.ExecutableTargetName, makefile);
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
        if (!this->Rcc.ExecutableTargetName.empty()) {
          ccDepends.push_back(this->Rcc.ExecutableTargetName);
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

    std::vector<std::string> headers;
    std::vector<std::string> headersFlags;
    std::vector<std::string> headersBuildPaths;
    std::vector<std::string> sources;
    std::vector<std::string> sourcesFlags;
    std::set<std::string> moc_skip;
    std::set<std::string> uic_skip;

    // Filter headers
    {
      auto headerCount = this->AutogenTarget.Headers.size();
      headers.reserve(headerCount);
      headersFlags.reserve(headerCount);

      std::vector<MUFile const*> sortedHeaders;
      {
        sortedHeaders.reserve(headerCount);
        for (auto const& pair : this->AutogenTarget.Headers) {
          sortedHeaders.emplace_back(pair.second.get());
        }
        std::sort(sortedHeaders.begin(), sortedHeaders.end(),
                  [](MUFile const* a, MUFile const* b) {
                    return (a->RealPath < b->RealPath);
                  });
      }

      for (MUFile const* const muf : sortedHeaders) {
        if (muf->Generated && !this->CMP0071Accept) {
          continue;
        }
        if (muf->SkipMoc) {
          moc_skip.insert(muf->RealPath);
        }
        if (muf->SkipUic) {
          uic_skip.insert(muf->RealPath);
        }
        if (muf->MocIt || muf->UicIt) {
          headers.emplace_back(muf->RealPath);
          std::string flags;
          flags += muf->MocIt ? 'M' : 'm';
          flags += muf->UicIt ? 'U' : 'u';
          headersFlags.emplace_back(std::move(flags));
        }
      }
    }
    // Header build paths
    {
      cmFilePathChecksum const fpathCheckSum(makefile);
      std::unordered_set<std::string> emitted;
      for (std::string const& hdr : headers) {
        std::string basePath = fpathCheckSum.getPart(hdr);
        basePath += "/moc_";
        basePath += cmSystemTools::GetFilenameWithoutLastExtension(hdr);
        for (unsigned int ii = 1; ii != 1024; ++ii) {
          std::string path = basePath;
          if (ii > 1) {
            path += '_';
            path += std::to_string(ii);
          }
          path += ".cpp";
          if (emitted.emplace(path).second) {
            headersBuildPaths.emplace_back(std::move(path));
            break;
          }
        }
      }
    }

    // Filter sources
    {
      auto sourcesCount = this->AutogenTarget.Sources.size();
      sources.reserve(sourcesCount);
      sourcesFlags.reserve(sourcesCount);

      std::vector<MUFile const*> sorted;
      sorted.reserve(sourcesCount);
      for (auto const& pair : this->AutogenTarget.Sources) {
        sorted.emplace_back(pair.second.get());
      }
      std::sort(sorted.begin(), sorted.end(),
                [](MUFile const* a, MUFile const* b) {
                  return (a->RealPath < b->RealPath);
                });

      for (MUFile const* const muf : sorted) {
        if (muf->Generated && !this->CMP0071Accept) {
          continue;
        }
        if (muf->SkipMoc) {
          moc_skip.insert(muf->RealPath);
        }
        if (muf->SkipUic) {
          uic_skip.insert(muf->RealPath);
        }
        if (muf->MocIt || muf->UicIt) {
          sources.emplace_back(muf->RealPath);
          std::string flags;
          flags += muf->MocIt ? 'M' : 'm';
          flags += muf->UicIt ? 'U' : 'u';
          sourcesFlags.emplace_back(std::move(flags));
        }
      }
    }

    ofs.Write("# Qt\n");
    ofs.WriteUInt("AM_QT_VERSION_MAJOR", this->QtVersion.Major);
    ofs.Write("AM_QT_MOC_EXECUTABLE", this->Moc.Executable);
    ofs.Write("AM_QT_UIC_EXECUTABLE", this->Uic.Executable);

    ofs.Write("# Files\n");
    ofs.Write("AM_CMAKE_EXECUTABLE", cmSystemTools::GetCMakeCommand());
    ofs.Write("AM_SETTINGS_FILE", this->AutogenTarget.SettingsFile);
    ofs.WriteConfig("AM_SETTINGS_FILE",
                    this->AutogenTarget.ConfigSettingsFile);
    ofs.Write("AM_PARSE_CACHE_FILE", this->AutogenTarget.ParseCacheFile);
    ofs.WriteStrings("AM_HEADERS", headers);
    ofs.WriteStrings("AM_HEADERS_FLAGS", headersFlags);
    ofs.WriteStrings("AM_HEADERS_BUILD_PATHS", headersBuildPaths);
    ofs.WriteStrings("AM_SOURCES", sources);
    ofs.WriteStrings("AM_SOURCES_FLAGS", sourcesFlags);

    // Write moc settings
    if (this->Moc.Enabled) {
      ofs.Write("# MOC settings\n");
      ofs.WriteStrings("AM_MOC_SKIP", moc_skip);
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
      // Add skipped .ui files
      uic_skip.insert(this->Uic.SkipUi.begin(), this->Uic.SkipUi.end());

      ofs.Write("# UIC settings\n");
      ofs.WriteStrings("AM_UIC_SKIP", uic_skip);
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
      ofs.WriteStrings("ARCC_RCC_LIST_OPTIONS",
                       this->Rcc.ExecutableFeatures->ListOptions);

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

void cmQtAutoGenInitializer::RegisterGeneratedSource(
  std::string const& filename)
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmSourceFile* gFile = makefile->GetOrCreateSource(filename, true);
  gFile->SetProperty("GENERATED", "1");
  gFile->SetProperty("SKIP_AUTOGEN", "1");
}

bool cmQtAutoGenInitializer::AddGeneratedSource(std::string const& filename,
                                                GenVarsT const& genVars,
                                                bool prepend)
{
  // Register source at makefile
  this->RegisterGeneratedSource(filename);
  // Add source file to target
  this->Target->AddSource(filename, prepend);
  // Add source file to source group
  return this->AddToSourceGroup(filename, genVars.GenNameUpper);
}

bool cmQtAutoGenInitializer::AddToSourceGroup(std::string const& fileName,
                                              std::string const& genNameUpper)
{
  cmMakefile* makefile = this->Target->Target->GetMakefile();
  cmSourceGroup* sourceGroup = nullptr;
  // Acquire source group
  {
    std::string property;
    std::string groupName;
    {
      // Prefer generator specific source group name
      std::array<std::string, 2> props{ { genNameUpper + "_SOURCE_GROUP",
                                          "AUTOGEN_SOURCE_GROUP" } };
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
        std::string err;
        err += genNameUpper;
        err += " error in ";
        err += property;
        err += ": Could not find or create the source group ";
        err += cmQtAutoGen::Quoted(groupName);
        cmSystemTools::Error(err);
        return false;
      }
    }
  }
  if (sourceGroup != nullptr) {
    sourceGroup->AddGroupFile(fileName);
  }
  return true;
}

void cmQtAutoGenInitializer::AddCleanFile(std::string const& fileName)
{
  Target->Target->AppendProperty("ADDITIONAL_CLEAN_FILES", fileName.c_str(),
                                 false);
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
  // Qt version variable prefixes
  static std::array<std::string, 3> const prefixes{ { "Qt6Core", "Qt5Core",
                                                      "QT" } };

  std::vector<cmQtAutoGen::IntegerVersion> result;
  result.reserve(prefixes.size() * 2);
  // Adds a version to the result (nullptr safe)
  auto addVersion = [&result](const char* major, const char* minor) {
    cmQtAutoGen::IntegerVersion ver(CharPtrToUInt(major),
                                    CharPtrToUInt(minor));
    if (ver.Major != 0) {
      result.emplace_back(ver);
    }
  };
  cmMakefile* makefile = target->Target->GetMakefile();

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

bool cmQtAutoGenInitializer::GetQtExecutable(GenVarsT& genVars,
                                             const std::string& executable,
                                             bool ignoreMissingTarget) const
{
  auto print_err = [this, &genVars](std::string const& err) {
    std::string msg = genVars.GenNameUpper;
    msg += " for target ";
    msg += this->Target->GetName();
    msg += ": ";
    msg += err;
    cmSystemTools::Error(msg);
  };

  // Custom executable
  {
    std::string const prop = genVars.GenNameUpper + "_EXECUTABLE";
    std::string const val = this->Target->Target->GetSafeProperty(prop);
    if (!val.empty()) {
      // Evaluate generator expression
      {
        cmListFileBacktrace lfbt =
          this->Target->Target->GetMakefile()->GetBacktrace();
        cmGeneratorExpression ge(lfbt);
        std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(val);
        genVars.Executable =
          cge->Evaluate(this->Target->GetLocalGenerator(), "");
      }
      if (genVars.Executable.empty() && !ignoreMissingTarget) {
        print_err(prop + " evaluates to an empty value");
        return false;
      }

      // Create empty compiler features.
      genVars.ExecutableFeatures =
        std::make_shared<cmQtAutoGen::CompilerFeatures>();
      return true;
    }
  }

  // Find executable target
  {
    // Find executable target name
    std::string targetName;
    if (this->QtVersion.Major == 4) {
      targetName = "Qt4::";
    } else if (this->QtVersion.Major == 5) {
      targetName = "Qt5::";
    } else if (this->QtVersion.Major == 6) {
      targetName = "Qt6::";
    }
    targetName += executable;

    // Find target
    cmLocalGenerator* localGen = this->Target->GetLocalGenerator();
    cmGeneratorTarget* target = localGen->FindGeneratorTargetToUse(targetName);
    if (target != nullptr) {
      genVars.ExecutableTargetName = targetName;
      genVars.ExecutableTarget = target;
      if (target->IsImported()) {
        genVars.Executable = target->ImportedGetLocation("");
      } else {
        genVars.Executable = target->GetLocation("");
      }
    } else {
      if (ignoreMissingTarget) {
        // Create empty compiler features.
        genVars.ExecutableFeatures =
          std::make_shared<cmQtAutoGen::CompilerFeatures>();
        return true;
      }
      std::string err = "Could not find ";
      err += executable;
      err += " executable target ";
      err += targetName;
      print_err(err);
      return false;
    }
  }

  // Get executable features
  {
    std::string err;
    genVars.ExecutableFeatures = this->GlobalInitializer->GetCompilerFeatures(
      executable, genVars.Executable, err);
    if (!genVars.ExecutableFeatures) {
      print_err(err);
      return false;
    }
  }

  return true;
}
