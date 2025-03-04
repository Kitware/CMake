/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmConfigure.h" // IWYU pragma: keep

#include "cmJSONHelpers.h"

#include <functional>
#include <string>
#include <vector>

#include <cm3p/json/value.h>

#include "cmJSONState.h"

namespace JsonErrors {
ErrorGenerator EXPECTED_TYPE(std::string const& type)
{
  return [type](Json::Value const* value, cmJSONState* state) -> void {
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

void INVALID_STRING(Json::Value const* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("a string")(value, state);
}

void INVALID_BOOL(Json::Value const* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("a bool")(value, state);
}

void INVALID_INT(Json::Value const* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("an integer")(value, state);
}

void INVALID_UINT(Json::Value const* value, cmJSONState* state)
{
  JsonErrors::EXPECTED_TYPE("an unsigned integer")(value, state);
}

ObjectErrorGenerator INVALID_NAMED_OBJECT(
  std::function<std::string(Json::Value const*, cmJSONState*)> const&
    nameGenerator)
{
  return [nameGenerator](
           ObjectError errorType,
           Json::Value::Members const& extraFields) -> ErrorGenerator {
    return [nameGenerator, errorType, extraFields](
             Json::Value const* value, cmJSONState* state) -> void {
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
                              Json::Value::Members const& extraFields)
{
  return INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState*) -> std::string { return "Object"; })(
    errorType, extraFields);
}

ErrorGenerator INVALID_NAMED_OBJECT_KEY(
  ObjectError errorType, Json::Value::Members const& extraFields)
{
  return INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState* state) -> std::string {
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
