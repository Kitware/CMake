/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmPkgConfigResolver.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmPkgConfigParser.h"

namespace {

void TrimBack(std::string& str)
{
  if (!str.empty()) {
    auto it = str.end() - 1;
    for (; std::isspace(*it); --it) {
      if (it == str.begin()) {
        str.clear();
        return;
      }
    }
    str.erase(++it, str.end());
  }
}

std::string AppendAndTrim(std::string& str, cm::string_view sv)
{
  auto size = str.length();
  str += sv;
  if (str.empty()) {
    return {};
  }

  auto begin = str.begin() + size;
  auto cur = str.end() - 1;

  while (cur != begin && std::isspace(*cur)) {
    --cur;
  }

  if (std::isspace(*cur)) {
    return {};
  }

  return { &*begin, static_cast<std::size_t>(cur - begin) + 1 };
}

} // namespace

std::string cmPkgConfigResult::StrOrDefault(const std::string& key,
                                            cm::string_view def)
{
  auto it = Keywords.find(key);
  return it == Keywords.end() ? std::string{ def } : it->second;
};

std::string cmPkgConfigResult::Name()
{
  return StrOrDefault("Name");
}

std::string cmPkgConfigResult::Description()
{
  return StrOrDefault("Description");
}

std::string cmPkgConfigResult::Version()
{
  return StrOrDefault("Version");
}

std::vector<cmPkgConfigDependency> cmPkgConfigResult::Conflicts()
{
  auto it = Keywords.find("Conflicts");
  if (it == Keywords.end()) {
    return {};
  }

  return cmPkgConfigResolver::ParseDependencies(it->second);
}

std::vector<cmPkgConfigDependency> cmPkgConfigResult::Provides()
{
  auto it = Keywords.find("Provides");
  if (it == Keywords.end()) {
    return {};
  }

  return cmPkgConfigResolver::ParseDependencies(it->second);
}

std::vector<cmPkgConfigDependency> cmPkgConfigResult::Requires(bool priv)
{
  auto it = Keywords.find(priv ? "Requires.private" : "Requires");
  if (it == Keywords.end()) {
    return {};
  }

  return cmPkgConfigResolver::ParseDependencies(it->second);
}

cmPkgConfigCflagsResult cmPkgConfigResult::Cflags(bool priv)
{
  std::string cflags;
  auto it = Keywords.find(priv ? "Cflags.private" : "Cflags");
  if (it != Keywords.end()) {
    cflags += it->second;
  }

  it = Keywords.find(priv ? "CFlags.private" : "CFlags");
  if (it != Keywords.end()) {
    if (!cflags.empty()) {
      cflags += " ";
    }
    cflags += it->second;
  }

  auto tokens = cmPkgConfigResolver::TokenizeFlags(cflags);

  if (env.AllowSysCflags) {
    if (env.SysrootDir) {
      return cmPkgConfigResolver::MangleCflags(tokens, *env.SysrootDir);
    }
    return cmPkgConfigResolver::MangleCflags(tokens);
  }

  if (env.SysCflags) {
    if (env.SysrootDir) {
      return cmPkgConfigResolver::MangleCflags(tokens, *env.SysrootDir,
                                               *env.SysCflags);
    }
    return cmPkgConfigResolver::MangleCflags(tokens, *env.SysCflags);
  }

  if (env.SysrootDir) {
    return cmPkgConfigResolver::MangleCflags(
      tokens, *env.SysrootDir, std::vector<std::string>{ "/usr/include" });
  }

  return cmPkgConfigResolver::MangleCflags(
    tokens, std::vector<std::string>{ "/usr/include" });
}

