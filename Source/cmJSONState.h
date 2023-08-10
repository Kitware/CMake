/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "cmJSONState.h"
#include "cmStringAlgorithms.h"

namespace Json {
class Value;
}

class cmJSONState
{
  using Location = struct
  {
    int line;
    int column;
  };

public:
  using JsonPair = std::pair<const std::string, const Json::Value*>;
  cmJSONState() = default;
  cmJSONState(const std::string& filename, Json::Value* root);
  void AddError(std::string const& errMsg);
  void AddErrorAtValue(std::string const& errMsg, const Json::Value* value);
  void AddErrorAtOffset(std::string const& errMsg, std::ptrdiff_t offset);
  std::string GetErrorMessage(bool showContext = true);
  std::string key();
  std::string key_after(std::string const& key);
  const Json::Value* value_after(std::string const& key);
  void push_stack(std::string const& key, const Json::Value* value);
  void pop_stack();

  class Error
  {
  public:
    Error(Location loc, std::string errMsg)
      : location(loc)
      , message(std::move(errMsg)){};
    Error(std::string errMsg)
      : location({ -1, -1 })
      , message(std::move(errMsg)){};
    std::string GetErrorMessage() const
    {
      std::string output = message;
      if (location.line > 0) {
        output = cmStrCat("Error: @", location.line, ",", location.column,
                          ": ", output);
      }
      return output;
    }
    Location GetLocation() const { return location; }

  private:
    Location location;
    std::string message;
  };

  std::vector<JsonPair> parseStack;
  std::vector<Error> errors;
  std::string doc;

private:
  std::string GetJsonContext(Location loc);
  Location LocateInDocument(ptrdiff_t offset);
};
