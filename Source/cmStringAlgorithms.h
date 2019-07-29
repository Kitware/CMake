/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmStringAlgorithms_h
#define cmStringAlgorithms_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmRange.h"
#include "cm_string_view.hxx"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

/** String range type.  */
typedef cmRange<std::vector<std::string>::const_iterator> cmStringRange;

/** Callable string comparison struct.  */
struct cmStrCmp
{
  cmStrCmp(std::string str)
    : Test_(std::move(str))
  {
  }

  bool operator()(cm::string_view sv) const { return Test_ == sv; }

private:
  std::string const Test_;
};

template <typename Range>
std::string cmJoin(Range const& r, const char* delimiter)
{
  if (r.empty()) {
    return std::string();
  }
  std::ostringstream os;
  typedef typename Range::value_type ValueType;
  typedef typename Range::const_iterator InputIt;
  const InputIt first = r.begin();
  InputIt last = r.end();
  --last;
  std::copy(first, last, std::ostream_iterator<ValueType>(os, delimiter));

  os << *last;

  return os.str();
}

template <typename Range>
std::string cmJoin(Range const& r, std::string const& delimiter)
{
  return cmJoin(r, delimiter.c_str());
}

template <typename Range>
std::string cmWrap(std::string const& prefix, Range const& r,
                   std::string const& suffix, std::string const& sep)
{
  if (r.empty()) {
    return std::string();
  }
  return prefix + cmJoin(r, suffix + sep + prefix) + suffix;
}

template <typename Range>
std::string cmWrap(char prefix, Range const& r, char suffix,
                   std::string const& sep)
{
  return cmWrap(std::string(1, prefix), r, std::string(1, suffix), sep);
}

/** Returns true if string @a str starts with the character @a prefix.  */
inline bool cmHasPrefix(cm::string_view str, char prefix)
{
  return !str.empty() && (str.front() == prefix);
}

/** Returns true if string @a str starts with string @a prefix.  */
inline bool cmHasPrefix(cm::string_view str, cm::string_view prefix)
{
  return str.compare(0, prefix.size(), prefix) == 0;
}

/** Returns true if string @a str starts with string @a prefix.  */
template <size_t N>
inline bool cmHasLiteralPrefix(cm::string_view str, const char (&prefix)[N])
{
  return cmHasPrefix(str, cm::string_view(prefix, N - 1));
}

/** Returns true if string @a str ends with the character @a suffix.  */
inline bool cmHasSuffix(cm::string_view str, char suffix)
{
  return !str.empty() && (str.back() == suffix);
}

/** Returns true if string @a str ends with string @a suffix.  */
inline bool cmHasSuffix(cm::string_view str, cm::string_view suffix)
{
  return str.size() >= suffix.size() &&
    str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/** Returns true if string @a str ends with string @a suffix.  */
template <size_t N>
inline bool cmHasLiteralSuffix(cm::string_view str, const char (&suffix)[N])
{
  return cmHasSuffix(str, cm::string_view(suffix, N - 1));
}

/** Removes an existing suffix character of from the string @a str.  */
inline void cmStripSuffixIfExists(std::string& str, char suffix)
{
  if (cmHasSuffix(str, suffix)) {
    str.pop_back();
  }
}

/** Removes an existing suffix string of from the string @a str.  */
inline void cmStripSuffixIfExists(std::string& str, cm::string_view suffix)
{
  if (cmHasSuffix(str, suffix)) {
    str.resize(str.size() - suffix.size());
  }
}

#endif
