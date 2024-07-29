/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmCMakePkgConfigCommand.h"

#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm/filesystem>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPkgConfigParser.h"
#include "cmPkgConfigResolver.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include <cmllpkgc/llpkgc.h>

// IWYU wants this
namespace {
struct ExtractArguments;
}

namespace {

cm::optional<std::string> GetPkgConfigBin(cmMakefile& mf)
{
  cm::optional<std::string> result;

  auto pkgcfg = mf.GetDefinition("CMAKE_PKG_CONFIG_BIN");
  if (pkgcfg.IsNOTFOUND()) {
    return result;
  }

  if (pkgcfg) {
    result = *pkgcfg;
    return result;
  }

  std::string path = cmSystemTools::FindProgram("pkgconf");
  if (path.empty()) {
    path = cmSystemTools::FindProgram("pkg-config");
    if (path.empty()) {
      mf.AddCacheDefinition("CMAKE_PKG_CONFIG_BIN", "pkg-config-NOTFOUND",
                            "Location of pkg-config or pkgconf binary",
                            cmStateEnums::FILEPATH);
      return result;
    }
  }

  mf.AddCacheDefinition("CMAKE_PKG_CONFIG_BIN", path,
                        "Location of pkg-config or pkgconf binary",
                        cmStateEnums::FILEPATH);

  result = std::move(path);
  return result;
}

std::vector<std::string> GetLocations(cmMakefile& mf, const char* cachevar,
                                      const char* envvar, const char* desc,
                                      const char* pcvar, bool need_pkgconf,
                                      std::vector<std::string> default_locs)
{
  auto def = mf.GetDefinition(cachevar);
  if (def) {
    return cmList(def);
  }

  std::string paths;
  if (cmSystemTools::GetEnv(envvar, paths)) {
    cmPkgConfigResolver::ReplaceSep(paths);
    mf.AddCacheDefinition(cachevar, paths, desc, cmStateEnums::STRING);
    return cmList(paths);
  }

  auto pkgcfg = GetPkgConfigBin(mf);
  if (!pkgcfg || (need_pkgconf && (pkgcfg->find("pkgconf") == pkgcfg->npos))) {
    mf.AddCacheDefinition(cachevar, cmList::to_string(default_locs), desc,
                          cmStateEnums::STRING);
    return default_locs;
  }

  std::string out;
  cmSystemTools::RunSingleCommand({ *pkgcfg, pcvar, "pkg-config" }, &out,
                                  nullptr, nullptr, nullptr,
                                  cmSystemTools::OUTPUT_NONE);

  cmPkgConfigResolver::ReplaceSep(out);
  out = cmTrimWhitespace(out);
  mf.AddCacheDefinition(cachevar, out, desc, cmStateEnums::STRING);
  return cmList(out);
}

std::vector<std::string> GetPcLibDirs(cmMakefile& mf)
{
  std::vector<std::string> default_locs = {
#ifndef _WIN32
    "/usr/lib/pkgconfig", "/usr/share/pkgconfig"
#endif
  };
  return GetLocations(mf, "CMAKE_PKG_CONFIG_PC_LIB_DIRS", "PKG_CONFIG_LIBDIR",
                      "Default search locations for package files",
                      "--variable=pc_path", false, std::move(default_locs));
}

std::vector<std::string> GetSysLibDirs(cmMakefile& mf)
{
  std::vector<std::string> default_locs = {
#ifndef _WIN32
    "/lib", "/usr/lib"
#endif
  };
  return GetLocations(
    mf, "CMAKE_PKG_CONFIG_SYS_LIB_DIRS", "PKG_CONFIG_SYSTEM_LIBRARY_PATH",
    "System library directories filtered by flag mangling",
    "--variable=pc_system_libdirs", true, std::move(default_locs));
}

std::vector<std::string> GetSysCflags(cmMakefile& mf)
{
  std::vector<std::string> default_locs = {
#ifndef _WIN32
    "/usr/include"
#endif
  };
  return GetLocations(
    mf, "CMAKE_PKG_CONFIG_SYS_INCLUDE_DIRS", "PKG_CONFIG_SYSTEM_INCLUDE_PATH",
    "System include directories filtered by flag mangling",
    "--variable=pc_system_includedirs", true, std::move(default_locs));
}

std::vector<std::string> GetPkgConfSysLibs(cmMakefile& mf)
{
  auto def = mf.GetDefinition("CMAKE_PKG_CONFIG_PKGCONF_LIB_DIRS");
  if (def) {
    return cmList(def);
  }

  std::string paths;
  if (!cmSystemTools::GetEnv("LIBRARY_PATH", paths)) {
    return {};
  }

  cmPkgConfigResolver::ReplaceSep(paths);
  mf.AddCacheDefinition("CMAKE_PKG_CONFIG_PKGCONF_LIB_DIRS", paths,
                        "Additional system library directories filtered by "
                        "flag mangling in PKGCONF mode",
                        cmStateEnums::STRING);
  return cmList(paths);
}

std::vector<std::string> GetPkgConfSysCflags(cmMakefile& mf)
{
  auto def = mf.GetDefinition("CMAKE_PKG_CONFIG_PKGCONF_INCLUDES");
  if (def) {
    return cmList(def);
  }

  std::string paths;
  auto get_and_append = [&](const char* var) {
    if (paths.empty()) {
      cmSystemTools::GetEnv(var, paths);
    } else {
      std::string tmp;
      cmSystemTools::GetEnv(var, tmp);
      if (!tmp.empty()) {
        paths += ";" + tmp;
      }
    }
  };

  get_and_append("CPATH");
  get_and_append("C_INCLUDE_PATH");
  get_and_append("CPLUS_INCLUDE_PATH");
  get_and_append("OBJC_INCLUDE_PATH");

#ifdef _WIN32
  get_and_append("INCLUDE");
#endif

  cmPkgConfigResolver::ReplaceSep(paths);
  mf.AddCacheDefinition("CMAKE_PKG_CONFIG_PKGCONF_INCLUDES", paths,
                        "Additional system include directories filtered by "
                        "flag mangling in PKGCONF mode",
                        cmStateEnums::STRING);
  return cmList(paths);
}

std::vector<std::string> GetPcPath(cmMakefile& mf)
{
  auto def = mf.GetDefinition("CMAKE_PKG_CONFIG_PC_PATH");
  if (def) {
    return cmList(def);
  }

  std::string pcpath;
  if (cmSystemTools::GetEnv("PKG_CONFIG_PATH", pcpath)) {
    auto result = cmSystemTools::SplitString(pcpath, cmPkgConfigResolver::Sep);
    mf.AddCacheDefinition(
      "CMAKE_PKG_CONFIG_PC_PATH", cmList::to_string(result),
      "Additional search locations for package files", cmStateEnums::STRING);
    return result;
  }

  mf.AddCacheDefinition("CMAKE_PKG_CONFIG_PC_PATH", "",
                        "Additional search locations for package files",
                        cmStateEnums::STRING);
  return {};
}

cm::optional<std::string> GetPath(cmMakefile& mf, const char* cachevar,
                                  const char* envvar, const char* desc)
{
  cm::optional<std::string> result;

  auto def = mf.GetDefinition(cachevar);
  if (def) {
    result = *def;
    return result;
  }

  std::string path;
  if (cmSystemTools::GetEnv(envvar, path)) {
    mf.AddCacheDefinition(cachevar, path, desc, cmStateEnums::FILEPATH);
    result = std::move(path);
    return result;
  }

  return result;
}

cm::optional<std::string> GetSysrootDir(cmMakefile& mf)
{
  return GetPath(mf, "CMAKE_PKG_CONFIG_SYSROOT_DIR", "PKG_CONFIG_SYSROOT_DIR",
                 "System root used for re-rooting package includes and "
                 "library directories");
}

cm::optional<std::string> GetTopBuildDir(cmMakefile& mf)
{
  return GetPath(mf, "CMAKE_PKG_CONFIG_TOP_BUILD_DIR",
                 "PKG_CONFIG_TOP_BUILD_DIR",
                 "Package file top_build_dir variable default value");
}

bool GetBool(cmMakefile& mf, const char* cachevar, const char* envvar,
             const char* desc)
{
  auto def = mf.GetDefinition(cachevar);
  if (def) {
    return def.IsOn();
  }

  if (cmSystemTools::HasEnv(envvar)) {
    mf.AddCacheDefinition(cachevar, "ON", desc, cmStateEnums::BOOL);
    return true;
  }

  return false;
}

bool GetDisableUninstalled(cmMakefile& mf)
{
  return GetBool(mf, "CMAKE_PKG_CONFIG_DISABLE_UNINSTALLED",
                 "PKG_CONFIG_DISABLE_UNINSTALLED",
                 "Disable search for `-uninstalled` (build tree) packages");
}

bool GetAllowSysLibs(cmMakefile& mf)
{
  return GetBool(mf, "CMAKE_PKG_CONFIG_ALLOW_SYS_LIBS",
                 "PKG_CONFIG_ALLOW_SYSTEM_LIBS",
                 "Allow system library directories during flag mangling");
}

bool GetAllowSysInclude(cmMakefile& mf)
{
  return GetBool(mf, "CMAKE_PKG_CONFIG_ALLOW_SYS_INCLUDES",
                 "PKG_CONFIG_ALLOW_SYSTEM_CFLAGS",
                 "Allow system include paths during flag manglging");
}

struct CommonArguments : ArgumentParser::ParseResult
{
  bool Required = false;
  bool Exact = false;
  bool Quiet = false;