cmPkgConfigLibsResult cmPkgConfigResult::Libs(bool priv)
{
  auto it = Keywords.find(priv ? "Libs.private" : "Libs");
  if (it == Keywords.end()) {
    return cmPkgConfigLibsResult();
  }

  auto tokens = cmPkgConfigResolver::TokenizeFlags(it->second);

  if (env.AllowSysLibs) {
    if (env.SysrootDir) {
      return cmPkgConfigResolver::MangleLibs(tokens, *env.SysrootDir);
    }
    return cmPkgConfigResolver::MangleLibs(tokens);
  }

  if (env.SysLibs) {
    if (env.SysrootDir) {
      return cmPkgConfigResolver::MangleLibs(tokens, *env.SysrootDir,
                                             *env.SysLibs);
    }
    return cmPkgConfigResolver::MangleLibs(tokens, *env.SysLibs);
  }

  if (env.SysrootDir) {
    return cmPkgConfigResolver::MangleLibs(
      tokens, *env.SysrootDir, std::vector<std::string>{ "/usr/lib" });
  }

  return cmPkgConfigResolver::MangleLibs(
    tokens, std::vector<std::string>{ "/usr/lib" });
}

void cmPkgConfigResolver::ReplaceSep(std::string& list)
{
#ifndef _WIN32
  std::replace(list.begin(), list.end(), ':', ';');
#else
  static_cast<void>(list); // Unused parameter
#endif
}

cm::optional<cmPkgConfigResult> cmPkgConfigResolver::ResolveStrict(
  const std::vector<cmPkgConfigEntry>& entries, cmPkgConfigEnv env)
{
  cm::optional<cmPkgConfigResult> result;
  cmPkgConfigResult config;
  auto& keys = config.Keywords;

  if (env.SysrootDir) {
    config.Variables["pc_sysrootdir"] = *env.SysrootDir;
  } else {
    config.Variables["pc_sysrootdir"] = "/";
  }

  if (env.TopBuildDir) {
    config.Variables["pc_top_builddir"] = *env.TopBuildDir;
  }

  config.env = std::move(env);

  for (const auto& entry : entries) {
    std::string key(entry.Key);
    if (entry.IsVariable) {
      if (config.Variables.find(key) != config.Variables.end()) {
        return result;
      }
      auto var = HandleVariableStrict(entry, config.Variables);
      if (!var) {
        return result;
      }
      config.Variables[key] = *var;
    } else {
      if (key == "Cflags" && keys.find("CFlags") != keys.end()) {
        return result;
      }
      if (key == "CFlags" && keys.find("Cflags") != keys.end()) {
        return result;
      }
      if (key == "Cflags.private" &&
          keys.find("CFlags.private") != keys.end()) {
        return result;
      }
      if (key == "CFlags.private" &&
          keys.find("Cflags.private") != keys.end()) {
        return result;
      }
      if (keys.find(key) != keys.end()) {
        return result;
      }
      keys[key] = HandleKeyword(entry, config.Variables);
    }
  }

  if (keys.find("Name") == keys.end() ||
      keys.find("Description") == keys.end() ||
      keys.find("Version") == keys.end()) {
    return result;
  }

  result = std::move(config);
  return result;
}

cm::optional<cmPkgConfigResult> cmPkgConfigResolver::ResolvePermissive(
  const std::vector<cmPkgConfigEntry>& entries, cmPkgConfigEnv env)
{
  cm::optional<cmPkgConfigResult> result;

  cmPkgConfigResult config = ResolveBestEffort(entries, std::move(env));
  const auto& keys = config.Keywords;

  if (keys.find("Name") == keys.end() ||
      keys.find("Description") == keys.end() ||
      keys.find("Version") == keys.end()) {
    return result;
  }

  result = std::move(config);
  return result;
}

cmPkgConfigResult cmPkgConfigResolver::ResolveBestEffort(
  const std::vector<cmPkgConfigEntry>& entries, cmPkgConfigEnv env)
{
  cmPkgConfigResult result;

  if (env.SysrootDir) {
    result.Variables["pc_sysrootdir"] = *env.SysrootDir;
  } else {
    result.Variables["pc_sysrootdir"] = "/";
  }

  if (env.TopBuildDir) {
    result.Variables["pc_top_builddir"] = *env.TopBuildDir;
  }

  result.env = std::move(env);

  for (const auto& entry : entries) {
    std::string key(entry.Key);
    if (entry.IsVariable) {
      result.Variables[key] =
        HandleVariablePermissive(entry, result.Variables);
    } else {
      result.Keywords[key] += HandleKeyword(entry, result.Variables);
    }
  }
  return result;
}

