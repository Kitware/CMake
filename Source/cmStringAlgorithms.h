/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmStringAlgorithms_h
#define cmStringAlgorithms_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <cctype>
#include <cstring>
#include <initializer_list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmRange.h"

/** String range type.  */
using cmStringRange = cmRange<std::vector<std::string>::const_iterator>;

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

/** Returns true if the character @a ch is a whitespace character.  **/
inline bool cmIsSpace(char ch)
{
  return ((ch & 0x80) == 0) && std::isspace(ch);
}

/** Returns a string that has whitespace removed from the start and the end. */
std::string cmTrimWhitespace(cm::string_view str);

/** Returns a string that has quotes removed from the start and the end. */
std::string cmRemoveQuotes(cm::string_view str);

/** Escape quotes in a string.  */
std::string cmEscapeQuotes(cm::string_view str);

/** Joins elements of a range with separator into a single string.  */
template <typename Range>
std::string cmJoin(Range const& rng, cm::string_view separator)
{
  if (rng.empty()) {
    return std::string();
  }

  std::ostringstream os;
  auto it = rng.begin();
  auto const end = rng.end();
  os << *it;
  while (++it != end) {
    os << separator << *it;
  }
  return os.str();
}

/** Extract tokens that are separated by any of the characters in @a sep.  */
std::vector<std::string> cmTokenize(cm::string_view str, cm::string_view sep);

/**
 * Expand the ; separated string @a arg into multiple arguments.
 * All found arguments are appended to @a argsOut.
 */
void cmExpandList(cm::string_view arg, std::vector<std::string>& argsOut,
                  bool emptyArgs = false);

/**
 * Expand out any arguments in the string range [@a first, @a last) that have
 * ; separated strings into multiple arguments.  All found arguments are
 * appended to @a argsOut.
 */
template <class InputIt>
void cmExpandLists(InputIt first, InputIt last,
                   std::vector<std::string>& argsOut)
{
  for (; first != last; ++first) {
    ExpandList(*first, argsOut);
  }
}

/**
 * Same as cmExpandList but a new vector is created containing
 * the expanded arguments from the string @a arg.
 */
std::vector<std::string> cmExpandedList(cm::string_view arg,
                                        bool emptyArgs = false);

/**
 * Same as cmExpandList but a new vector is created containing the expanded
 * versions of all arguments in the string range [@a first, @a last).
 */
template <class InputIt>
std::vector<std::string> cmExpandedLists(InputIt first, InputIt last)
{
  std::vector<std::string> argsOut;
  for (; first != last; ++first) {
    cmExpandList(*first, argsOut);
  }
  return argsOut;
}

/** Concatenate string pieces into a single string.  */
std::string cmCatViews(std::initializer_list<cm::string_view> views);

/** Utility class for cmStrCat.  */
class cmAlphaNum
{
public:
  cmAlphaNum(cm::string_view view)
    : View_(view)
  {
  }
  cmAlphaNum(std::string const& str)
    : View_(str)
  {
  }
  cmAlphaNum(const char* str)
    : View_(str)
  {
  }
  cmAlphaNum(char ch)
    : View_(Digits_, 1)
  {
    Digits_[0] = ch;
  }
  cmAlphaNum(int val);
  cmAlphaNum(unsigned int val);
  cmAlphaNum(long int val);
  cmAlphaNum(unsigned long int val);
  cmAlphaNum(long long int val);
  cmAlphaNum(unsigned long long int val);
  cmAlphaNum(float val);
  cmAlphaNum(double val);

  cm::string_view View() const { return View_; }

private:
  cm::string_view View_;
  char Digits_[32];
};

/** Concatenate string pieces and numbers into a single string.  */
template <typename... AV>
inline std::string cmStrCat(cmAlphaNum const& a, cmAlphaNum const& b,
                            AV const&... args)
{
  return cmCatViews(
    { a.View(), b.View(), static_cast<cmAlphaNum const&>(args).View()... });
}

/** Joins wrapped elements of a range with separator into a single string.  */
template <typename Range>
std::string cmWrap(cm::string_view prefix, Range const& rng,
                   cm::string_view suffix, cm::string_view sep)
{
  if (rng.empty()) {
    return std::string();
  }
  return cmCatViews(
    { prefix, cmJoin(rng, cmCatViews({ suffix, sep, prefix })), suffix });
}

/** Joins wrapped elements of a range with separator into a single string.  */
template <typename Range>
std::string cmWrap(char prefix, Range const& rng, char suffix,
                   cm::string_view sep)
{
  return cmWrap(cm::string_view(&prefix, 1), rng, cm::string_view(&suffix, 1),
                sep);
}

/**
 * Does a string indicates that CMake/CPack/CTest internally
 * forced this value. This is not the same as On, but this
 * may be considered as "internally switched on".
 */
bool cmIsInternallyOn(cm::string_view val);
inline bool cmIsInternallyOn(const char* val)
{
  if (!val) {
    return false;
  }
  return cmIsInternallyOn(cm::string_view(val));
}

/** Return true if value is NOTFOUND or ends in -NOTFOUND.  */
bool cmIsNOTFOUND(cm::string_view val);

/**
 * Does a string indicate a true or ON value? This is not the same as ifdef.
 */
bool cmIsOn(cm::string_view val);
inline bool cmIsOn(const char* val)
{
  if (!val) {
    return false;
  }
  return cmIsOn(cm::string_view(val));
}

/**
 * Does a string indicate a false or off value ? Note that this is
 * not the same as !IsOn(...) because there are a number of
 * ambiguous values such as "/usr/local/bin" a path will result in
 * IsON and IsOff both returning false. Note that the special path
 * NOTFOUND, *-NOTFOUND or IGNORE will cause IsOff to return true.
 */
bool cmIsOff(cm::string_view val);
inline bool cmIsOff(const char* val)
{
  if (!val) {
    return true;
  }
  return cmIsOff(cm::string_view(val));
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

/** Converts a string to long. Expects that the whole string is an integer.  */
bool cmStrToLong(const char* str, long* value);
bool cmStrToLong(std::string const& str, long* value);

/** Converts a string to unsigned long. Expects that the whole string is an
 * integer */
bool cmStrToULong(const char* str, unsigned long* value);
bool cmStrToULong(std::string const& str, unsigned long* value);

#endif