  enum StrictnessType
  {
    STRICTNESS_STRICT,
    STRICTNESS_PERMISSIVE,
    STRICTNESS_BEST_EFFORT,
  };

  StrictnessType Strictness = STRICTNESS_PERMISSIVE;
  std::string StrictnessError;

  ArgumentParser::Continue SetStrictness(cm::string_view strictness)
  {
    if (strictness == "STRICT"_s) {
      Strictness = STRICTNESS_STRICT;
    } else if (strictness == "PERMISSIVE"_s) {
      Strictness = STRICTNESS_PERMISSIVE;
    } else if (strictness == "BEST_EFFORT"_s) {
      Strictness = STRICTNESS_BEST_EFFORT;
    } else {
      StrictnessError =
        cmStrCat("Invalid 'STRICTNESS' '", strictness,
                 "'; must be one of 'STRICT', 'PERMISSIVE', or 'BEST_EFFORT'");
    }
    return ArgumentParser::Continue::Yes;
  }

  enum EnvModeType
  {
    ENVMODE_FDO,
    ENVMODE_PKGCONF,
    ENVMODE_IGNORE,
  };

  EnvModeType EnvMode = ENVMODE_PKGCONF;
  std::string EnvModeError;

  ArgumentParser::Continue SetEnvMode(cm::string_view envMode)
  {
    if (envMode == "FDO"_s) {
      EnvMode = ENVMODE_FDO;
    } else if (envMode == "PKGCONF"_s) {
      EnvMode = ENVMODE_PKGCONF;
    } else if (envMode == "IGNORE"_s) {
      EnvMode = ENVMODE_IGNORE;
    } else {
      EnvModeError =
        cmStrCat("Invalid 'ENV_MODE' '", envMode,
                 "'; must be one of 'FDO', 'PKGCONF', or 'IGNORE'");
    }
    return ArgumentParser::Continue::Yes;
  }