std::string cmPkgConfigResolver::HandleVariablePermissive(
  const cmPkgConfigEntry& entry,
  const std::unordered_map<std::string, std::string>& variables)
{
  std::string result;
  for (const auto& segment : entry.Val) {
    if (!segment.IsVariable) {
      result += segment.Data;
    } else if (entry.Key != segment.Data) {
      auto it = variables.find(std::string{ segment.Data });
      if (it != variables.end()) {
        result += it->second;
      }
    }
  }

  TrimBack(result);
  return result;
}

cm::optional<std::string> cmPkgConfigResolver::HandleVariableStrict(
  const cmPkgConfigEntry& entry,
  const std::unordered_map<std::string, std::string>& variables)
{
  cm::optional<std::string> result;

  std::string value;
  for (const auto& segment : entry.Val) {
    if (!segment.IsVariable) {
      value += segment.Data;
    } else if (entry.Key == segment.Data) {
      return result;
    } else {
      auto it = variables.find(std::string{ segment.Data });
      if (it != variables.end()) {
        value += it->second;
      } else {
        return result;
      }
    }
  }

  TrimBack(value);
  result = std::move(value);
  return result;
}

std::string cmPkgConfigResolver::HandleKeyword(
  const cmPkgConfigEntry& entry,
  const std::unordered_map<std::string, std::string>& variables)
{
  std::string result;
  for (const auto& segment : entry.Val) {
    if (!segment.IsVariable) {
      result += segment.Data;
    } else {
      auto it = variables.find(std::string{ segment.Data });
      if (it != variables.end()) {
        result += it->second;
      }
    }
  }

  TrimBack(result);
  return result;
}

std::vector<cm::string_view> cmPkgConfigResolver::TokenizeFlags(
  const std::string& flagline)
{
  std::vector<cm::string_view> result;

  auto it = flagline.begin();
  while (it != flagline.end() && std::isspace(*it)) {
    ++it;
  }

  while (it != flagline.end()) {
    const char* start = &(*it);
    std::size_t len = 0;

    for (; it != flagline.end() && !std::isspace(*it); ++it) {
      ++len;
    }

    for (; it != flagline.end() && std::isspace(*it); ++it) {
      ++len;
    }

    result.emplace_back(start, len);
  }

  return result;
}

