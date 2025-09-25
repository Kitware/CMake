/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cctype>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmRange.h"
#include "cmValue.h"

/** String range type.  */
using cmStringRange = cmRange<std::vector<std::string>::const_iterator>;

/** Returns length of a literal string.  */
template <size_t N>
constexpr size_t cmStrLen(char const (&)[N])
{
  return N - 1;
}

/** Callable string comparison struct.  */
struct cmStrCmp
{
  cmStrCmp(std::string str)
    : Test_(std::move(str))
  {
  }

  bool operator()(cm::string_view sv) const { return this->Test_ == sv; }

private:
  std::string const Test_;
};

/**
 * Test if two strings are identical, ignoring case.
 *
 * Note that this is not guaranteed to work correctly on non-ASCII strings.
 */
bool cmStrCaseEq(cm::string_view a, cm::string_view b);

/** Returns true if the character @a ch is a whitespace character.  **/
inline bool cmIsSpace(char ch)
{
  // isspace takes 'int' but documents that the value must be representable
  // by 'unsigned char', or be EOF.  Cast to 'unsigned char' to avoid sign
  // extension while converting to 'int'.
  return std::isspace(static_cast<unsigned char>(ch));
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

/** Generic function to join strings range with separator
 *  and initial leading string into a single string.
 */
template <typename Range>
std::string cmJoinStrings(Range const& rng, cm::string_view separator,
                          cm::string_view initial)
{
  if (rng.empty()) {
    return { std::begin(initial), std::end(initial) };
  }

  std::string result;
  result.reserve(std::accumulate(
    std::begin(rng), std::end(rng),
    initial.size() + (rng.size() - 1) * separator.size(),
    [](std::size_t sum, typename Range::value_type const& item) {
      return sum + item.size();
    }));
  result.append(std::begin(initial), std::end(initial));

  auto begin = std::begin(rng);
  auto end = std::end(rng);
  result += *begin;

  for (++begin; begin != end; ++begin) {
    result.append(std::begin(separator), std::end(separator));
    result += *begin;
  }

  return result;
}

/**
 * Faster overloads for std::string ranges.
 * If @a initial is provided, it prepends the resulted string without
 * @a separator between them.
 */
std::string cmJoin(std::vector<std::string> const& rng,
                   cm::string_view separator, cm::string_view initial = {});

std::string cmJoin(cmStringRange const& rng, cm::string_view separator,
                   cm::string_view initial = {});

enum class cmTokenizerMode
{
  /// A backward-compatible behavior when in the case of no
  /// tokens have found in an input text it'll return one empty
  /// token in the result container (vector).
  Legacy,
  /// The new behavior is to return an empty vector.
  New
};

/**
 * \brief A generic version of a tokenizer.
 *
 * Extract tokens from the input string separated by any
 * of the characters in `sep` and assign them to the
 * given output iterator.
 *
 * The `mode` parameter defines the behavior in the case when
 * no tokens have found in the input text.
 *
 */
template <typename StringT, typename OutIt, typename Sep = char>
void cmTokenize(OutIt outIt, cm::string_view str, Sep sep,
                cmTokenizerMode mode)
{
  auto hasTokens = false;
  // clang-format off
  for (auto start = str.find_first_not_of(sep)
    , end = str.find_first_of(sep, start)
    ; start != cm::string_view::npos
    ; start = str.find_first_not_of(sep, end)
    , end = str.find_first_of(sep, start)
    , hasTokens = true
    ) {
    *outIt++ = StringT{ str.substr(start, end - start) };
  }
  // clang-format on
  if (!hasTokens && mode == cmTokenizerMode::Legacy) {
    *outIt = {};
  }
}

/**
 * \brief Extract tokens that are separated by any of the
 * characters in `sep`.
 *
 * Backward compatible signature.
 *
 * \return A vector of strings.
 */
template <typename Sep = char>
std::vector<std::string> cmTokenize(
  cm::string_view str, Sep sep, cmTokenizerMode mode = cmTokenizerMode::Legacy)
{
  using StringType = std::string;
  std::vector<StringType> tokens;
  cmTokenize<StringType>(std::back_inserter(tokens), str, sep, mode);
  return tokens;
}

/**
 * \brief Extract tokens that are separated by any of the
 * characters in `sep`.
 *
 * \return A vector of string views.
 */
template <typename Sep = char>
std::vector<cm::string_view> cmTokenizedView(
  cm::string_view str, Sep sep, cmTokenizerMode mode = cmTokenizerMode::Legacy)
{
  using StringType = cm::string_view;
  std::vector<StringType> tokens;
  cmTokenize<StringType>(std::back_inserter(tokens), str, sep, mode);
  return tokens;
}

/** Concatenate string pieces into a single string.  */
std::string cmCatViews(
  std::initializer_list<std::pair<cm::string_view, std::string*>> views);

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
  cmAlphaNum(std::string&& str)
    : RValueString_(&str)
  {
  }
  cmAlphaNum(char const* str)
    : View_(str ? cm::string_view(str) : cm::string_view())
  {
  }
  cmAlphaNum(char ch)
    : View_(this->Digits_, 1)
  {
    this->Digits_[0] = ch;
  }
  cmAlphaNum(int val);
  cmAlphaNum(unsigned int val);
  cmAlphaNum(long int val);
  cmAlphaNum(unsigned long int val);
  cmAlphaNum(long long int val);
  cmAlphaNum(unsigned long long int val);
  cmAlphaNum(float val);
  cmAlphaNum(double val);
  cmAlphaNum(cmValue value)
    : View_(*value)
  {
  }

  cm::string_view View() const
  {
    if (this->RValueString_) {
      return *this->RValueString_;
    }
    return this->View_;
  }

  std::string* RValueString() const { return this->RValueString_; }

private:
  std::string* RValueString_ = nullptr;
  cm::string_view View_;
  char Digits_[32];
};

