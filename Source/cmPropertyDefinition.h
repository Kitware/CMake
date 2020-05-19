/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmPropertyDefinition_h
#define cmPropertyDefinition_h

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
                       std::string fullDescription, bool chained);

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

private:
  std::string ShortDescription;
  std::string FullDescription;
  bool Chained;
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
                      const std::string& FullDescription, bool chain);

  // get the property definition if present, otherwise nullptr
  cmPropertyDefinition const* GetPropertyDefinition(
    const std::string& name, cmProperty::ScopeType scope) const;

private:
  using key_type = std::pair<std::string, cmProperty::ScopeType>;
  std::map<key_type, cmPropertyDefinition> Map_;
};

#endif
