/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <string>

#include <cm/string_view>

class cmProperty
{
public:
  enum ScopeType
  {
    TARGET,
    SOURCE_FILE,
    DIRECTORY,
    GLOBAL,
    CACHE,
    TEST,
    VARIABLE,
    CACHED_VARIABLE,
    INSTALL
  };
};

class cmProp
{
public:
  cmProp() noexcept = default;
  cmProp(std::nullptr_t) noexcept {}
  explicit cmProp(const std::string* value) noexcept
    : Value(value)
  {
  }
  explicit cmProp(const std::string& value) noexcept
    : Value(&value)
  {
  }
  cmProp(const cmProp& other) noexcept = default;

  cmProp& operator=(const cmProp& other) noexcept = default;
  cmProp& operator=(std::nullptr_t) noexcept
  {
    this->Value = nullptr;
    return *this;
  }

  const std::string* Get() const noexcept { return this->Value; }
  const char* GetCStr() const noexcept
  {
    return this->Value == nullptr ? nullptr : this->Value->c_str();
  }

  const std::string* operator->() const noexcept
  {
    return this->Value == nullptr ? &cmProp::Empty : this->Value;
  }
  const std::string& operator*() const noexcept
  {
    return this->Value == nullptr ? cmProp::Empty : *this->Value;
  }

  explicit operator bool() const noexcept { return this->Value != nullptr; }
  operator const std::string&() const noexcept { return this->operator*(); }
  operator cm::string_view() const noexcept { return this->operator*(); }

  /**
   * Does the value indicate a true or ON value?
   */
  bool IsOn() const noexcept
  {
    return this->Value != nullptr &&
      cmProp::IsOn(cm::string_view(*this->Value));
  }
  /**
   * Does the value indicate a false or off value ? Note that this is
   * not the same as !IsOn(...) because there are a number of
   * ambiguous values such as "/usr/local/bin" a path will result in
   * IsOn and IsOff both returning false. Note that the special path
   * NOTFOUND, *-NOTFOUND or IGNORE will cause IsOff to return true.
   */
  bool IsOff() const noexcept
  {
    return this->Value == nullptr ||
      cmProp::IsOff(cm::string_view(*this->Value));
  }
  /** Return true if value is NOTFOUND or ends in -NOTFOUND.  */
  bool IsNOTFOUND() const noexcept
  {
    return this->Value != nullptr &&
      cmProp::IsNOTFOUND(cm::string_view(*this->Value));
  }
  bool IsEmpty() const noexcept
  {
    return this->Value == nullptr || this->Value->empty();
  }

  bool IsSet() const noexcept
  {
    return !this->IsEmpty() && !this->IsNOTFOUND();
  }

  /**
   * Does a string indicate a true or ON value?
   */
  static bool IsOn(const char* value) noexcept
  {
    return value != nullptr && IsOn(cm::string_view(value));
  }
  static bool IsOn(cm::string_view) noexcept;

  /**
   * Compare method has same semantic as std::optional::compare
   */
  int Compare(cmProp value) const noexcept;
  int Compare(cm::string_view value) const noexcept;

  /**
   * Does a string indicate a false or off value ? Note that this is
   * not the same as !IsOn(...) because there are a number of
   * ambiguous values such as "/usr/local/bin" a path will result in
   * IsOn and IsOff both returning false. Note that the special path
   * NOTFOUND, *-NOTFOUND or IGNORE will cause IsOff to return true.
   */
  static bool IsOff(const char* value) noexcept
  {
    return value == nullptr || IsOff(cm::string_view(value));
  }
  static bool IsOff(cm::string_view) noexcept;

  /** Return true if value is NOTFOUND or ends in -NOTFOUND.  */
  static bool IsNOTFOUND(const char* value) noexcept
  {
    return value == nullptr || IsNOTFOUND(cm::string_view(value));
  }
  static bool IsNOTFOUND(cm::string_view) noexcept;

  static bool IsEmpty(const char* value) noexcept
  {
    return value == nullptr || *value == '\0';
  }
  static bool IsEmpty(cm::string_view value) noexcept { return value.empty(); }

private:
  static std::string Empty;
  const std::string* Value = nullptr;
};

std::ostream& operator<<(std::ostream& o, cmProp v);

inline bool operator==(cmProp l, cmProp r) noexcept
{
  return l.Compare(r) == 0;
}
inline bool operator!=(cmProp l, cmProp r) noexcept
{
  return l.Compare(r) != 0;
}
inline bool operator<(cmProp l, cmProp r) noexcept
{
  return l.Compare(r) < 0;
}
inline bool operator<=(cmProp l, cmProp r) noexcept
{
  return l.Compare(r) <= 0;
}
inline bool operator>(cmProp l, cmProp r) noexcept
{
  return l.Compare(r) > 0;
}
inline bool operator>=(cmProp l, cmProp r) noexcept
{
  return l.Compare(r) >= 0;
}

inline bool operator==(cmProp l, cm::string_view r) noexcept
{
  return l.Compare(r) == 0;
}
inline bool operator!=(cmProp l, cm::string_view r) noexcept
{
  return l.Compare(r) != 0;
}
inline bool operator<(cmProp l, cm::string_view r) noexcept
{
  return l.Compare(r) < 0;
}
inline bool operator<=(cmProp l, cm::string_view r) noexcept
{
  return l.Compare(r) <= 0;
}
inline bool operator>(cmProp l, cm::string_view r) noexcept
{
  return l.Compare(r) > 0;
}
inline bool operator>=(cmProp l, cm::string_view r) noexcept
{
  return l.Compare(r) >= 0;
}

inline bool operator==(cmProp l, std::nullptr_t) noexcept
{
  return l.Compare(cmProp{}) == 0;
}
inline bool operator!=(cmProp l, std::nullptr_t) noexcept
{
  return l.Compare(cmProp{}) != 0;
}
inline bool operator<(cmProp l, std::nullptr_t) noexcept
{
  return l.Compare(cmProp{}) < 0;
}
inline bool operator<=(cmProp l, std::nullptr_t) noexcept
{
  return l.Compare(cmProp{}) <= 0;
}
inline bool operator>(cmProp l, std::nullptr_t) noexcept
{
  return l.Compare(cmProp{}) > 0;
}
inline bool operator>=(cmProp l, std::nullptr_t) noexcept
{
  return l.Compare(cmProp{}) >= 0;
}

/**
 * Temporary wrapper
 */
inline const char* cmToCStr(cmProp p)
{
  return p.GetCStr();
}
