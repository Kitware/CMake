/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPropertyDefinition.h"

#include <utility>

cmPropertyDefinition::cmPropertyDefinition(std::string name,
                                           cmProperty::ScopeType scope,
                                           std::string shortDescription,
                                           std::string fullDescription,
                                           bool chain)
  : Name(std::move(name))
  , ShortDescription(std::move(shortDescription))
  , FullDescription(std::move(fullDescription))
  , Scope(scope)
  , Chained(chain)
{
}
