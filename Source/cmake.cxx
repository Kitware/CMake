/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmake.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(CMAKE_BOOT_MINGW)
#  include <cm/iterator>
#endif

#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cm_sys_stat.h"

#include "cmCMakePresetsFile.h"
#include "cmCommandLineArgument.h"
#include "cmCommands.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmDocumentationFormatter.h"
#include "cmDuration.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmFileTimeCache.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmLinkLineComputer.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#if !defined(CMAKE_BOOTSTRAP)
#  include "cmMakefileProfilingData.h"
#endif
#include "cmMessenger.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmUtils.hxx"
#include "cmVersionConfig.h"
#include "cmWorkingDirectory.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <unordered_map>

#  include <cm3p/json/writer.h>

#  include "cmFileAPI.h"
#  include "cmGraphVizWriter.h"
#  include "cmVariableWatch.h"
#endif

#if defined(__MINGW32__) && defined(CMAKE_BOOTSTRAP)
#  define CMAKE_BOOT_MINGW
#endif

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(CMAKE_BOOT_MINGW)
#    include <cmext/memory>

#    include "cmGlobalBorlandMakefileGenerator.h"
#    include "cmGlobalJOMMakefileGenerator.h"
#    include "cmGlobalNMakeMakefileGenerator.h"
#    include "cmGlobalVisualStudio10Generator.h"
#    include "cmGlobalVisualStudio11Generator.h"
#    include "cmGlobalVisualStudio12Generator.h"
#    include "cmGlobalVisualStudio14Generator.h"
#    include "cmGlobalVisualStudio9Generator.h"
#    include "cmGlobalVisualStudioVersionedGenerator.h"
#    include "cmVSSetupHelper.h"

#    define CMAKE_HAVE_VS_GENERATORS
#  endif
#  include "cmGlobalMSYSMakefileGenerator.h"
#  include "cmGlobalMinGWMakefileGenerator.h"
#else
#endif
#if defined(CMAKE_USE_WMAKE)
#  include "cmGlobalWatcomWMakeGenerator.h"
#endif
#if !defined(CMAKE_BOOTSTRAP)
#  include "cmGlobalNinjaGenerator.h"
#  include "cmGlobalUnixMakefileGenerator3.h"
#elif defined(CMAKE_BOOTSTRAP_MAKEFILES)
#  include "cmGlobalUnixMakefileGenerator3.h"
#elif defined(CMAKE_BOOTSTRAP_NINJA)
#  include "cmGlobalNinjaGenerator.h"
#endif

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmExtraCodeBlocksGenerator.h"
#  include "cmExtraCodeLiteGenerator.h"
#  include "cmExtraEclipseCDT4Generator.h"
#  include "cmExtraKateGenerator.h"
#  include "cmExtraSublimeTextGenerator.h"
#endif

#if defined(__linux__) || defined(_WIN32)
#  include "cmGlobalGhsMultiGenerator.h"
#endif

#if defined(__APPLE__)
#  if !defined(CMAKE_BOOTSTRAP)
#    include "cmGlobalXCodeGenerator.h"

#    define CMAKE_USE_XCODE 1
#  endif
#  include <sys/resource.h>
#  include <sys/time.h>
#endif

namespace {

#if !defined(CMAKE_BOOTSTRAP)
using JsonValueMapType = std::unordered_map<std::string, Json::Value>;
#endif

auto IgnoreAndTrueLambda = [](std::string const&, cmake*) -> bool {
  return true;
};

using CommandArgument =
  cmCommandLineArgument<bool(std::string const& value, cmake* state)>;

} // namespace

static bool cmakeCheckStampFile(const std::string& stampName);
static bool cmakeCheckStampList(const std::string& stampList);

#ifndef CMAKE_BOOTSTRAP
static void cmWarnUnusedCliWarning(const std::string& variable, int /*unused*/,
                                   void* ctx, const char* /*unused*/,
                                   const cmMakefile* /*unused*/)
{
  cmake* cm = reinterpret_cast<cmake*>(ctx);
  cm->MarkCliAsUsed(variable);
}
#endif

cmake::cmake(Role role, cmState::Mode mode)
  : FileTimeCache(cm::make_unique<cmFileTimeCache>())
#ifndef CMAKE_BOOTSTRAP
  , VariableWatch(cm::make_unique<cmVariableWatch>())
#endif
  , State(cm::make_unique<cmState>())
  , Messenger(cm::make_unique<cmMessenger>())
{
  this->TraceFile.close();
  this->State->SetMode(mode);
  this->CurrentSnapshot = this->State->CreateBaseSnapshot();

#ifdef __APPLE__
  struct rlimit rlp;
  if (!getrlimit(RLIMIT_STACK, &rlp)) {
    if (rlp.rlim_cur != rlp.rlim_max) {
      rlp.rlim_cur = rlp.rlim_max;
      setrlimit(RLIMIT_STACK, &rlp);
    }
  }
#endif

  this->AddDefaultGenerators();
  this->AddDefaultExtraGenerators();
  if (role == RoleScript || role == RoleProject) {
    this->AddScriptingCommands();
  }
  if (role == RoleProject) {
    this->AddProjectCommands();
  }

  if (mode == cmState::Project) {
    this->LoadEnvironmentPresets();
  }

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();

  // Set up a list of source and header extensions.
  // These are used to find files when the extension is not given.
  {
    auto setupExts = [](FileExtensions& exts,
                        std::initializer_list<cm::string_view> extList) {
      // Fill ordered vector
      exts.ordered.reserve(extList.size());
      for (cm::string_view ext : extList) {
        exts.ordered.emplace_back(ext);
      };
      // Fill unordered set
      exts.unordered.insert(exts.ordered.begin(), exts.ordered.end());
    };

    // The "c" extension MUST precede the "C" extension.
    setupExts(
      this->CLikeSourceFileExtensions,
      { "c", "C", "c++", "cc", "cpp", "cxx", "cu", "mpp", "m", "M", "mm" });
    setupExts(this->HeaderFileExtensions,
              { "h", "hh", "h++", "hm", "hpp", "hxx", "in", "txx" });
    setupExts(this->CudaFileExtensions, { "cu" });
    setupExts(this->FortranFileExtensions,
              { "f", "F", "for", "f77", "f90", "f95", "f03" });
    setupExts(this->ISPCFileExtensions, { "ispc" });
  }
}

cmake::~cmake() = default;

#if !defined(CMAKE_BOOTSTRAP)
Json::Value cmake::ReportVersionJson() const
{
  Json::Value version = Json::objectValue;
  version["string"] = CMake_VERSION;
  version["major"] = CMake_VERSION_MAJOR;
  version["minor"] = CMake_VERSION_MINOR;
  version["suffix"] = CMake_VERSION_SUFFIX;
  version["isDirty"] = (CMake_VERSION_IS_DIRTY == 1);
  version["patch"] = CMake_VERSION_PATCH;
  return version;
}

Json::Value cmake::ReportCapabilitiesJson() const
{
  Json::Value obj = Json::objectValue;

  // Version information:
  obj["version"] = this->ReportVersionJson();

  // Generators:
  std::vector<cmake::GeneratorInfo> generatorInfoList;
  this->GetRegisteredGenerators(generatorInfoList);

  JsonValueMapType generatorMap;
  for (cmake::GeneratorInfo const& gi : generatorInfoList) {
    if (gi.isAlias) { // skip aliases, they are there for compatibility reasons
                      // only
      continue;
    }

    if (gi.extraName.empty()) {
      Json::Value gen = Json::objectValue;
      gen["name"] = gi.name;
      gen["toolsetSupport"] = gi.supportsToolset;
      gen["platformSupport"] = gi.supportsPlatform;
      gen["extraGenerators"] = Json::arrayValue;
      generatorMap[gi.name] = gen;
    } else {
      Json::Value& gen = generatorMap[gi.baseName];
      gen["extraGenerators"].append(gi.extraName);
    }
  }

  Json::Value generators = Json::arrayValue;
  for (auto const& i : generatorMap) {
    generators.append(i.second);
  }
  obj["generators"] = generators;
  obj["fileApi"] = cmFileAPI::ReportCapabilities();
  obj["serverMode"] = false;

  return obj;
}
#endif

std::string cmake::ReportCapabilities() const
{
  std::string result;
#if !defined(CMAKE_BOOTSTRAP)
  Json::FastWriter writer;
  result = writer.write(this->ReportCapabilitiesJson());
#else
  result = "Not supported";
#endif
  return result;
}

void cmake::CleanupCommandsAndMacros()
{
  this->CurrentSnapshot = this->State->Reset();
  this->State->RemoveUserDefinedCommands();
  this->CurrentSnapshot.SetDefaultDefinitions();
  // FIXME: InstalledFiles probably belongs in the global generator.
  this->InstalledFiles.clear();
}

#ifndef CMAKE_BOOTSTRAP
void cmake::SetWarningFromPreset(const std::string& name,
                                 const cm::optional<bool>& warning,
                                 const cm::optional<bool>& error)
{
  if (warning) {
    if (*warning) {
      this->DiagLevels[name] = std::max(this->DiagLevels[name], DIAG_WARN);
    } else {
      this->DiagLevels[name] = DIAG_IGNORE;
    }
  }
  if (error) {
    if (*error) {
      this->DiagLevels[name] = DIAG_ERROR;
    } else {
      this->DiagLevels[name] = std::min(this->DiagLevels[name], DIAG_WARN);
    }
  }
}

void cmake::ProcessPresetVariables()
{
  for (auto const& var : this->UnprocessedPresetVariables) {
    if (!var.second) {
      continue;
    }
    cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
    if (!var.second->Type.empty()) {
      type = cmState::StringToCacheEntryType(var.second->Type);
    }
    this->ProcessCacheArg(var.first, var.second->Value, type);
  }
}

void cmake::PrintPresetVariables()
{
  bool first = true;
  for (auto const& var : this->UnprocessedPresetVariables) {
    if (!var.second) {
      continue;
    }
    cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
    if (!var.second->Type.empty()) {
      type = cmState::StringToCacheEntryType(var.second->Type);
    }
    if (first) {
      std::cout << "Preset CMake variables:\n\n";
      first = false;
    }
    std::cout << "  " << var.first;
    if (type != cmStateEnums::UNINITIALIZED) {
      std::cout << ':' << cmState::CacheEntryTypeToString(type);
    }
    std::cout << "=\"" << var.second->Value << "\"\n";
  }
  if (!first) {
    std::cout << '\n';
  }
  this->UnprocessedPresetVariables.clear();
}

void cmake::ProcessPresetEnvironment()
{
  for (auto const& var : this->UnprocessedPresetEnvironment) {
    if (var.second) {
      cmSystemTools::PutEnv(cmStrCat(var.first, '=', *var.second));
    }
  }
}

void cmake::PrintPresetEnvironment()
{
  bool first = true;
  for (auto const& var : this->UnprocessedPresetEnvironment) {
    if (!var.second) {
      continue;
    }
    if (first) {
      std::cout << "Preset environment variables:\n\n";
      first = false;
    }
    std::cout << "  " << var.first << "=\"" << *var.second << "\"\n";
  }
  if (!first) {
    std::cout << '\n';
  }
  this->UnprocessedPresetEnvironment.clear();
}
#endif

// Parse the args
bool cmake::SetCacheArgs(const std::vector<std::string>& args)
{
  auto findPackageMode = false;
  auto seenScriptOption = false;

  auto DefineLambda = [](std::string const& entry, cmake* state) -> bool {
    std::string var;
    std::string value;
    cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;
    if (cmState::ParseCacheEntry(entry, var, value, type)) {
#ifndef CMAKE_BOOTSTRAP
      state->UnprocessedPresetVariables.erase(var);
#endif
      state->ProcessCacheArg(var, value, type);
    } else {
      cmSystemTools::Error(cmStrCat("Parse error in command line argument: ",
                                    entry, "\n Should be: VAR:type=value\n"));
      return false;
    }
    return true;
  };

  auto WarningLambda = [](cm::string_view entry, cmake* state) -> bool {
    bool foundNo = false;
    bool foundError = false;

    if (cmHasLiteralPrefix(entry, "no-")) {
      foundNo = true;
      entry.remove_prefix(3);
    }

    if (cmHasLiteralPrefix(entry, "error=")) {
      foundError = true;
      entry.remove_prefix(6);
    }

    if (entry.empty()) {
      cmSystemTools::Error("No warning name provided.");
      return false;
    }

    std::string const name = std::string(entry);
    if (!foundNo && !foundError) {
      // -W<name>
      state->DiagLevels[name] = std::max(state->DiagLevels[name], DIAG_WARN);
    } else if (foundNo && !foundError) {
      // -Wno<name>
      state->DiagLevels[name] = DIAG_IGNORE;
    } else if (!foundNo && foundError) {
      // -Werror=<name>
      state->DiagLevels[name] = DIAG_ERROR;
    } else {
      // -Wno-error=<name>
      // This can downgrade an error to a warning, but should not enable
      // or disable a warning in the first place.
      auto dli = state->DiagLevels.find(name);
      if (dli != state->DiagLevels.end()) {
        dli->second = std::min(dli->second, DIAG_WARN);
      }
    }
    return true;
  };

  auto UnSetLambda = [](std::string const& entryPattern,
                        cmake* state) -> bool {
    cmsys::RegularExpression regex(
      cmsys::Glob::PatternToRegex(entryPattern, true, true));
    // go through all cache entries and collect the vars which will be
    // removed
    std::vector<std::string> entriesToDelete;
    std::vector<std::string> cacheKeys = state->State->GetCacheEntryKeys();
    for (std::string const& ck : cacheKeys) {
      cmStateEnums::CacheEntryType t = state->State->GetCacheEntryType(ck);
      if (t != cmStateEnums::STATIC) {
        if (regex.find(ck)) {
          entriesToDelete.push_back(ck);
        }
      }
    }

    // now remove them from the cache
    for (std::string const& currentEntry : entriesToDelete) {
#ifndef CMAKE_BOOTSTRAP
      state->UnprocessedPresetVariables.erase(currentEntry);
#endif
      state->State->RemoveCacheEntry(currentEntry);
    }
    return true;
  };

  auto ScriptLambda = [&](std::string const& path, cmake* state) -> bool {
    // Register fake project commands that hint misuse in script mode.
    GetProjectCommandsInScriptMode(state->GetState());
    // Documented behaviour of CMAKE{,_CURRENT}_{SOURCE,BINARY}_DIR is to be
    // set to $PWD for -P mode.
    state->SetHomeDirectory(cmSystemTools::GetCurrentWorkingDirectory());
    state->SetHomeOutputDirectory(cmSystemTools::GetCurrentWorkingDirectory());
    state->ReadListFile(args, path);
    seenScriptOption = true;
    return true;
  };

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "-D", "-D must be followed with VAR=VALUE.",
                     CommandArgument::Values::One, DefineLambda },
    CommandArgument{ "-W", "-W must be followed with [no-]<name>.",
                     CommandArgument::Values::One, WarningLambda },
    CommandArgument{ "-U", "-U must be followed with VAR.",
                     CommandArgument::Values::One, UnSetLambda },
    CommandArgument{ "-C", "-C must be followed by a file name.",
                     CommandArgument::Values::One,
                     [&](std::string const& value, cmake* state) -> bool {
                       cmSystemTools::Stdout("loading initial cache file " +
                                             value + "\n");
                       // Resolve script path specified on command line
                       // relative to $PWD.
                       auto path = cmSystemTools::CollapseFullPath(value);
                       state->ReadListFile(args, path);
                       return true;
                     } },
    CommandArgument{ "-P", "-P must be followed by a file name.",
                     CommandArgument::Values::One, ScriptLambda },
    CommandArgument{ "--find-package", CommandArgument::Values::Zero,
                     [&](std::string const&, cmake*) -> bool {
                       findPackageMode = true;
                       return true;
                     } },
  };
  for (decltype(args.size()) i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];

    if (arg == "--" && seenScriptOption) {
      // Stop processing CMake args and avoid possible errors
      // when arbitrary args are given to CMake script.
      break;
    }
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        const bool parsedCorrectly = m.parse(arg, i, args, this);
        if (!parsedCorrectly) {
          return false;
        }
      }
    }
  }

  if (findPackageMode) {
    return this->FindPackage(args);
  }

  return true;
}

