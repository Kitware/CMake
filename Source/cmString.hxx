/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

namespace cm {

class String;

/**
 * Trait to convert type T into a String.
 * Implementations must derive from 'std::true_type'
 * and define an 'into_string' member that accepts
 * type T (by value or reference) and returns one of:
 *
 * - 'std::string' to construct an owned instance.
 * - 'cm::string_view' to construct a borrowed or null instances.
 *   The buffer from which the view is borrowed must outlive
 *   all copies of the resulting String, e.g. static storage.
 * - 'cm::String' for already-constructed instances.
 */
template <typename T>
struct IntoString : std::false_type
{
};

template <typename T>
struct IntoString<T&> : IntoString<T>
{
};

template <typename T>
struct IntoString<T const> : IntoString<T>
{
};

template <typename T>
struct IntoString<T const*> : IntoString<T*>
{
};

template <typename T, std::string::size_type N>
struct IntoString<T const[N]> : IntoString<T[N]>
{
};

template <>
struct IntoString<char*> : std::true_type
{
  static String into_string(const char* s);
};

template <>
struct IntoString<std::nullptr_t> : std::true_type
{
  static string_view into_string(std::nullptr_t) { return string_view(); }
};

template <std::string::size_type N>
struct IntoString<char[N]> : std::true_type
{
  static std::string into_string(char const (&s)[N])
  {
    return std::string(s, N - 1);
  }
};

template <>
struct IntoString<std::string> : std::true_type
{
  static std::string into_string(std::string s) { return s; }
};

template <>
struct IntoString<char> : std::true_type
{
  static std::string into_string(char const& c) { return std::string(1, c); }
};

/**
 * Trait to convert type T into a 'cm::string_view'.
 * Implementations must derive from 'std::true_type' and
 * define a 'view' member that accepts type T (by reference)
 * and returns a 'cm::string_view'.
 */
template <typename T>
struct AsStringView : std::false_type
{
};

template <typename T>
struct AsStringView<T&> : AsStringView<T>
{
};

template <typename T>
struct AsStringView<T const> : AsStringView<T>
{
};

template <typename T>
struct AsStringView<T const*> : AsStringView<T*>
{
};

template <typename T, std::string::size_type N>
struct AsStringView<T const[N]> : AsStringView<T[N]>
{
};

template <>
struct AsStringView<char*> : std::true_type
{
  static string_view view(const char* s) { return s; }
};

template <std::string::size_type N>
struct AsStringView<char[N]> : std::true_type
{
  static string_view view(char const (&s)[N]) { return string_view(s, N - 1); }
};

template <>
struct AsStringView<std::string> : std::true_type
{
  static string_view view(std::string const& s) { return s; }
};

template <>
struct AsStringView<char> : std::true_type
{
  static string_view view(const char& s) { return string_view(&s, 1); }
};

template <>
struct AsStringView<string_view> : std::true_type
{
  static string_view view(string_view const& s) { return s; }
};

template <>
struct AsStringView<static_string_view> : std::true_type
{
  static string_view view(static_string_view const& s) { return s; }
};

template <>
struct AsStringView<String> : std::true_type
{
  static string_view view(String const& s);
};

/**
 * \class String
 *
 * A custom string type that holds a view of a string buffer
 * and optionally shares ownership of the buffer.  Instances
 * may have one of the following states:
 *
 * - null: views and owns nothing.
 *   Conversion to 'bool' is 'false'.
 *   'data()' and 'c_str()' return nullptr.
 *   'size()' returns 0.
 *   'str()' returns an empty string.
 *
 * - borrowed: views a string but does not own it.  This is used
 *   to bind to static storage (e.g. string literals) or for
 *   temporary instances that do not outlive the borrowed buffer.
 *   Copies and substrings still borrow the original buffer.
 *   Mutation allocates a new internal string and converts to
 *   the 'owned' state.
 *   Conversion to 'bool' is 'true'.
 *   'c_str()' may internally mutate to the 'owned' state.
 *   'str()' internally mutates to the 'owned' state.
 *
 * - owned: views an immutable 'std::string' instance owned internally.
 *   Copies and substrings share ownership of the internal string.
 *   Mutation allocates a new internal string.
 *   Conversion to 'bool' is 'true'.
 */
class String
{
  enum class Private
  {
  };

public:
  using traits_type = std::string::traits_type;
  using value_type = string_view::value_type;
  using pointer = string_view::pointer;
  using const_pointer = string_view::const_pointer;
  using reference = string_view::reference;
  using const_reference = string_view::const_reference;
  using const_iterator = string_view::const_iterator;
  using iterator = string_view::const_iterator;
  using const_reverse_iterator = string_view::const_reverse_iterator;
  using reverse_iterator = string_view::const_reverse_iterator;
  using difference_type = string_view::difference_type;
  using size_type = string_view::size_type;

