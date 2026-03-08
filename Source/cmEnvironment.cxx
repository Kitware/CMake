/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmEnvironment.h"

#include <utility>

#include <cm/string_view>
#include <cm/vector>
#include <cmext/algorithm>

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