void cmake::ProcessCacheArg(const std::string& var, const std::string& value,
                            cmStateEnums::CacheEntryType type)
{
  // The value is transformed if it is a filepath for example, so
  // we can't compare whether the value is already in the cache until
  // after we call AddCacheEntry.
  bool haveValue = false;
  std::string cachedValue;
  if (this->WarnUnusedCli) {
    if (cmProp v = this->State->GetInitializedCacheValue(var)) {
      haveValue = true;
      cachedValue = *v;
    }
  }

  this->AddCacheEntry(var, value.c_str(),
                      "No help, variable specified on the command line.",
                      type);

  if (this->WarnUnusedCli) {
    if (!haveValue ||
        cachedValue != *this->State->GetInitializedCacheValue(var)) {
      this->WatchUnusedCli(var);
    }
  }
}

void cmake::ReadListFile(const std::vector<std::string>& args,
                         const std::string& path)
{
  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator* gg = this->GetGlobalGenerator();

  // if a generator was not specified use a generic one
  std::unique_ptr<cmGlobalGenerator> gen;
  if (!gg) {
    gen = cm::make_unique<cmGlobalGenerator>(this);
    gg = gen.get();
  }

  // read in the list file to fill the cache
  if (!path.empty()) {
    this->CurrentSnapshot = this->State->Reset();
    cmStateSnapshot snapshot = this->GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(this->GetHomeOutputDirectory());
    snapshot.GetDirectory().SetCurrentSource(this->GetHomeDirectory());
    snapshot.SetDefaultDefinitions();
    cmMakefile mf(gg, snapshot);
    if (this->GetWorkingMode() != NORMAL_MODE) {
      std::string file(cmSystemTools::CollapseFullPath(path));
      cmSystemTools::ConvertToUnixSlashes(file);
      mf.SetScriptModeFile(file);

      mf.SetArgcArgv(args);
    }
    if (!mf.ReadListFile(path)) {
      cmSystemTools::Error("Error processing file: " + path);
    }
  }
}

bool cmake::FindPackage(const std::vector<std::string>& args)
{
  this->SetHomeDirectory(cmSystemTools::GetCurrentWorkingDirectory());
  this->SetHomeOutputDirectory(cmSystemTools::GetCurrentWorkingDirectory());

  this->SetGlobalGenerator(cm::make_unique<cmGlobalGenerator>(this));

  cmStateSnapshot snapshot = this->GetCurrentSnapshot();
  snapshot.GetDirectory().SetCurrentBinary(
    cmSystemTools::GetCurrentWorkingDirectory());
  snapshot.GetDirectory().SetCurrentSource(
    cmSystemTools::GetCurrentWorkingDirectory());
  // read in the list file to fill the cache
  snapshot.SetDefaultDefinitions();
  auto mfu = cm::make_unique<cmMakefile>(this->GetGlobalGenerator(), snapshot);
  cmMakefile* mf = mfu.get();
  this->GlobalGenerator->AddMakefile(std::move(mfu));

  mf->SetArgcArgv(args);

  std::string systemFile = mf->GetModulesFile("CMakeFindPackageMode.cmake");
  mf->ReadListFile(systemFile);

  std::string language = mf->GetSafeDefinition("LANGUAGE");
  std::string mode = mf->GetSafeDefinition("MODE");
  std::string packageName = mf->GetSafeDefinition("NAME");
  bool packageFound = mf->IsOn("PACKAGE_FOUND");
  bool quiet = mf->IsOn("PACKAGE_QUIET");

  if (!packageFound) {
    if (!quiet) {
      printf("%s not found.\n", packageName.c_str());
    }
  } else if (mode == "EXIST"_s) {
    if (!quiet) {
      printf("%s found.\n", packageName.c_str());
    }
  } else if (mode == "COMPILE"_s) {
    std::string includes = mf->GetSafeDefinition("PACKAGE_INCLUDE_DIRS");
    std::vector<std::string> includeDirs = cmExpandedList(includes);

    this->GlobalGenerator->CreateGenerationObjects();
    const auto& lg = this->GlobalGenerator->LocalGenerators[0];
    std::string includeFlags =
      lg->GetIncludeFlags(includeDirs, nullptr, language);

    std::string definitions = mf->GetSafeDefinition("PACKAGE_DEFINITIONS");
    printf("%s %s\n", includeFlags.c_str(), definitions.c_str());
  } else if (mode == "LINK"_s) {
    const char* targetName = "dummy";
    std::vector<std::string> srcs;
    cmTarget* tgt = mf->AddExecutable(targetName, srcs, true);
    tgt->SetProperty("LINKER_LANGUAGE", language);

    std::string libs = mf->GetSafeDefinition("PACKAGE_LIBRARIES");
    std::vector<std::string> libList = cmExpandedList(libs);
    for (std::string const& lib : libList) {
      tgt->AddLinkLibrary(*mf, lib, GENERAL_LibraryType);
    }

    std::string buildType = mf->GetSafeDefinition("CMAKE_BUILD_TYPE");
    buildType = cmSystemTools::UpperCase(buildType);

    std::string linkLibs;
    std::string frameworkPath;
    std::string linkPath;
    std::string flags;
    std::string linkFlags;
    this->GlobalGenerator->CreateGenerationObjects();
    cmGeneratorTarget* gtgt =
      this->GlobalGenerator->FindGeneratorTarget(tgt->GetName());
    cmLocalGenerator* lg = gtgt->GetLocalGenerator();
    cmLinkLineComputer linkLineComputer(lg,
                                        lg->GetStateSnapshot().GetDirectory());
    lg->GetTargetFlags(&linkLineComputer, buildType, linkLibs, flags,
                       linkFlags, frameworkPath, linkPath, gtgt);
    linkLibs = frameworkPath + linkPath + linkLibs;

    printf("%s\n", linkLibs.c_str());

    /*    if ( use_win32 )
          {
          tgt->SetProperty("WIN32_EXECUTABLE", "ON");
          }
        if ( use_macbundle)
          {
          tgt->SetProperty("MACOSX_BUNDLE", "ON");
          }*/
  }

  return packageFound;
}

void cmake::LoadEnvironmentPresets()
{
  std::string envGenVar;
  bool hasEnvironmentGenerator = false;
  if (cmSystemTools::GetEnv("CMAKE_GENERATOR", envGenVar)) {
    hasEnvironmentGenerator = true;
    this->EnvironmentGenerator = envGenVar;
  }

  auto readGeneratorVar = [&](std::string const& name, std::string& key) {
    std::string varValue;
    if (cmSystemTools::GetEnv(name, varValue)) {
      if (hasEnvironmentGenerator) {
        key = varValue;
      } else if (!this->GetIsInTryCompile()) {
        std::string message =
          cmStrCat("Warning: Environment variable ", name,
                   " will be ignored, because CMAKE_GENERATOR is not set.");
        cmSystemTools::Message(message, "Warning");
      }
    }
  };

  readGeneratorVar("CMAKE_GENERATOR_INSTANCE", this->GeneratorInstance);
  readGeneratorVar("CMAKE_GENERATOR_PLATFORM", this->GeneratorPlatform);
  readGeneratorVar("CMAKE_GENERATOR_TOOLSET", this->GeneratorToolset);
}

namespace {
enum class ListPresets
{
  None,
  Configure,
  Build,
  Test,
  All,
};
}

// Parse the args
void cmake::SetArgs(const std::vector<std::string>& args)
{
  bool haveToolset = false;
  bool havePlatform = false;
  bool haveBArg = false;
  bool scriptMode = false;
  std::string possibleUnknownArg;
#if !defined(CMAKE_BOOTSTRAP)
  std::string profilingFormat;
  std::string profilingOutput;
  std::string presetName;

  ListPresets listPresets = ListPresets::None;
#endif

  auto SourceArgLambda = [](std::string const& value, cmake* state) -> bool {
    std::string path = cmSystemTools::CollapseFullPath(value);
    cmSystemTools::ConvertToUnixSlashes(path);
    state->SetHomeDirectory(path);
    return true;
  };

  auto BuildArgLambda = [&](std::string const& value, cmake* state) -> bool {
    std::string path = cmSystemTools::CollapseFullPath(value);
    cmSystemTools::ConvertToUnixSlashes(path);
    state->SetHomeOutputDirectory(path);
    haveBArg = true;
    return true;
  };

  auto PlatformLambda = [&](std::string const& value, cmake* state) -> bool {
    if (havePlatform) {
      cmSystemTools::Error("Multiple -A options not allowed");
      return false;
    }
    state->SetGeneratorPlatform(value);
    havePlatform = true;
    return true;
  };

  auto ToolsetLamda = [&](std::string const& value, cmake* state) -> bool {
    if (haveToolset) {
      cmSystemTools::Error("Multiple -T options not allowed");
      return false;
    }
    state->SetGeneratorToolset(value);
    haveToolset = true;
    return true;
  };

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "-S", "No source directory specified for -S",
                     CommandArgument::Values::One, SourceArgLambda },
    CommandArgument{ "-H", "No source directory specified for -H",
                     CommandArgument::Values::One, SourceArgLambda },
    CommandArgument{ "-O", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda },
    CommandArgument{ "-B", "No build directory specified for -B",
                     CommandArgument::Values::One, BuildArgLambda },
    CommandArgument{ "-P", "-P must be followed by a file name.",
                     CommandArgument::Values::One,
                     [&](std::string const&, cmake*) -> bool {
                       scriptMode = true;
                       return true;
                     } },
    CommandArgument{ "-D", "-D must be followed with VAR=VALUE.",
                     CommandArgument::Values::One, IgnoreAndTrueLambda },
    CommandArgument{ "-C", "-C must be followed by a file name.",
                     CommandArgument::Values::One, IgnoreAndTrueLambda },
    CommandArgument{ "-U", "-U must be followed with VAR.",
                     CommandArgument::Values::One, IgnoreAndTrueLambda },
    CommandArgument{ "-W", "-W must be followed with [no-]<name>.",
                     CommandArgument::Values::One, IgnoreAndTrueLambda },
    CommandArgument{ "-A", "No platform specified for -A",
                     CommandArgument::Values::One, PlatformLambda },
    CommandArgument{ "-T", "No toolset specified for -T",
                     CommandArgument::Values::One, ToolsetLamda },

    CommandArgument{ "--check-build-system", CommandArgument::Values::Two,
                     [](std::string const& value, cmake* state) -> bool {
                       std::vector<std::string> values = cmExpandedList(value);
                       state->CheckBuildSystemArgument = values[0];
                       state->ClearBuildSystem = (atoi(values[1].c_str()) > 0);
                       return true;
                     } },
    CommandArgument{ "--check-stamp-file", CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       state->CheckStampFile = value;
                       return true;
                     } },
    CommandArgument{ "--check-stamp-list", CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       state->CheckStampList = value;
                       return true;
                     } },
    CommandArgument{ "--regenerate-during-build",
                     CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       state->RegenerateDuringBuild = true;
                       return true;
                     } },

    CommandArgument{ "--find-package", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda },

    CommandArgument{ "--graphviz", "No file specified for --graphviz",
                     CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       std::string path =
                         cmSystemTools::CollapseFullPath(value);
                       cmSystemTools::ConvertToUnixSlashes(path);
                       state->GraphVizFile = path;
                       return true;
                     } },

    CommandArgument{ "--debug-trycompile", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       std::cout << "debug trycompile on\n";
                       state->DebugTryCompileOn();
                       return true;
                     } },
    CommandArgument{ "--debug-output", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       std::cout << "Running with debug output on.\n";
                       state->SetDebugOutputOn(true);
                       return true;
                     } },

    CommandArgument{ "--log-level", "Invalid level specified for --log-level",
                     CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       const auto logLevel = StringToLogLevel(value);
                       if (logLevel == LogLevel::LOG_UNDEFINED) {
                         cmSystemTools::Error(
                           "Invalid level specified for --log-level");
                         return false;
                       }
                       state->SetLogLevel(logLevel);
                       state->LogLevelWasSetViaCLI = true;
                       return true;
                     } },
    // This is supported for backward compatibility. This option only
    // appeared in the 3.15.x release series and was renamed to
    // --log-level in 3.16.0
    CommandArgument{ "--loglevel", "Invalid level specified for --loglevel",
                     CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       const auto logLevel = StringToLogLevel(value);
                       if (logLevel == LogLevel::LOG_UNDEFINED) {
                         cmSystemTools::Error(
                           "Invalid level specified for --loglevel");
                         return false;
                       }
                       state->SetLogLevel(logLevel);
                       state->LogLevelWasSetViaCLI = true;
                       return true;
                     } },

    CommandArgument{ "--log-context", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       state->SetShowLogContext(true);
                       return true;
                     } },
    CommandArgument{
      "--debug-find", CommandArgument::Values::Zero,
      [](std::string const&, cmake* state) -> bool {
        std::cout << "Running with debug output on for the `find` commands.\n";
        state->SetDebugFindOutputOn(true);
        return true;
      } },
    CommandArgument{ "--trace-expand", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       std::cout << "Running with expanded trace output on.\n";
                       state->SetTrace(true);
                       state->SetTraceExpand(true);
                       return true;
                     } },
    CommandArgument{ "--trace-format", CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       state->SetTrace(true);
                       const auto traceFormat = StringToTraceFormat(value);
                       if (traceFormat == TraceFormat::TRACE_UNDEFINED) {
                         cmSystemTools::Error(
                           "Invalid format specified for --trace-format. "
                           "Valid formats are human, json-v1.");
                         return false;
                       }
                       state->SetTraceFormat(traceFormat);
                       return true;
                     } },
    CommandArgument{ "--trace-source", CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       std::string file(value);
                       cmSystemTools::ConvertToUnixSlashes(file);
                       state->AddTraceSource(file);
                       state->SetTrace(true);
                       return true;
                     } },
    CommandArgument{ "--trace-redirect", CommandArgument::Values::One,
                     [](std::string const& value, cmake* state) -> bool {
                       std::string file(value);
                       cmSystemTools::ConvertToUnixSlashes(file);
                       state->SetTraceFile(file);
                       state->SetTrace(true);
                       return true;
                     } },
    CommandArgument{ "--trace", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       std::cout << "Running with trace output on.\n";
                       state->SetTrace(true);
                       state->SetTraceExpand(false);
                       return true;
                     } },
    CommandArgument{ "--warn-uninitialized", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       std::cout << "Warn about uninitialized values.\n";
                       state->SetWarnUninitialized(true);
                       return true;
                     } },
    CommandArgument{ "--warn-unused-vars", CommandArgument::Values::Zero,
                     IgnoreAndTrueLambda }, // Option was removed.
    CommandArgument{ "--no-warn-unused-cli", CommandArgument::Values::Zero,
                     [](std::string const&, cmake* state) -> bool {
                       std::cout
                         << "Not searching for unused variables given on the "
                         << "command line.\n";
                       state->SetWarnUnusedCli(false);
                       return true;
                     } },
    CommandArgument{
      "--check-system-vars", CommandArgument::Values::Zero,
      [](std::string const&, cmake* state) -> bool {
        std::cout << "Also check system files when warning about unused and "
                  << "uninitialized variables.\n";
        state->SetCheckSystemVars(true);
        return true;
      } }
  };

