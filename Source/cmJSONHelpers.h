/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/json/value.h>

#include "cmJSONState.h"

template <typename T>
using cmJSONHelper =
  std::function<bool(T& out, const Json::Value* value, cmJSONState* state)>;

using ErrorGenerator = std::function<void(const Json::Value*, cmJSONState*)>;

namespace JsonErrors {
enum ObjectError
{
  RequiredMissing,
  InvalidObject,
  ExtraField,
  MissingRequired
};
using ErrorGenerator = std::function<void(const Json::Value*, cmJSONState*)>;
using ObjectErrorGenerator =
  std::function<ErrorGenerator(ObjectError, const Json::Value::Members&)>;
const auto EXPECTED_TYPE = [](const std::string& type) {
  return [type](const Json::Value* value, cmJSONState* state) -> void {
#if !defined(CMAKE_BOOTSTRAP)
    if (state->key().empty()) {
      state->AddErrorAtValue(cmStrCat("Expected ", type), value);
      return;
    }
    std::string errMsg = cmStrCat("\"", state->key(), "\" expected ", type);
    if (value && value->isConvertibleTo(Json::ValueType::stringValue)) {
      errMsg = cmStrCat(errMsg, ", got: ", value->asString());
    }
    state->AddErrorAtValue(errMsg, value);
#endif
  };
};
const auto INVALID_STRING = [](const Json::Value* value,
                               cmJSONState* state) -> void {
  JsonErrors::EXPECTED_TYPE("a string")(value, state);
};
const auto INVALID_BOOL = [](const Json::Value* value,
                             cmJSONState* state) -> void {
  JsonErrors::EXPECTED_TYPE("a bool")(value, state);
};
const auto INVALID_INT = [](const Json::Value* value,
                            cmJSONState* state) -> void {
  JsonErrors::EXPECTED_TYPE("an integer")(value, state);
};
const auto INVALID_UINT = [](const Json::Value* value,
                             cmJSONState* state) -> void {
  JsonErrors::EXPECTED_TYPE("an unsigned integer")(value, state);
};
const auto INVALID_NAMED_OBJECT =
  [](const std::function<std::string(const Json::Value*, cmJSONState*)>&
       nameGenerator) -> ObjectErrorGenerator {
  return [nameGenerator](
           ObjectError errorType,
           const Json::Value::Members& extraFields) -> ErrorGenerator {
    return [nameGenerator, errorType, extraFields](
             const Json::Value* value, cmJSONState* state) -> void {
#if !defined(CMAKE_BOOTSTRAP)
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
#endif
    };
  };
};
const auto INVALID_OBJECT =
  [](ObjectError errorType,
     const Json::Value::Members& extraFields) -> ErrorGenerator {
  return INVALID_NAMED_OBJECT(
    [](const Json::Value*, cmJSONState*) -> std::string { return "Object"; })(
    errorType, extraFields);
};
const auto INVALID_NAMED_OBJECT_KEY =
  [](ObjectError errorType,
     const Json::Value::Members& extraFields) -> ErrorGenerator {
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
};
}

struct cmJSONHelperBuilder
{

  template <typename T>
  class Object
  {
  public:
    Object(JsonErrors::ObjectErrorGenerator error = JsonErrors::INVALID_OBJECT,
           bool allowExtra = true)
      : Error(std::move(error))
      , AllowExtra(allowExtra)
    {
    }

    template <typename U, typename M, typename F>
    Object& Bind(const cm::string_view& name, M U::*member, F func,
                 bool required = true)
    {
      return this->BindPrivate(
        name,
        [func, member](T& out, const Json::Value* value, cmJSONState* state)
          -> bool { return func(out.*member, value, state); },
        required);
    }
    template <typename M, typename F>
    Object& Bind(const cm::string_view& name, std::nullptr_t, F func,
                 bool required = true)
    {
      return this->BindPrivate(
        name,
        [func](T& /*out*/, const Json::Value* value,
               cmJSONState* state) -> bool {
          M dummy;
          return func(dummy, value, state);
        },
        required);
    }
    template <typename F>
    Object& Bind(const cm::string_view& name, F func, bool required = true)
    {
      return this->BindPrivate(name, MemberFunction(func), required);
    }