cmPkgConfigCflagsResult cmPkgConfigResolver::MangleCflags(
  const std::vector<cm::string_view>& flags)
{
  cmPkgConfigCflagsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-I", 0) == 0) {
      result.Includes.emplace_back(AppendAndTrim(result.Flagline, flag));
    } else {
      result.CompileOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigCflagsResult cmPkgConfigResolver::MangleCflags(
  const std::vector<cm::string_view>& flags, const std::string& sysroot)
{
  cmPkgConfigCflagsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-I", 0) == 0) {
      std::string reroot = Reroot(flag, "-I", sysroot);
      result.Includes.emplace_back(AppendAndTrim(result.Flagline, reroot));
    } else {
      result.CompileOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigCflagsResult cmPkgConfigResolver::MangleCflags(
  const std::vector<cm::string_view>& flags,
  const std::vector<std::string>& syspaths)
{
  cmPkgConfigCflagsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-I", 0) == 0) {
      cm::string_view noprefix{ flag.data() + 2, flag.size() - 2 };

      if (std::all_of(syspaths.begin(), syspaths.end(),
                      [&](const std::string& path) {
                        return noprefix.rfind(path, 0) == noprefix.npos;
                      })) {
        result.Includes.emplace_back(AppendAndTrim(result.Flagline, flag));
      }

    } else {
      result.CompileOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigCflagsResult cmPkgConfigResolver::MangleCflags(
  const std::vector<cm::string_view>& flags, const std::string& sysroot,
  const std::vector<std::string>& syspaths)
{
  cmPkgConfigCflagsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-I", 0) == 0) {
      std::string reroot = Reroot(flag, "-I", sysroot);
      cm::string_view noprefix{ reroot.data() + 2, reroot.size() - 2 };

      if (std::all_of(syspaths.begin(), syspaths.end(),
                      [&](const std::string& path) {
                        return noprefix.rfind(path, 0) == noprefix.npos;
                      })) {
        result.Includes.emplace_back(AppendAndTrim(result.Flagline, reroot));
      }

    } else {
      result.CompileOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigLibsResult cmPkgConfigResolver::MangleLibs(
  const std::vector<cm::string_view>& flags)
{
  cmPkgConfigLibsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-L", 0) == 0) {
      result.LibDirs.emplace_back(AppendAndTrim(result.Flagline, flag));
    } else if (flag.rfind("-l", 0) == 0) {
      result.LibNames.emplace_back(AppendAndTrim(result.Flagline, flag));
    } else {
      result.LinkOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigLibsResult cmPkgConfigResolver::MangleLibs(
  const std::vector<cm::string_view>& flags, const std::string& sysroot)
{
  cmPkgConfigLibsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-L", 0) == 0) {
      std::string reroot = Reroot(flag, "-L", sysroot);
      result.LibDirs.emplace_back(AppendAndTrim(result.Flagline, reroot));
    } else if (flag.rfind("-l", 0) == 0) {
      result.LibNames.emplace_back(AppendAndTrim(result.Flagline, flag));
    } else {
      result.LinkOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigLibsResult cmPkgConfigResolver::MangleLibs(
  const std::vector<cm::string_view>& flags,
  const std::vector<std::string>& syspaths)
{
  cmPkgConfigLibsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-L", 0) == 0) {
      cm::string_view noprefix{ flag.data() + 2, flag.size() - 2 };

      if (std::all_of(syspaths.begin(), syspaths.end(),
                      [&](const std::string& path) {
                        return noprefix.rfind(path, 0) == noprefix.npos;
                      })) {
        result.LibDirs.emplace_back(AppendAndTrim(result.Flagline, flag));
      }

    } else if (flag.rfind("-l", 0) == 0) {
      result.LibNames.emplace_back(AppendAndTrim(result.Flagline, flag));
    } else {
      result.LinkOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

cmPkgConfigLibsResult cmPkgConfigResolver::MangleLibs(
  const std::vector<cm::string_view>& flags, const std::string& sysroot,
  const std::vector<std::string>& syspaths)
{
  cmPkgConfigLibsResult result;

  for (auto flag : flags) {
    if (flag.rfind("-L", 0) == 0) {
      std::string reroot = Reroot(flag, "-L", sysroot);
      cm::string_view noprefix{ reroot.data() + 2, reroot.size() - 2 };

      if (std::all_of(syspaths.begin(), syspaths.end(),
                      [&](const std::string& path) {
                        return noprefix.rfind(path, 0) == noprefix.npos;
                      })) {
        result.LibDirs.emplace_back(AppendAndTrim(result.Flagline, reroot));
      }

    } else if (flag.rfind("-l", 0) == 0) {
      result.LibNames.emplace_back(AppendAndTrim(result.Flagline, flag));
    } else {
      result.LinkOptions.emplace_back(AppendAndTrim(result.Flagline, flag));
    }
  }

  return result;
}

std::string cmPkgConfigResolver::Reroot(cm::string_view flag,
                                        cm::string_view prefix,
                                        const std::string& sysroot)
{
  std::string result = std::string{ prefix };
  result += sysroot;
  result += cm::string_view{ flag.data() + prefix.length(),
                             flag.size() - prefix.length() };
  return result;
}

cmPkgConfigVersionReq cmPkgConfigResolver::ParseVersion(
  std::string::const_iterator& cur, std::string::const_iterator end)
{
  cmPkgConfigVersionReq result;
  if (*cur == '=') {
    result.Operation = result.EQ;
    ++cur;
  } else if (*cur == '>') {
    ++cur;

    if (cur == end) {
      result.Operation = result.GT;
      return result;
    }

    if (*cur == '=') {
      result.Operation = result.GT_EQ;
      ++cur;
    } else {
      result.Operation = result.GT;
    }

  } else if (*cur == '<') {
    ++cur;

    if (cur == end) {
      result.Operation = result.LT;
      return result;
    }

    if (*cur == '=') {
      result.Operation = result.LT_EQ;
      ++cur;
    } else {
      result.Operation = result.LT;
    }

  } else if (*cur == '!') {
    ++cur;

    if (cur == end) {
      result.Operation = result.ANY;
      return result;
    }

    if (*cur == '=') {
      result.Operation = result.NEQ;
      ++cur;
    } else {
      result.Operation = result.ANY;
    }
  }

  for (;; ++cur) {
    if (cur == end) {
      return result;
    }

    if (!std::isspace(*cur)) {
      break;
    }
  }

  for (; cur != end && !std::isspace(*cur) && *cur != ','; ++cur) {
    result.Version += *cur;
  }

  return result;
}

std::vector<cmPkgConfigDependency> cmPkgConfigResolver::ParseDependencies(
  const std::string& deps)
{

  std::vector<cmPkgConfigDependency> result;

  auto cur = deps.begin();
  auto end = deps.end();

  while (cur != end) {
    while ((std::isspace(*cur) || *cur == ',')) {
      if (++cur == end) {
        return result;
      }
    }

    result.emplace_back();
    auto& dep = result.back();

    while (!std::isspace(*cur) && *cur != ',') {
      dep.Name += *cur;
      if (++cur == end) {
        return result;
      }
    }

    auto in_operator = [&]() -> bool {
      for (;; ++cur) {
        if (cur == end) {
          return false;
        }

        if (*cur == '>' || *cur == '=' || *cur == '<' || *cur == '!') {
          return true;
        }

        if (!std::isspace(*cur)) {
          return false;
        }
      }
    };

    if (!in_operator()) {
      continue;
    }

    dep.VerReq = ParseVersion(cur, end);
  }

  return result;
}

bool cmPkgConfigResolver::CheckVersion(const cmPkgConfigVersionReq& desired,
                                       const std::string& provided)
{

  if (desired.Operation == cmPkgConfigVersionReq::ANY) {
    return true;
  }

  // https://blog.jasonantman.com/2014/07/how-yum-and-rpm-compare-versions/

  auto check_with_op = [&](int comp) -> bool {
    switch (desired.Operation) {
      case cmPkgConfigVersionReq::EQ:
        return comp == 0;
      case cmPkgConfigVersionReq::NEQ:
        return comp != 0;
      case cmPkgConfigVersionReq::GT:
        return comp < 0;
      case cmPkgConfigVersionReq::GT_EQ:
        return comp <= 0;
      case cmPkgConfigVersionReq::LT:
        return comp > 0;
      case cmPkgConfigVersionReq::LT_EQ:
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
    while (a_cur != a_end && !std::isalnum(*a_cur) && *a_cur != '~') {
      ++a_cur;
    }

    while (b_cur != b_end && !std::isalnum(*b_cur) && *b_cur != '~') {
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

    if (std::isdigit(*a_cur)) {
      is_num = true;
      while (a_cur != a_end && std::isdigit(*a_cur)) {
        ++a_cur;
      }

      while (b_cur != b_end && std::isdigit(*b_cur)) {
        ++b_cur;
      }

    } else {
      is_num = false;
      while (a_cur != a_end && std::isalpha(*a_cur)) {
        ++a_cur;
      }

      while (b_cur != b_end && std::isalpha(*b_cur)) {
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

cmPkgConfigVersionReq cmPkgConfigResolver::ParseVersion(
  const std::string& version)
{
  cmPkgConfigVersionReq result;

  auto cur = version.begin();
  auto end = version.end();

  if (cur == end) {
    result.Operation = cmPkgConfigVersionReq::EQ;
    return result;
  }

  result = ParseVersion(cur, end);
  cur = version.begin();

  if (*cur != '=' && *cur != '!' && *cur != '<' && *cur != '>') {
    result.Operation = cmPkgConfigVersionReq::EQ;
  }

  return result;
}
