/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cmValue.h"

/** \class cmPropertyMap
 * \brief String property map.
 */
class cmPropertyMap
{
public:
  // -- General

  //! Clear property list
  void Clear();

  // -- Properties

  //! Set the property value
  void SetProperty(std::string const& name, cmValue value);
  void SetProperty(std::string const& name, std::string const& value)
  {
    this->SetProperty(name, cmValue(value));
  }

  //! Append to the property value
  void AppendProperty(std::string const& name, std::string const& value,
                      bool asString = false);

  //! Get the property value
  cmValue GetPropertyValue(std::string const& name) const;

  //! Remove the property @a name from the map
  void RemoveProperty(std::string const& name);

  // -- Lists

  //! Get a sorted list of property keys
  std::vector<std::string> GetKeys() const;

  //! Get a sorted by key list of property key,value pairs
  std::vector<std::pair<std::string, std::string>> GetList() const;

private:
  std::unordered_map<std::string, std::string> Map_;
};