    bool operator()(T& out, const Json::Value* value, cmJSONState* state) const
    {
      Json::Value::Members extraFields;
      bool success = true;
      if (!value && this->AnyRequired) {
        Error(JsonErrors::ObjectError::RequiredMissing, extraFields)(value,
                                                                     state);
        return false;
      }
      if (value && !value->isObject()) {
        Error(JsonErrors::ObjectError::InvalidObject, extraFields)(value,
                                                                   state);
        return false;
      }
      if (value) {
        extraFields = value->getMemberNames();
      }

      for (auto const& m : this->Members) {
        std::string name(m.Name.data(), m.Name.size());
        state->push_stack(name, value);
        if (value && value->isMember(name)) {
          if (!m.Function(out, &(*value)[name], state)) {
            success = false;
          }
          extraFields.erase(
            std::find(extraFields.begin(), extraFields.end(), name));
        } else if (!m.Required) {
          if (!m.Function(out, nullptr, state)) {
            success = false;
          }
        } else {
          Error(JsonErrors::ObjectError::MissingRequired, extraFields)(value,
                                                                       state);
          success = false;
        }
        state->pop_stack();
      }

      if (!this->AllowExtra && !extraFields.empty()) {
        Error(JsonErrors::ObjectError::ExtraField, extraFields)(value, state);
        success = false;
      }
      return success;
    }

  private:
    // Not a true cmJSONHelper, it just happens to match the signature
    using MemberFunction = std::function<bool(T& out, const Json::Value* value,
                                              cmJSONState* state)>;
    struct Member
    {
      cm::string_view Name;
      MemberFunction Function;
      bool Required;
    };
    std::vector<Member> Members;
    bool AnyRequired = false;
    JsonErrors::ObjectErrorGenerator Error;
    bool AllowExtra;

    Object& BindPrivate(const cm::string_view& name, MemberFunction&& func,
                        bool required)
    {
      Member m;
      m.Name = name;
      m.Function = std::move(func);
      m.Required = required;
      this->Members.push_back(std::move(m));
      if (required) {
        this->AnyRequired = true;
      }
      return *this;
    }
  };

  static cmJSONHelper<std::string> String(
    const JsonErrors::ErrorGenerator& error = JsonErrors::INVALID_STRING,
    const std::string& defval = "")
  {
    return [error, defval](std::string& out, const Json::Value* value,
                           cmJSONState* state) -> bool {
      if (!value) {
        out = defval;
        return true;
      }
      if (!value->isString()) {
        error(value, state);
        ;
        return false;
      }
      out = value->asString();
      return true;
    };
  };

  static cmJSONHelper<std::string> String(const std::string& defval)
  {
    return String(JsonErrors::INVALID_STRING, defval);
  };

  static cmJSONHelper<int> Int(
    const JsonErrors::ErrorGenerator& error = JsonErrors::INVALID_INT,
    int defval = 0)
  {
    return [error, defval](int& out, const Json::Value* value,
                           cmJSONState* state) -> bool {
      if (!value) {
        out = defval;
        return true;
      }
      if (!value->isInt()) {
        error(value, state);
        ;
        return false;
      }
      out = value->asInt();
      return true;
    };
  }

  static cmJSONHelper<int> Int(int defval)
  {
    return Int(JsonErrors::INVALID_INT, defval);
  };

  static cmJSONHelper<unsigned int> UInt(
    const JsonErrors::ErrorGenerator& error = JsonErrors::INVALID_UINT,
    unsigned int defval = 0)
  {
    return [error, defval](unsigned int& out, const Json::Value* value,
                           cmJSONState* state) -> bool {
      if (!value) {
        out = defval;
        return true;
      }
      if (!value->isUInt()) {
        error(value, state);
        ;
        return false;
      }
      out = value->asUInt();
      return true;
    };
  }

