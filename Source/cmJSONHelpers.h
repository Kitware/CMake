/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/json/value.h>

template <typename T, typename E, typename... CallState>
using cmJSONHelper =
  std::function<E(T& out, const Json::Value* value, CallState&&... state)>;

template <typename E, typename... CallState>
struct cmJSONHelperBuilder
{
  template <typename T>
  class Object
  {
  public:
    Object(E&& success, E&& fail, bool allowExtra = true)
      : Success(std::move(success))
      , Fail(std::move(fail))
      , AllowExtra(allowExtra)
    {
    }

    template <typename U, typename M, typename F>
    Object& Bind(const cm::string_view& name, M U::*member, F func,
                 bool required = true)
    {
      return this->BindPrivate(name,
                               [func, member](T& out, const Json::Value* value,
                                              CallState&&... state) -> E {
                                 return func(out.*member, value,
                                             std::forward(state)...);
                               },
                               required);
    }
    template <typename M, typename F>
    Object& Bind(const cm::string_view& name, std::nullptr_t, F func,
                 bool required = true)
    {
      return this->BindPrivate(name,
                               [func](T& /*out*/, const Json::Value* value,
                                      CallState&&... state) -> E {
                                 M dummy;
                                 return func(dummy, value,
                                             std::forward(state)...);
                               },
                               required);
    }
    template <typename F>
    Object& Bind(const cm::string_view& name, F func, bool required = true)
    {
      return this->BindPrivate(name, MemberFunction(func), required);
    }

    E operator()(T& out, const Json::Value* value, CallState&&... state) const
    {
      if (!value && this->AnyRequired) {
        return this->Fail;
      }
      if (value && !value->isObject()) {
        return this->Fail;
      }
      Json::Value::Members extraFields;
      if (value) {
        extraFields = value->getMemberNames();
      }

      for (auto const& m : this->Members) {
        std::string name(m.Name.data(), m.Name.size());
        if (value && value->isMember(name)) {
          E result = m.Function(out, &(*value)[name], std::forward(state)...);
          if (result != this->Success) {
            return result;
          }
          extraFields.erase(
            std::find(extraFields.begin(), extraFields.end(), name));
        } else if (!m.Required) {
          E result = m.Function(out, nullptr, std::forward(state)...);
          if (result != this->Success) {
            return result;
          }
        } else {
          return this->Fail;
        }
      }

      return this->AllowExtra || extraFields.empty() ? this->Success
                                                     : this->Fail;
    }

  private:
    // Not a true cmJSONHelper, it just happens to match the signature
    using MemberFunction =
      std::function<E(T& out, const Json::Value* value, CallState&&... state)>;
    struct Member
    {
      cm::string_view Name;
      MemberFunction Function;
      bool Required;
    };
    std::vector<Member> Members;
    bool AnyRequired = false;
    E Success;
    E Fail;
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
  static cmJSONHelper<std::string, E, CallState...> String(
    E success, E fail, const std::string& defval = "")
  {
    return [success, fail, defval](std::string& out, const Json::Value* value,
                                   CallState&&... /*state*/) -> E {
      if (!value) {
        out = defval;
        return success;
      }
      if (!value->isString()) {
        return fail;
      }
      out = value->asString();
      return success;
    };
  }

  static cmJSONHelper<int, E, CallState...> Int(E success, E fail,
                                                int defval = 0)
  {
    return [success, fail, defval](int& out, const Json::Value* value,
                                   CallState&&... /*state*/) -> E {
      if (!value) {
        out = defval;
        return success;
      }
      if (!value->isInt()) {
        return fail;
      }
      out = value->asInt();
      return success;
    };
  }

  static cmJSONHelper<unsigned int, E, CallState...> UInt(
    E success, E fail, unsigned int defval = 0)
  {
    return [success, fail, defval](unsigned int& out, const Json::Value* value,
                                   CallState&&... /*state*/) -> E {
      if (!value) {
        out = defval;
        return success;
      }
      if (!value->isUInt()) {
        return fail;
      }
      out = value->asUInt();
      return success;
    };
  }

  static cmJSONHelper<bool, E, CallState...> Bool(E success, E fail,
                                                  bool defval = false)
  {
    return [success, fail, defval](bool& out, const Json::Value* value,
                                   CallState&&... /*state*/) -> E {
      if (!value) {
        out = defval;
        return success;
      }
      if (!value->isBool()) {
        return fail;
      }
      out = value->asBool();
      return success;
    };
  }

  template <typename T, typename F, typename Filter>
  static cmJSONHelper<std::vector<T>, E, CallState...> VectorFilter(
    E success, E fail, F func, Filter filter)
  {
    return [success, fail, func, filter](std::vector<T>& out,
                                         const Json::Value* value,
                                         CallState&&... state) -> E {
      if (!value) {
        out.clear();
        return success;
      }
      if (!value->isArray()) {
        return fail;
      }
      out.clear();
      for (auto const& item : *value) {
        T t;
        E result = func(t, &item, std::forward(state)...);
        if (result != success) {
          return result;
        }
        if (!filter(t)) {
          continue;
        }
        out.push_back(std::move(t));
      }
      return success;
    };
  }

  template <typename T, typename F>
  static cmJSONHelper<std::vector<T>, E, CallState...> Vector(E success,
                                                              E fail, F func)
  {
    return VectorFilter<T, F>(success, fail, func,
                              [](const T&) { return true; });
  }

  template <typename T, typename F, typename Filter>
  static cmJSONHelper<std::map<std::string, T>, E, CallState...> MapFilter(
    E success, E fail, F func, Filter filter)
  {
    return [success, fail, func, filter](std::map<std::string, T>& out,
                                         const Json::Value* value,
                                         CallState&&... state) -> E {
      if (!value) {
        out.clear();
        return success;
      }
      if (!value->isObject()) {
        return fail;
      }
      out.clear();
      for (auto const& key : value->getMemberNames()) {
        if (!filter(key)) {
          continue;
        }
        T t;
        E result = func(t, &(*value)[key], std::forward(state)...);
        if (result != success) {
          return result;
        }
        out[key] = std::move(t);
      }
      return success;
    };
  }

  template <typename T, typename F>
  static cmJSONHelper<std::map<std::string, T>, E, CallState...> Map(E success,
                                                                     E fail,
                                                                     F func)
  {
    return MapFilter<T, F>(success, fail, func,
                           [](const std::string&) { return true; });
  }

  template <typename T, typename F>
  static cmJSONHelper<cm::optional<T>, E, CallState...> Optional(E success,
                                                                 F func)
  {
    return [success, func](cm::optional<T>& out, const Json::Value* value,
                           CallState&&... state) -> E {
      if (!value) {
        out.reset();
        return success;
      }
      out.emplace();
      return func(*out, value, std::forward(state)...);
    };
  }

  template <typename T, typename F>
  static cmJSONHelper<T, E, CallState...> Required(E fail, F func)
  {
    return [fail, func](T& out, const Json::Value* value,
                        CallState&&... state) -> E {
      if (!value) {
        return fail;
      }
      return func(out, value, std::forward(state)...);
    };
  }
};
