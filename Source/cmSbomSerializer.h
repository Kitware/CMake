/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include <cm/optional>

class cmSbomObject;
class cmSbomSerializer
{
public:
  cmSbomSerializer() = default;
  cmSbomSerializer(cmSbomSerializer const&) = default;
  cmSbomSerializer(cmSbomSerializer&&) = default;
  cmSbomSerializer& operator=(cmSbomSerializer const&) = default;
  cmSbomSerializer& operator=(cmSbomSerializer&&) = default;

  virtual void BeginObject() {}
  virtual void EndObject() {}
  virtual void BeginArray() {}
  virtual void EndArray() {}

  virtual void AddReference(std::string const& id) = 0;

  virtual void AddString(std::string const& key, std::string const& value) = 0;
  virtual void AddVisitable(std::string const& key,
                            cmSbomObject const& visitable) = 0;

  virtual void AddVectorIfPresent(std::string const& key,
                                  std::vector<cmSbomObject> const& vec) = 0;
  virtual void AddVectorIfPresent(std::string const& key,
                                  std::vector<std::string> const& vec) = 0;

  virtual bool WriteSbom(std::ostream& os, cmSbomObject const& document) = 0;
  virtual ~cmSbomSerializer() = default;
};
