/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmPkgConfigResolver.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include "cmsys/String.h"

#include "cmOutputConverter.h"
#include "cmPkgConfigParser.h"
#include "cmStringAlgorithms.h"

namespace {

bool HasPrefix(std::string const& value, char const* prefix)
{
  std::size_t const len = std::strlen(prefix);
  return value.size() >= len && value.compare(0, len, prefix) == 0;
}

void AppendFlag(std::string& flagline, std::string const& flag)
{
  if (!flagline.empty()) {
    flagline += ' ';
  }
  flagline += flag;
}

bool IsSystemPath(std::string const& path,
                  std::vector<std::string> const& systemPaths)
{
  return std::find(systemPaths.begin(), systemPaths.end(), path) !=
    systemPaths.end();
}

std::vector<std::string> DefaultSystemIncludePaths()
{
  return std::vector<std::string>{ "/usr/include" };
}

std::vector<std::string> DefaultSystemLibraryPaths()
{
  return std::vector<std::string>{ "/usr/lib" };
}

std::vector<std::string> const& SystemCflags(cmPkgConfigEnv const& env,
                                             std::vector<std::string>& def)
{
  if (env.SysCflags) {
    return *env.SysCflags;
  }
  def = DefaultSystemIncludePaths();
  return def;
}

std::vector<std::string> const& SystemLibs(cmPkgConfigEnv const& env,
                                           std::vector<std::string>& def)
{
  if (env.SysLibs) {
    return *env.SysLibs;
  }
  def = DefaultSystemLibraryPaths();
  return def;
}

} // namespace

std::string cmPkgConfigResolver::VersionReqString(
  cmPkgConfigDependencySpec const& spec)
{
  switch (spec.Operation) {
    case cmPkgConfigDependencyOperation::None:
      return "";
    case cmPkgConfigDependencyOperation::LessThan:
      return cmStrCat('<', spec.Version);
    case cmPkgConfigDependencyOperation::LessThanEqual:
      return cmStrCat("<="_s, spec.Version);
    case cmPkgConfigDependencyOperation::Equal:
      return cmStrCat('=', spec.Version);
    case cmPkgConfigDependencyOperation::NotEqual:
      return cmStrCat("!="_s, spec.Version);
    case cmPkgConfigDependencyOperation::GreaterThanEqual:
      return cmStrCat(">="_s, spec.Version);
    case cmPkgConfigDependencyOperation::GreaterThan:
      return cmStrCat('>', spec.Version);
  }
  return "";
}

cmPkgConfigResolver::cmPkgConfigResolver(cmPkgConfigParser parser,
                                         cmPkgConfigEnv const& env,
                                         std::string const& pcFileDir)
  : Parser{ std::move(parser) }
  , Env{ &env }
{
  this->Parser.ParseComplete();
  this->ConfigureParser(pcFileDir);
}

cm::optional<cmPkgConfigResolver> cmPkgConfigResolver::Resolve(
  cmPkgConfigParser parser, cmPkgConfigEnv const& env, Strictness strictness,
  std::string const& pcFileDir)
{
  cmPkgConfigResolver resolver{ std::move(parser), env, pcFileDir };
  if (strictness != Strictness::BestEffort && !resolver.HasRequiredFields()) {
    return {};
  }
  if (strictness == Strictness::Strict && resolver.HasStrictConflicts()) {
    return {};
  }
  return resolver;
}

cm::optional<cmPkgConfigResolver> cmPkgConfigResolver::ResolveStrict(
  cmPkgConfigParser parser, cmPkgConfigEnv const& env,
  std::string const& pcFileDir)
{
  return Resolve(std::move(parser), env, Strictness::Strict, pcFileDir);
}

cm::optional<cmPkgConfigResolver> cmPkgConfigResolver::ResolvePermissive(
  cmPkgConfigParser parser, cmPkgConfigEnv const& env,
  std::string const& pcFileDir)
{
  return Resolve(std::move(parser), env, Strictness::Permissive, pcFileDir);
}

cmPkgConfigResolver cmPkgConfigResolver::ResolveBestEffort(
  cmPkgConfigParser parser, cmPkgConfigEnv const& env,
  std::string const& pcFileDir)
{
  return cmPkgConfigResolver{ std::move(parser), env, pcFileDir };
}

