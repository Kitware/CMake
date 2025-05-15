/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmCMakePkgConfigCommand.h"

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
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
#include "cmTarget.h"
#include "cmValue.h"
#include <cmllpkgc/llpkgc.h>

// IWYU wants this
namespace {
struct ExtractArguments;
struct PopulateArguments;
struct ImportArguments;
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

std::vector<std::string> GetLocations(cmMakefile& mf, char const* cachevar,
                                      char const* envvar, char const* desc,
                                      char const* pcvar, bool need_pkgconf,
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
  auto get_and_append = [&](char const* var) {
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

cm::optional<std::string> GetPath(cmMakefile& mf, char const* cachevar,
                                  char const* envvar, char const* desc)
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

bool GetBool(cmMakefile& mf, char const* cachevar, char const* envvar,
             char const* desc)
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

struct ImportEnv
{
  bool required;
  bool quiet;
  bool exact;
  bool err;
  CommonArguments::StrictnessType strictness;
  cmExecutionStatus& status;
};

void warn_or_error(std::string const& err, ImportEnv& imEnv)
{
  if (imEnv.required) {
    imEnv.status.SetError(err);
    cmSystemTools::SetFatalErrorOccurred();
  } else if (!imEnv.quiet) {
    imEnv.status.GetMakefile().IssueMessage(MessageType::WARNING, err);
  }
  imEnv.err = true;
}

cm::optional<cmPkgConfigResult> ReadPackage(std::string const& package,
                                            ImportEnv& imEnv,
                                            cmPkgConfigEnv& pcEnv)
{
  cm::optional<cmPkgConfigResult> result;
  cm::filesystem::path path{ package };

  if (path.extension() == ".pc") {
    if (!cmSystemTools::FileExists(path.string())) {
      return result;
    }
  } else {

    if (pcEnv.DisableUninstalled && !*pcEnv.DisableUninstalled) {
      auto uninstalled = path;
      uninstalled.concat("-uninstalled.pc");
      uninstalled =
        cmSystemTools::FindFile(uninstalled.string(), pcEnv.search, true);
      if (uninstalled.empty()) {
        path = cmSystemTools::FindFile(path.concat(".pc").string(),
                                       pcEnv.search, true);
        if (path.empty()) {
          return result;
        }
      } else {
        path = uninstalled;
      }
    } else {
      path = cmSystemTools::FindFile(path.concat(".pc").string(), pcEnv.search,
                                     true);
      if (path.empty()) {
        return result;
      }
    }
  }

  auto len = cmSystemTools::FileLength(path.string());

  // Windows requires this weird string -> c_str dance
  cmsys::ifstream ifs(path.string().c_str(), std::ios::binary);

  if (!ifs) {
    warn_or_error(cmStrCat("Could not open file '", path.string(), "'"),
                  imEnv);
    return result;
  }

  std::unique_ptr<char[]> buf(new char[len]);
  ifs.read(buf.get(), len);

  // Shouldn't have hit eof on previous read, should hit eof now
  if (ifs.fail() || ifs.eof() || ifs.get() != EOF) {
    warn_or_error(cmStrCat("Error while reading file '", path.string(), "'"),
                  imEnv);
    return result;
  }

  using StrictnessType = CommonArguments::StrictnessType;

  cmPkgConfigParser parser;
  auto err = parser.Finish(buf.get(), len);

  if (imEnv.strictness != StrictnessType::STRICTNESS_BEST_EFFORT &&
      err != PCE_OK) {
    warn_or_error(cmStrCat("Parsing failed for file '", path.string(), "'"),
                  imEnv);
    return result;
  }

  if (imEnv.strictness == StrictnessType::STRICTNESS_STRICT) {
    result = cmPkgConfigResolver::ResolveStrict(parser.Data(), pcEnv);
  } else if (imEnv.strictness == StrictnessType::STRICTNESS_PERMISSIVE) {
    result = cmPkgConfigResolver::ResolvePermissive(parser.Data(), pcEnv);
  } else {
    result = cmPkgConfigResolver::ResolveBestEffort(parser.Data(), pcEnv);
  }

  if (!result) {
    warn_or_error(cmStrCat("Resolution failed for file '", path.string(), "'"),
                  imEnv);
  }

  return result;
}

cm::optional<cmPkgConfigResult> ImportPackage(
  std::string const& package, cm::optional<std::string> version,
  ImportEnv& imEnv, cmPkgConfigEnv& pcEnv)
{
  auto result = ReadPackage(package, imEnv, pcEnv);

  if (!result) {
    if (!imEnv.err) {
      warn_or_error(cmStrCat("Could not find pkg-config: '", package, "'"),
                    imEnv);
    }
    return result;
  }

  if (imEnv.exact) {
    std::string ver;

    if (version) {
      ver = cmPkgConfigResolver::ParseVersion(*version).Version;
    }

    if (ver != result->Version()) {
      warn_or_error(
        cmStrCat("Package '", package, "' version '", result->Version(),
                 "' does not meet exact version requirement '", ver, "'"),
        imEnv);
      return {};
    }

  } else if (version) {
    auto rv = cmPkgConfigResolver::ParseVersion(*version);
    if (!cmPkgConfigResolver::CheckVersion(rv, result->Version())) {
      warn_or_error(
        cmStrCat("Package '", package, "' version '", result->Version(),
                 "' does not meet version requirement '", *version, "'"),
        imEnv);
      return {};
    }
  }

  result->env = &pcEnv;
  return result;
}

struct pkgStackEntry
{
  cmPkgConfigVersionReq ver;
  std::string parent;
};

cm::optional<cmPkgConfigResult> ImportPackage(
  std::string const& package, std::vector<pkgStackEntry> const& reqs,
  ImportEnv& imEnv, cmPkgConfigEnv& pcEnv)
{
  auto result = ReadPackage(package, imEnv, pcEnv);

  if (!result) {
    if (!imEnv.err) {
      std::string req_str = cmStrCat("'", reqs.begin()->parent, "'");
      for (auto it = reqs.begin() + 1; it != reqs.end(); ++it) {
        req_str = cmStrCat(req_str, ", '", it->parent, "'");
      }
      warn_or_error(cmStrCat("Could not find pkg-config: '", package,
                             "' required by: ", req_str),
                    imEnv);
    }
    return result;
  }

  auto ver = result->Version();
  for (auto const& req : reqs) {

    if (!cmPkgConfigResolver::CheckVersion(req.ver, ver)) {
      warn_or_error(cmStrCat("Package '", package, "' version '", ver,
                             "' does not meet version requirement '",
                             req.ver.string(), "' of '", req.parent, "'"),
                    imEnv);
      return {};
    }
  }

  result->env = &pcEnv;
  return result;
}

cm::optional<std::pair<cmPkgConfigEnv, ImportEnv>> HandleCommon(
  CommonArguments& args, cmExecutionStatus& status)
{

  auto& mf = status.GetMakefile();

  if (!args.CheckArgs(status)) {
    return {};
  }

  cmPkgConfigEnv pcEnv;

  if (args.PcLibdir) {
    pcEnv.LibDirs = std::move(*args.PcLibdir);
  }

  if (args.PcPath) {
    pcEnv.Path = std::move(*args.PcPath);
  }

  pcEnv.DisableUninstalled = args.DisableUninstalled;

  if (args.SysrootDir) {
    pcEnv.SysrootDir = std::move(*args.SysrootDir);
  }

  if (args.TopBuildDir) {
    pcEnv.TopBuildDir = std::move(*args.TopBuildDir);
  }

  CollectEnv(mf, pcEnv, args.EnvMode);

  if (pcEnv.Path) {
    pcEnv.search = *pcEnv.Path;
    if (pcEnv.LibDirs) {
      pcEnv.search += *pcEnv.LibDirs;
    }
  } else if (pcEnv.LibDirs) {
    pcEnv.search = *pcEnv.LibDirs;
  }

  return std::pair<cmPkgConfigEnv, ImportEnv>{
    pcEnv,
    { args.Required, args.Quiet, args.Exact, false, args.Strictness, status }
  };
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

auto const ExtractParser =
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
  auto maybeEnv = HandleCommon(parsedArgs, status);

  if (!maybeEnv) {
    return !parsedArgs.Required;
  }
  auto& pcEnv = maybeEnv->first;
  auto& imEnv = maybeEnv->second;

  auto maybePackage =
    ImportPackage(*parsedArgs.Package, parsedArgs.Version, imEnv, pcEnv);
  if (!maybePackage) {
    return !parsedArgs.Required;
  }
  auto& package = *maybePackage;

  if (parsedArgs.AllowSystemIncludes) {
    pcEnv.AllowSysCflags = *parsedArgs.AllowSystemIncludes;
  }

  if (parsedArgs.AllowSystemLibs) {
    pcEnv.AllowSysLibs = *parsedArgs.AllowSystemLibs;
  }

  if (parsedArgs.SystemIncludeDirs) {
    pcEnv.SysCflags = *parsedArgs.SystemIncludeDirs;
  }

  if (parsedArgs.SystemLibraryDirs) {
    pcEnv.SysLibs = *parsedArgs.SystemLibraryDirs;
  }

  auto& mf = status.GetMakefile();
  mf.AddDefinition("CMAKE_PKG_CONFIG_NAME", package.Name());
  mf.AddDefinition("CMAKE_PKG_CONFIG_DESCRIPTION", package.Description());
  mf.AddDefinition("CMAKE_PKG_CONFIG_VERSION", package.Version());

  auto make_list = [&](char const* def,
                       std::vector<cmPkgConfigDependency> const& deps) {
    std::vector<cm::string_view> vec;
    vec.reserve(deps.size());

    for (auto const& dep : deps) {
      vec.emplace_back(dep.Name);
    }

    mf.AddDefinition(def, cmList::to_string(vec));
  };

  make_list("CMAKE_PKG_CONFIG_CONFLICTS", package.Conflicts());
  make_list("CMAKE_PKG_CONFIG_PROVIDES", package.Provides());
  make_list("CMAKE_PKG_CONFIG_REQUIRES", package.Requires());
  make_list("CMAKE_PKG_CONFIG_REQUIRES_PRIVATE", package.Requires(true));

  auto cflags = package.Cflags();
  mf.AddDefinition("CMAKE_PKG_CONFIG_CFLAGS", cflags.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_INCLUDES",
                   cmList::to_string(cflags.Includes));
  mf.AddDefinition("CMAKE_PKG_CONFIG_COMPILE_OPTIONS",
                   cmList::to_string(cflags.CompileOptions));

  cflags = package.Cflags(true);
  mf.AddDefinition("CMAKE_PKG_CONFIG_CFLAGS_PRIVATE", cflags.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_INCLUDES_PRIVATE",
                   cmList::to_string(cflags.Includes));
  mf.AddDefinition("CMAKE_PKG_CONFIG_COMPILE_OPTIONS_PRIVATE",
                   cmList::to_string(cflags.CompileOptions));

  auto libs = package.Libs();
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBS", libs.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBDIRS",
                   cmList::to_string(libs.LibDirs));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBNAMES",
                   cmList::to_string(libs.LibNames));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LINK_OPTIONS",
                   cmList::to_string(libs.LinkOptions));

