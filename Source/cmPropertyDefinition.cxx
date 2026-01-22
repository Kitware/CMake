/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPropertyDefinition.h"

#include <tuple>
#include <utility>

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
  auto it = this->Map_.find(name);
  if (it == this->Map_.end()) {
    it = this->Map_.emplace(name, ScopeMap()).first;
  }
  ScopeMap& scopeMap = it->second;
  auto scopeIter = scopeMap.find(scope);
  if (scopeIter == scopeMap.end()) {
    // try_emplace() since C++17
    scopeMap.emplace(std::piecewise_construct, std::forward_as_tuple(scope),
                     std::forward_as_tuple(ShortDescription, FullDescription,
                                           chain, initializeFromVariable));
  }
}

cmPropertyDefinition const* cmPropertyDefinitionMap::GetPropertyDefinition(
  std::string const& name, cmProperty::ScopeType scope) const
{
  auto nameIter = this->Map_.find(name);
  if (nameIter != this->Map_.end()) {
    auto scopeIter = nameIter->second.find(scope);
    if (scopeIter != nameIter->second.end()) {
      return &scopeIter->second;
    }
  }

  return nullptr;
}