void cmPkgConfigResolver::ConfigureParser(std::string const& pcFileDir)
{
  if (this->Env->SysrootDir) {
    this->Parser.SetVariable(
      "pc_sysrootdir",
      cmOutputConverter::EscapeForShell(*this->Env->SysrootDir,
                                        cmOutputConverter::Shell_Flag_IsUnix));
  } else {
    this->Parser.SetVariable("pc_sysrootdir", "/");
  }

  if (this->Env->TopBuildDir) {
    this->Parser.SetVariable(
      "pc_top_builddir",
      cmOutputConverter::EscapeForShell(*this->Env->TopBuildDir,
                                        cmOutputConverter::Shell_Flag_IsUnix));
  }

  this->Parser.SetVariable("pcfiledir",
                           cmOutputConverter::EscapeForShell(
                             pcFileDir, cmOutputConverter::Shell_Flag_IsUnix));
}

bool cmPkgConfigResolver::HasRequiredFields() const
{
  return this->Parser.HasProperty("Name") &&
    this->Parser.HasProperty("Description") &&
    this->Parser.HasProperty("Version");
}

bool cmPkgConfigResolver::HasStrictConflicts() const
{
  return (this->Parser.HasProperty("Cflags") &&
          this->Parser.HasProperty("CFlags")) ||
    (this->Parser.HasProperty("Cflags.private") &&
     this->Parser.HasProperty("CFlags.private"));
}

std::string cmPkgConfigResolver::Literal(std::string const& key) const
{
  return this->Parser.GetLiteral(key);
}

std::string cmPkgConfigResolver::Name() const
{
  return this->Literal("Name");
}

std::string cmPkgConfigResolver::Description() const
{
  return this->Literal("Description");
}

std::string cmPkgConfigResolver::Version() const
{
  return this->Literal("Version");
}

std::vector<std::string> cmPkgConfigResolver::Fragments(
  std::string const& key) const
{
  return this->Parser.GetFragments(key);
}

std::vector<cmPkgConfigDependencySpec> cmPkgConfigResolver::Dependencies(
  std::string const& key) const
{
  return this->Parser.GetDependencies(key);
}

std::vector<cmPkgConfigDependencySpec> cmPkgConfigResolver::Conflicts() const
{
  return this->Dependencies("Conflicts");
}

std::vector<cmPkgConfigDependencySpec> cmPkgConfigResolver::Provides() const
{
  return this->Dependencies("Provides");
}

std::vector<cmPkgConfigDependencySpec> cmPkgConfigResolver::Requires(
  bool priv) const
{
  return this->Dependencies(priv ? "Requires.private" : "Requires");
}

cmPkgConfigCflagsResult cmPkgConfigResolver::Cflags(bool priv) const
{
  auto flags = this->Fragments(priv ? "Cflags.private" : "Cflags");
  auto alt = this->Fragments(priv ? "CFlags.private" : "CFlags");
  flags.insert(flags.end(), alt.begin(), alt.end());
  return MangleCflags(flags, *this->Env);
}

cmPkgConfigLibsResult cmPkgConfigResolver::Libs(bool priv) const
{
  return MangleLibs(this->Fragments(priv ? "Libs.private" : "Libs"),
                    *this->Env);
}

cmPkgConfigCflagsResult cmPkgConfigResolver::MangleCflags(
  std::vector<std::string> const& flags, cmPkgConfigEnv const& env)
{
  cmPkgConfigCflagsResult result;
  std::vector<std::string> defaultSystemPaths;
  auto const& systemPaths = SystemCflags(env, defaultSystemPaths);

  for (auto const& flag : flags) {
    std::string mangled = flag;
    if (HasPrefix(flag, "-I") && env.SysrootDir) {
      mangled = Reroot(flag, "-I", *env.SysrootDir);
    }

    if (HasPrefix(mangled, "-I")) {
      if (!env.AllowSysCflags &&
          IsSystemPath(mangled.substr(2), systemPaths)) {
        continue;
      }
      AppendFlag(result.Flagline, mangled);
      result.Includes.push_back(mangled);
    } else {
      AppendFlag(result.Flagline, mangled);
      result.CompileOptions.push_back(mangled);
    }
  }

  return result;
}

cmPkgConfigLibsResult cmPkgConfigResolver::MangleLibs(
  std::vector<std::string> const& flags, cmPkgConfigEnv const& env)
{
  cmPkgConfigLibsResult result;
  std::vector<std::string> defaultSystemPaths;
  auto const& systemPaths = SystemLibs(env, defaultSystemPaths);

  for (auto const& flag : flags) {
    std::string mangled = flag;
    if (HasPrefix(flag, "-L") && env.SysrootDir) {
      mangled = Reroot(flag, "-L", *env.SysrootDir);
    }

    if (HasPrefix(mangled, "-L")) {
      if (!env.AllowSysLibs && IsSystemPath(mangled.substr(2), systemPaths)) {
        continue;
      }
      AppendFlag(result.Flagline, mangled);
      result.LibDirs.push_back(mangled);
    } else if (HasPrefix(mangled, "-l")) {
      AppendFlag(result.Flagline, mangled);
      result.LibNames.push_back(mangled);
    } else {
      AppendFlag(result.Flagline, mangled);
      result.LinkOptions.push_back(mangled);
    }
  }

  return result;
}

