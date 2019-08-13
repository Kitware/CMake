/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmStringAlgorithms.h"

#include <algorithm>
#include <cstdio>

std::string cmTrimWhitespace(cm::string_view str)
{
  auto start = str.begin();
  while (start != str.end() && cmIsSpace(*start)) {
    ++start;
  }
  if (start == str.end()) {
    return std::string();
  }
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
  MakeDigits(View_, Digits_, "%i", val);
}

cmAlphaNum::cmAlphaNum(unsigned int val)
{
  MakeDigits(View_, Digits_, "%u", val);
}

cmAlphaNum::cmAlphaNum(long int val)
{
  MakeDigits(View_, Digits_, "%li", val);
}

cmAlphaNum::cmAlphaNum(unsigned long int val)
{
  MakeDigits(View_, Digits_, "%lu", val);
}

cmAlphaNum::cmAlphaNum(long long int val)
{
  MakeDigits(View_, Digits_, "%lli", val);
}

cmAlphaNum::cmAlphaNum(unsigned long long int val)
{
  MakeDigits(View_, Digits_, "%llu", val);
}

cmAlphaNum::cmAlphaNum(float val)
{
  MakeDigits(View_, Digits_, "%g", static_cast<double>(val));
}

cmAlphaNum::cmAlphaNum(double val)
{
  MakeDigits(View_, Digits_, "%g", val);
}

std::string cmCatViews(std::initializer_list<cm::string_view> views)
{
  std::size_t total_size = 0;
  for (cm::string_view const& view : views) {
    total_size += view.size();
  }

  std::string result(total_size, '\0');
  std::string::iterator sit = result.begin();
  for (cm::string_view const& view : views) {
    sit = std::copy_n(view.data(), view.size(), sit);
  }
  return result;
}