#if defined(CMAKE_HAVE_VS_GENERATORS)
  arguments.emplace_back("--vs-solution-file", CommandArgument::Values::One,
                         [](std::string const& value, cmake* state) -> bool {
                           state->VSSolutionFile = value;
                           return true;
                         });
#endif

#if !defined(CMAKE_BOOTSTRAP)
  arguments.emplace_back("--profiling-format",
                         "No format specified for --profiling-format",
                         CommandArgument::Values::One,
                         [&](std::string const& value, cmake*) -> bool {
                           profilingFormat = value;
                           return true;
                         });
  arguments.emplace_back(
    "--profiling-output", "No path specified for --profiling-output",
    CommandArgument::Values::One,
    [&](std::string const& value, cmake*) -> bool {
      profilingOutput = cmSystemTools::CollapseFullPath(value);
      cmSystemTools::ConvertToUnixSlashes(profilingOutput);
      return true;
    });
  arguments.emplace_back("--preset", "No preset specified for --preset",
                         CommandArgument::Values::One,
                         [&](std::string const& value, cmake*) -> bool {
                           presetName = value;
                           return true;
                         });
  arguments.emplace_back(
    "--list-presets", CommandArgument::Values::ZeroOrOne,
    [&](std::string const& value, cmake*) -> bool {
      if (value.empty() || value == "configure") {
        listPresets = ListPresets::Configure;
      } else if (value == "build") {
        listPresets = ListPresets::Build;
      } else if (value == "test") {
        listPresets = ListPresets::Test;
      } else if (value == "all") {
        listPresets = ListPresets::All;
      } else {
        cmSystemTools::Error(
          "Invalid value specified for --list-presets.\n"
          "Valid values are configure, build, test, or all. "
          "When no value is passed the default is configure.");
        return false;
      }

      return true;
    });

#endif

  bool badGeneratorName = false;
  CommandArgument generatorCommand(
    "-G", "No generator specified for -G", CommandArgument::Values::One,
    [&](std::string const& value, cmake* state) -> bool {
      bool valid = state->CreateAndSetGlobalGenerator(value, true);
      badGeneratorName = !valid;
      return valid;
    });

  for (decltype(args.size()) i = 1; i < args.size(); ++i) {
    // iterate each argument
    std::string const& arg = args[i];

    // Generator flag has special handling for when to print help
    // so it becomes the exception
    if (generatorCommand.matches(arg)) {
      bool parsed = generatorCommand.parse(arg, i, args, this);
      if (!parsed && !badGeneratorName) {
        this->PrintGeneratorList();
        return;
      }
      continue;
    }

    bool matched = false;
    bool parsedCorrectly = true; // needs to be true so we can ignore
                                 // arguments so as -E
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        matched = true;
        parsedCorrectly = m.parse(arg, i, args, this);
        break;
      }
    }

    // We have an issue where arguments to a "-P" script mode
    // can be provided before the "-P" argument. This means
    // that we need to lazily check this argument after checking
    // all args.
    // Additionally it can't be the source/binary tree location
    if (!parsedCorrectly) {
      cmSystemTools::Error("Run 'cmake --help' for all supported options.");
      exit(1);
    } else if (!matched && cmHasLiteralPrefix(arg, "-")) {
      possibleUnknownArg = arg;
    } else if (!matched) {
      this->SetDirectoriesFromFile(arg);
    }
  }

  if (!possibleUnknownArg.empty() && !scriptMode) {
    cmSystemTools::Error(cmStrCat("Unknown argument ", possibleUnknownArg));
    cmSystemTools::Error("Run 'cmake --help' for all supported options.");
    exit(1);
  }

  // Empty instance, platform and toolset if only a generator is specified
  if (this->GlobalGenerator) {
    this->GeneratorInstance = "";
    if (!this->GeneratorPlatformSet) {
      this->GeneratorPlatform = "";
    }
    if (!this->GeneratorToolsetSet) {
      this->GeneratorToolset = "";
    }
  }

#if !defined(CMAKE_BOOTSTRAP)
  if (!profilingOutput.empty() || !profilingFormat.empty()) {
    if (profilingOutput.empty()) {
      cmSystemTools::Error(
        "--profiling-format specified but no --profiling-output!");
      return;
    }
    if (profilingFormat == "google-trace"_s) {
      try {
        this->ProfilingOutput =
          cm::make_unique<cmMakefileProfilingData>(profilingOutput);
      } catch (std::runtime_error& e) {
        cmSystemTools::Error(
          cmStrCat("Could not start profiling: ", e.what()));
        return;
      }
    } else {
      cmSystemTools::Error("Invalid format specified for --profiling-format");
      return;
    }
  }
#endif

  const bool haveSourceDir = !this->GetHomeDirectory().empty();
  const bool haveBinaryDir = !this->GetHomeOutputDirectory().empty();
  const bool havePreset =
#ifdef CMAKE_BOOTSTRAP
    false;
#else
    !presetName.empty();
#endif

  if (this->CurrentWorkingMode == cmake::NORMAL_MODE && !haveSourceDir &&
      !haveBinaryDir && !havePreset) {
    this->IssueMessage(
      MessageType::WARNING,
      "No source or binary directory provided. Both will be assumed to be "
      "the same as the current working directory, but note that this "
      "warning will become a fatal error in future CMake releases.");
  }

  if (!haveSourceDir) {
    this->SetHomeDirectory(cmSystemTools::GetCurrentWorkingDirectory());
  }
  if (!haveBinaryDir) {
    this->SetHomeOutputDirectory(cmSystemTools::GetCurrentWorkingDirectory());
  }

#if !defined(CMAKE_BOOTSTRAP)
  if (listPresets != ListPresets::None || !presetName.empty()) {
    cmCMakePresetsFile settingsFile;
    auto result = settingsFile.ReadProjectPresets(this->GetHomeDirectory());
    if (result != cmCMakePresetsFile::ReadFileResult::READ_OK) {
      cmSystemTools::Error(
        cmStrCat("Could not read presets from ", this->GetHomeDirectory(),
                 ": ", cmCMakePresetsFile::ResultToString(result)));
      return;
    }

    if (listPresets != ListPresets::None) {
      if (listPresets == ListPresets::Configure) {
        this->PrintPresetList(settingsFile);
      } else if (listPresets == ListPresets::Build) {
        settingsFile.PrintBuildPresetList();
      } else if (listPresets == ListPresets::Test) {
        settingsFile.PrintTestPresetList();
      } else if (listPresets == ListPresets::All) {
        settingsFile.PrintAllPresets();
      }

      this->SetWorkingMode(WorkingMode::HELP_MODE);
      return;
    }

    auto preset = settingsFile.ConfigurePresets.find(presetName);
    if (preset == settingsFile.ConfigurePresets.end()) {
      cmSystemTools::Error(cmStrCat("No such preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    presetName, '"'));
      this->PrintPresetList(settingsFile);
      return;
    }
    if (preset->second.Unexpanded.Hidden) {
      cmSystemTools::Error(cmStrCat("Cannot use hidden preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    presetName, '"'));
      this->PrintPresetList(settingsFile);
      return;
    }
    auto const& expandedPreset = preset->second.Expanded;
    if (!expandedPreset) {
      cmSystemTools::Error(cmStrCat("Could not evaluate preset \"",
                                    preset->second.Unexpanded.Name,
                                    "\": Invalid macro expansion"));
      return;
    }

    if (!this->State->IsCacheLoaded() && !haveBArg) {
      this->SetHomeOutputDirectory(expandedPreset->BinaryDir);
    }
    if (!this->GlobalGenerator) {
      if (!this->CreateAndSetGlobalGenerator(expandedPreset->Generator,
                                             false)) {
        return;
      }
    }
    this->UnprocessedPresetVariables = expandedPreset->CacheVariables;
    this->UnprocessedPresetEnvironment = expandedPreset->Environment;

    if (!expandedPreset->ArchitectureStrategy ||
        expandedPreset->ArchitectureStrategy ==
          cmCMakePresetsFile::ArchToolsetStrategy::Set) {
      if (!this->GeneratorPlatformSet) {
        this->SetGeneratorPlatform(expandedPreset->Architecture);
      }
    }
    if (!expandedPreset->ToolsetStrategy ||
        expandedPreset->ToolsetStrategy ==
          cmCMakePresetsFile::ArchToolsetStrategy::Set) {
      if (!this->GeneratorToolsetSet) {
        this->SetGeneratorToolset(expandedPreset->Toolset);
      }
    }

    this->SetWarningFromPreset("dev", expandedPreset->WarnDev,
                               expandedPreset->ErrorDev);
    this->SetWarningFromPreset("deprecated", expandedPreset->WarnDeprecated,
                               expandedPreset->ErrorDeprecated);
    if (expandedPreset->WarnUninitialized == true) {
      this->SetWarnUninitialized(true);
    }
    if (expandedPreset->WarnUnusedCli == false) {
      this->SetWarnUnusedCli(false);
    }
    if (expandedPreset->WarnSystemVars == true) {
      this->SetCheckSystemVars(true);
    }
    if (expandedPreset->DebugOutput == true) {
      this->SetDebugOutputOn(true);
    }
    if (expandedPreset->DebugTryCompile == true) {
      this->DebugTryCompileOn();
    }
    if (expandedPreset->DebugFind == true) {
      this->SetDebugFindOutputOn(true);
    }
  }
#endif
}

cmake::LogLevel cmake::StringToLogLevel(const std::string& levelStr)
{
  using LevelsPair = std::pair<std::string, LogLevel>;
  static const std::vector<LevelsPair> levels = {
    { "error", LogLevel::LOG_ERROR },     { "warning", LogLevel::LOG_WARNING },
    { "notice", LogLevel::LOG_NOTICE },   { "status", LogLevel::LOG_STATUS },
    { "verbose", LogLevel::LOG_VERBOSE }, { "debug", LogLevel::LOG_DEBUG },
    { "trace", LogLevel::LOG_TRACE }
  };

  const auto levelStrLowCase = cmSystemTools::LowerCase(levelStr);

  const auto it = std::find_if(levels.cbegin(), levels.cend(),
                               [&levelStrLowCase](const LevelsPair& p) {
                                 return p.first == levelStrLowCase;
                               });
  return (it != levels.cend()) ? it->second : LogLevel::LOG_UNDEFINED;
}

cmake::TraceFormat cmake::StringToTraceFormat(const std::string& traceStr)
{
  using TracePair = std::pair<std::string, TraceFormat>;
  static const std::vector<TracePair> levels = {
    { "human", TraceFormat::TRACE_HUMAN },
    { "json-v1", TraceFormat::TRACE_JSON_V1 },
  };

  const auto traceStrLowCase = cmSystemTools::LowerCase(traceStr);

  const auto it = std::find_if(levels.cbegin(), levels.cend(),
                               [&traceStrLowCase](const TracePair& p) {
                                 return p.first == traceStrLowCase;
                               });
  return (it != levels.cend()) ? it->second : TraceFormat::TRACE_UNDEFINED;
}

void cmake::SetTraceFile(const std::string& file)
{
  this->TraceFile.close();
  this->TraceFile.open(file.c_str());
  if (!this->TraceFile) {
    std::stringstream ss;
    ss << "Error opening trace file " << file << ": "
       << cmSystemTools::GetLastSystemError();
    cmSystemTools::Error(ss.str());
    return;
  }
  std::cout << "Trace will be written to " << file << "\n";
}

void cmake::PrintTraceFormatVersion()
{
  if (!this->GetTrace()) {
    return;
  }

  std::string msg;

  switch (this->GetTraceFormat()) {
    case TraceFormat::TRACE_JSON_V1: {
#ifndef CMAKE_BOOTSTRAP
      Json::Value val;
      Json::Value version;
      Json::StreamWriterBuilder builder;
      builder["indentation"] = "";
      version["major"] = 1;
      version["minor"] = 1;
      val["version"] = version;
      msg = Json::writeString(builder, val);
#endif
      break;
    }
    case TraceFormat::TRACE_HUMAN:
      msg = "";
      break;
    case TraceFormat::TRACE_UNDEFINED:
      msg = "INTERNAL ERROR: Trace format is TRACE_UNDEFINED";
      break;
  }

  if (msg.empty()) {
    return;
  }

  auto& f = this->GetTraceFile();
  if (f) {
    f << msg << '\n';
  } else {
    cmSystemTools::Message(msg);
  }
}

void cmake::SetDirectoriesFromFile(const std::string& arg)
{
  // Check if the argument refers to a CMakeCache.txt or
  // CMakeLists.txt file.
  std::string listPath;
  std::string cachePath;
  bool argIsFile = false;
  if (cmSystemTools::FileIsDirectory(arg)) {
    std::string path = cmSystemTools::CollapseFullPath(arg);
    cmSystemTools::ConvertToUnixSlashes(path);
    std::string cacheFile = cmStrCat(path, "/CMakeCache.txt");
    std::string listFile = cmStrCat(path, "/CMakeLists.txt");
    if (cmSystemTools::FileExists(cacheFile)) {
      cachePath = path;
    }
    if (cmSystemTools::FileExists(listFile)) {
      listPath = path;
    }
  } else if (cmSystemTools::FileExists(arg)) {
    argIsFile = true;
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath);
    name = cmSystemTools::LowerCase(name);
    if (name == "cmakecache.txt"_s) {
      cachePath = cmSystemTools::GetFilenamePath(fullPath);
    } else if (name == "cmakelists.txt"_s) {
      listPath = cmSystemTools::GetFilenamePath(fullPath);
    }
  } else {
    // Specified file or directory does not exist.  Try to set things
    // up to produce a meaningful error message.
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath);
    name = cmSystemTools::LowerCase(name);
    if (name == "cmakecache.txt"_s || name == "cmakelists.txt"_s) {
      argIsFile = true;
      listPath = cmSystemTools::GetFilenamePath(fullPath);
    } else {
      listPath = fullPath;
    }
  }

  // If there is a CMakeCache.txt file, use its settings.
  if (!cachePath.empty()) {
    if (this->LoadCache(cachePath)) {
      cmProp existingValue =
        this->State->GetCacheEntryValue("CMAKE_HOME_DIRECTORY");
      if (existingValue) {
        this->SetHomeOutputDirectory(cachePath);
        this->SetHomeDirectory(*existingValue);
        return;
      }
    }
  }

  // If there is a CMakeLists.txt file, use it as the source tree.
  if (!listPath.empty()) {
    this->SetHomeDirectory(listPath);

    if (argIsFile) {
      // Source CMakeLists.txt file given.  It was probably dropped
      // onto the executable in a GUI.  Default to an in-source build.
      this->SetHomeOutputDirectory(listPath);
    } else {
      // Source directory given on command line.  Use current working
      // directory as build tree if -B hasn't been given already
      if (this->GetHomeOutputDirectory().empty()) {
        std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
        this->SetHomeOutputDirectory(cwd);
      }
    }
    return;
  }

  if (this->GetHomeDirectory().empty()) {
    // We didn't find a CMakeLists.txt and it wasn't specified
    // with -S. Assume it is the path to the source tree
    std::string full = cmSystemTools::CollapseFullPath(arg);
    this->SetHomeDirectory(full);
  }
  if (this->GetHomeOutputDirectory().empty()) {
    // We didn't find a CMakeCache.txt and it wasn't specified
    // with -B. Assume the current working directory as the build tree.
    std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
    this->SetHomeOutputDirectory(cwd);
  }
}

// at the end of this CMAKE_ROOT and CMAKE_COMMAND should be added to the
// cache
int cmake::AddCMakePaths()
{
  // Save the value in the cache
  this->AddCacheEntry("CMAKE_COMMAND",
                      cmSystemTools::GetCMakeCommand().c_str(),
                      "Path to CMake executable.", cmStateEnums::INTERNAL);
#ifndef CMAKE_BOOTSTRAP
  this->AddCacheEntry(
    "CMAKE_CTEST_COMMAND", cmSystemTools::GetCTestCommand().c_str(),
    "Path to ctest program executable.", cmStateEnums::INTERNAL);
  this->AddCacheEntry(
    "CMAKE_CPACK_COMMAND", cmSystemTools::GetCPackCommand().c_str(),
    "Path to cpack program executable.", cmStateEnums::INTERNAL);
#endif
  if (!cmSystemTools::FileExists(
        (cmSystemTools::GetCMakeRoot() + "/Modules/CMake.cmake"))) {
    // couldn't find modules
    cmSystemTools::Error(
      "Could not find CMAKE_ROOT !!!\n"
      "CMake has most likely not been installed correctly.\n"
      "Modules directory not found in\n" +
      cmSystemTools::GetCMakeRoot());
    return 0;
  }
  this->AddCacheEntry("CMAKE_ROOT", cmSystemTools::GetCMakeRoot().c_str(),
                      "Path to CMake installation.", cmStateEnums::INTERNAL);

  return 1;
}

void cmake::AddDefaultExtraGenerators()
{
#if !defined(CMAKE_BOOTSTRAP)
  this->ExtraGenerators.push_back(cmExtraCodeBlocksGenerator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraCodeLiteGenerator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraEclipseCDT4Generator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraKateGenerator::GetFactory());
  this->ExtraGenerators.push_back(cmExtraSublimeTextGenerator::GetFactory());
#endif
}

void cmake::GetRegisteredGenerators(std::vector<GeneratorInfo>& generators,
                                    bool includeNamesWithPlatform) const
{
  for (const auto& gen : this->Generators) {
    std::vector<std::string> names = gen->GetGeneratorNames();

    if (includeNamesWithPlatform) {
      cm::append(names, gen->GetGeneratorNamesWithPlatform());
    }

    for (std::string const& name : names) {
      GeneratorInfo info;
      info.supportsToolset = gen->SupportsToolset();
      info.supportsPlatform = gen->SupportsPlatform();
      info.supportedPlatforms = gen->GetKnownPlatforms();
      info.defaultPlatform = gen->GetDefaultPlatformName();
      info.name = name;
      info.baseName = name;
      info.isAlias = false;
      generators.push_back(std::move(info));
    }
  }

  for (cmExternalMakefileProjectGeneratorFactory* eg : this->ExtraGenerators) {
    const std::vector<std::string> genList =
      eg->GetSupportedGlobalGenerators();
    for (std::string const& gen : genList) {
      GeneratorInfo info;
      info.name = cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
        gen, eg->GetName());
      info.baseName = gen;
      info.extraName = eg->GetName();
      info.supportsPlatform = false;
      info.supportsToolset = false;
      info.isAlias = false;
      generators.push_back(std::move(info));
    }
    for (std::string const& a : eg->Aliases) {
      GeneratorInfo info;
      info.name = a;
      if (!genList.empty()) {
        info.baseName = genList.at(0);
      }
      info.extraName = eg->GetName();
      info.supportsPlatform = false;
      info.supportsToolset = false;
      info.isAlias = true;
      generators.push_back(std::move(info));
    }
  }
}

