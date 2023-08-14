/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
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
  void SetProperty(const std::string& name, std::nullptr_t);
  void SetProperty(const std::string& name, cmValue value);
  void SetProperty(const std::string& name, const std::string& value)
  {
    this->SetProperty(name, cmValue(value));
  }

  //! Append to the property value
  void AppendProperty(const std::string& name, const std::string& value,
                      bool asString = false);

  //! Get the property value
  cmValue GetPropertyValue(const std::string& name) const;

  //! Remove the property @a name from the map
  void RemoveProperty(const std::string& name);

  // -- Lists

  //! Get a sorted list of property keys
  std::vector<std::string> GetKeys() const;

  //! Get a sorted by key list of property key,value pairs
  std::vector<std::pair<std::string, std::string>> GetList() const;

private:
  std::unordered_map<std::string, std::string> Map_;
};