std::string cmPkgConfigResolver::Reroot(std::string const& flag,
                                        char const* prefix,
                                        std::string const& sysroot)
{
  std::string result = prefix;
  result += sysroot;
  result += flag.substr(std::strlen(prefix));
  return result;
}

void cmPkgConfigResolver::ReplaceSep(std::string& list)
{
#ifndef _WIN32
  std::replace(list.begin(), list.end(), ':', ';');
#else
  static_cast<void>(list);
#endif
}

bool cmPkgConfigResolver::CheckVersion(
  cmPkgConfigDependencySpec const& desired, std::string const& provided)
{
  if (desired.Operation == cmPkgConfigDependencyOperation::None) {
    return true;
  }

  auto check_with_op = [&](int comp) -> bool {
    switch (desired.Operation) {
      case cmPkgConfigDependencyOperation::Equal:
        return comp == 0;
      case cmPkgConfigDependencyOperation::NotEqual:
        return comp != 0;
      case cmPkgConfigDependencyOperation::GreaterThan:
        return comp < 0;
      case cmPkgConfigDependencyOperation::GreaterThanEqual:
        return comp <= 0;
      case cmPkgConfigDependencyOperation::LessThan:
        return comp > 0;
      case cmPkgConfigDependencyOperation::LessThanEqual:
        return comp >= 0;
      default:
        return true;
    }
  };

  if (desired.Version == provided) {
    return check_with_op(0);
  }

  auto a_cur = desired.Version.begin();
  auto a_end = desired.Version.end();

  auto b_cur = provided.begin();
  auto b_end = provided.end();

  while (a_cur != a_end && b_cur != b_end) {
    while (a_cur != a_end && !cmsysString_isalnum(*a_cur) && *a_cur != '~') {
      ++a_cur;
    }

    while (b_cur != b_end && !cmsysString_isalnum(*b_cur) && *b_cur != '~') {
      ++b_cur;
    }

    if (a_cur == a_end || b_cur == b_end) {
      break;
    }

    if (*a_cur == '~' || *b_cur == '~') {
      if (*a_cur != '~') {
        return check_with_op(1);
      }

      if (*b_cur != '~') {
        return check_with_op(-1);
      }

      ++a_cur;
      ++b_cur;
      continue;
    }

    auto a_seg = a_cur;
    auto b_seg = b_cur;
    bool is_num;

    if (cmsysString_isdigit(*a_cur)) {
      is_num = true;
      while (a_cur != a_end && cmsysString_isdigit(*a_cur)) {
        ++a_cur;
      }

      while (b_cur != b_end && cmsysString_isdigit(*b_cur)) {
        ++b_cur;
      }

    } else {
      is_num = false;
      while (a_cur != a_end && cmsysString_isalpha(*a_cur)) {
        ++a_cur;
      }

      while (b_cur != b_end && cmsysString_isalpha(*b_cur)) {
        ++b_cur;
      }
    }

    auto a_len = std::distance(a_seg, a_cur);
    auto b_len = std::distance(b_seg, b_cur);

    if (!b_len) {
      return check_with_op(is_num ? 1 : -1);
    }

    if (is_num) {
      while (a_seg != a_cur && *a_seg == '0') {
        ++a_seg;
      }

      while (b_seg != b_cur && *b_seg == '0') {
        ++b_seg;
      }

      a_len = std::distance(a_seg, a_cur);
      b_len = std::distance(b_seg, b_cur);

      if (a_len != b_len) {
        return check_with_op(a_len > b_len ? 1 : -1);
      }

      auto cmp = std::memcmp(&*a_seg, &*b_seg, a_len);
      if (cmp) {
        return check_with_op(cmp);
      }
    } else {
      auto cmp = std::memcmp(&*a_seg, &*b_seg, std::min(a_len, b_len));
      if (cmp) {
        return check_with_op(cmp);
      }

      if (a_len != b_len) {
        return check_with_op(a_len > b_len ? 1 : -1);
      }
    }
  }

  if (a_cur == a_end) {
    if (b_cur == b_end) {
      return check_with_op(0);
    }
    return check_with_op(-1);
  }

  return check_with_op(1);
}