  libs = package.Libs(true);
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBS_PRIVATE", libs.Flagline);
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBDIRS_PRIVATE",
                   cmList::to_string(libs.LibDirs));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LIBNAMES_PRIVATE",
                   cmList::to_string(libs.LibNames));
  mf.AddDefinition("CMAKE_PKG_CONFIG_LINK_OPTIONS_PRIVATE",
                   cmList::to_string(libs.LinkOptions));

  return true;
}

using pkgStack = std::unordered_map<std::string, std::vector<pkgStackEntry>>;
using pkgProviders = std::unordered_map<std::string, std::string>;

cmTarget* CreateCMakeTarget(std::string const& name, std::string const& prefix,
                            cmPkgConfigResult& pkg, pkgProviders& providers,
                            cmMakefile& mf)
{
  auto* tgt = mf.AddForeignTarget("pkgcfg", cmStrCat(prefix, name));

  tgt->AppendProperty("VERSION", pkg.Version());

  auto libs = pkg.Libs();
  for (auto const& flag : libs.LibNames) {
    tgt->AppendProperty("INTERFACE_LINK_LIBRARIES", flag.substr(2));
  }
  for (auto const& flag : libs.LibDirs) {
    tgt->AppendProperty("INTERFACE_LINK_DIRECTORIES", flag.substr(2));
  }
  tgt->AppendProperty("INTERFACE_LINK_OPTIONS",
                      cmList::to_string(libs.LinkOptions));

  auto cflags = pkg.Cflags();
  for (auto const& flag : cflags.Includes) {
    tgt->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES", flag.substr(2));
  }
  tgt->AppendProperty("INTERFACE_COMPILE_OPTIONS",
                      cmList::to_string(cflags.CompileOptions));

  for (auto& dep : pkg.Requires()) {
    auto it = providers.find(dep.Name);
    if (it != providers.end()) {
      tgt->AppendProperty("INTERFACE_LINK_LIBRARIES", it->second);
      continue;
    }

    tgt->AppendProperty("INTERFACE_LINK_LIBRARIES",
                        cmStrCat("@foreign_pkgcfg::", prefix, dep.Name));
  }
  return tgt;
}

