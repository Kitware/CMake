/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <ostream>
#include <string>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmSbomSerializer.h"

struct cmSpdxSerializer final : cmSbomSerializer
{
  cmSpdxSerializer();

  ~cmSpdxSerializer() override = default;

  void BeginObject() override;

  void BeginArray() override;

  void EndObject() override;

  void EndArray() override;

  void AddReference(std::string const& id) override;

  void AddString(std::string const& key, std::string const& value) override;

  void AddVisitable(std::string const& key,
                    cmSbomObject const& visitable) override;

  void AddVectorIfPresent(std::string const& key,
                          std::vector<cmSbomObject> const& vec) override;

  void AddVectorIfPresent(std::string const& key,
                          std::vector<std::string> const& vec) override;

  bool WriteSbom(std::ostream& os, cmSbomObject const& document) override;

  Json::Value GetJson() { return CurrentValue; }

private:
  Json::Value CurrentValue;
  std::string CurrentKey;
  std::unique_ptr<Json::StreamWriter> Writer;
};
