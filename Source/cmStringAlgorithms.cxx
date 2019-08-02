/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmStringAlgorithms.h"

#include <algorithm>
#include <cstdio>

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