  static cmJSONHelper<unsigned int> UInt(unsigned int defval)
  {
    return UInt(JsonErrors::INVALID_UINT, defval);
  }

  static cmJSONHelper<bool> Bool(
    const JsonErrors::ErrorGenerator& error = JsonErrors::INVALID_BOOL,
    bool defval = false)
  {
    return [error, defval](bool& out, const Json::Value* value,
                           cmJSONState* state) -> bool {
      if (!value) {
        out = defval;
        return true;
      }
      if (!value->isBool()) {
        error(value, state);
        ;
        return false;
      }
      out = value->asBool();
      return true;
    };
  }

  static cmJSONHelper<bool> Bool(bool defval)
  {
    return Bool(JsonErrors::INVALID_BOOL, defval);
  }

  template <typename T, typename F, typename Filter>
  static cmJSONHelper<std::vector<T>> VectorFilter(
    const JsonErrors::ErrorGenerator& error, F func, Filter filter)
  {
    return [error, func, filter](std::vector<T>& out, const Json::Value* value,
                                 cmJSONState* state) -> bool {
      bool success = true;
      if (!value) {
        out.clear();
        return true;
      }
      if (!value->isArray()) {
        error(value, state);
        return false;
      }
      out.clear();
      int index = 0;
      for (auto const& item : *value) {
        state->push_stack(cmStrCat("$vector_item_", index++), &item);
        T t;
        if (!func(t, &item, state)) {
          success = false;
        }
        if (!filter(t)) {
          state->pop_stack();
          continue;
        }
        out.push_back(std::move(t));
        state->pop_stack();
      }
      return success;
    };
  }

  template <typename T, typename F>
  static cmJSONHelper<std::vector<T>> Vector(JsonErrors::ErrorGenerator error,
                                             F func)
  {
    return VectorFilter<T, F>(std::move(error), func,
                              [](const T&) { return true; });
  }

  template <typename T, typename F, typename Filter>
  static cmJSONHelper<std::map<std::string, T>> MapFilter(
    const JsonErrors::ErrorGenerator& error, F func, Filter filter)
  {
    return [error, func, filter](std::map<std::string, T>& out,
                                 const Json::Value* value,
                                 cmJSONState* state) -> bool {
      bool success = true;
      if (!value) {
        out.clear();
        return true;
      }
      if (!value->isObject()) {
        error(value, state);
        ;
        return false;
      }
      out.clear();
      for (auto const& key : value->getMemberNames()) {
        state->push_stack(cmStrCat(key, ""), &(*value)[key]);
        if (!filter(key)) {
          state->pop_stack();
          continue;
        }
        T t;
        if (!func(t, &(*value)[key], state)) {
          success = false;
        }
        out[key] = std::move(t);
        state->pop_stack();
      }
      return success;
    };
  }

  template <typename T, typename F>
  static cmJSONHelper<std::map<std::string, T>> Map(
    const JsonErrors::ErrorGenerator& error, F func)
  {
    return MapFilter<T, F>(error, func,
                           [](const std::string&) { return true; });
  }

  template <typename T, typename F>
  static cmJSONHelper<cm::optional<T>> Optional(F func)
  {
    return [func](cm::optional<T>& out, const Json::Value* value,
                  cmJSONState* state) -> bool {
      if (!value) {
        out.reset();
        return true;
      }
      out.emplace();
      return func(*out, value, state);
    };
  }

  template <typename T, typename F>
  static cmJSONHelper<T> Required(const JsonErrors::ErrorGenerator& error,
                                  F func)
  {
    return [error, func](T& out, const Json::Value* value,
                         cmJSONState* state) -> bool {
      if (!value) {
        error(value, state);
        ;
        return false;
      }
      return func(out, value, state);
    };
  }
};