static std::pair<std::unique_ptr<cmExternalMakefileProjectGenerator>,
                 std::string>
createExtraGenerator(
  const std::vector<cmExternalMakefileProjectGeneratorFactory*>& in,
  const std::string& name)
{
  for (cmExternalMakefileProjectGeneratorFactory* i : in) {
    const std::vector<std::string> generators =
      i->GetSupportedGlobalGenerators();
    if (i->GetName() == name) { // Match aliases
      return { i->CreateExternalMakefileProjectGenerator(), generators.at(0) };
    }
    for (std::string const& g : generators) {
      const std::string fullName =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
          g, i->GetName());
      if (fullName == name) {
        return { i->CreateExternalMakefileProjectGenerator(), g };
      }
    }
  }
  return { nullptr, name };
}

std::unique_ptr<cmGlobalGenerator> cmake::CreateGlobalGenerator(
  const std::string& gname, bool allowArch)
{
  std::pair<std::unique_ptr<cmExternalMakefileProjectGenerator>, std::string>
    extra = createExtraGenerator(this->ExtraGenerators, gname);
  std::unique_ptr<cmExternalMakefileProjectGenerator>& extraGenerator =
    extra.first;
  const std::string& name = extra.second;

  std::unique_ptr<cmGlobalGenerator> generator;
  for (const auto& g : this->Generators) {
    generator = g->CreateGlobalGenerator(name, allowArch, this);
    if (generator) {
      break;
    }
  }

  if (generator) {
    generator->SetExternalMakefileProjectGenerator(std::move(extraGenerator));
  }

  return generator;
}

bool cmake::CreateAndSetGlobalGenerator(const std::string& name,
                                        bool allowArch)
{
  auto gen = this->CreateGlobalGenerator(name, allowArch);
  if (!gen) {
    std::string kdevError;
    std::string vsError;
    if (name.find("KDevelop3", 0) != std::string::npos) {
      kdevError = "\nThe KDevelop3 generator is not supported anymore.";
    }
    if (!allowArch && cmHasLiteralPrefix(name, "Visual Studio ") &&
        name.length() >= cmStrLen("Visual Studio xx xxxx ")) {
      vsError = "\nUsing platforms in Visual Studio generator names is not "
                "supported in CMakePresets.json.";
    }

    cmSystemTools::Error(
      cmStrCat("Could not create named generator ", name, kdevError, vsError));
    this->PrintGeneratorList();
    return false;
  }

  this->SetGlobalGenerator(std::move(gen));
  return true;
}

#ifndef CMAKE_BOOTSTRAP
void cmake::PrintPresetList(const cmCMakePresetsFile& file) const
{
  std::vector<GeneratorInfo> generators;
  this->GetRegisteredGenerators(generators, false);
  auto filter =
    [&generators](const cmCMakePresetsFile::ConfigurePreset& preset) -> bool {
    auto condition = [&preset](const GeneratorInfo& info) -> bool {
      return info.name == preset.Generator;
    };
    auto it = std::find_if(generators.begin(), generators.end(), condition);
    return it != generators.end();
  };

  file.PrintConfigurePresetList(filter);
}
#endif

void cmake::SetHomeDirectory(const std::string& dir)
{
  this->State->SetSourceDirectory(dir);
  if (this->CurrentSnapshot.IsValid()) {
    this->CurrentSnapshot.SetDefinition("CMAKE_SOURCE_DIR", dir);
  }
}

std::string const& cmake::GetHomeDirectory() const
{
  return this->State->GetSourceDirectory();
}

void cmake::SetHomeOutputDirectory(const std::string& dir)
{
  this->State->SetBinaryDirectory(dir);
  if (this->CurrentSnapshot.IsValid()) {
    this->CurrentSnapshot.SetDefinition("CMAKE_BINARY_DIR", dir);
  }
}

std::string const& cmake::GetHomeOutputDirectory() const
{
  return this->State->GetBinaryDirectory();
}

std::string cmake::FindCacheFile(const std::string& binaryDir)
{
  std::string cachePath = binaryDir;
  cmSystemTools::ConvertToUnixSlashes(cachePath);
  std::string cacheFile = cmStrCat(cachePath, "/CMakeCache.txt");
  if (!cmSystemTools::FileExists(cacheFile)) {
    // search in parent directories for cache
    std::string cmakeFiles = cmStrCat(cachePath, "/CMakeFiles");
    if (cmSystemTools::FileExists(cmakeFiles)) {
      std::string cachePathFound =
        cmSystemTools::FileExistsInParentDirectories("CMakeCache.txt",
                                                     cachePath, "/");
      if (!cachePathFound.empty()) {
        cachePath = cmSystemTools::GetFilenamePath(cachePathFound);
      }
    }
  }
  return cachePath;
}

void cmake::SetGlobalGenerator(std::unique_ptr<cmGlobalGenerator> gg)
{
  if (!gg) {
    cmSystemTools::Error("Error SetGlobalGenerator called with null");
    return;
  }
  if (this->GlobalGenerator) {
    // restore the original environment variables CXX and CC
    // Restore CC
    std::string env = "CC=";
    if (!this->CCEnvironment.empty()) {
      env += this->CCEnvironment;
    }
    cmSystemTools::PutEnv(env);
    env = "CXX=";
    if (!this->CXXEnvironment.empty()) {
      env += this->CXXEnvironment;
    }
    cmSystemTools::PutEnv(env);
  }

  // set the new
  this->GlobalGenerator = std::move(gg);

  // set the global flag for unix style paths on cmSystemTools as soon as
  // the generator is set.  This allows gmake to be used on windows.
  cmSystemTools::SetForceUnixPaths(this->GlobalGenerator->GetForceUnixPaths());

  // Save the environment variables CXX and CC
  if (!cmSystemTools::GetEnv("CXX", this->CXXEnvironment)) {
    this->CXXEnvironment.clear();
  }
  if (!cmSystemTools::GetEnv("CC", this->CCEnvironment)) {
    this->CCEnvironment.clear();
  }
}

