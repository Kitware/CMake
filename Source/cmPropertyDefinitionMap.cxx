/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPropertyDefinitionMap.h"

#include <tuple>
#include <utility>

void cmPropertyDefinitionMap::DefineProperty(
  const std::string& name, cmProperty::ScopeType scope,
  const std::string& ShortDescription, const std::string& FullDescription,
  bool chain)
{
  auto it = this->find(name);
  if (it == this->end()) {
    // try_emplace() since C++17
    this->emplace(std::piecewise_construct, std::forward_as_tuple(name),
                  std::forward_as_tuple(name, scope, ShortDescription,
                                        FullDescription, chain));
  }
}

bool cmPropertyDefinitionMap::IsPropertyDefined(const std::string& name) const
{
  return this->find(name) != this->end();
}

bool cmPropertyDefinitionMap::IsPropertyChained(const std::string& name) const
{
  auto it = this->find(name);
  if (it == this->end()) {
    return false;
  }

  return it->second.IsChained();
}
