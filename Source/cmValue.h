/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <string>

#include <cm/string_view>

class cmValue
{
public:
  cmValue() noexcept = default;
  cmValue(std::nullptr_t) noexcept {}
  explicit cmValue(const std::string* value) noexcept
    : Value(value)
  {
  }
  explicit cmValue(const std::string& value) noexcept
    : Value(&value)
  {
  }
  cmValue(const cmValue& other) noexcept = default;

  cmValue& operator=(const cmValue& other) noexcept = default;
  cmValue& operator=(std::nullptr_t) noexcept
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
    return this->Value == nullptr ? &cmValue::Empty : this->Value;
  }
  const std::string& operator*() const noexcept
  {
    return this->Value == nullptr ? cmValue::Empty : *this->Value;
  }

  explicit operator bool() const noexcept { return this->Value != nullptr; }
  operator const std::string&() const noexcept { return this->operator*(); }
  explicit operator cm::string_view() const noexcept
  {
    return this->operator*();
  }

  /**
   * Does the value indicate a true or ON value?
   */
  bool IsOn() const noexcept
  {
    return this->Value != nullptr &&
      cmValue::IsOn(cm::string_view(*this->Value));
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
      cmValue::IsOff(cm::string_view(*this->Value));
  }
  /** Return true if value is NOTFOUND or ends in -NOTFOUND.  */
  bool IsNOTFOUND() const noexcept
  {
    return this->Value != nullptr &&
      cmValue::IsNOTFOUND(cm::string_view(*this->Value));
  }
  bool IsEmpty() const noexcept
  {
    return this->Value == nullptr || this->Value->empty();
  }

  /**
   * Does a string indicates that CMake/CPack/CTest internally
   *  forced this value. This is not the same as On, but this
   * may be considered as "internally switched on".
   */
  bool IsInternallyOn() const noexcept
  {
    return this->Value != nullptr &&
      cmValue::IsInternallyOn(cm::string_view(*this->Value));
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
  int Compare(cmValue value) const noexcept;
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

  /**
   * Does a string indicates that CMake/CPack/CTest internally
   * forced this value. This is not the same as On, but this
   * may be considered as "internally switched on".
   */
  static bool IsInternallyOn(const char* value) noexcept
  {
    return value != nullptr && IsInternallyOn(cm::string_view(value));
  }
  static bool IsInternallyOn(cm::string_view) noexcept;

private:
  static std::string Empty;
  const std::string* Value = nullptr;
};

std::ostream& operator<<(std::ostream& o, cmValue v);

inline bool operator==(cmValue l, cmValue r) noexcept
{
  return l.Compare(r) == 0;
}
inline bool operator!=(cmValue l, cmValue r) noexcept
{
  return l.Compare(r) != 0;
}
inline bool operator<(cmValue l, cmValue r) noexcept
{
  return l.Compare(r) < 0;
}
inline bool operator<=(cmValue l, cmValue r) noexcept
{
  return l.Compare(r) <= 0;
}
inline bool operator>(cmValue l, cmValue r) noexcept
{
  return l.Compare(r) > 0;
}
inline bool operator>=(cmValue l, cmValue r) noexcept
{
  return l.Compare(r) >= 0;
}

inline bool operator==(cmValue l, cm::string_view r) noexcept
{
  return l.Compare(r) == 0;
}
inline bool operator!=(cmValue l, cm::string_view r) noexcept
{
  return l.Compare(r) != 0;
}
inline bool operator<(cmValue l, cm::string_view r) noexcept
{
  return l.Compare(r) < 0;
}
inline bool operator<=(cmValue l, cm::string_view r) noexcept
{
  return l.Compare(r) <= 0;
}
inline bool operator>(cmValue l, cm::string_view r) noexcept
{
  return l.Compare(r) > 0;
}
inline bool operator>=(cmValue l, cm::string_view r) noexcept
{
  return l.Compare(r) >= 0;
}

inline bool operator==(cmValue l, std::nullptr_t) noexcept
{
  return l.Compare(cmValue{}) == 0;
}
inline bool operator!=(cmValue l, std::nullptr_t) noexcept
{
  return l.Compare(cmValue{}) != 0;
}
inline bool operator<(cmValue l, std::nullptr_t) noexcept
{
  return l.Compare(cmValue{}) < 0;
}
inline bool operator<=(cmValue l, std::nullptr_t) noexcept
{
  return l.Compare(cmValue{}) <= 0;
}
inline bool operator>(cmValue l, std::nullptr_t) noexcept
{
  return l.Compare(cmValue{}) > 0;
}
inline bool operator>=(cmValue l, std::nullptr_t) noexcept
{
  return l.Compare(cmValue{}) >= 0;
}

/**
 * Does a string indicate a true or ON value? This is not the same as ifdef.
 */
inline bool cmIsOn(cm::string_view val)
{
  return cmValue::IsOn(val);
}
inline bool cmIsOn(const char* val)
{
  return cmValue::IsOn(val);
}
inline bool cmIsOn(cmValue val)
{
  return val.IsOn();
}

/**
 * Does a string indicate a false or off value ? Note that this is
 * not the same as !IsOn(...) because there are a number of
 * ambiguous values such as "/usr/local/bin" a path will result in
 * IsON and IsOff both returning false. Note that the special path
 * NOTFOUND, *-NOTFOUND or IGNORE will cause IsOff to return true.
 */
inline bool cmIsOff(cm::string_view val)
{
  return cmValue::IsOff(val);
}
inline bool cmIsOff(const char* val)
{
  return cmValue::IsOff(val);
}
inline bool cmIsOff(cmValue val)
{
  return val.IsOff();
}

/** Return true if value is NOTFOUND or ends in -NOTFOUND.  */
inline bool cmIsNOTFOUND(cm::string_view val)
{
  return cmValue::IsNOTFOUND(val);
}
inline bool cmIsNOTFOUND(cmValue val)
{
  return val.IsNOTFOUND();
}

/** Check for non-empty Property/Variable value.  */
inline bool cmNonempty(cm::string_view val)
{
  return !cmValue::IsEmpty(val);
}
inline bool cmNonempty(const char* val)
{
  return !cmValue::IsEmpty(val);
}
inline bool cmNonempty(cmValue val)
{
  return !val.IsEmpty();
}

/**
 * Does a string indicates that CMake/CPack/CTest internally
 * forced this value. This is not the same as On, but this
 * may be considered as "internally switched on".
 */
inline bool cmIsInternallyOn(cm::string_view val)
{
  return cmValue::IsInternallyOn(val);
}
inline bool cmIsInternallyOn(const char* val)
{
  return cmValue::IsInternallyOn(val);
}
inline bool cmIsInternallyOn(cmValue val)
{
  return val.IsInternallyOn();
}
