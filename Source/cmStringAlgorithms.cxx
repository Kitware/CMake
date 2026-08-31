/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmStringAlgorithms.h"

#include <algorithm>
#include <cerrno>
#include <cstddef> // IWYU pragma: keep
#include <cstdio>
#include <cstdlib>
#include <limits>

#include "cmsys/String.h"

bool cmStrCaseEq(cm::string_view s1, cm::string_view s2)
{
  if (s1.size() != s2.size()) {
    return false;
  }

  return std::equal(s1.begin(), s1.end(), s2.begin(), [](char a, char b) {
    return cmsysString_tolower(a) == cmsysString_tolower(b);
  });
}

std::string cmTrimWhitespace(cm::string_view str)
{
  // XXX(clang-tidy): This declaration and the next cannot be `const auto*`
  // because the qualification of `auto` is platform-dependent.
  // NOLINTNEXTLINE(readability-qualified-auto)
  auto start = str.begin();
  while (start != str.end() && cmsysString_isspace(*start)) {
    ++start;
  }
  if (start == str.end()) {
    return std::string();
  }
  // NOLINTNEXTLINE(readability-qualified-auto)
  auto stop = str.end() - 1;
  while (cmsysString_isspace(*stop)) {
    --stop;
  }
  return std::string(start, stop + 1);
}

cm::string_view cmStripWhitespace(cm::string_view str)
{
  std::string::size_type const l = str.size();

  std::string::size_type s = 0;
  while (s < l && cmsysString_isspace(str[s])) {
    ++s;
  }
  if (s == l) {
    return cm::string_view{};
  }
  std::string::size_type e = l - 1;
  while (cmsysString_isspace(str[e])) {
    --e;
  }
  return str.substr(s, e + 1 - s);
}

std::string cmRemoveQuotes(cm::string_view str)
{
  // We process only strings that have two quotes at least.
  // Also front() and back() are only defined behavior on non empty strings.
  if (str.size() >= 2 &&    //
      str.front() == '"' && //
      str.back() == '"') {
    // Remove a quote from the front and back
    str.remove_prefix(1);
    str.remove_suffix(1);
  }
  return std::string(str);
}

std::string cmEscapeQuotes(cm::string_view str)
{
  std::string result;
  result.reserve(str.size());
  for (char const ch : str) {
    if (ch == '"') {
      result += '\\';
    }
    result += ch;
  }
  return result;
}

std::size_t cmLevenshteinDistance(cm::string_view a, cm::string_view b)
{
  if (a == b) {
    return 0;
  }

  std::size_t const aSize = a.size();
  std::size_t const bSize = b.size();
  if (aSize == 0) {
    return bSize;
  }
  if (bSize == 0) {
    return aSize;
  }

  std::vector<std::size_t> previous(bSize + 1, 0);
  std::vector<std::size_t> current(bSize + 1, 0);

  for (std::size_t j = 0; j <= bSize; ++j) {
    previous[j] = j;
  }

  for (std::size_t i = 1; i <= aSize; ++i) {
    current[0] = i;
    for (std::size_t j = 1; j <= bSize; ++j) {
      std::size_t const substitutionCost = (a[i - 1] == b[j - 1]) ? 0 : 1;
      current[j] = std::min({ previous[j] + 1, current[j - 1] + 1,
                              previous[j - 1] + substitutionCost });
    }
    previous.swap(current);
  }

  return previous[bSize];
}

std::string cmFindClosestString(cm::string_view input,
                                std::vector<std::string> const& candidates)
{
  if (candidates.empty()) {
    return std::string();
  }

  std::string best;
  std::size_t bestDistance = std::numeric_limits<std::size_t>::max();
  for (std::string const& candidate : candidates) {
    std::size_t const distance = cmLevenshteinDistance(input, candidate);
    if (distance < bestDistance) {
      bestDistance = distance;
      best = candidate;
    }
  }

  // Scale the acceptable edit distance with the input length (roughly one
  // typo per 5 characters), but keep it within [kMinDistance, kMaxDistance]
  // so very short inputs still allow one edit and very long inputs don't
  // start accepting barely-related matches.
  std::size_t const kMinDistance = 1;
  std::size_t const kMaxDistance = 4;
  std::size_t const maxDistance =
    std::min(kMaxDistance, std::max(kMinDistance, input.size() / 5 + 1));
  if (bestDistance > maxDistance) {
    return std::string();
  }

  return best;
}

namespace {
template <std::size_t N, typename T>
inline void MakeDigits(cm::string_view& view, char (&digits)[N],
                       char const* pattern, T value)
{
  int res = std::snprintf(digits, N, pattern, value);
  if (res > 0 && res < static_cast<int>(N)) {
    view = cm::string_view(digits, static_cast<std::size_t>(res));
  }
}
} // unnamed namespace

