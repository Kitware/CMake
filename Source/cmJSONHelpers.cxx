/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConfigure.h" // IWYU pragma: keep

#include "cmJSONHelpers.h"

#include <functional>
#include <string>
#include <vector>

#include <cm3p/json/value.h>

#include "cmJSONState.h"

namespace JsonErrors {
ErrorGenerator EXPECTED_TYPE(const std::string& type)
{
  return [type](const Json::Value* value, cmJSONState* state) -> void {
    if (state->key().empty()) {
      state->AddErrorAtValue(cmStrCat("Expected ", type), value);
      return;
    }
    std::string errMsg = cmStrCat("\"", state->key(), "\" expected ", type);
    if (value && value->isConvertibleTo(Json::ValueType::stringValue)) {
      errMsg = cmStrCat(errMsg, ", got: ", value->asString());
    }
    state->AddErrorAtValue(errMsg, value);
  };
}

void INVALID_STRING(const Json::Value* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("a string")(value, state);
}

void INVALID_BOOL(const Json::Value* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("a bool")(value, state);
}

void INVALID_INT(const Json::Value* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("an integer")(value, state);
}

void INVALID_UINT(const Json::Value* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("an unsigned integer")(value, state);
}

ObjectErrorGenerator INVALID_NAMED_OBJECT(
  const std::function<std::string(const Json::Value*, cmJSONState*)>&
    nameGenerator)
{
  return [nameGenerator](
           ObjectError errorType,
           const Json::Value::Members& extraFields) -> ErrorGenerator {
    return [nameGenerator, errorType, extraFields](
             const Json::Value* value, cmJSONState* state) -> void {
      std::string name = nameGenerator(value, state);
      switch (errorType) {
        case ObjectError::RequiredMissing:
          state->AddErrorAtValue(cmStrCat("Invalid Required ", name), value);
          break;
        case ObjectError::InvalidObject:
          state->AddErrorAtValue(cmStrCat("Invalid ", name), value);
          break;
        case ObjectError::ExtraField: {
          for (auto const& member : extraFields) {
            if (value) {
              state->AddErrorAtValue(
                cmStrCat("Invalid extra field \"", member, "\" in ", name),
                &(*value)[member]);
            } else {
              state->AddError(
                cmStrCat("Invalid extra field \"", member, "\" in ", name));
            }
          }
        } break;
        case ObjectError::MissingRequired:
          state->AddErrorAtValue(cmStrCat("Missing required field \"",
                                          state->key(), "\" in ", name),
                                 value);
          break;
      }
    };
  };
}

ErrorGenerator INVALID_OBJECT(ObjectError errorType,
                              const Json::Value::Members& extraFields)
{
  return INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState*) -> std::string { return "Object"; })(
    errorType, extraFields);
}

ErrorGenerator INVALID_NAMED_OBJECT_KEY(
  ObjectError errorType, const Json::Value::Members& extraFields)
{
  return INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState* state) -> std::string {
      for (auto it = state->parseStack.rbegin();
           it != state->parseStack.rend(); ++it) {
        if (it->first.rfind("$vector_item_", 0) == 0) {
          continue;
        }
        return cmStrCat("\"", it->first, "\"");
      }
      return "root";
    })(errorType, extraFields);
}
}