  cm::optional<std::string> Package;
  cm::optional<std::string> Version;
  cm::optional<std::string> SysrootDir;
  cm::optional<std::string> TopBuildDir;

  cm::optional<bool> DisableUninstalled;

  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> PcPath;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> PcLibdir;

  bool CheckArgs(cmExecutionStatus& status) const
  {

    if (!Package) {
      status.SetError("A package name or absolute path must be specified");
      return false;
    }

    if (!StrictnessError.empty()) {
      status.SetError(StrictnessError);
      return false;
    }

    if (!EnvModeError.empty()) {
      status.SetError(EnvModeError);
      return false;
    }

    return true;
  }
};

#define BIND_COMMON(argtype)                                                  \
  (cmArgumentParser<argtype>{})                                               \
    .Bind(1, &argtype::Package)                                               \
    .Bind(2, &argtype::Version)                                               \
    .Bind("REQUIRED"_s, &argtype::Required)                                   \
    .Bind("EXACT"_s, &argtype::Exact)                                         \
    .Bind("QUIET"_s, &argtype::Quiet)                                         \
    .Bind("STRICTNESS"_s, &argtype::SetStrictness)                            \
    .Bind("ENV_MODE"_s, &argtype::SetEnvMode)                                 \
    .Bind("PC_SYSROOT_DIR"_s, &argtype::SysrootDir)                           \
    .Bind("TOP_BUILD_DIR"_s, &argtype::TopBuildDir)                           \
    .Bind("DISABLE_UNINSTALLED"_s, &argtype::DisableUninstalled)              \
    .Bind("PC_LIBDIR"_s, &argtype::PcLibdir)                                  \
    .Bind("PC_PATH"_s, &argtype::PcPath)

void CollectEnv(cmMakefile& mf, cmPkgConfigEnv& env,
                CommonArguments::EnvModeType mode)
{
  if (mode == CommonArguments::EnvModeType::ENVMODE_IGNORE) {
    return;
  }

  if (!env.Path) {
    env.Path = GetPcPath(mf);
  }

  if (!env.LibDirs) {
    env.LibDirs = GetPcLibDirs(mf);
  }

  if (!env.DisableUninstalled) {
    env.DisableUninstalled = GetDisableUninstalled(mf);
  }

  if (!env.SysrootDir) {
    env.SysrootDir = GetSysrootDir(mf);
  }

  if (!env.TopBuildDir) {
    env.TopBuildDir = GetTopBuildDir(mf);
  }

  env.AllowSysCflags = GetAllowSysInclude(mf);
  env.SysCflags = GetSysCflags(mf);

  env.AllowSysLibs = GetAllowSysLibs(mf);
  env.SysLibs = GetSysLibDirs(mf);

  if (mode == CommonArguments::EnvModeType::ENVMODE_FDO) {
    return;
  }

  *env.SysCflags += GetPkgConfSysCflags(mf);
  *env.SysLibs += GetPkgConfSysLibs(mf);
}

cm::optional<cmPkgConfigResult> HandleCommon(CommonArguments& args,
                                             cmExecutionStatus& status)
{

  auto& mf = status.GetMakefile();

  if (!args.CheckArgs(status)) {
    return {};
  }

  auto warn_or_error = [&](const std::string& err) {
    if (args.Required) {
      status.SetError(err);
      cmSystemTools::SetFatalErrorOccurred();
    } else if (!args.Quiet) {
      mf.IssueMessage(MessageType::WARNING, err);
    }
  };

  cm::filesystem::path path{ *args.Package };

  cmPkgConfigEnv env;

  if (args.PcLibdir) {
    env.LibDirs = std::move(*args.PcLibdir);
  }

  if (args.PcPath) {
    env.Path = std::move(*args.PcPath);
  }

  if (args.DisableUninstalled) {
    env.DisableUninstalled = args.DisableUninstalled;
  }

  if (args.SysrootDir) {
    env.SysrootDir = std::move(*args.SysrootDir);
  }

  if (args.TopBuildDir) {
    env.TopBuildDir = std::move(*args.TopBuildDir);
  }

  CollectEnv(mf, env, args.EnvMode);

  if (path.extension() == ".pc") {
    if (!cmSystemTools::FileExists(path.string())) {
      warn_or_error(cmStrCat("Could not find '", *args.Package, "'"));
      return {};
    }
  } else {

    std::vector<std::string> search;
    if (env.Path) {
      search = *env.Path;
      if (env.LibDirs) {
        search += *env.LibDirs;
      }
    } else if (env.LibDirs) {
      search = *env.LibDirs;
    }

    if (env.DisableUninstalled && !*env.DisableUninstalled) {
      auto uninstalled = path;
      uninstalled.concat("-uninstalled.pc");
      uninstalled =
        cmSystemTools::FindFile(uninstalled.string(), search, true);
      if (uninstalled.empty()) {
        path =
          cmSystemTools::FindFile(path.concat(".pc").string(), search, true);
        if (path.empty()) {
          warn_or_error(cmStrCat("Could not find '", *args.Package, "'"));
          return {};
        }
      } else {
        path = uninstalled;
      }
    } else {
      path =
        cmSystemTools::FindFile(path.concat(".pc").string(), search, true);
      if (path.empty()) {
        warn_or_error(cmStrCat("Could not find '", *args.Package, "'"));
        return {};
      }
    }
  }

  auto len = cmSystemTools::FileLength(path.string());

  // Windows requires this weird string -> c_str dance
  cmsys::ifstream ifs(path.string().c_str(), std::ios::binary);

  if (!ifs) {
    warn_or_error(cmStrCat("Could not open file '", path.string(), "'"));
    return {};
  }

  std::unique_ptr<char[]> buf(new char[len]);
  ifs.read(buf.get(), len);

  // Shouldn't have hit eof on previous read, should hit eof now
  if (ifs.fail() || ifs.eof() || ifs.get() != EOF) {
    warn_or_error(cmStrCat("Error while reading file '", path.string(), "'"));
    return {};
  }

  using StrictnessType = CommonArguments::StrictnessType;

  cmPkgConfigParser parser;
  auto err = parser.Finish(buf.get(), len);

  if (args.Strictness != StrictnessType::STRICTNESS_BEST_EFFORT &&
      err != PCE_OK) {
    warn_or_error(cmStrCat("Parsing failed for file '", path.string(), "'"));
    return {};
  }

  cm::optional<cmPkgConfigResult> result;
  if (args.Strictness == StrictnessType::STRICTNESS_STRICT) {
    result = cmPkgConfigResolver::ResolveStrict(parser.Data(), std::move(env));
  } else if (args.Strictness == StrictnessType::STRICTNESS_PERMISSIVE) {
    result =
      cmPkgConfigResolver::ResolvePermissive(parser.Data(), std::move(env));
  } else {
    result =
      cmPkgConfigResolver::ResolveBestEffort(parser.Data(), std::move(env));
  }

  if (!result) {
    warn_or_error(
      cmStrCat("Resolution failed for file '", path.string(), "'"));
  } else if (args.Exact) {
    std::string ver;

    if (args.Version) {
      ver = cmPkgConfigResolver::ParseVersion(*args.Version).Version;
    }

    if (ver != result->Version()) {
      warn_or_error(
        cmStrCat("Package '", *args.Package, "' version '", result->Version(),
                 "' does not meet exact version requirement '", ver, "'"));
      return {};
    }

  } else if (args.Version) {
    auto rv = cmPkgConfigResolver::ParseVersion(*args.Version);
    if (!cmPkgConfigResolver::CheckVersion(rv, result->Version())) {
      warn_or_error(
        cmStrCat("Package '", *args.Package, "' version '", result->Version(),
                 "' does not meet version requirement '", *args.Version, "'"));
      return {};
    }
  }

  return result;
}

struct ExtractArguments : CommonArguments
{
  cm::optional<bool> AllowSystemIncludes;
  cm::optional<bool> AllowSystemLibs;

  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>>
    SystemIncludeDirs;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>>
    SystemLibraryDirs;
};

const auto ExtractParser =
  BIND_COMMON(ExtractArguments)
    .Bind("ALLOW_SYSTEM_INCLUDES"_s, &ExtractArguments::AllowSystemIncludes)
    .Bind("ALLOW_SYSTEM_LIBS"_s, &ExtractArguments::AllowSystemLibs)
    .Bind("SYSTEM_INCLUDE_DIRS"_s, &ExtractArguments::SystemIncludeDirs)
    .Bind("SYSTEM_LIBRARY_DIRS"_s, &ExtractArguments::SystemLibraryDirs);

bool HandleExtractCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{

  std::vector<std::string> unparsed;
  auto parsedArgs = ExtractParser.Parse(args, &unparsed);
  auto maybeResolved = HandleCommon(parsedArgs, status);

  if (!maybeResolved) {
    return !parsedArgs.Required;
  }

  auto& resolved = *maybeResolved;
  auto version = resolved.Version();

  if (parsedArgs.AllowSystemIncludes) {
    resolved.env.AllowSysCflags = *parsedArgs.AllowSystemIncludes;
  }

  if (parsedArgs.AllowSystemLibs) {
    resolved.env.AllowSysLibs = *parsedArgs.AllowSystemLibs;
  }

  if (parsedArgs.SystemIncludeDirs) {
    resolved.env.SysCflags = *parsedArgs.SystemIncludeDirs;
  }

  if (parsedArgs.SystemLibraryDirs) {
    resolved.env.SysLibs = *parsedArgs.SystemLibraryDirs;
  }

  auto& mf = status.GetMakefile();
  mf.AddDefinition("CMAKE_PKG_CONFIG_NAME", resolved.Name());
  mf.AddDefinition("CMAKE_PKG_CONFIG_DESCRIPTION", resolved.Description());
  mf.AddDefinition("CMAKE_PKG_CONFIG_VERSION", version);

  auto make_list = [&](const char* def,
                       const std::vector<cmPkgConfigDependency>& deps) {
    std::vector<cm::string_view> vec;
    vec.reserve(deps.size());

    for (const auto& dep : deps) {
      vec.emplace_back(dep.Name);
    }

    mf.AddDefinition(def, cmList::to_string(vec));
  };

  make_list("CMAKE_PKG_CONFIG_CONFLICTS", resolved.Conflicts());
  make_list("CMAKE_PKG_CONFIG_PROVIDES", resolved.Provides());
  make_list("CMAKE_PKG_CONFIG_REQUIRES", resolved.Requires());
  make_list("CMAKE_PKG_CONFIG_REQUIRES_PRIVATE", resolved.Requires(true));

  auto cflags = resolved.Cflags();
  mf.AddDefinition("CMAKE_PKG_CONFIG_CFLAGS", cflags.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_INCLUDES",
                   cmList::to_string(cflags.Includes));
  mf.AddDefinition("CMAKE_PKG_CONFIG_COMPILE_OPTIONS",
                   cmList::to_string(cflags.CompileOptions));

  cflags = resolved.Cflags(true);
  mf.AddDefinition("CMAKE_PKG_CONFIG_CFLAGS_PRIVATE", cflags.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_INCLUDES_PRIVATE",
                   cmList::to_string(cflags.Includes));
  mf.AddDefinition("CMAKE_PKG_CONFIG_COMPILE_OPTIONS_PRIVATE",
                   cmList::to_string(cflags.CompileOptions));

  auto libs = resolved.Libs();
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBS", libs.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBDIRS",
                   cmList::to_string(libs.LibDirs));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBNAMES",
                   cmList::to_string(libs.LibNames));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LINK_OPTIONS",
                   cmList::to_string(libs.LinkOptions));

  libs = resolved.Libs(true);
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBS_PRIVATE", libs.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBDIRS_PRIVATE",
                   cmList::to_string(libs.LibDirs));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBNAMES_PRIVATE",
                   cmList::to_string(libs.LibNames));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LINK_OPTIONS_PRIVATE",
                   cmList::to_string(libs.LinkOptions));

  return true;
}
} // namespace

bool cmCMakePkgConfigCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("must be called with at least two arguments.");
    return false;
  }

  static cmSubcommandTable const subcommand{
    { "EXTRACT"_s, HandleExtractCommand },
  };

  return subcommand(args[0], args, status);
}
