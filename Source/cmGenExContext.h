/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

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

  void SetBoundOperands(std::vector<std::string> operands);
  void SetBoundOperand(std::string value);
  std::size_t BoundOperandCount() const;
  bool HasBoundOperand(std::size_t index = 0) const;
  std::string const& GetBoundOperand(std::size_t index = 0) const;

private:
  cm::optional<cmPolicies::PolicyStatus> CMP0189;
  std::vector<std::string> BoundOperands;
};

inline void Context::SetBoundOperands(std::vector<std::string> operands)
{
  this->BoundOperands = std::move(operands);
}
inline void Context::SetBoundOperand(std::string value)
{
  this->SetBoundOperands({ std::move(value) });
}
inline std::size_t Context::BoundOperandCount() const
{
  return this->BoundOperands.size();
}
inline bool Context::HasBoundOperand(std::size_t index) const
{
  return index < this->BoundOperandCount();
}
inline std::string const& Context::GetBoundOperand(std::size_t index) const
{
  return this->BoundOperands[index];
}
}
}