  static size_type const npos = string_view::npos;

  /** Construct a null string.  */
  String() = default;

  /** Construct from any type implementing the IntoString trait.  */
  template <typename T,
            typename = typename std::enable_if<IntoString<T>::value>::type>
  String(T&& s)
    : String(IntoString<T>::into_string(std::forward<T>(s)), Private())
  {
  }

  /**
   * Construct via static_string_view constructor.
   * explicit is required to avoid ambiguous overloaded operators (i.e ==,
   * etc...) with the ones provided by string_view.
   */
  explicit String(static_string_view s)
    : String(s, Private())
  {
  }
  /**
   * Construct via string_view constructor.
   * explicit is required to avoid ambiguous overloaded operators (i.e ==,
   * etc...) with the ones provided by string_view.
   */
  explicit String(string_view s)
    : String(std::string(s), Private())
  {
  }

  /** Construct via std::string initializer list constructor.  */
  String(std::initializer_list<char> il)
    : String(std::string(il))
  {
  }

  /** Construct by copying the specified buffer.  */
  String(const char* d, size_type s)
    : String(std::string(d, s))
  {
  }

  /** Construct by copying from input iterator range.  */
  template <typename InputIterator>
  String(InputIterator first, InputIterator last)
    : String(std::string(first, last))
  {
  }

  /** Construct a string with 'n' copies of character 'c'.  */
  String(size_type n, char c)
    : String(std::string(n, c))
  {
  }

  /** Construct from a substring of another String instance.
      This shares ownership of the other string's buffer
      but views only a substring.  */
  String(String const& s, size_type pos, size_type count = npos)
    : string_(s.string_)
    , view_(s.data() + pos, std::min(count, s.size() - pos))
  {
  }

  /** Construct by moving from another String instance.
      The other instance is left as a null string.  */
  String(String&& s) noexcept
    : string_(std::move(s.string_))
    , view_(s.view_)
  {
    s.view_ = string_view();
  }

  /** Construct by copying from another String instance.
      This shares ownership of the other string's buffer.  */
  String(String const&) noexcept = default;

  ~String() = default;

  /** Construct by borrowing an externally-owned buffer.  The buffer
      must outlive the returned instance and all copies of it.  */
  static String borrow(string_view v) { return String(v, Private()); }

  /** Assign by moving from another String instance.
      The other instance is left as a null string.  */
  String& operator=(String&& s) noexcept
  {
    this->string_ = std::move(s.string_);
    this->view_ = s.view_;
    s.view_ = string_view();
    return *this;
  }

  /** Assign by copying from another String instance.
      This shares ownership of the other string's buffer.  */
  String& operator=(String const&) noexcept = default;

  String& operator=(static_string_view s)
  {
    *this = String(s);
    return *this;
  }
  String& operator=(string_view s)
  {
    *this = String(s);
    return *this;
  }

  /** Assign from any type implementing the IntoString trait.  */
  template <typename T>
  typename // NOLINT(*)
    std::enable_if<IntoString<T>::value, String&>::type
    operator=(T&& s)
  {
    *this = String(std::forward<T>(s));
    return *this;
  }

