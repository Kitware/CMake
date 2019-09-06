/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmStringAlgorithms.h"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

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

void cmExpandList(cm::string_view arg, std::vector<std::string>& argsOut,
                  bool emptyArgs)
{
  // If argument is empty, it is an empty list.
  if (!emptyArgs && arg.empty()) {
    return;
  }

  // if there are no ; in the name then just copy the current string
  if (arg.find(';') == cm::string_view::npos) {
    argsOut.emplace_back(arg);
    return;
  }

  std::string newArg;
  // Break the string at non-escaped semicolons not nested in [].
  int squareNesting = 0;
  cm::string_view::iterator last = arg.begin();
  cm::string_view::iterator const cend = arg.end();
  for (cm::string_view::iterator c = last; c != cend; ++c) {
    switch (*c) {
      case '\\': {
        // We only want to allow escaping of semicolons.  Other
        // escapes should not be processed here.
        cm::string_view::iterator cnext = c + 1;
        if ((cnext != cend) && *cnext == ';') {
          newArg.append(last, c);
          // Skip over the escape character
          last = cnext;
          c = cnext;
        }
      } break;
      case '[': {
        ++squareNesting;
      } break;
      case ']': {
        --squareNesting;
      } break;
      case ';': {
        // Break the string here if we are not nested inside square
        // brackets.
        if (squareNesting == 0) {
          newArg.append(last, c);
          // Skip over the semicolon
          last = c + 1;
          if (!newArg.empty() || emptyArgs) {
            // Add the last argument if the string is not empty.
            argsOut.push_back(newArg);
            newArg.clear();
          }
        }
      } break;
      default: {
        // Just append this character.
      } break;
    }
  }
  newArg.append(last, cend);
  if (!newArg.empty() || emptyArgs) {
    // Add the last argument if the string is not empty.
    argsOut.push_back(std::move(newArg));
  }
}

std::vector<std::string> cmExpandedList(cm::string_view arg, bool emptyArgs)
{
  std::vector<std::string> argsOut;
  cmExpandList(arg, argsOut, emptyArgs);
  return argsOut;
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

bool cmIsInternallyOn(cm::string_view val)
{
  return (val.size() == 4) &&           //
    (val[0] == 'I' || val[0] == 'i') && //
    (val[1] == '_') &&                  //
    (val[2] == 'O' || val[2] == 'o') && //
    (val[3] == 'N' || val[3] == 'n');
}

bool cmIsNOTFOUND(cm::string_view val)
{
  return (val == "NOTFOUND") || cmHasLiteralSuffix(val, "-NOTFOUND");
}

bool cmIsOn(cm::string_view val)
{
  switch (val.size()) {
    case 1:
      return val[0] == '1' || val[0] == 'Y' || val[0] == 'y';
    case 2:
      return                                //
        (val[0] == 'O' || val[0] == 'o') && //
        (val[1] == 'N' || val[1] == 'n');
    case 3:
      return                                //
        (val[0] == 'Y' || val[0] == 'y') && //
        (val[1] == 'E' || val[1] == 'e') && //
        (val[2] == 'S' || val[2] == 's');
    case 4:
      return                                //
        (val[0] == 'T' || val[0] == 't') && //
        (val[1] == 'R' || val[1] == 'r') && //
        (val[2] == 'U' || val[2] == 'u') && //
        (val[3] == 'E' || val[3] == 'e');
    default:
      break;
  }

  return false;
}

bool cmIsOff(cm::string_view val)
{
  switch (val.size()) {
    case 0:
      return true;
    case 1:
      return val[0] == '0' || val[0] == 'N' || val[0] == 'n';
    case 2:
      return                                //
        (val[0] == 'N' || val[0] == 'n') && //
        (val[1] == 'O' || val[1] == 'o');
    case 3:
      return                                //
        (val[0] == 'O' || val[0] == 'o') && //
        (val[1] == 'F' || val[1] == 'f') && //
        (val[2] == 'F' || val[2] == 'f');
    case 5:
      return                                //
        (val[0] == 'F' || val[0] == 'f') && //
        (val[1] == 'A' || val[1] == 'a') && //
        (val[2] == 'L' || val[2] == 'l') && //
        (val[3] == 'S' || val[3] == 's') && //
        (val[4] == 'E' || val[4] == 'e');
    case 6:
      return                                //
        (val[0] == 'I' || val[0] == 'i') && //
        (val[1] == 'G' || val[1] == 'g') && //
        (val[2] == 'N' || val[2] == 'n') && //
        (val[3] == 'O' || val[3] == 'o') && //
        (val[4] == 'R' || val[4] == 'r') && //
        (val[5] == 'E' || val[5] == 'e');
    default:
      break;
  }

  return cmIsNOTFOUND(val);
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