int cmake::DoPreConfigureChecks()
{
  // Make sure the Source directory contains a CMakeLists.txt file.
  std::string srcList = cmStrCat(this->GetHomeDirectory(), "/CMakeLists.txt");
  if (!cmSystemTools::FileExists(srcList)) {
    std::ostringstream err;
    if (cmSystemTools::FileIsDirectory(this->GetHomeDirectory())) {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not appear to contain CMakeLists.txt.\n";
    } else if (cmSystemTools::FileExists(this->GetHomeDirectory())) {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" is a file, not a directory.\n";
    } else {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not exist.\n";
    }
    err << "Specify --help for usage, or press the help button on the CMake "
           "GUI.";
    cmSystemTools::Error(err.str());
    return -2;
  }

  // do a sanity check on some values
  if (this->State->GetInitializedCacheValue("CMAKE_HOME_DIRECTORY")) {
    std::string cacheStart =
      cmStrCat(*this->State->GetInitializedCacheValue("CMAKE_HOME_DIRECTORY"),
               "/CMakeLists.txt");
    std::string currentStart =
      cmStrCat(this->GetHomeDirectory(), "/CMakeLists.txt");
    if (!cmSystemTools::SameFile(cacheStart, currentStart)) {
      std::string message =
        cmStrCat("The source \"", currentStart,
                 "\" does not match the source \"", cacheStart,
                 "\" used to generate cache.  Re-run cmake with a different "
                 "source directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    return 0;
  }
  return 1;
}
struct SaveCacheEntry
{
  std::string key;
  std::string value;
  std::string help;
  cmStateEnums::CacheEntryType type;
};

int cmake::HandleDeleteCacheVariables(const std::string& var)
{
  std::vector<std::string> argsSplit = cmExpandedList(var, true);
  // erase the property to avoid infinite recursion
  this->State->SetGlobalProperty("__CMAKE_DELETE_CACHE_CHANGE_VARS_", "");
  if (this->State->GetIsInTryCompile()) {
    return 0;
  }
  std::vector<SaveCacheEntry> saved;
  std::ostringstream warning;
  /* clang-format off */
  warning
    << "You have changed variables that require your cache to be deleted.\n"
    << "Configure will be re-run and you may have to reset some variables.\n"
    << "The following variables have changed:\n";
  /* clang-format on */
  for (auto i = argsSplit.begin(); i != argsSplit.end(); ++i) {
    SaveCacheEntry save;
    save.key = *i;
    warning << *i << "= ";
    i++;
    if (i != argsSplit.end()) {
      save.value = *i;
      warning << *i << "\n";
    } else {
      warning << "\n";
      i -= 1;
    }
    cmProp existingValue = this->State->GetCacheEntryValue(save.key);
    if (existingValue) {
      save.type = this->State->GetCacheEntryType(save.key);
      if (cmProp help =
            this->State->GetCacheEntryProperty(save.key, "HELPSTRING")) {
        save.help = *help;
      }
    } else {
      save.type = cmStateEnums::CacheEntryType::UNINITIALIZED;
    }
    saved.push_back(std::move(save));
  }

  // remove the cache
  this->DeleteCache(this->GetHomeOutputDirectory());
  // load the empty cache
  this->LoadCache();
  // restore the changed compilers
  for (SaveCacheEntry const& i : saved) {
    this->AddCacheEntry(i.key, i.value.c_str(), i.help.c_str(), i.type);
  }
  cmSystemTools::Message(warning.str());
  // avoid reconfigure if there were errors
  if (!cmSystemTools::GetErrorOccuredFlag()) {
    // re-run configure
    return this->Configure();
  }
  return 0;
}

int cmake::Configure()
{
  DiagLevel diagLevel;

  if (this->DiagLevels.count("deprecated") == 1) {

    diagLevel = this->DiagLevels["deprecated"];
    if (diagLevel == DIAG_IGNORE) {
      this->SetSuppressDeprecatedWarnings(true);
      this->SetDeprecatedWarningsAsErrors(false);
    } else if (diagLevel == DIAG_WARN) {
      this->SetSuppressDeprecatedWarnings(false);
      this->SetDeprecatedWarningsAsErrors(false);
    } else if (diagLevel == DIAG_ERROR) {
      this->SetSuppressDeprecatedWarnings(false);
      this->SetDeprecatedWarningsAsErrors(true);
    }
  }

  if (this->DiagLevels.count("dev") == 1) {
    bool setDeprecatedVariables = false;

    cmProp cachedWarnDeprecated =
      this->State->GetCacheEntryValue("CMAKE_WARN_DEPRECATED");
    cmProp cachedErrorDeprecated =
      this->State->GetCacheEntryValue("CMAKE_ERROR_DEPRECATED");

    // don't overwrite deprecated warning setting from a previous invocation
    if (!cachedWarnDeprecated && !cachedErrorDeprecated) {
      setDeprecatedVariables = true;
    }

    diagLevel = this->DiagLevels["dev"];
    if (diagLevel == DIAG_IGNORE) {
      this->SetSuppressDevWarnings(true);
      this->SetDevWarningsAsErrors(false);

      if (setDeprecatedVariables) {
        this->SetSuppressDeprecatedWarnings(true);
        this->SetDeprecatedWarningsAsErrors(false);
      }
    } else if (diagLevel == DIAG_WARN) {
      this->SetSuppressDevWarnings(false);
      this->SetDevWarningsAsErrors(false);

      if (setDeprecatedVariables) {
        this->SetSuppressDeprecatedWarnings(false);
        this->SetDeprecatedWarningsAsErrors(false);
      }
    } else if (diagLevel == DIAG_ERROR) {
      this->SetSuppressDevWarnings(false);
      this->SetDevWarningsAsErrors(true);

      if (setDeprecatedVariables) {
        this->SetSuppressDeprecatedWarnings(false);
        this->SetDeprecatedWarningsAsErrors(true);
      }
    }
  }

  // Cache variables may have already been set by a previous invocation,
  // so we cannot rely on command line options alone. Always ensure our
  // messenger is in sync with the cache.
  cmProp value = this->State->GetCacheEntryValue("CMAKE_WARN_DEPRECATED");
  this->Messenger->SetSuppressDeprecatedWarnings(value && cmIsOff(*value));

  value = this->State->GetCacheEntryValue("CMAKE_ERROR_DEPRECATED");
  this->Messenger->SetDeprecatedWarningsAsErrors(cmIsOn(value));

  value = this->State->GetCacheEntryValue("CMAKE_SUPPRESS_DEVELOPER_WARNINGS");
  this->Messenger->SetSuppressDevWarnings(cmIsOn(value));

  value = this->State->GetCacheEntryValue("CMAKE_SUPPRESS_DEVELOPER_ERRORS");
  this->Messenger->SetDevWarningsAsErrors(value && cmIsOff(*value));

  int ret = this->ActualConfigure();
  cmProp delCacheVars =
    this->State->GetGlobalProperty("__CMAKE_DELETE_CACHE_CHANGE_VARS_");
  if (delCacheVars && !delCacheVars->empty()) {
    return this->HandleDeleteCacheVariables(*delCacheVars);
  }
  return ret;
}

int cmake::ActualConfigure()
{
  // Construct right now our path conversion table before it's too late:
  this->UpdateConversionPathTable();
  this->CleanupCommandsAndMacros();

  int res = this->DoPreConfigureChecks();
  if (res < 0) {
    return -2;
  }
  if (!res) {
    this->AddCacheEntry(
      "CMAKE_HOME_DIRECTORY", this->GetHomeDirectory().c_str(),
      "Source directory with the top level CMakeLists.txt file for this "
      "project",
      cmStateEnums::INTERNAL);
  }

  // no generator specified on the command line
  if (!this->GlobalGenerator) {
    cmProp genName = this->State->GetInitializedCacheValue("CMAKE_GENERATOR");
    cmProp extraGenName =
      this->State->GetInitializedCacheValue("CMAKE_EXTRA_GENERATOR");
    if (genName) {
      std::string fullName =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
          *genName, extraGenName ? *extraGenName : "");
      this->GlobalGenerator = this->CreateGlobalGenerator(fullName);
    }
    if (this->GlobalGenerator) {
      // set the global flag for unix style paths on cmSystemTools as
      // soon as the generator is set.  This allows gmake to be used
      // on windows.
      cmSystemTools::SetForceUnixPaths(
        this->GlobalGenerator->GetForceUnixPaths());
    } else {
      this->CreateDefaultGlobalGenerator();
    }
    if (!this->GlobalGenerator) {
      cmSystemTools::Error("Could not create generator");
      return -1;
    }
  }

  cmProp genName = this->State->GetInitializedCacheValue("CMAKE_GENERATOR");
  if (genName) {
    if (!this->GlobalGenerator->MatchesGeneratorName(*genName)) {
      std::string message =
        cmStrCat("Error: generator : ", this->GlobalGenerator->GetName(),
                 "\nDoes not match the generator used previously: ", *genName,
                 "\nEither remove the CMakeCache.txt file and CMakeFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  }
  if (!this->State->GetInitializedCacheValue("CMAKE_GENERATOR")) {
    this->AddCacheEntry("CMAKE_GENERATOR",
                        this->GlobalGenerator->GetName().c_str(),
                        "Name of generator.", cmStateEnums::INTERNAL);
    this->AddCacheEntry("CMAKE_EXTRA_GENERATOR",
                        this->GlobalGenerator->GetExtraGeneratorName().c_str(),
                        "Name of external makefile project generator.",
                        cmStateEnums::INTERNAL);
  }

  if (cmProp instance =
        this->State->GetInitializedCacheValue("CMAKE_GENERATOR_INSTANCE")) {
    if (this->GeneratorInstanceSet && this->GeneratorInstance != *instance) {
      std::string message =
        cmStrCat("Error: generator instance: ", this->GeneratorInstance,
                 "\nDoes not match the instance used previously: ", *instance,
                 "\nEither remove the CMakeCache.txt file and CMakeFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    this->AddCacheEntry(
      "CMAKE_GENERATOR_INSTANCE", this->GeneratorInstance.c_str(),
      "Generator instance identifier.", cmStateEnums::INTERNAL);
  }

  if (cmProp platformName =
        this->State->GetInitializedCacheValue("CMAKE_GENERATOR_PLATFORM")) {
    if (this->GeneratorPlatformSet &&
        this->GeneratorPlatform != *platformName) {
      std::string message = cmStrCat(
        "Error: generator platform: ", this->GeneratorPlatform,
        "\nDoes not match the platform used previously: ", *platformName,
        "\nEither remove the CMakeCache.txt file and CMakeFiles "
        "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    this->AddCacheEntry("CMAKE_GENERATOR_PLATFORM",
                        this->GeneratorPlatform.c_str(),
                        "Name of generator platform.", cmStateEnums::INTERNAL);
  }

  if (cmProp tsName =
        this->State->GetInitializedCacheValue("CMAKE_GENERATOR_TOOLSET")) {
    if (this->GeneratorToolsetSet && this->GeneratorToolset != *tsName) {
      std::string message =
        cmStrCat("Error: generator toolset: ", this->GeneratorToolset,
                 "\nDoes not match the toolset used previously: ", *tsName,
                 "\nEither remove the CMakeCache.txt file and CMakeFiles "
                 "directory or choose a different binary directory.");
      cmSystemTools::Error(message);
      return -2;
    }
  } else {
    this->AddCacheEntry("CMAKE_GENERATOR_TOOLSET",
                        this->GeneratorToolset.c_str(),
                        "Name of generator toolset.", cmStateEnums::INTERNAL);
  }

  // reset any system configuration information, except for when we are
  // InTryCompile. With TryCompile the system info is taken from the parent's
  // info to save time
  if (!this->State->GetIsInTryCompile()) {
    this->GlobalGenerator->ClearEnabledLanguages();

    this->TruncateOutputLog("CMakeOutput.log");
    this->TruncateOutputLog("CMakeError.log");
  }

#if !defined(CMAKE_BOOTSTRAP)
  this->FileAPI = cm::make_unique<cmFileAPI>(this);
  this->FileAPI->ReadQueries();
#endif

  // actually do the configure
  this->GlobalGenerator->Configure();
  // Before saving the cache
  // if the project did not define one of the entries below, add them now
  // so users can edit the values in the cache:

  // We used to always present LIBRARY_OUTPUT_PATH and
  // EXECUTABLE_OUTPUT_PATH.  They are now documented as old-style and
  // should no longer be used.  Therefore we present them only if the
  // project requires compatibility with CMake 2.4.  We detect this
  // here by looking for the old CMAKE_BACKWARDS_COMPATIBILITY
  // variable created when CMP0001 is not set to NEW.
  if (this->State->GetInitializedCacheValue("CMAKE_BACKWARDS_COMPATIBILITY")) {
    if (!this->State->GetInitializedCacheValue("LIBRARY_OUTPUT_PATH")) {
      this->AddCacheEntry(
        "LIBRARY_OUTPUT_PATH", "",
        "Single output directory for building all libraries.",
        cmStateEnums::PATH);
    }
    if (!this->State->GetInitializedCacheValue("EXECUTABLE_OUTPUT_PATH")) {
      this->AddCacheEntry(
        "EXECUTABLE_OUTPUT_PATH", "",
        "Single output directory for building all executables.",
        cmStateEnums::PATH);
    }
  }

  const auto& mf = this->GlobalGenerator->GetMakefiles()[0];
  if (mf->IsOn("CTEST_USE_LAUNCHERS") &&
      !this->State->GetGlobalProperty("RULE_LAUNCH_COMPILE")) {
    cmSystemTools::Error(
      "CTEST_USE_LAUNCHERS is enabled, but the "
      "RULE_LAUNCH_COMPILE global property is not defined.\n"
      "Did you forget to include(CTest) in the toplevel "
      "CMakeLists.txt ?");
  }

  this->State->SaveVerificationScript(this->GetHomeOutputDirectory());
  this->SaveCache(this->GetHomeOutputDirectory());
  if (cmSystemTools::GetErrorOccuredFlag()) {
    return -1;
  }
  return 0;
}

std::unique_ptr<cmGlobalGenerator> cmake::EvaluateDefaultGlobalGenerator()
{
  if (!this->EnvironmentGenerator.empty()) {
    auto gen = this->CreateGlobalGenerator(this->EnvironmentGenerator);
    if (!gen) {
      cmSystemTools::Error("CMAKE_GENERATOR was set but the specified "
                           "generator doesn't exist. Using CMake default.");
    } else {
      return gen;
    }
  }
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(CMAKE_BOOT_MINGW)
  std::string found;
  // Try to find the newest VS installed on the computer and
  // use that as a default if -G is not specified
  const std::string vsregBase = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\";
  static const char* const vsVariants[] = {
    /* clang-format needs this comment to break after the opening brace */
    "VisualStudio\\", "VCExpress\\", "WDExpress\\"
  };
  struct VSVersionedGenerator
  {
    const char* MSVersion;
    const char* GeneratorName;
  };
  static VSVersionedGenerator const vsGenerators[] = {
    { "14.0", "Visual Studio 14 2015" }, //
    { "12.0", "Visual Studio 12 2013" }, //
    { "11.0", "Visual Studio 11 2012" }, //
    { "10.0", "Visual Studio 10 2010" }, //
    { "9.0", "Visual Studio 9 2008" }
  };
  static const char* const vsEntries[] = {
    "\\Setup\\VC;ProductDir", //
    ";InstallDir"             //
  };
  if (cmVSSetupAPIHelper(16).IsVSInstalled()) {
    found = "Visual Studio 16 2019";
  } else if (cmVSSetupAPIHelper(15).IsVSInstalled()) {
    found = "Visual Studio 15 2017";
  } else {
    for (VSVersionedGenerator const* g = cm::cbegin(vsGenerators);
         found.empty() && g != cm::cend(vsGenerators); ++g) {
      for (const char* const* v = cm::cbegin(vsVariants);
           found.empty() && v != cm::cend(vsVariants); ++v) {
        for (const char* const* e = cm::cbegin(vsEntries);
             found.empty() && e != cm::cend(vsEntries); ++e) {
          std::string const reg = vsregBase + *v + g->MSVersion + *e;
          std::string dir;
          if (cmSystemTools::ReadRegistryValue(reg, dir,
                                               cmSystemTools::KeyWOW64_32) &&
              cmSystemTools::PathExists(dir)) {
            found = g->GeneratorName;
          }
        }
      }
    }
  }
  auto gen = this->CreateGlobalGenerator(found);
  if (!gen) {
    gen = cm::make_unique<cmGlobalNMakeMakefileGenerator>(this);
  }
  return std::unique_ptr<cmGlobalGenerator>(std::move(gen));
#elif defined(CMAKE_BOOTSTRAP_NINJA)
  return std::unique_ptr<cmGlobalGenerator>(
    cm::make_unique<cmGlobalNinjaGenerator>(this));
#else
  return std::unique_ptr<cmGlobalGenerator>(
    cm::make_unique<cmGlobalUnixMakefileGenerator3>(this));
#endif
}

void cmake::CreateDefaultGlobalGenerator()
{
  auto gen = this->EvaluateDefaultGlobalGenerator();
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(CMAKE_BOOT_MINGW)
  // This print could be unified for all platforms
  std::cout << "-- Building for: " << gen->GetName() << "\n";
#endif
  this->SetGlobalGenerator(std::move(gen));
}

void cmake::PreLoadCMakeFiles()
{
  std::vector<std::string> args;
  std::string pre_load = this->GetHomeDirectory();
  if (!pre_load.empty()) {
    pre_load += "/PreLoad.cmake";
    if (cmSystemTools::FileExists(pre_load)) {
      this->ReadListFile(args, pre_load);
    }
  }
  pre_load = this->GetHomeOutputDirectory();
  if (!pre_load.empty()) {
    pre_load += "/PreLoad.cmake";
    if (cmSystemTools::FileExists(pre_load)) {
      this->ReadListFile(args, pre_load);
    }
  }
}

// handle a command line invocation
int cmake::Run(const std::vector<std::string>& args, bool noconfigure)
{
  // Process the arguments
  this->SetArgs(args);
  if (cmSystemTools::GetErrorOccuredFlag()) {
    return -1;
  }
  if (this->GetWorkingMode() == HELP_MODE) {
    return 0;
  }

  // Log the trace format version to the desired output
  if (this->GetTrace()) {
    this->PrintTraceFormatVersion();
  }

  // If we are given a stamp list file check if it is really out of date.
  if (!this->CheckStampList.empty() &&
      cmakeCheckStampList(this->CheckStampList)) {
    return 0;
  }

  // If we are given a stamp file check if it is really out of date.
  if (!this->CheckStampFile.empty() &&
      cmakeCheckStampFile(this->CheckStampFile)) {
    return 0;
  }

  if (this->GetWorkingMode() == NORMAL_MODE) {
    // load the cache
    if (this->LoadCache() < 0) {
      cmSystemTools::Error("Error executing cmake::LoadCache(). Aborting.\n");
      return -1;
    }
  } else {
    this->AddCMakePaths();
  }

#ifndef CMAKE_BOOTSTRAP
  this->ProcessPresetVariables();
  this->ProcessPresetEnvironment();
#endif
  // Add any cache args
  if (!this->SetCacheArgs(args)) {
    cmSystemTools::Error("Run 'cmake --help' for all supported options.");
    return -1;
  }
#ifndef CMAKE_BOOTSTRAP
  this->PrintPresetVariables();
  this->PrintPresetEnvironment();
#endif

  // In script mode we terminate after running the script.
  if (this->GetWorkingMode() != NORMAL_MODE) {
    if (cmSystemTools::GetErrorOccuredFlag()) {
      return -1;
    }
    return 0;
  }

  // If MAKEFLAGS are given in the environment, remove the environment
  // variable.  This will prevent try-compile from succeeding when it
  // should fail (if "-i" is an option).  We cannot simply test
  // whether "-i" is given and remove it because some make programs
  // encode the MAKEFLAGS variable in a strange way.
  if (cmSystemTools::HasEnv("MAKEFLAGS")) {
    cmSystemTools::PutEnv("MAKEFLAGS=");
  }

  this->PreLoadCMakeFiles();

  if (noconfigure) {
    return 0;
  }

  // now run the global generate
  // Check the state of the build system to see if we need to regenerate.
  if (!this->CheckBuildSystem()) {
    return 0;
  }

  int ret = this->Configure();
  if (ret) {
#if defined(CMAKE_HAVE_VS_GENERATORS)
    if (!this->VSSolutionFile.empty() && this->GlobalGenerator) {
      // CMake is running to regenerate a Visual Studio build tree
      // during a build from the VS IDE.  The build files cannot be
      // regenerated, so we should stop the build.
      cmSystemTools::Message("CMake Configure step failed.  "
                             "Build files cannot be regenerated correctly.  "
                             "Attempting to stop IDE build.");
      cmGlobalVisualStudioGenerator& gg =
        cm::static_reference_cast<cmGlobalVisualStudioGenerator>(
          this->GlobalGenerator);
      gg.CallVisualStudioMacro(cmGlobalVisualStudioGenerator::MacroStop,
                               this->VSSolutionFile);
    }
#endif
    return ret;
  }
  ret = this->Generate();
  if (ret) {
    cmSystemTools::Message("CMake Generate step failed.  "
                           "Build files cannot be regenerated correctly.");
    return ret;
  }
  std::string message = cmStrCat("Build files have been written to: ",
                                 this->GetHomeOutputDirectory());
  this->UpdateProgress(message, -1);
  return ret;
}

int cmake::Generate()
{
  if (!this->GlobalGenerator) {
    return -1;
  }
  if (!this->GlobalGenerator->Compute()) {
    return -1;
  }
  this->GlobalGenerator->Generate();
  if (!this->GraphVizFile.empty()) {
    std::cout << "Generate graphviz: " << this->GraphVizFile << std::endl;
    this->GenerateGraphViz(this->GraphVizFile);
  }
  if (this->WarnUnusedCli) {
    this->RunCheckForUnusedVariables();
  }
  if (cmSystemTools::GetErrorOccuredFlag()) {
    return -1;
  }
  // Save the cache again after a successful Generate so that any internal
  // variables created during Generate are saved. (Specifically target GUIDs
  // for the Visual Studio and Xcode generators.)
  this->SaveCache(this->GetHomeOutputDirectory());

#if !defined(CMAKE_BOOTSTRAP)
  this->FileAPI->WriteReplies();
#endif

  return 0;
}

void cmake::AddCacheEntry(const std::string& key, const char* value,
                          const char* helpString, int type)
{
  this->State->AddCacheEntry(key, value, helpString,
                             cmStateEnums::CacheEntryType(type));
  this->UnwatchUnusedCli(key);

  if (key == "CMAKE_WARN_DEPRECATED"_s) {
    this->Messenger->SetSuppressDeprecatedWarnings(value && cmIsOff(value));
  } else if (key == "CMAKE_ERROR_DEPRECATED"_s) {
    this->Messenger->SetDeprecatedWarningsAsErrors(cmIsOn(value));
  } else if (key == "CMAKE_SUPPRESS_DEVELOPER_WARNINGS"_s) {
    this->Messenger->SetSuppressDevWarnings(cmIsOn(value));
  } else if (key == "CMAKE_SUPPRESS_DEVELOPER_ERRORS"_s) {
    this->Messenger->SetDevWarningsAsErrors(value && cmIsOff(value));
  }
}

bool cmake::DoWriteGlobVerifyTarget() const
{
  return this->State->DoWriteGlobVerifyTarget();
}

std::string const& cmake::GetGlobVerifyScript() const
{
  return this->State->GetGlobVerifyScript();
}

std::string const& cmake::GetGlobVerifyStamp() const
{
  return this->State->GetGlobVerifyStamp();
}

void cmake::AddGlobCacheEntry(bool recurse, bool listDirectories,
                              bool followSymlinks, const std::string& relative,
                              const std::string& expression,
                              const std::vector<std::string>& files,
                              const std::string& variable,
                              cmListFileBacktrace const& backtrace)
{
  this->State->AddGlobCacheEntry(recurse, listDirectories, followSymlinks,
                                 relative, expression, files, variable,
                                 backtrace);
}

std::vector<std::string> cmake::GetAllExtensions() const
{
  std::vector<std::string> allExt = this->CLikeSourceFileExtensions.ordered;
  allExt.insert(allExt.end(), this->HeaderFileExtensions.ordered.begin(),
                this->HeaderFileExtensions.ordered.end());
  // cuda extensions are also in SourceFileExtensions so we ignore it here
  allExt.insert(allExt.end(), this->FortranFileExtensions.ordered.begin(),
                this->FortranFileExtensions.ordered.end());
  allExt.insert(allExt.end(), this->ISPCFileExtensions.ordered.begin(),
                this->ISPCFileExtensions.ordered.end());
  return allExt;
}

std::string cmake::StripExtension(const std::string& file) const
{
  auto dotpos = file.rfind('.');
  if (dotpos != std::string::npos) {
#if defined(_WIN32) || defined(__APPLE__)
    auto ext = cmSystemTools::LowerCase(file.substr(dotpos + 1));
#else
    auto ext = cm::string_view(file).substr(dotpos + 1);
#endif
    if (this->IsAKnownExtension(ext)) {
      return file.substr(0, dotpos);
    }
  }
  return file;
}

cmProp cmake::GetCacheDefinition(const std::string& name) const
{
  return this->State->GetInitializedCacheValue(name);
}

void cmake::AddScriptingCommands() const
{
  GetScriptingCommands(this->GetState());
}

void cmake::AddProjectCommands() const
{
  GetProjectCommands(this->GetState());
}

void cmake::AddDefaultGenerators()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(CMAKE_BOOT_MINGW)
  this->Generators.push_back(
    cmGlobalVisualStudioVersionedGenerator::NewFactory16());
  this->Generators.push_back(
    cmGlobalVisualStudioVersionedGenerator::NewFactory15());
  this->Generators.push_back(cmGlobalVisualStudio14Generator::NewFactory());
  this->Generators.push_back(cmGlobalVisualStudio12Generator::NewFactory());
  this->Generators.push_back(cmGlobalVisualStudio11Generator::NewFactory());
  this->Generators.push_back(cmGlobalVisualStudio10Generator::NewFactory());
  this->Generators.push_back(cmGlobalVisualStudio9Generator::NewFactory());
  this->Generators.push_back(cmGlobalBorlandMakefileGenerator::NewFactory());
  this->Generators.push_back(cmGlobalNMakeMakefileGenerator::NewFactory());
  this->Generators.push_back(cmGlobalJOMMakefileGenerator::NewFactory());
#  endif
  this->Generators.push_back(cmGlobalMSYSMakefileGenerator::NewFactory());
  this->Generators.push_back(cmGlobalMinGWMakefileGenerator::NewFactory());
#endif
#if !defined(CMAKE_BOOTSTRAP)
#  if defined(__linux__) || defined(_WIN32)
  this->Generators.push_back(cmGlobalGhsMultiGenerator::NewFactory());
#  endif
  this->Generators.push_back(cmGlobalUnixMakefileGenerator3::NewFactory());
  this->Generators.push_back(cmGlobalNinjaGenerator::NewFactory());
  this->Generators.push_back(cmGlobalNinjaMultiGenerator::NewFactory());
#elif defined(CMAKE_BOOTSTRAP_NINJA)
  this->Generators.push_back(cmGlobalNinjaGenerator::NewFactory());
#elif defined(CMAKE_BOOTSTRAP_MAKEFILES)
  this->Generators.push_back(cmGlobalUnixMakefileGenerator3::NewFactory());
#endif
#if defined(CMAKE_USE_WMAKE)
  this->Generators.push_back(cmGlobalWatcomWMakeGenerator::NewFactory());
#endif
#ifdef CMAKE_USE_XCODE
  this->Generators.push_back(cmGlobalXCodeGenerator::NewFactory());
#endif
}

bool cmake::ParseCacheEntry(const std::string& entry, std::string& var,
                            std::string& value,
                            cmStateEnums::CacheEntryType& type)
{
  return cmState::ParseCacheEntry(entry, var, value, type);
}

int cmake::LoadCache()
{
  // could we not read the cache
  if (!this->LoadCache(this->GetHomeOutputDirectory())) {
    // if it does exist, but isn't readable then warn the user
    std::string cacheFile =
      cmStrCat(this->GetHomeOutputDirectory(), "/CMakeCache.txt");
    if (cmSystemTools::FileExists(cacheFile)) {
      cmSystemTools::Error(
        "There is a CMakeCache.txt file for the current binary tree but "
        "cmake does not have permission to read it. Please check the "
        "permissions of the directory you are trying to run CMake on.");
      return -1;
    }
  }

  // setup CMAKE_ROOT and CMAKE_COMMAND
  if (!this->AddCMakePaths()) {
    return -3;
  }
  return 0;
}

bool cmake::LoadCache(const std::string& path)
{
  std::set<std::string> emptySet;
  return this->LoadCache(path, true, emptySet, emptySet);
}

bool cmake::LoadCache(const std::string& path, bool internal,
                      std::set<std::string>& excludes,
                      std::set<std::string>& includes)
{
  bool result = this->State->LoadCache(path, internal, excludes, includes);
  static const auto entries = { "CMAKE_CACHE_MAJOR_VERSION",
                                "CMAKE_CACHE_MINOR_VERSION" };
  for (auto const& entry : entries) {
    this->UnwatchUnusedCli(entry);
  }
  return result;
}

bool cmake::SaveCache(const std::string& path)
{
  bool result = this->State->SaveCache(path, this->GetMessenger());
  static const auto entries = { "CMAKE_CACHE_MAJOR_VERSION",
                                "CMAKE_CACHE_MINOR_VERSION",
                                "CMAKE_CACHE_PATCH_VERSION",
                                "CMAKE_CACHEFILE_DIR" };
  for (auto const& entry : entries) {
    this->UnwatchUnusedCli(entry);
  }
  return result;
}

bool cmake::DeleteCache(const std::string& path)
{
  return this->State->DeleteCache(path);
}

void cmake::SetProgressCallback(ProgressCallbackType f)
{
  this->ProgressCallback = std::move(f);
}

void cmake::UpdateProgress(const std::string& msg, float prog)
{
  if (this->ProgressCallback && !this->State->GetIsInTryCompile()) {
    this->ProgressCallback(msg, prog);
  }
}

bool cmake::GetIsInTryCompile() const
{
  return this->State->GetIsInTryCompile();
}

void cmake::SetIsInTryCompile(bool b)
{
  this->State->SetIsInTryCompile(b);
}

void cmake::AppendGlobalGeneratorsDocumentation(
  std::vector<cmDocumentationEntry>& v)
{
  const auto defaultGenerator = this->EvaluateDefaultGlobalGenerator();
  const std::string defaultName = defaultGenerator->GetName();
  bool foundDefaultOne = false;

  for (const auto& g : this->Generators) {
    cmDocumentationEntry e;
    g->GetDocumentation(e);
    if (!foundDefaultOne && cmHasPrefix(e.Name, defaultName)) {
      e.CustomNamePrefix = '*';
      foundDefaultOne = true;
    }
    v.push_back(std::move(e));
  }
}

void cmake::AppendExtraGeneratorsDocumentation(
  std::vector<cmDocumentationEntry>& v)
{
  for (cmExternalMakefileProjectGeneratorFactory* eg : this->ExtraGenerators) {
    const std::string doc = eg->GetDocumentation();
    const std::string name = eg->GetName();

    // Aliases:
    for (std::string const& a : eg->Aliases) {
      cmDocumentationEntry e;
      e.Name = a;
      e.Brief = doc;
      v.push_back(std::move(e));
    }

    // Full names:
    const std::vector<std::string> generators =
      eg->GetSupportedGlobalGenerators();
    for (std::string const& g : generators) {
      cmDocumentationEntry e;
      e.Name =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(g, name);
      e.Brief = doc;
      v.push_back(std::move(e));
    }
  }
}

std::vector<cmDocumentationEntry> cmake::GetGeneratorsDocumentation()
{
  std::vector<cmDocumentationEntry> v;
  this->AppendGlobalGeneratorsDocumentation(v);
  this->AppendExtraGeneratorsDocumentation(v);
  return v;
}

void cmake::PrintGeneratorList()
{
#ifndef CMAKE_BOOTSTRAP
  cmDocumentation doc;
  auto generators = this->GetGeneratorsDocumentation();
  doc.AppendSection("Generators", generators);
  std::cerr << "\n";
  doc.PrintDocumentation(cmDocumentation::ListGenerators, std::cerr);
#endif
}

void cmake::UpdateConversionPathTable()
{
  // Update the path conversion table with any specified file:
  cmProp tablepath =
    this->State->GetInitializedCacheValue("CMAKE_PATH_TRANSLATION_FILE");

  if (tablepath) {
    cmsys::ifstream table(tablepath->c_str());
    if (!table) {
      cmSystemTools::Error("CMAKE_PATH_TRANSLATION_FILE set to " + *tablepath +
                           ". CMake can not open file.");
      cmSystemTools::ReportLastSystemError("CMake can not open file.");
    } else {
      std::string a;
      std::string b;
      while (!table.eof()) {
        // two entries per line
        table >> a;
        table >> b;
        cmSystemTools::AddTranslationPath(a, b);
      }
    }
  }
}

int cmake::CheckBuildSystem()
{
  // We do not need to rerun CMake.  Check dependency integrity.
  const bool verbose = isCMakeVerbose();

  // This method will check the integrity of the build system if the
  // option was given on the command line.  It reads the given file to
  // determine whether CMake should rerun.

  // If no file is provided for the check, we have to rerun.
  if (this->CheckBuildSystemArgument.empty()) {
    if (verbose) {
      cmSystemTools::Stdout("Re-run cmake no build system arguments\n");
    }
    return 1;
  }

  // If the file provided does not exist, we have to rerun.
  if (!cmSystemTools::FileExists(this->CheckBuildSystemArgument)) {
    if (verbose) {
      std::ostringstream msg;
      msg << "Re-run cmake missing file: " << this->CheckBuildSystemArgument
          << "\n";
      cmSystemTools::Stdout(msg.str());
    }
    return 1;
  }

  // Read the rerun check file and use it to decide whether to do the
  // global generate.
  // Actually, all we need is the `set` command.
  cmake cm(RoleScript, cmState::Unknown);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);
  cmMakefile mf(&gg, cm.GetCurrentSnapshot());
  if (!mf.ReadListFile(this->CheckBuildSystemArgument) ||
      cmSystemTools::GetErrorOccuredFlag()) {
    if (verbose) {
      std::ostringstream msg;
      msg << "Re-run cmake error reading : " << this->CheckBuildSystemArgument
          << "\n";
      cmSystemTools::Stdout(msg.str());
    }
    // There was an error reading the file.  Just rerun.
    return 1;
  }

  if (this->ClearBuildSystem) {
    // Get the generator used for this build system.
    std::string genName = mf.GetSafeDefinition("CMAKE_DEPENDS_GENERATOR");
    if (!cmNonempty(genName)) {
      genName = "Unix Makefiles";
    }

    // Create the generator and use it to clear the dependencies.
    std::unique_ptr<cmGlobalGenerator> ggd =
      this->CreateGlobalGenerator(genName);
    if (ggd) {
      cm.GetCurrentSnapshot().SetDefaultDefinitions();
      cmMakefile mfd(ggd.get(), cm.GetCurrentSnapshot());
      auto lgd = ggd->CreateLocalGenerator(&mfd);
      lgd->ClearDependencies(&mfd, verbose);
    }
  }

  // If any byproduct of makefile generation is missing we must re-run.
  std::vector<std::string> products;
  mf.GetDefExpandList("CMAKE_MAKEFILE_PRODUCTS", products);
  for (std::string const& p : products) {
    if (!(cmSystemTools::FileExists(p) || cmSystemTools::FileIsSymlink(p))) {
      if (verbose) {
        std::ostringstream msg;
        msg << "Re-run cmake, missing byproduct: " << p << "\n";
        cmSystemTools::Stdout(msg.str());
      }
      return 1;
    }
  }

  // Get the set of dependencies and outputs.
  std::vector<std::string> depends;
  std::vector<std::string> outputs;
  if (mf.GetDefExpandList("CMAKE_MAKEFILE_DEPENDS", depends)) {
    mf.GetDefExpandList("CMAKE_MAKEFILE_OUTPUTS", outputs);
  }
  if (depends.empty() || outputs.empty()) {
    // Not enough information was provided to do the test.  Just rerun.
    if (verbose) {
      cmSystemTools::Stdout("Re-run cmake no CMAKE_MAKEFILE_DEPENDS "
                            "or CMAKE_MAKEFILE_OUTPUTS :\n");
    }
    return 1;
  }

  // Find the newest dependency.
  auto dep = depends.begin();
  std::string dep_newest = *dep++;
  for (; dep != depends.end(); ++dep) {
    int result = 0;
    if (this->FileTimeCache->Compare(dep_newest, *dep, &result)) {
      if (result < 0) {
        dep_newest = *dep;
      }
    } else {
      if (verbose) {
        cmSystemTools::Stdout(
          "Re-run cmake: build system dependency is missing\n");
      }
      return 1;
    }
  }

  // Find the oldest output.
  auto out = outputs.begin();
  std::string out_oldest = *out++;
  for (; out != outputs.end(); ++out) {
    int result = 0;
    if (this->FileTimeCache->Compare(out_oldest, *out, &result)) {
      if (result > 0) {
        out_oldest = *out;
      }
    } else {
      if (verbose) {
        cmSystemTools::Stdout(
          "Re-run cmake: build system output is missing\n");
      }
      return 1;
    }
  }

  // If any output is older than any dependency then rerun.
  {
    int result = 0;
    if (!this->FileTimeCache->Compare(out_oldest, dep_newest, &result) ||
        result < 0) {
      if (verbose) {
        std::ostringstream msg;
        msg << "Re-run cmake file: " << out_oldest
            << " older than: " << dep_newest << "\n";
        cmSystemTools::Stdout(msg.str());
      }
      return 1;
    }
  }

  // No need to rerun.
  return 0;
}

void cmake::TruncateOutputLog(const char* fname)
{
  std::string fullPath = cmStrCat(this->GetHomeOutputDirectory(), '/', fname);
  struct stat st;
  if (::stat(fullPath.c_str(), &st)) {
    return;
  }
  if (!this->State->GetInitializedCacheValue("CMAKE_CACHEFILE_DIR")) {
    cmSystemTools::RemoveFile(fullPath);
    return;
  }
  off_t fsize = st.st_size;
  const off_t maxFileSize = 50 * 1024;
  if (fsize < maxFileSize) {
    // TODO: truncate file
    return;
  }
}

void cmake::MarkCliAsUsed(const std::string& variable)
{
  this->UsedCliVariables[variable] = true;
}

void cmake::GenerateGraphViz(const std::string& fileName) const
{
#ifndef CMAKE_BOOTSTRAP
  cmGraphVizWriter gvWriter(fileName, this->GetGlobalGenerator());

  std::string settingsFile =
    cmStrCat(this->GetHomeOutputDirectory(), "/CMakeGraphVizOptions.cmake");
  std::string fallbackSettingsFile =
    cmStrCat(this->GetHomeDirectory(), "/CMakeGraphVizOptions.cmake");

  gvWriter.ReadSettings(settingsFile, fallbackSettingsFile);

  gvWriter.Write();

#endif
}

void cmake::SetProperty(const std::string& prop, const char* value)
{
  this->State->SetGlobalProperty(prop, value);
}

void cmake::AppendProperty(const std::string& prop, const std::string& value,
                           bool asString)
{
  this->State->AppendGlobalProperty(prop, value, asString);
}

cmProp cmake::GetProperty(const std::string& prop)
{
  return this->State->GetGlobalProperty(prop);
}

bool cmake::GetPropertyAsBool(const std::string& prop)
{
  return this->State->GetGlobalPropertyAsBool(prop);
}

cmInstalledFile* cmake::GetOrCreateInstalledFile(cmMakefile* mf,
                                                 const std::string& name)
{
  auto i = this->InstalledFiles.find(name);

  if (i != this->InstalledFiles.end()) {
    cmInstalledFile& file = i->second;
    return &file;
  }
  cmInstalledFile& file = this->InstalledFiles[name];
  file.SetName(mf, name);
  return &file;
}

cmInstalledFile const* cmake::GetInstalledFile(const std::string& name) const
{
  auto i = this->InstalledFiles.find(name);

  if (i != this->InstalledFiles.end()) {
    cmInstalledFile const& file = i->second;
    return &file;
  }
  return nullptr;
}

int cmake::GetSystemInformation(std::vector<std::string>& args)
{
  // so create the directory
  std::string resultFile;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  std::string destPath = cwd + "/__cmake_systeminformation";
  cmSystemTools::RemoveADirectory(destPath);
  if (!cmSystemTools::MakeDirectory(destPath)) {
    std::cerr << "Error: --system-information must be run from a "
                 "writable directory!\n";
    return 1;
  }

  // process the arguments
  bool writeToStdout = true;
  for (unsigned int i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];
    if (cmHasLiteralPrefix(arg, "-G")) {
      std::string value = arg.substr(2);
      if (value.empty()) {
        ++i;
        if (i >= args.size()) {
          cmSystemTools::Error("No generator specified for -G");
          this->PrintGeneratorList();
          return -1;
        }
        value = args[i];
      }
      auto gen = this->CreateGlobalGenerator(value);
      if (!gen) {
        cmSystemTools::Error("Could not create named generator " + value);
        this->PrintGeneratorList();
      } else {
        this->SetGlobalGenerator(std::move(gen));
      }
    }
    // no option assume it is the output file
    else {
      if (!cmSystemTools::FileIsFullPath(arg)) {
        resultFile = cmStrCat(cwd, '/');
      }
      resultFile += arg;
      writeToStdout = false;
    }
  }

  // we have to find the module directory, so we can copy the files
  this->AddCMakePaths();
  std::string modulesPath =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Modules");
  std::string inFile = cmStrCat(modulesPath, "/SystemInformation.cmake");
  std::string outFile = cmStrCat(destPath, "/CMakeLists.txt");

  // Copy file
  if (!cmsys::SystemTools::CopyFileAlways(inFile, outFile)) {
    std::cerr << "Error copying file \"" << inFile << "\" to \"" << outFile
              << "\".\n";
    return 1;
  }

  // do we write to a file or to stdout?
  if (resultFile.empty()) {
    resultFile = cmStrCat(cwd, "/__cmake_systeminformation/results.txt");
  }

  {
    // now run cmake on the CMakeLists file
    cmWorkingDirectory workdir(destPath);
    if (workdir.Failed()) {
      // We created the directory and we were able to copy the CMakeLists.txt
      // file to it, so we wouldn't expect to get here unless the default
      // permissions are questionable or some other process has deleted the
      // directory
      std::cerr << "Failed to change to directory " << destPath << " : "
                << std::strerror(workdir.GetLastResult()) << std::endl;
      return 1;
    }
    std::vector<std::string> args2;
    args2.push_back(args[0]);
    args2.push_back(destPath);
    args2.push_back("-DRESULT_FILE=" + resultFile);
    int res = this->Run(args2, false);

    if (res != 0) {
      std::cerr << "Error: --system-information failed on internal CMake!\n";
      return res;
    }
  }

  // echo results to stdout if needed
  if (writeToStdout) {
    FILE* fin = cmsys::SystemTools::Fopen(resultFile, "r");
    if (fin) {
      const int bufferSize = 4096;
      char buffer[bufferSize];
      size_t n;
      while ((n = fread(buffer, 1, bufferSize, fin)) > 0) {
        for (char* c = buffer; c < buffer + n; ++c) {
          putc(*c, stdout);
        }
        fflush(stdout);
      }
      fclose(fin);
    }
  }

  // clean up the directory
  cmSystemTools::RemoveADirectory(destPath);
  return 0;
}

static bool cmakeCheckStampFile(const std::string& stampName)
{
  // The stamp file does not exist.  Use the stamp dependencies to
  // determine whether it is really out of date.  This works in
  // conjunction with cmLocalVisualStudio7Generator to avoid
  // repeatedly re-running CMake when the user rebuilds the entire
  // solution.
  std::string stampDepends = cmStrCat(stampName, ".depend");
#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream fin(stampDepends.c_str(), std::ios::in | std::ios::binary);
#else
  cmsys::ifstream fin(stampDepends.c_str());
#endif
  if (!fin) {
    // The stamp dependencies file cannot be read.  Just assume the
    // build system is really out of date.
    std::cout << "CMake is re-running because " << stampName
              << " dependency file is missing.\n";
    return false;
  }

  // Compare the stamp dependencies against the dependency file itself.
  {
    cmFileTimeCache ftc;
    std::string dep;
    while (cmSystemTools::GetLineFromStream(fin, dep)) {
      int result;
      if (!dep.empty() && dep[0] != '#' &&
          (!ftc.Compare(stampDepends, dep, &result) || result < 0)) {
        // The stamp depends file is older than this dependency.  The
        // build system is really out of date.
        std::cout << "CMake is re-running because " << stampName
                  << " is out-of-date.\n";
        std::cout << "  the file '" << dep << "'\n";
        std::cout << "  is newer than '" << stampDepends << "'\n";
        std::cout << "  result='" << result << "'\n";
        return false;
      }
    }
  }

  // The build system is up to date.  The stamp file has been removed
  // by the VS IDE due to a "rebuild" request.  Restore it atomically.
  std::ostringstream stampTempStream;
  stampTempStream << stampName << ".tmp" << cmSystemTools::RandomSeed();
  std::string stampTemp = stampTempStream.str();
  {
    // TODO: Teach cmGeneratedFileStream to use a random temp file (with
    // multiple tries in unlikely case of conflict) and use that here.
    cmsys::ofstream stamp(stampTemp.c_str());
    stamp << "# CMake generation timestamp file for this directory.\n";
  }
  if (cmSystemTools::RenameFile(stampTemp, stampName)) {
    // CMake does not need to re-run because the stamp file is up-to-date.
    return true;
  }
  cmSystemTools::RemoveFile(stampTemp);
  cmSystemTools::Error("Cannot restore timestamp " + stampName);
  return false;
}

static bool cmakeCheckStampList(const std::string& stampList)
{
  // If the stamp list does not exist CMake must rerun to generate it.
  if (!cmSystemTools::FileExists(stampList)) {
    std::cout << "CMake is re-running because generate.stamp.list "
              << "is missing.\n";
    return false;
  }
  cmsys::ifstream fin(stampList.c_str());
  if (!fin) {
    std::cout << "CMake is re-running because generate.stamp.list "
              << "could not be read.\n";
    return false;
  }

  // Check each stamp.
  std::string stampName;
  while (cmSystemTools::GetLineFromStream(fin, stampName)) {
    if (!cmakeCheckStampFile(stampName)) {
      return false;
    }
  }
  return true;
}

void cmake::IssueMessage(MessageType t, std::string const& text,
                         cmListFileBacktrace const& backtrace) const
{
  this->Messenger->IssueMessage(t, text, backtrace);
}

std::vector<std::string> cmake::GetDebugConfigs()
{
  std::vector<std::string> configs;
  if (cmProp config_list =
        this->State->GetGlobalProperty("DEBUG_CONFIGURATIONS")) {
    // Expand the specified list and convert to upper-case.
    cmExpandList(*config_list, configs);
    std::transform(configs.begin(), configs.end(), configs.begin(),
                   cmSystemTools::UpperCase);
  }
  // If no configurations were specified, use a default list.
  if (configs.empty()) {
    configs.emplace_back("DEBUG");
  }
  return configs;
}

int cmake::Build(int jobs, std::string dir, std::vector<std::string> targets,
                 std::string config, std::vector<std::string> nativeOptions,
                 bool clean, bool verbose, const std::string& presetName,
                 bool listPresets)
{
  this->SetHomeDirectory("");
  this->SetHomeOutputDirectory("");

#if !defined(CMAKE_BOOTSTRAP)
  if (!presetName.empty() || listPresets) {
    this->SetHomeDirectory(cmSystemTools::GetCurrentWorkingDirectory());
    this->SetHomeOutputDirectory(cmSystemTools::GetCurrentWorkingDirectory());

    cmCMakePresetsFile settingsFile;
    auto result = settingsFile.ReadProjectPresets(this->GetHomeDirectory());
    if (result != cmCMakePresetsFile::ReadFileResult::READ_OK) {
      cmSystemTools::Error(
        cmStrCat("Could not read presets from ", this->GetHomeDirectory(),
                 ": ", cmCMakePresetsFile::ResultToString(result)));
      return 1;
    }

    if (listPresets) {
      settingsFile.PrintBuildPresetList();
      return 0;
    }

    auto presetPair = settingsFile.BuildPresets.find(presetName);
    if (presetPair == settingsFile.BuildPresets.end()) {
      cmSystemTools::Error(cmStrCat("No such build preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    presetName, '"'));
      settingsFile.PrintBuildPresetList();
      return 1;
    }

    if (presetPair->second.Unexpanded.Hidden) {
      cmSystemTools::Error(cmStrCat("Cannot use hidden build preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    presetName, '"'));
      settingsFile.PrintBuildPresetList();
      return 1;
    }

    auto const& expandedPreset = presetPair->second.Expanded;
    if (!expandedPreset) {
      cmSystemTools::Error(cmStrCat("Could not evaluate build preset \"",
                                    presetName,
                                    "\": Invalid macro expansion"));
      settingsFile.PrintBuildPresetList();
      return 1;
    }

    auto configurePresetPair =
      settingsFile.ConfigurePresets.find(expandedPreset->ConfigurePreset);
    if (configurePresetPair == settingsFile.ConfigurePresets.end()) {
      cmSystemTools::Error(cmStrCat("No such configure preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    expandedPreset->ConfigurePreset, '"'));
      this->PrintPresetList(settingsFile);
      return 1;
    }

    if (configurePresetPair->second.Unexpanded.Hidden) {
      cmSystemTools::Error(cmStrCat("Cannot use hidden configure preset in ",
                                    this->GetHomeDirectory(), ": \"",
                                    expandedPreset->ConfigurePreset, '"'));
      this->PrintPresetList(settingsFile);
      return 1;
    }

    auto const& expandedConfigurePreset = configurePresetPair->second.Expanded;
    if (!expandedConfigurePreset) {
      cmSystemTools::Error(cmStrCat("Could not evaluate configure preset \"",
                                    expandedPreset->ConfigurePreset,
                                    "\": Invalid macro expansion"));
      return 1;
    }

    dir = expandedConfigurePreset->BinaryDir;

    this->UnprocessedPresetEnvironment = expandedPreset->Environment;
    this->ProcessPresetEnvironment();

    if (jobs == cmake::DEFAULT_BUILD_PARALLEL_LEVEL && expandedPreset->Jobs) {
      jobs = *expandedPreset->Jobs;
    }

    if (targets.empty()) {
      targets.insert(targets.begin(), expandedPreset->Targets.begin(),
                     expandedPreset->Targets.end());
    }

    if (config.empty()) {
      config = expandedPreset->Configuration;
    }

    if (!clean && expandedPreset->CleanFirst) {
      clean = *expandedPreset->CleanFirst;
    }

    if (!verbose && expandedPreset->Verbose) {
      verbose = *expandedPreset->Verbose;
    }

    if (nativeOptions.empty()) {
      nativeOptions.insert(nativeOptions.begin(),
                           expandedPreset->NativeToolOptions.begin(),
                           expandedPreset->NativeToolOptions.end());
    }
  }
#endif

  if (!cmSystemTools::FileIsDirectory(dir)) {
    std::cerr << "Error: " << dir << " is not a directory\n";
    return 1;
  }

  std::string cachePath = FindCacheFile(dir);
  if (!this->LoadCache(cachePath)) {
    std::cerr << "Error: could not load cache\n";
    return 1;
  }
  cmProp cachedGenerator = this->State->GetCacheEntryValue("CMAKE_GENERATOR");
  if (!cachedGenerator) {
    std::cerr << "Error: could not find CMAKE_GENERATOR in Cache\n";
    return 1;
  }
  auto gen = this->CreateGlobalGenerator(*cachedGenerator);
  if (!gen) {
    std::cerr << "Error: could create CMAKE_GENERATOR \"" << *cachedGenerator
              << "\"\n";
    return 1;
  }
  this->SetGlobalGenerator(std::move(gen));
  cmProp cachedGeneratorInstance =
    this->State->GetCacheEntryValue("CMAKE_GENERATOR_INSTANCE");
  if (cachedGeneratorInstance) {
    cmMakefile mf(this->GetGlobalGenerator(), this->GetCurrentSnapshot());
    if (!this->GlobalGenerator->SetGeneratorInstance(*cachedGeneratorInstance,
                                                     &mf)) {
      return 1;
    }
  }
  cmProp cachedGeneratorPlatform =
    this->State->GetCacheEntryValue("CMAKE_GENERATOR_PLATFORM");
  if (cachedGeneratorPlatform) {
    cmMakefile mf(this->GetGlobalGenerator(), this->GetCurrentSnapshot());
    if (!this->GlobalGenerator->SetGeneratorPlatform(*cachedGeneratorPlatform,
                                                     &mf)) {
      return 1;
    }
  }
  cmProp cachedGeneratorToolset =
    this->State->GetCacheEntryValue("CMAKE_GENERATOR_TOOLSET");
  if (cachedGeneratorToolset) {
    cmMakefile mf(this->GetGlobalGenerator(), this->GetCurrentSnapshot());
    if (!this->GlobalGenerator->SetGeneratorToolset(*cachedGeneratorToolset,
                                                    true, &mf)) {
      return 1;
    }
  }
  std::string output;
  std::string projName;
  cmProp cachedProjectName =
    this->State->GetCacheEntryValue("CMAKE_PROJECT_NAME");
  if (!cachedProjectName) {
    std::cerr << "Error: could not find CMAKE_PROJECT_NAME in Cache\n";
    return 1;
  }
  projName = *cachedProjectName;

  if (cmIsOn(this->State->GetCacheEntryValue("CMAKE_VERBOSE_MAKEFILE"))) {
    verbose = true;
  }

#ifdef CMAKE_HAVE_VS_GENERATORS
  // For VS generators, explicitly check if regeneration is necessary before
  // actually starting the build. If not done separately from the build
  // itself, there is the risk of building an out-of-date solution file due
  // to limitations of the underlying build system.
  std::string const stampList = cachePath + "/" + "CMakeFiles/" +
    cmGlobalVisualStudio9Generator::GetGenerateStampList();

  // Note that the stampList file only exists for VS generators.
  if (cmSystemTools::FileExists(stampList)) {

    // Check if running for Visual Studio 9 - we need to explicitly run
    // the glob verification script before starting the build
    this->AddScriptingCommands();
    if (this->GlobalGenerator->MatchesGeneratorName("Visual Studio 9 2008")) {
      std::string const globVerifyScript =
        cachePath + "/" + "CMakeFiles/" + "VerifyGlobs.cmake";
      if (cmSystemTools::FileExists(globVerifyScript)) {
        std::vector<std::string> args;
        this->ReadListFile(args, globVerifyScript);
      }
    }

    if (!cmakeCheckStampList(stampList)) {
      // Correctly initialize the home (=source) and home output (=binary)
      // directories, which is required for running the generation step.
      std::string homeOrig = this->GetHomeDirectory();
      std::string homeOutputOrig = this->GetHomeOutputDirectory();
      this->SetDirectoriesFromFile(cachePath);

      this->AddProjectCommands();

      int ret = this->Configure();
      if (ret) {
        cmSystemTools::Message("CMake Configure step failed.  "
                               "Build files cannot be regenerated correctly.");
        return ret;
      }
      ret = this->Generate();
      if (ret) {
        cmSystemTools::Message("CMake Generate step failed.  "
                               "Build files cannot be regenerated correctly.");
        return ret;
      }
      std::string message = cmStrCat("Build files have been written to: ",
                                     this->GetHomeOutputDirectory());
      this->UpdateProgress(message, -1);

      // Restore the previously set directories to their original value.
      this->SetHomeDirectory(homeOrig);
      this->SetHomeOutputDirectory(homeOutputOrig);
    }
  }
#endif

  if (!this->GlobalGenerator->ReadCacheEntriesForBuild(*this->State)) {
    return 1;
  }

  this->GlobalGenerator->PrintBuildCommandAdvice(std::cerr, jobs);
  return this->GlobalGenerator->Build(
    jobs, "", dir, projName, targets, output, "", config, clean, false,
    verbose, cmDuration::zero(), cmSystemTools::OUTPUT_PASSTHROUGH,
    nativeOptions);
}

bool cmake::Open(const std::string& dir, bool dryRun)
{
  this->SetHomeDirectory("");
  this->SetHomeOutputDirectory("");
  if (!cmSystemTools::FileIsDirectory(dir)) {
    std::cerr << "Error: " << dir << " is not a directory\n";
    return false;
  }

  std::string cachePath = FindCacheFile(dir);
  if (!this->LoadCache(cachePath)) {
    std::cerr << "Error: could not load cache\n";
    return false;
  }
  cmProp genName = this->State->GetCacheEntryValue("CMAKE_GENERATOR");
  if (!genName) {
    std::cerr << "Error: could not find CMAKE_GENERATOR in Cache\n";
    return false;
  }
  cmProp extraGenName =
    this->State->GetInitializedCacheValue("CMAKE_EXTRA_GENERATOR");
  std::string fullName =
    cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
      *genName, extraGenName ? *extraGenName : "");

  std::unique_ptr<cmGlobalGenerator> gen =
    this->CreateGlobalGenerator(fullName);
  if (!gen) {
    std::cerr << "Error: could create CMAKE_GENERATOR \"" << fullName
              << "\"\n";
    return false;
  }

  cmProp cachedProjectName =
    this->State->GetCacheEntryValue("CMAKE_PROJECT_NAME");
  if (!cachedProjectName) {
    std::cerr << "Error: could not find CMAKE_PROJECT_NAME in Cache\n";
    return false;
  }

  return gen->Open(dir, *cachedProjectName, dryRun);
}

void cmake::WatchUnusedCli(const std::string& var)
{
#ifndef CMAKE_BOOTSTRAP
  this->VariableWatch->AddWatch(var, cmWarnUnusedCliWarning, this);
  if (!cm::contains(this->UsedCliVariables, var)) {
    this->UsedCliVariables[var] = false;
  }
#endif
}

void cmake::UnwatchUnusedCli(const std::string& var)
{
#ifndef CMAKE_BOOTSTRAP
  this->VariableWatch->RemoveWatch(var, cmWarnUnusedCliWarning);
  this->UsedCliVariables.erase(var);
#endif
}

void cmake::RunCheckForUnusedVariables()
{
#ifndef CMAKE_BOOTSTRAP
  bool haveUnused = false;
  std::ostringstream msg;
  msg << "Manually-specified variables were not used by the project:";
  for (auto const& it : this->UsedCliVariables) {
    if (!it.second) {
      haveUnused = true;
      msg << "\n  " << it.first;
    }
  }
  if (haveUnused) {
    this->IssueMessage(MessageType::WARNING, msg.str());
  }
#endif
}

bool cmake::GetSuppressDevWarnings() const
{
  return this->Messenger->GetSuppressDevWarnings();
}

void cmake::SetSuppressDevWarnings(bool b)
{
  std::string value;

  // equivalent to -Wno-dev
  if (b) {
    value = "TRUE";
  }
  // equivalent to -Wdev
  else {
    value = "FALSE";
  }

  this->AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", value.c_str(),
                      "Suppress Warnings that are meant for"
                      " the author of the CMakeLists.txt files.",
                      cmStateEnums::INTERNAL);
}

bool cmake::GetSuppressDeprecatedWarnings() const
{
  return this->Messenger->GetSuppressDeprecatedWarnings();
}

void cmake::SetSuppressDeprecatedWarnings(bool b)
{
  std::string value;

  // equivalent to -Wno-deprecated
  if (b) {
    value = "FALSE";
  }
  // equivalent to -Wdeprecated
  else {
    value = "TRUE";
  }

  this->AddCacheEntry("CMAKE_WARN_DEPRECATED", value.c_str(),
                      "Whether to issue warnings for deprecated "
                      "functionality.",
                      cmStateEnums::INTERNAL);
}

bool cmake::GetDevWarningsAsErrors() const
{
  return this->Messenger->GetDevWarningsAsErrors();
}

void cmake::SetDevWarningsAsErrors(bool b)
{
  std::string value;

  // equivalent to -Werror=dev
  if (b) {
    value = "FALSE";
  }
  // equivalent to -Wno-error=dev
  else {
    value = "TRUE";
  }

  this->AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_ERRORS", value.c_str(),
                      "Suppress errors that are meant for"
                      " the author of the CMakeLists.txt files.",
                      cmStateEnums::INTERNAL);
}

bool cmake::GetDeprecatedWarningsAsErrors() const
{
  return this->Messenger->GetDeprecatedWarningsAsErrors();
}

void cmake::SetDeprecatedWarningsAsErrors(bool b)
{
  std::string value;

  // equivalent to -Werror=deprecated
  if (b) {
    value = "TRUE";
  }
  // equivalent to -Wno-error=deprecated
  else {
    value = "FALSE";
  }

  this->AddCacheEntry("CMAKE_ERROR_DEPRECATED", value.c_str(),
                      "Whether to issue deprecation errors for macros"
                      " and functions.",
                      cmStateEnums::INTERNAL);
}

#if !defined(CMAKE_BOOTSTRAP)
cmMakefileProfilingData& cmake::GetProfilingOutput()
{
  return *(this->ProfilingOutput);
}

bool cmake::IsProfilingEnabled() const
{
  return static_cast<bool>(this->ProfilingOutput);
}
#endif
