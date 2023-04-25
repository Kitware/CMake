/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmStringAlgorithms.h"

#include <algorithm>
#include <cerrno>
#include <cstddef> // IWYU pragma: keep
#include <cstdio>
#include <cstdlib>
#include <iterator>

std::string cmTrimWhitespace(cm::string_view str)
{
  // XXX(clang-tidy): This declaration and the next cannot be `const auto*`
  // because the qualification of `auto` is platform-dependent.
  // NOLINTNEXTLINE(readability-qualified-auto)
  auto start = str.begin();
  while (start != str.end() && cmIsSpace(*start)) {
    ++start;
  }
  if (start == str.end()) {
    return std::string();
  }
  // NOLINTNEXTLINE(readability-qualified-auto)
  auto stop = str.end() - 1;
  while (cmIsSpace(*stop)) {
    --stop;
  }
  return std::string(start, stop + 1);
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
  for (const char ch : str) {
    if (ch == '"') {
      result += '\\';
    }
    result += ch;
  }
  return result;
}

std::vector<std::string> cmTokenize(cm::string_view str, cm::string_view sep)
{
  std::vector<std::string> tokens;
  cm::string_view::size_type tokend = 0;

  do {
    cm::string_view::size_type tokstart = str.find_first_not_of(sep, tokend);
    if (tokstart == cm::string_view::npos) {
      break; // no more tokens
    }
    tokend = str.find_first_of(sep, tokstart);
    if (tokend == cm::string_view::npos) {
      tokens.emplace_back(str.substr(tokstart));
    } else {
      tokens.emplace_back(str.substr(tokstart, tokend - tokstart));
    }
  } while (tokend != cm::string_view::npos);

  if (tokens.empty()) {
    tokens.emplace_back();
  }
  return tokens;
}

namespace {
template <std::size_t N, typename T>
inline void MakeDigits(cm::string_view& view, char (&digits)[N],
                       const char* pattern, T value)
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

bool cmStrToLong(const char* str, long* value)
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

bool cmStrToULong(const char* str, unsigned long* value)
{
  errno = 0;
  char* endp;
  while (cmIsSpace(*str)) {
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

bool cmStrToLongLong(const char* str, long long* value)
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

bool cmStrToULongLong(const char* str, unsigned long long* value)
{
  errno = 0;
  char* endp;
  while (cmIsSpace(*str)) {
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

template <typename Range>
std::size_t getJoinedLength(Range const& rng, cm::string_view separator)
{
  std::size_t rangeLength{};
  for (auto const& item : rng) {
    rangeLength += item.size();
  }

  auto const separatorsLength = (rng.size() - 1) * separator.size();

  return rangeLength + separatorsLength;
}

template <typename Range>
std::string cmJoinImpl(Range const& rng, cm::string_view separator,
                       cm::string_view initial)
{
  if (rng.empty()) {
    return { std::begin(initial), std::end(initial) };
  }

  std::string result;
  result.reserve(initial.size() + getJoinedLength(rng, separator));
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

std::string cmJoin(std::vector<std::string> const& rng,
                   cm::string_view separator, cm::string_view initial)
{
  return cmJoinImpl(rng, separator, initial);
}

std::string cmJoin(cmStringRange const& rng, cm::string_view separator,
                   cm::string_view initial)
{
  return cmJoinImpl(rng, separator, initial);
}
