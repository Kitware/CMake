/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <utility>

#include "cmProperty.h"

/** \class cmPropertyDefinition
 * \brief Property meta-information
 *
 * This class contains the following meta-information about property:
 * - Various documentation strings;
 * - If the property is chained.
 */
class cmPropertyDefinition
{
public:
  /// Constructor
  cmPropertyDefinition(std::string shortDescription,
                       std::string fullDescription, bool chained,
                       std::string initializeFromVariable);

  /// Is the property chained?
  bool IsChained() const { return this->Chained; }

  /// Get the documentation (short version)
  std::string const& GetShortDescription() const
  {
    return this->ShortDescription;
  }

  /// Get the documentation (full version)
  std::string const& GetFullDescription() const
  {
    return this->FullDescription;
  }

  /// Get the variable the property is initialized from
  std::string const& GetInitializeFromVariable() const
  {
    return this->InitializeFromVariable;
  }

private:
  std::string ShortDescription;
  std::string FullDescription;
  bool Chained;
  std::string InitializeFromVariable;
};

/** \class cmPropertyDefinitionMap
 * \brief Map property name and scope to their definition
 */
class cmPropertyDefinitionMap
{
public:
  // define the property
  void DefineProperty(std::string const& name, cmProperty::ScopeType scope,
                      std::string const& ShortDescription,
                      std::string const& FullDescription, bool chain,
                      std::string const& initializeFromVariable);

  // get the property definition if present, otherwise nullptr
  cmPropertyDefinition const* GetPropertyDefinition(
    std::string const& name, cmProperty::ScopeType scope) const;

  using KeyType = std::pair<std::string, cmProperty::ScopeType>;
  std::map<KeyType, cmPropertyDefinition> const& GetMap() const
  {
    return this->Map_;
  }

private:
  std::map<KeyType, cmPropertyDefinition> Map_;
};