bool CheckPackageDependencies(
  std::string const& name, std::string const& prefix, cmPkgConfigResult& pkg,
  pkgStack& inStack,
  std::unordered_map<std::string, cmPkgConfigResult>& outStack,
  pkgProviders& providers, ImportEnv& imEnv)
{
  for (auto& dep : pkg.Requires()) {
    auto prov_it = providers.find(dep.Name);
    if (prov_it != providers.end()) {
      continue;
    }

    auto* tgt = imEnv.status.GetMakefile().FindTargetToUse(
      cmStrCat("@foreign_pkgcfg::", prefix, dep.Name),
      cmStateEnums::TargetDomain::FOREIGN);
    if (tgt) {
      auto ver = tgt->GetProperty("VERSION");
      if (!cmPkgConfigResolver::CheckVersion(dep.VerReq, *ver)) {
        warn_or_error(cmStrCat("Package '", dep.Name, "' version '", *ver,
                               "' does not meet version requirement '",
                               dep.VerReq.string(), "' of '", name, "'"),
                      imEnv);
        return false;
      }
      continue;
    }

    auto it = outStack.find(dep.Name);
    if (it != outStack.end()) {
      auto ver = it->second.Version();
      if (!cmPkgConfigResolver::CheckVersion(dep.VerReq, ver)) {
        warn_or_error(cmStrCat("Package '", dep.Name, "' version '", ver,
                               "' does not meet version requirement '",
                               dep.VerReq.string(), "' of '", name, "'"),
                      imEnv);
        return false;
      }
      continue;
    }

    inStack[dep.Name].emplace_back(
      pkgStackEntry{ std::move(dep.VerReq), name });
  }

  return true;
}