  /** Assign via std::string initializer list constructor.  */
  String& operator=(std::initializer_list<char> il)
  {
    *this = String(il);
    return *this;
  }

  /** Return true if the instance is not a null string.  */
  explicit operator bool() const noexcept { return this->data() != nullptr; }

  /** Return a view of the string.  */
  string_view view() const noexcept { return this->view_; }
  operator string_view() const noexcept { return this->view(); }

  /** Return true if the instance is an empty stringn or null string.  */
  bool empty() const noexcept { return this->view_.empty(); }

  /** Return a pointer to the start of the string.  */
  const char* data() const noexcept { return this->view_.data(); }

  /** Return the length of the string in bytes.  */
  size_type size() const noexcept { return this->view_.size(); }
  size_type length() const noexcept { return this->view_.length(); }

  /** Return the character at the given position.
      No bounds checking is performed.  */
  char operator[](size_type pos) const noexcept { return this->view_[pos]; }

  /** Return the character at the given position.
      If the position is out of bounds, throws std::out_of_range.  */
  char at(size_type pos) const { return this->view_.at(pos); }

  char front() const noexcept { return this->view_.front(); }

  char back() const noexcept { return this->view_.back(); }

  /** Return true if this instance is stable and otherwise false.
      An instance is stable if it is in the 'null' state or if it is
      an 'owned' state not produced by substring operations, or
      after a call to 'stabilize()' or 'str()'.  */
  bool is_stable() const;

  /** If 'is_stable()' does not return true, mutate so it does.  */
  void stabilize();

  /** Get a pointer to a normal std::string if 'is_stable()' returns
      true and otherwise nullptr.  The pointer is valid until this
      instance is mutated or destroyed.  */
  std::string const* str_if_stable() const;

  /** Get a reference to a normal std::string.  The reference
      is valid until this instance is mutated or destroyed.  */
  std::string const& str();

  /** Get a pointer to a C-style null-terminated string
      containing the same value as this instance.  The pointer
      is valid until this instance is mutated, destroyed,
      or str() is called.  */
  const char* c_str();

  const_iterator begin() const noexcept { return this->view_.begin(); }
  const_iterator end() const noexcept { return this->view_.end(); }
  const_iterator cbegin() const noexcept { return this->begin(); }
  const_iterator cend() const noexcept { return this->end(); }

  const_reverse_iterator rbegin() const noexcept
  {
    return this->view_.rbegin();
  }
  const_reverse_iterator rend() const noexcept { return this->view_.rend(); }
  const_reverse_iterator crbegin() const noexcept { return this->rbegin(); }
  const_reverse_iterator crend() const noexcept { return this->rend(); }

  /** Append to the string using any type that implements the
      AsStringView trait.  */
  template <typename T>
  typename std::enable_if<AsStringView<T>::value, String&>::type operator+=(
    T&& s)
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    std::string r;
    r.reserve(this->size() + v.size());
    r.assign(this->data(), this->size());
    r.append(v.data(), v.size());
    return *this = std::move(r);
  }

  /** Assign to an empty string.  */
  void clear() { *this = ""_s; }

  /** Insert 'count' copies of 'ch' at position 'index'.  */
  String& insert(size_type index, size_type count, char ch);

  /** Erase 'count' characters starting at position 'index'.  */
  String& erase(size_type index = 0, size_type count = npos);

  void push_back(char ch)
  {
    std::string s;
    s.reserve(this->size() + 1);
    s.assign(this->data(), this->size());
    s.push_back(ch);
    *this = std::move(s);
  }

  void pop_back() { *this = String(*this, 0, this->size() - 1); }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, String&>::type replace(
    size_type pos, size_type count, T&& s)
  {
    const_iterator first = this->begin() + pos;
    const_iterator last = first + count;
    return this->replace(first, last, std::forward<T>(s));
  }

