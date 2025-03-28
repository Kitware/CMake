/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/json/value.h>

#include "cmJSONState.h"
#include "cmStringAlgorithms.h"

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

ErrorGenerator EXPECTED_TYPE(const std::string& type);

void INVALID_STRING(const Json::Value* value, cmJSONState* state);

void INVALID_BOOL(const Json::Value* value, cmJSONState* state);

void INVALID_INT(const Json::Value* value, cmJSONState* state);

void INVALID_UINT(const Json::Value* value, cmJSONState* state);

ObjectErrorGenerator INVALID_NAMED_OBJECT(
  const std::function<std::string(const Json::Value*, cmJSONState*)>&
    nameGenerator);

ErrorGenerator INVALID_OBJECT(ObjectError errorType,
                              const Json::Value::Members& extraFields);

ErrorGenerator INVALID_NAMED_OBJECT_KEY(
  ObjectError errorType, const Json::Value::Members& extraFields);
}

#if __cplusplus >= 201703L
namespace details {
// A meta-function to check if a given callable type
// can be called with the only string ref arg.
template <typename F, typename Enable = void>
struct is_bool_filter
{
  static constexpr bool value = false;
};

template <typename F>
struct is_bool_filter<F,
                      std::enable_if_t<std::is_same_v<
                        std::invoke_result_t<F, const std::string&>, bool>>>
{
  static constexpr bool value = true;
};
}
#endif

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

      if (state->allowComments) {
        extraFields.erase(
          std::remove(extraFields.begin(), extraFields.end(), "$comment"),
          extraFields.end());
      }

      bool success = true;
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
      Member(cm::string_view name, MemberFunction func, bool required)
        : Name{ name }
        , Function{ std::move(func) }
        , Required{ required }
      {
      }
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
      this->Members.emplace_back(name, std::move(func), required);
      this->AnyRequired = this->AnyRequired || required;
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

  enum class FilterResult
  {
    Continue, ///< A filter has accepted a given key (and value)
    Skip,     ///< A filter has rejected a given key (or value)
    Error     ///< A filter has found and reported an error
  };

  /// Iterate over the object's members and call a filter callable to
  /// decide what to do with the current key/value.
  /// A filter returns one of the `FilterResult` values.
  /// A container type is an associative or a sequence
  /// container of pairs (key, value).
  template <typename Container, typename F, typename Filter>
  static cmJSONHelper<Container> FilteredObject(
    const JsonErrors::ErrorGenerator& error, F func, Filter filter)
  {
    return [error, func, filter](Container& out, const Json::Value* value,
                                 cmJSONState* state) -> bool {
      // NOTE Some compile-time code path don't use `filter` at all.
      // So, suppress "unused lambda capture" warning is needed.
      static_cast<void>(filter);

      if (!value) {
        out.clear();
        return true;
      }
      if (!value->isObject()) {
        error(value, state);
        return false;
      }
      out.clear();
      auto outIt = std::inserter(out, out.end());
      bool success = true;
      for (auto const& key : value->getMemberNames()) {
        state->push_stack(key, &(*value)[key]);
#if __cplusplus >= 201703L
        if constexpr (std::is_same_v<Filter, std::true_type>) {
          // Filtering functionality isn't needed at all...
        } else if constexpr (details::is_bool_filter<Filter>::value) {
          // A given `Filter` is `bool(const std::string&)` callable.
          if (!filter(key)) {
            state->pop_stack();
            continue;
          }
        } else {
#endif
          // A full-featured `Filter` has been given
          auto res = filter(key, &(*value)[key], state);
          if (res == FilterResult::Skip) {
            state->pop_stack();
            continue;
          }
          if (res == FilterResult::Error) {
            state->pop_stack();
            success = false;
            break;
          }
#if __cplusplus >= 201703L
        }
#endif
        typename Container::value_type::second_type t;
        // ATTENTION Call the function first (for it's side-effects),
        // then accumulate the result!
        success = func(t, &(*value)[key], state) && success;
        outIt = typename Container::value_type{ key, std::move(t) };
        state->pop_stack();
      }
      return success;
    };
  }

  template <typename T, typename F, typename Filter>
  static cmJSONHelper<std::map<std::string, T>> MapFilter(
    const JsonErrors::ErrorGenerator& error, F func, Filter filter)
  {
    // clang-format off
    return FilteredObject<std::map<std::string, T>>(
      error, func,
#if __cplusplus >= 201703L
      // In C++ 17 a filter callable can be passed as is.
      // Depending on its type `FilteredObject()` will call
      // it with a key only (backward compatible behavior)
      // or with 3 args supported by the full-featured
      // filtering feature.
      filter
#else
      // For C++14 and below, to keep backward compatibility
      // with CMake Presets code, `MapFilter()` can accept only
      // `bool(const std::string&)` callables.
      [filter](const std::string &key, const Json::Value * /*value*/,
              cmJSONState * /*state*/) -> FilterResult {
        // Simple adaptor to translate `bool` to `FilterResult`
        return filter(key) ? FilterResult::Continue : FilterResult::Skip;
      }
#endif
    );
    // clang-format on
  }

  template <typename T, typename F>
  static cmJSONHelper<std::map<std::string, T>> Map(
    const JsonErrors::ErrorGenerator& error, F func)
  {
    // clang-format off
    return FilteredObject<std::map<std::string, T>>(
      error, func,
#if __cplusplus >= 201703L
      // With C++ 17 and above, pass a marker type, that no
      // filtering is needed at all.
      std::true_type()
#else
      // In C++ 14 and below, pass an always-true dummy functor.
      [](const std::string& /*key*/, const Json::Value* /*value*/,
         cmJSONState* /*state*/) -> FilterResult {
        return FilterResult::Continue;
      }
#endif
    );
    // clang-format on
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
        return false;
      }
      return func(out, value, state);
    };
  }

  template <typename T, typename F, typename P>
  static cmJSONHelper<T> Checked(const JsonErrors::ErrorGenerator& error,
                                 F func, P predicate)
  {
    return [error, func, predicate](T& out, const Json::Value* value,
                                    cmJSONState* state) -> bool {
      bool result = func(out, value, state);
      if (result && !predicate(out)) {
        error(value, state);
        result = false;
      }
      return result;
    };
  }
};