cmAlphaNum::cmAlphaNum(int val)
{
  MakeDigits(this->View_, this->Digits_, "%i", val);
}

cmAlphaNum::cmAlphaNum(unsigned int val)
{
  MakeDigits(this->View_, this->Digits_, "%u", val);
}

cmAlphaNum::cmAlphaNum(long int val)
{
  MakeDigits(this->View_, this->Digits_, "%li", val);
}

cmAlphaNum::cmAlphaNum(unsigned long int val)
{
  MakeDigits(this->View_, this->Digits_, "%lu", val);
}

cmAlphaNum::cmAlphaNum(long long int val)
{
  MakeDigits(this->View_, this->Digits_, "%lli", val);
}

cmAlphaNum::cmAlphaNum(unsigned long long int val)
{
  MakeDigits(this->View_, this->Digits_, "%llu", val);
}

cmAlphaNum::cmAlphaNum(float val)
{
  MakeDigits(this->View_, this->Digits_, "%g", static_cast<double>(val));
}

cmAlphaNum::cmAlphaNum(double val)
{
  MakeDigits(this->View_, this->Digits_, "%g", val);
}

std::string cmCatViews(
  std::initializer_list<std::pair<cm::string_view, std::string*>> views)
{
  std::size_t totalSize = 0;
  std::string* rvalueString = nullptr;
  std::size_t rvalueStringLength = 0;
  std::size_t rvalueStringOffset = 0;
  for (auto const& view : views) {
    // Find the rvalue string with the largest capacity.
    if (view.second &&
        (!rvalueString ||
         view.second->capacity() > rvalueString->capacity())) {
      rvalueString = view.second;
      rvalueStringLength = rvalueString->length();
      rvalueStringOffset = totalSize;
    }
    totalSize += view.first.size();
  }

  std::string result;
  std::string::size_type initialLen = 0;
  if (rvalueString && rvalueString->capacity() >= totalSize) {
    result = std::move(*rvalueString);
  } else {
    rvalueString = nullptr;
  }
  result.resize(totalSize);
  if (rvalueString && rvalueStringOffset > 0) {
    std::copy_backward(result.begin(), result.begin() + rvalueStringLength,
                       result.begin() + rvalueStringOffset +
                         rvalueStringLength);
  }
  std::string::iterator sit = result.begin() + initialLen;
  for (auto const& view : views) {
    if (rvalueString && view.second == rvalueString) {
      sit += rvalueStringLength;
    } else {
      sit = std::copy_n(view.first.data(), view.first.size(), sit);
    }
  }
  return result;
}

bool cmStrToLong(char const* str, long* value)
{
  errno = 0;
  char* endp;
  *value = strtol(str, &endp, 10);
  return (*endp == '\0') && (endp != str) && (errno == 0);
}

bool cmStrToLong(std::string const& str, long* value)
{
  return cmStrToLong(str.c_str(), value);
}

bool cmStrToULong(char const* str, unsigned long* value)
{
  errno = 0;
  char* endp;
  while (cmsysString_isspace(*str)) {
    ++str;
  }
  if (*str == '-') {
    return false;
  }
  *value = strtoul(str, &endp, 10);
  return (*endp == '\0') && (endp != str) && (errno == 0);
}

bool cmStrToULong(std::string const& str, unsigned long* value)
{
  return cmStrToULong(str.c_str(), value);
}

bool cmStrToLongLong(char const* str, long long* value)
{
  errno = 0;
  char* endp;
  *value = strtoll(str, &endp, 10);
  return (*endp == '\0') && (endp != str) && (errno == 0);
}

bool cmStrToLongLong(std::string const& str, long long* value)
{
  return cmStrToLongLong(str.c_str(), value);
}

bool cmStrToULongLong(char const* str, unsigned long long* value)
{
  errno = 0;
  char* endp;
  while (cmsysString_isspace(*str)) {
    ++str;
  }
  if (*str == '-') {
    return false;
  }
  *value = strtoull(str, &endp, 10);
  return (*endp == '\0') && (endp != str) && (errno == 0);
}

bool cmStrToULongLong(std::string const& str, unsigned long long* value)
{
  return cmStrToULongLong(str.c_str(), value);
}

std::string cmJoin(std::vector<std::string> const& rng,
                   cm::string_view separator, cm::string_view initial)
{
  return cmJoinStrings(rng, separator, initial);
}

std::string cmJoin(cmStringRange rng, cm::string_view separator,
                   cm::string_view initial)
{
  return cmJoinStrings(rng, separator, initial);
}