  template <typename InputIterator>
  String& replace(const_iterator first, const_iterator last,
                  InputIterator first2, InputIterator last2)
  {
    std::string out;
    out.append(this->view_.begin(), first);
    out.append(first2, last2);
    out.append(last, this->view_.end());
    return *this = std::move(out);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, String&>::type replace(
    const_iterator first, const_iterator last, T&& s)
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    std::string out;
    out.reserve((first - this->view_.begin()) + v.size() +
                (this->view_.end() - last));
    out.append(this->view_.begin(), first);
    out.append(v.data(), v.size());
    out.append(last, this->view_.end());
    return *this = std::move(out);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, String&>::type replace(
    size_type pos, size_type count, T&& s, size_type pos2,
    size_type count2 = npos)
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    v = v.substr(pos2, count2);
    return this->replace(pos, count, v);
  }

  String& replace(size_type pos, size_type count, size_type count2, char ch)
  {
    const_iterator first = this->begin() + pos;
    const_iterator last = first + count;
    return this->replace(first, last, count2, ch);
  }

  String& replace(const_iterator first, const_iterator last, size_type count2,
                  char ch)
  {
    std::string out;
    out.reserve(static_cast<size_type>(first - this->view_.begin()) + count2 +
                static_cast<size_type>(this->view_.end() - last));
    out.append(this->view_.begin(), first);
    out.append(count2, ch);
    out.append(last, this->view_.end());
    return *this = std::move(out);
  }

  size_type copy(char* dest, size_type count, size_type pos = 0) const;

  void resize(size_type count) { this->resize(count, char()); }

  void resize(size_type count, char ch)
  {
    std::string s;
    s.reserve(count);
    if (count <= this->size()) {
      s.assign(this->data(), count);
    } else {
      s.assign(this->data(), this->size());
      s.resize(count, ch);
    }
    *this = std::move(s);
  }

  void swap(String& other)
  {
    std::swap(this->string_, other.string_);
    std::swap(this->view_, other.view_);
  }

  /** Return a substring starting at position 'pos' and
      consisting of at most 'count' characters.  */
  String substr(size_type pos = 0, size_type count = npos) const;

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, int>::type compare(
    T&& s) const
  {
    return this->view_.compare(AsStringView<T>::view(std::forward<T>(s)));
  }

  int compare(size_type pos1, size_type count1, string_view v) const
  {
    return this->view_.compare(pos1, count1, v);
  }

  int compare(size_type pos1, size_type count1, string_view v, size_type pos2,
              size_type count2) const
  {
    return this->view_.compare(pos1, count1, v, pos2, count2);
  }

  int compare(size_type pos1, size_type count1, const char* s) const
  {
    return this->view_.compare(pos1, count1, s);
  }

  int compare(size_type pos1, size_type count1, const char* s,
              size_type count2) const
  {
    return this->view_.compare(pos1, count1, s, count2);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, size_type>::type find(
    T&& s, size_type pos = 0) const
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    return this->view_.find(v, pos);
  }

  size_type find(const char* s, size_type pos, size_type count) const
  {
    return this->view_.find(s, pos, count);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, size_type>::type rfind(
    T&& s, size_type pos = npos) const
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    return this->view_.rfind(v, pos);
  }

  size_type rfind(const char* s, size_type pos, size_type count) const
  {
    return this->view_.rfind(s, pos, count);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, size_type>::type
  find_first_of(T&& s, size_type pos = 0) const
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    return this->view_.find_first_of(v, pos);
  }

  size_type find_first_of(const char* s, size_type pos, size_type count) const
  {
    return this->view_.find_first_of(s, pos, count);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, size_type>::type
  find_first_not_of(T&& s, size_type pos = 0) const
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    return this->view_.find_first_not_of(v, pos);
  }

