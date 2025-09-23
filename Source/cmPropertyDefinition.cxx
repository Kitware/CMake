/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPropertyDefinition.h"

#include <tuple>

cmPropertyDefinition::cmPropertyDefinition(std::string shortDescription,
                                           std::string fullDescription,
                                           bool chained,
                                           std::string initializeFromVariable)
  : ShortDescription(std::move(shortDescription))
  , FullDescription(std::move(fullDescription))
  , Chained(chained)
  , InitializeFromVariable(std::move(initializeFromVariable))
{
}

void cmPropertyDefinitionMap::DefineProperty(
  std::string const& name, cmProperty::ScopeType scope,
  std::string const& ShortDescription, std::string const& FullDescription,
  bool chain, std::string const& initializeFromVariable)
{
  auto it = this->Map_.find(KeyType(name, scope));
  if (it == this->Map_.end()) {
    // try_emplace() since C++17
    this->Map_.emplace(std::piecewise_construct,
                       std::forward_as_tuple(name, scope),
                       std::forward_as_tuple(ShortDescription, FullDescription,
                                             chain, initializeFromVariable));
  }
}

cmPropertyDefinition const* cmPropertyDefinitionMap::GetPropertyDefinition(
  std::string const& name, cmProperty::ScopeType scope) const
{
  auto it = this->Map_.find(KeyType(name, scope));
  if (it != this->Map_.end()) {
    return &it->second;
  }

  return nullptr;
}
