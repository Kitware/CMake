/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_map>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

// From cmPkgConfigParser.h, IWYU doesn't like including the header
struct cmPkgConfigEntry;

struct cmPkgConfigCflagsResult
{
  std::string Flagline;
  std::vector<std::string> Includes;
  std::vector<std::string> CompileOptions;
};

struct cmPkgConfigLibsResult
{
  std::string Flagline;
  std::vector<std::string> LibDirs;
  std::vector<std::string> LibNames;
  std::vector<std::string> LinkOptions;
};

struct cmPkgConfigVersionReq
{
  enum
  {
    ANY = 0,
    LT,
    LT_EQ,
    EQ,
    NEQ,
    GT_EQ,
    GT,
  } Operation = ANY;
  std::string Version;
};

struct cmPkgConfigDependency
{
  std::string Name;
  cmPkgConfigVersionReq VerReq;
};

struct cmPkgConfigEnv
{
  cm::optional<std::vector<std::string>> Path;
  cm::optional<std::vector<std::string>> LibDirs;
  cm::optional<std::vector<std::string>> SysCflags;
  cm::optional<std::vector<std::string>> SysLibs;

  cm::optional<std::string> SysrootDir;
  cm::optional<std::string> TopBuildDir;

  cm::optional<bool> DisableUninstalled;

  bool AllowSysCflags = true;
  bool AllowSysLibs = true;
};

class cmPkgConfigResult
{
public:
  std::unordered_map<std::string, std::string> Keywords;
  std::unordered_map<std::string, std::string> Variables;

  std::string Name();
  std::string Description();
  std::string Version();

  std::vector<cmPkgConfigDependency> Conflicts();
  std::vector<cmPkgConfigDependency> Provides();
  std::vector<cmPkgConfigDependency> Requires(bool priv = false);

  cmPkgConfigCflagsResult Cflags(bool priv = false);
  cmPkgConfigLibsResult Libs(bool priv = false);

  cmPkgConfigEnv env;

private:
  std::string StrOrDefault(std::string const& key, cm::string_view def = "");
};

class cmPkgConfigResolver
{
  friend class cmPkgConfigResult;

public:
  static cm::optional<cmPkgConfigResult> ResolveStrict(
    std::vector<cmPkgConfigEntry> const& entries, cmPkgConfigEnv env);

  static cm::optional<cmPkgConfigResult> ResolvePermissive(
    std::vector<cmPkgConfigEntry> const& entries, cmPkgConfigEnv env);

  static cmPkgConfigResult ResolveBestEffort(
    std::vector<cmPkgConfigEntry> const& entries, cmPkgConfigEnv env);

  static cmPkgConfigVersionReq ParseVersion(std::string const& version);

  static bool CheckVersion(cmPkgConfigVersionReq const& desired,
                           std::string const& provided);

  static void ReplaceSep(std::string& list);

#ifdef _WIN32
  static char const Sep = ';';
#else
  static char const Sep = ':';
#endif

private:
  static std::string HandleVariablePermissive(
    cmPkgConfigEntry const& entry,
    std::unordered_map<std::string, std::string> const& variables);

  static cm::optional<std::string> HandleVariableStrict(
    cmPkgConfigEntry const& entry,
    std::unordered_map<std::string, std::string> const& variables);

  static std::string HandleKeyword(
    cmPkgConfigEntry const& entry,
    std::unordered_map<std::string, std::string> const& variables);

  static std::vector<cm::string_view> TokenizeFlags(
    std::string const& flagline);

  static cmPkgConfigCflagsResult MangleCflags(
    std::vector<cm::string_view> const& flags);

  static cmPkgConfigCflagsResult MangleCflags(
    std::vector<cm::string_view> const& flags, std::string const& sysroot);

  static cmPkgConfigCflagsResult MangleCflags(
    std::vector<cm::string_view> const& flags,
    std::vector<std::string> const& syspaths);

  static cmPkgConfigCflagsResult MangleCflags(
    std::vector<cm::string_view> const& flags, std::string const& sysroot,
    std::vector<std::string> const& syspaths);

  static cmPkgConfigLibsResult MangleLibs(
    std::vector<cm::string_view> const& flags);

  static cmPkgConfigLibsResult MangleLibs(
    std::vector<cm::string_view> const& flags, std::string const& sysroot);

  static cmPkgConfigLibsResult MangleLibs(
    std::vector<cm::string_view> const& flags,
    std::vector<std::string> const& syspaths);

  static cmPkgConfigLibsResult MangleLibs(
    std::vector<cm::string_view> const& flags, std::string const& sysroot,
    std::vector<std::string> const& syspaths);

  static std::string Reroot(cm::string_view flag, cm::string_view prefix,
                            std::string const& sysroot);

  static cmPkgConfigVersionReq ParseVersion(std::string::const_iterator& cur,
                                            std::string::const_iterator end);

  static std::vector<cmPkgConfigDependency> ParseDependencies(
    std::string const& deps);
};