/** Concatenate string pieces and numbers into a single string.  */
template <typename A, typename B, typename... AV>
inline std::string cmStrCat(A&& a, B&& b, AV&&... args)
{
  static auto const makePair =
    [](cmAlphaNum const& arg) -> std::pair<cm::string_view, std::string*> {
    return { arg.View(), arg.RValueString() };
  };

  return cmCatViews({ makePair(std::forward<A>(a)),
                      makePair(std::forward<B>(b)),
                      makePair(std::forward<AV>(args))... });
}

/** Joins wrapped elements of a range with separator into a single string.  */
template <typename Range>
std::string cmWrap(cm::string_view prefix, Range const& rng,
                   cm::string_view suffix, cm::string_view sep)
{
  if (rng.empty()) {
    return std::string();
  }
  return cmCatViews({ { prefix, nullptr },
                      { cmJoin(rng,
                               cmCatViews({ { suffix, nullptr },
                                            { sep, nullptr },
                                            { prefix, nullptr } })),
                        nullptr },
                      { suffix, nullptr } });
}

/** Joins wrapped elements of a range with separator into a single string.  */
template <typename Range>
std::string cmWrap(char prefix, Range const& rng, char suffix,
                   cm::string_view sep)
{
  return cmWrap(cm::string_view(&prefix, 1), rng, cm::string_view(&suffix, 1),
                sep);
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
inline bool cmHasPrefix(cm::string_view str, cmValue prefix)
{
  if (!prefix) {
    return false;
  }

  return str.compare(0, prefix->size(), *prefix) == 0;
}

/** Returns true if string @a str starts with string @a prefix.  */
template <size_t N>
inline bool cmHasLiteralPrefix(cm::string_view str, char const (&prefix)[N])
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
inline bool cmHasSuffix(cm::string_view str, cmValue suffix)
{
  if (!suffix) {
    return false;
  }

  return str.size() >= suffix->size() &&
    str.compare(str.size() - suffix->size(), suffix->size(), *suffix) == 0;
}

/** Returns true if string @a str ends with string @a suffix.  */
template <size_t N>
inline bool cmHasLiteralSuffix(cm::string_view str, char const (&suffix)[N])
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
bool cmStrToLong(char const* str, long* value);
bool cmStrToLong(std::string const& str, long* value);

/** Converts a string to unsigned long. Expects that the whole string is an
 * integer */
bool cmStrToULong(char const* str, unsigned long* value);
bool cmStrToULong(std::string const& str, unsigned long* value);

/** Converts a string to long long. Expects that the whole string
 * is an integer */
bool cmStrToLongLong(char const* str, long long* value);
bool cmStrToLongLong(std::string const& str, long long* value);

/** Converts a string to unsigned long long. Expects that the whole string
 * is an integer */
bool cmStrToULongLong(char const* str, unsigned long long* value);
bool cmStrToULongLong(std::string const& str, unsigned long long* value);
