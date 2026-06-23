/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <utility>

#include <cm/optional>

#include "cmPolicies.h"

class cmLocalGenerator;

namespace cm {
namespace GenEx {

struct Context final
{
  Context(cmLocalGenerator const* lg, std::string config,
          std::string language = std::string());

  cmLocalGenerator const* LG;
  std::string Config;
  std::string Language;

  void SetCMP0189(cmPolicies::PolicyStatus cmp0189);
  cmPolicies::PolicyStatus GetCMP0189() const;

  void SetBoundOperand(std::string value);
  bool HasBoundOperand() const;
  std::string const& GetBoundOperand() const;

private:
  cm::optional<cmPolicies::PolicyStatus> CMP0189;
  cm::optional<std::string> BoundOperand;
};

inline void Context::SetBoundOperand(std::string value)
{
  this->BoundOperand = std::move(value);
}
inline bool Context::HasBoundOperand() const
{
  return this->BoundOperand.has_value();
}
inline std::string const& Context::GetBoundOperand() const
{
  return *this->BoundOperand;
}
}
}