struct PopulateArguments : CommonArguments
{
  cm::optional<std::string> Prefix;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Providers;
};

#define BIND_POPULATE(argtype)                                                \
  BIND_COMMON(argtype)                                                        \
    .Bind("PREFIX"_s, &argtype::Prefix)                                       \
    .Bind("BIND_PC_REQUIRES"_s, &argtype::Providers)

auto const PopulateParser = BIND_POPULATE(PopulateArguments);

std::pair<bool, bool> PopulatePCTarget(PopulateArguments& args,
                                       cmExecutionStatus& status)
{

  std::string prefix = args.Prefix ? cmStrCat(*args.Prefix, "_"_s) : "";

  auto& mf = status.GetMakefile();
  auto maybeEnv = HandleCommon(args, status);

  if (!maybeEnv) {
    return { !args.Required, false };
  }
  auto& pcEnv = maybeEnv->first;
  auto& imEnv = maybeEnv->second;

  pkgProviders providers;
  if (args.Providers) {
    for (auto const& provider_str : *args.Providers) {
      auto assignment = provider_str.find('=');
      if (assignment != std::string::npos) {
        providers.emplace(provider_str.substr(0, assignment),
                          provider_str.substr(assignment + 1));
      } else {
        imEnv.status.SetError(cmStrCat(
          "No '=' found in BIND_PC_REQUIRES argument '", provider_str, "'"));
        cmSystemTools::SetFatalErrorOccurred();
        return { false, false };
      }
    }
  }

  pkgStack inStack;
  std::unordered_map<std::string, cmPkgConfigResult> outStack;

  auto maybePackage = ImportPackage(*args.Package, args.Version, imEnv, pcEnv);
  if (!maybePackage) {
    return { !args.Required, false };
  }
  imEnv.exact = false;

  if (!CheckPackageDependencies(*args.Package, prefix, *maybePackage, inStack,
                                outStack, providers, imEnv)) {
    return { !args.Required, false };
  }
  outStack[*args.Package] = std::move(*maybePackage);

  while (!inStack.empty()) {
    auto name = inStack.begin()->first;
    auto reqs = inStack.begin()->second;
    maybePackage = ImportPackage(name, reqs, imEnv, pcEnv);
    if (!maybePackage) {
      return { !args.Required, false };
    }
    if (!CheckPackageDependencies(name, prefix, *maybePackage, inStack,
                                  outStack, providers, imEnv)) {
      return { !args.Required, false };
    }
    inStack.erase(name);
    outStack[std::move(name)] = std::move(*maybePackage);
  }

  for (auto& entry : outStack) {
    CreateCMakeTarget(entry.first, prefix, entry.second, providers, mf);
  }

  return { true, true };
}

