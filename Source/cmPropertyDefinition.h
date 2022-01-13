/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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
  const std::string& GetShortDescription() const
  {
    return this->ShortDescription;
  }

  /// Get the documentation (full version)
  const std::string& GetFullDescription() const
  {
    return this->FullDescription;
  }

  /// Get the variable the property is initialized from
  const std::string& GetInitializeFromVariable() const
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
  void DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                      const std::string& ShortDescription,
                      const std::string& FullDescription, bool chain,
                      const std::string& initializeFromVariable);

  // get the property definition if present, otherwise nullptr
  cmPropertyDefinition const* GetPropertyDefinition(
    const std::string& name, cmProperty::ScopeType scope) const;

  using KeyType = std::pair<std::string, cmProperty::ScopeType>;
  const std::map<KeyType, cmPropertyDefinition>& GetMap() const
  {
    return this->Map_;
  }

private:
  std::map<KeyType, cmPropertyDefinition> Map_;
};