  size_type find_first_not_of(const char* s, size_type pos,
                              size_type count) const
  {
    return this->view_.find_first_not_of(s, pos, count);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, size_type>::type
  find_last_of(T&& s, size_type pos = npos) const
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    return this->view_.find_last_of(v, pos);
  }

  size_type find_last_of(const char* s, size_type pos, size_type count) const
  {
    return this->view_.find_last_of(s, pos, count);
  }

  template <typename T>
  typename std::enable_if<AsStringView<T>::value, size_type>::type
  find_last_not_of(T&& s, size_type pos = npos) const
  {
    string_view v = AsStringView<T>::view(std::forward<T>(s));
    return this->view_.find_last_not_of(v, pos);
  }

  size_type find_last_not_of(const char* s, size_type pos,
                             size_type count) const
  {
    return this->view_.find_last_not_of(s, pos, count);
  }

private:
  // Internal constructor to move from existing String.
  String(String&& s, Private) noexcept
    : String(std::move(s))
  {
  }

  // Internal constructor for dynamically allocated string.
  String(std::string&& s, Private);

  // Internal constructor for view of statically allocated string.
  String(string_view v, Private)
    : view_(v)
  {
  }

  void internally_mutate_to_stable_string();

  std::shared_ptr<std::string const> string_;
  string_view view_;
};

/**
 * Trait for comparable types.
 */
template <typename T>
struct IsComparable : std::false_type
{
};

template <typename T>
struct IsComparable<T&> : IsComparable<T>
{
};

template <typename T>
struct IsComparable<T const> : IsComparable<T>
{
};

template <typename T>
struct IsComparable<T const*> : IsComparable<T*>
{
};

template <typename T, std::string::size_type N>
struct IsComparable<T const[N]> : IsComparable<T[N]>
{
};

template <>
struct IsComparable<char*> : std::true_type
{
};

template <std::string::size_type N>
struct IsComparable<char[N]> : std::true_type
{
};

template <>
struct IsComparable<std::string> : std::true_type
{
};

template <>
struct IsComparable<char> : std::true_type
{
};

/** comparison operators */
inline bool operator==(const String& l, const String& r)
{
  return l.view() == r.view();
}
template <typename L>
typename std::enable_if<IsComparable<L>::value, bool>::type operator==(
  L&& l, const String& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) == r.view();
}
template <typename R>
typename std::enable_if<IsComparable<R>::value, bool>::type operator==(
  const String& l, R&& r)
{
  return l.view() == AsStringView<R>::view(std::forward<R>(r));
}

inline bool operator!=(const String& l, const String& r)
{
  return l.view() != r.view();
}
template <typename L>
typename std::enable_if<IsComparable<L>::value, bool>::type operator!=(
  L&& l, const String& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) != r.view();
}
template <typename R>
typename std::enable_if<IsComparable<R>::value, bool>::type operator!=(
  const String& l, R&& r)
{
  return l.view() != AsStringView<R>::view(std::forward<R>(r));
}

inline bool operator<(const String& l, const String& r)
{
  return l.view() < r.view();
}
template <typename L>
typename std::enable_if<IsComparable<L>::value, bool>::type operator<(
  L&& l, const String& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) < r.view();
}
template <typename R>
typename std::enable_if<IsComparable<R>::value, bool>::type operator<(
  const String& l, R&& r)
{
  return l.view() < AsStringView<R>::view(std::forward<R>(r));
}

inline bool operator<=(const String& l, const String& r)
{
  return l.view() <= r.view();
}
template <typename L>
typename std::enable_if<IsComparable<L>::value, bool>::type operator<=(
  L&& l, const String& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) <= r.view();
}
template <typename R>
typename std::enable_if<IsComparable<R>::value, bool>::type operator<=(
  const String& l, R&& r)
{
  return l.view() <= AsStringView<R>::view(std::forward<R>(r));
}

