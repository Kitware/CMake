/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmEnvironment.h"

#include <set>
#include <sstream>
#include <utility>

#include <cm/string_view>
#include <cm/vector>
#include <cmext/algorithm>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if defined(_WIN32)
#  include "cmsys/String.h"
#endif

bool cmEnvironment::EnvNameLess::operator()(std::string const& lhs,
                                            std::string const& rhs) const
{
#if defined(_WIN32)
  // Environment variable names are case-insensitive on Windows
  return cmsysString_strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
#else
  return lhs < rhs;
#endif
}

cmEnvironment::cmEnvironment(std::vector<std::string> const& env)
{
  for (std::string const& var : env) {
    this->PutEnv(var);
  }
}

void cmEnvironment::PutEnv(std::string const& env)
{
  auto const pos = env.find('=');
  if (pos != std::string::npos) {
    this->Map[env.substr(0, pos)] = env.substr(pos + 1);
  } else {
    this->Map[env] = cm::nullopt;
  }
}

void cmEnvironment::UnPutEnv(std::string const& env)
{
  this->Map[env] = cm::nullopt;
}

void cmEnvironment::Update(cmEnvironment&& other)
{
  for (auto& kv : other.Map) {
    this->Map[kv.first] = std::move(kv.second);
  }
  other.Map.clear();
}

std::vector<std::string> cmEnvironment::GetVariables() const
{
  auto result = std::vector<std::string>{};
  result.reserve(this->Map.size());
  for (auto const& elem : this->Map) {
    if (elem.second) {
      result.push_back(elem.first + '=' + *elem.second);
    }
  }
  return result;
}

std::string cmEnvironment::RecordDifference(
  cmEnvironment const& original) const
{
  cm::string_view nl;
  std::ostringstream os;
  for (auto const& elem : this->Map) {
    if (!elem.second) {
      // Signify that this variable is being actively unset
      os << nl << '#' << elem.first << '=';
      nl = "\n";
      continue;
    }
    auto const it = original.Map.find(elem.first);
    if (it != original.Map.end() && *elem.second == it->second) {
      // Skip variables that are unchanged
      continue;
    }
    os << nl << elem.first << '=' << *elem.second;
    nl = "\n";
  }
  return os.str();
}

namespace {

auto const ValidOperators = std::set<cm::string_view>{
  "set",
  "unset",
  "string_append",
  "string_prepend",
  "path_list_append",
  "path_list_prepend",
  "cmake_list_append",
  "cmake_list_prepend",
};

struct ListAppend
{
public:
  ListAppend(std::string val, char sep)
    : value(std::move(val))
    , separator(sep)
  {
  }

  void operator()(std::string& output) const
  {
    if (!output.empty()) {
      output += separator;
    }
    output += value;
  }

private:
  std::string value;
  char separator;
};

struct ListPrepend
{
public:
  ListPrepend(std::string val, char sep)
    : value(std::move(val))
    , separator(sep)
  {
  }

  void operator()(std::string& output) const
  {
    if (!output.empty()) {
      output.insert(output.begin(), separator);
    }
    output.insert(0, value);
  }

private:
  std::string value;
  char separator;
};

} // namespace

bool cmEnvironmentModification::Add(std::vector<std::string> const& envmod)
{
  bool ok = true;
  for (auto const& entry : envmod) {
    ok &= this->Add(entry);
  }
  return ok;
}

bool cmEnvironmentModification::Add(std::string const& envmod)
{
  // Split on `=`
  auto const eq_loc = envmod.find_first_of('=');
  if (eq_loc == std::string::npos) {
    cmSystemTools::Error(cmStrCat(
      "Error: Missing `=` after the variable name in: ", envmod, '\n'));
    return false;
  }

  // Split operation on `:`
  auto const op_value_start = eq_loc + 1;
  auto const colon_loc = envmod.find_first_of(':', op_value_start);
  if (colon_loc == std::string::npos) {
    cmSystemTools::Error(
      cmStrCat("Error: Missing `:` after the operation in: ", envmod, '\n'));
    return false;
  }

  auto entry = Entry{};
  entry.Name = envmod.substr(0, eq_loc);
  entry.Op = envmod.substr(op_value_start, colon_loc - op_value_start);
  entry.Value = envmod.substr(colon_loc + 1);

  if (entry.Op == "reset") {
    cm::erase_if(this->Entries,
                 [&entry](Entry const& e) { return e.Name == entry.Name; });
    return true;
  }

  if (!cm::contains(ValidOperators, entry.Op)) {
    cmSystemTools::Error(cmStrCat(
      "Error: Unrecognized environment manipulation argument: ", entry.Op,
      '\n'));
    return false;
  }

  this->Entries.push_back(std::move(entry));
  return true;
}

void cmEnvironmentModification::ApplyTo(cmEnvironment& env)
{
  char const path_sep = cmSystemTools::GetSystemPathlistSeparator();

  for (auto const& e : this->Entries) {
    if (e.Op == "set") {
      env.PutEnv(e.Name + "=" + e.Value);
    } else if (e.Op == "unset") {
      env.UnPutEnv(e.Name);
    } else if (e.Op == "string_append") {
      env.Modify(e.Name, [&e](std::string& output) { output += e.Value; });
    } else if (e.Op == "string_prepend") {
      env.Modify(e.Name,
                 [&e](std::string& output) { output.insert(0, e.Value); });
    } else if (e.Op == "path_list_append") {
      env.Modify(e.Name, ListAppend(e.Value, path_sep));
    } else if (e.Op == "path_list_prepend") {
      env.Modify(e.Name, ListPrepend(e.Value, path_sep));
    } else if (e.Op == "cmake_list_append") {
      env.Modify(e.Name, ListAppend(e.Value, ';'));
    } else if (e.Op == "cmake_list_prepend") {
      env.Modify(e.Name, ListPrepend(e.Value, ';'));
    }
  }
}
