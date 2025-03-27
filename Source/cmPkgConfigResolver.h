/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

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
  std::string StrOrDefault(const std::string& key, cm::string_view def = "");
};

class cmPkgConfigResolver
{
  friend class cmPkgConfigResult;

public:
  static cm::optional<cmPkgConfigResult> ResolveStrict(
    const std::vector<cmPkgConfigEntry>& entries, cmPkgConfigEnv env);

  static cm::optional<cmPkgConfigResult> ResolvePermissive(
    const std::vector<cmPkgConfigEntry>& entries, cmPkgConfigEnv env);

  static cmPkgConfigResult ResolveBestEffort(
    const std::vector<cmPkgConfigEntry>& entries, cmPkgConfigEnv env);

  static cmPkgConfigVersionReq ParseVersion(const std::string& version);

  static bool CheckVersion(const cmPkgConfigVersionReq& desired,
                           const std::string& provided);

  static void ReplaceSep(std::string& list);

#ifdef _WIN32
  static const char Sep = ';';
#else
  static const char Sep = ':';
#endif

private:
  static std::string HandleVariablePermissive(
    const cmPkgConfigEntry& entry,
    const std::unordered_map<std::string, std::string>& variables);

  static cm::optional<std::string> HandleVariableStrict(
    const cmPkgConfigEntry& entry,
    const std::unordered_map<std::string, std::string>& variables);

  static std::string HandleKeyword(
    const cmPkgConfigEntry& entry,
    const std::unordered_map<std::string, std::string>& variables);

  static std::vector<cm::string_view> TokenizeFlags(
    const std::string& flagline);

  static cmPkgConfigCflagsResult MangleCflags(
    const std::vector<cm::string_view>& flags);

  static cmPkgConfigCflagsResult MangleCflags(
    const std::vector<cm::string_view>& flags, const std::string& sysroot);

  static cmPkgConfigCflagsResult MangleCflags(
    const std::vector<cm::string_view>& flags,
    const std::vector<std::string>& syspaths);

  static cmPkgConfigCflagsResult MangleCflags(
    const std::vector<cm::string_view>& flags, const std::string& sysroot,
    const std::vector<std::string>& syspaths);

  static cmPkgConfigLibsResult MangleLibs(
    const std::vector<cm::string_view>& flags);

  static cmPkgConfigLibsResult MangleLibs(
    const std::vector<cm::string_view>& flags, const std::string& sysroot);

  static cmPkgConfigLibsResult MangleLibs(
    const std::vector<cm::string_view>& flags,
    const std::vector<std::string>& syspaths);

  static cmPkgConfigLibsResult MangleLibs(
    const std::vector<cm::string_view>& flags, const std::string& sysroot,
    const std::vector<std::string>& syspaths);

  static std::string Reroot(cm::string_view flag, cm::string_view prefix,
                            const std::string& sysroot);

  static cmPkgConfigVersionReq ParseVersion(std::string::const_iterator& cur,
                                            std::string::const_iterator end);

  static std::vector<cmPkgConfigDependency> ParseDependencies(
    const std::string& deps);
};