inline bool operator>(const String& l, const String& r)
{
  return l.view() > r.view();
}
template <typename L>
typename std::enable_if<IsComparable<L>::value, bool>::type operator>(
  L&& l, const String& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) > r.view();
}
template <typename R>
typename std::enable_if<IsComparable<R>::value, bool>::type operator>(
  const String& l, R&& r)
{
  return l.view() > AsStringView<R>::view(std::forward<R>(r));
}

inline bool operator>=(const String& l, const String& r)
{
  return l.view() >= r.view();
}
template <typename L>
typename std::enable_if<IsComparable<L>::value, bool>::type operator>=(
  L&& l, const String& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) >= r.view();
}
template <typename R>
typename std::enable_if<IsComparable<R>::value, bool>::type operator>=(
  const String& l, R&& r)
{
  return l.view() >= AsStringView<R>::view(std::forward<R>(r));
}

std::ostream& operator<<(std::ostream& os, String const& s);
std::string& operator+=(std::string& self, String const& s);

template <typename L, typename R>
struct StringOpPlus
{
  L l;
  R r;
#if defined(__SUNPRO_CC)
  StringOpPlus(L in_l, R in_r)
    : l(in_l)
    , r(in_r)
  {
  }
#endif
  operator std::string() const;
  std::string::size_type size() const
  {
    return this->l.size() + this->r.size();
  }
};

template <typename T>
struct StringAdd
{
  static const bool value = AsStringView<T>::value;
  using temp_type = string_view;
  template <typename S>
  static temp_type temp(S&& s)
  {
    return AsStringView<T>::view(std::forward<S>(s));
  }
};

template <typename L, typename R>
struct StringAdd<StringOpPlus<L, R>> : std::true_type
{
  using temp_type = StringOpPlus<L, R> const&;
  static temp_type temp(temp_type s) { return s; }
};

template <typename L, typename R>
StringOpPlus<L, R>::operator std::string() const
{
  std::string s;
  s.reserve(this->size());
  s += *this;
  return s;
}

template <typename L, typename R>
std::string& operator+=(std::string& s, StringOpPlus<L, R> const& a)
{
  s.reserve(s.size() + a.size());
  s += a.l;
  s += a.r;
  return s;
}

template <typename L, typename R>
String& operator+=(String& s, StringOpPlus<L, R> const& a)
{
  std::string r;
  r.reserve(s.size() + a.size());
  r.assign(s.data(), s.size());
  r += a.l;
  r += a.r;
  s = std::move(r);
  return s;
}

template <typename L, typename R>
std::ostream& operator<<(std::ostream& os, StringOpPlus<L, R> const& a)
{
  return os << a.l << a.r;
}

template <typename L, typename R>
struct IntoString<StringOpPlus<L, R>> : std::true_type
{
  static std::string into_string(StringOpPlus<L, R> const& a) { return a; }
};

template <typename L, typename R>
typename std::enable_if<StringAdd<L>::value && StringAdd<R>::value,
                        StringOpPlus<typename StringAdd<L>::temp_type,
                                     typename StringAdd<R>::temp_type>>::type
operator+(L&& l, R&& r)
{
  return { StringAdd<L>::temp(std::forward<L>(l)),
           StringAdd<R>::temp(std::forward<R>(r)) };
}

template <typename LL, typename LR, typename R>
typename std::enable_if<AsStringView<R>::value, bool>::type operator==(
  StringOpPlus<LL, LR> const& l, R&& r)
{
  return std::string(l) == AsStringView<R>::view(std::forward<R>(r));
}

template <typename L, typename RL, typename RR>
typename std::enable_if<AsStringView<L>::value, bool>::type operator==(
  L&& l, StringOpPlus<RL, RR> const& r)
{
  return AsStringView<L>::view(std::forward<L>(l)) == std::string(r);
}

} // namespace cm

namespace std {

template <>
struct hash<cm::String>
{
  using argument_type = cm::String;
  using result_type = size_t;

  result_type operator()(argument_type const& s) const noexcept
  {
    result_type const h(std::hash<cm::string_view>{}(s.view()));
    return h;
  }
};
}