bool HandlePopulateCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  std::vector<std::string> unparsed;
  auto parsedArgs = PopulateParser.Parse(args, &unparsed);

  std::string prefix =
    parsedArgs.Prefix ? cmStrCat(*parsedArgs.Prefix, "_"_s) : "";

  auto foreign_name =
    cmStrCat("@foreign_pkgcfg::", prefix, *parsedArgs.Package);
  auto found_var = cmStrCat("PKGCONFIG_", *parsedArgs.Package, "_FOUND");

  auto& mf = status.GetMakefile();

  if (mf.FindTargetToUse(foreign_name, cmStateEnums::TargetDomain::FOREIGN)) {
    mf.AddDefinition(found_var, "TRUE");
    return true;
  }

  auto result = PopulatePCTarget(parsedArgs, status);
  mf.AddDefinition(found_var, result.second ? "TRUE" : "FALSE");
  return result.first;
}

struct ImportArguments : PopulateArguments
{
  cm::optional<std::string> Name;
};

auto const ImportParser =
  BIND_POPULATE(ImportArguments).Bind("NAME"_s, &ImportArguments::Name);

bool HandleImportCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  std::vector<std::string> unparsed;
  auto parsedArgs = ImportParser.Parse(args, &unparsed);

  std::string prefix =
    parsedArgs.Prefix ? cmStrCat(*parsedArgs.Prefix, "_"_s) : "";

  auto foreign_name =
    cmStrCat("@foreign_pkgcfg::", prefix, *parsedArgs.Package);
  auto local_name =
    cmStrCat("PkgConfig::", parsedArgs.Name.value_or(*parsedArgs.Package));
  auto found_var = cmStrCat("PKGCONFIG_", *parsedArgs.Package, "_FOUND");

  auto& mf = status.GetMakefile();

  if (mf.FindTargetToUse(local_name)) {
    mf.AddDefinition(found_var, "TRUE");
    return true;
  }

  if (!mf.FindTargetToUse(foreign_name, cmStateEnums::TargetDomain::FOREIGN)) {
    auto result = PopulatePCTarget(parsedArgs, status);
    if (!result.second) {
      mf.AddDefinition(found_var, "FALSE");
      return result.first;
    }
  }

  mf.AddDefinition(found_var, "TRUE");
  auto* tgt = mf.AddImportedTarget(
    local_name, cmStateEnums::TargetType::INTERFACE_LIBRARY, false);
  tgt->AppendProperty("INTERFACE_LINK_LIBRARIES", foreign_name);
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
    { "POPULATE"_s, HandlePopulateCommand },
    { "IMPORT"_s, HandleImportCommand },
  };

  return subcommand(args[0], args, status);
}
