/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmList.h"
#include "cmRange.h"
#include "cmString.hxx"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

class cmMakefile;

//
// Class offering various string operations which is used by
// * CMake string() command
// * $<STRING> generator expression
//

namespace cm {

class CMakeString
{
public:
  using size_type = cm::String::size_type;
  static auto const npos = cm::String::npos;

  using string_range = cmRange<std::vector<std::string>::const_iterator>;

  CMakeString(std::string const& str)
    : String_(cm::String::borrow(str))
  {
  }
  CMakeString(cm::string_view str)
    : String_(cm::String::borrow(str))
  {
  }
  CMakeString(cm::String const& str)
    : String_(str)
  {
  }
  CMakeString(cm::String&& str)
    : String_(std::move(str))
  {
  }
  CMakeString(cmValue value)
    : String_(cm::String::borrow(*value))
  {
  }
  CMakeString(string_range range,
              cm::string_view separator = cm::string_view{})
    : String_(cmJoin(range, separator))
  {
  }

  CMakeString() = default;
  CMakeString(CMakeString const&) = default;
  CMakeString(CMakeString&&) = default;

  CMakeString& operator=(CMakeString const& string)
  {
    if (this != &string) {
      this->String_ = string.String_;
    }
    return *this;
  }
  CMakeString& operator=(CMakeString&& string) noexcept
  {
    if (this != &string) {
      this->String_ = std::move(string.String_);
    }
    return *this;
  }
  CMakeString& operator=(cm::string_view string)
  {
    this->String_ = string;
    return *this;
  }

  // conversions
  string_view view() const noexcept { return this->String_.view(); }
  operator cm::string_view() noexcept { return this->String_.view(); }
  operator std::string const&() { return this->String_.str(); }

  size_type Size() const { return this->String_.size(); }
  size_type Length() const { return this->String_.size(); }

  enum class CompOperator
  {
    EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL
  };
  bool Compare(CompOperator op, cm::string_view other);

  enum class FindFrom
  {
    Begin,
    End
  };
  size_type Find(cm::string_view substring,
                 FindFrom from = FindFrom::Begin) const
  {
    return from == FindFrom::Begin ? this->String_.find(substring)
                                   : this->String_.rfind(substring);
  }

  enum class Regex
  {
    No,
    Yes
  };
  // Throw std::invalid_argument if regular expression is invalid
  //       std::runtime_error if replacement failed
  CMakeString& Replace(std::string const& matchExpression,
                       std::string const& replaceExpression,
                       Regex regex = Regex::No,
                       cmMakefile* makefile = nullptr);

  enum class MatchItems
  {
    Once,
    All
  };
  // Throw std::invalid_argument if regular expression is invalid
  cmList Match(std::string const& matchExpression,
               MatchItems matchItems = MatchItems::Once,
               cmMakefile* makefile = nullptr) const;

  CMakeString& Append(cm::string_view str)
  {
    this->String_.append(str);
    return *this;
  }
  CMakeString& Append(string_range range)
  {
    this->Append(cmJoin(range, {}));
    return *this;
  }
  CMakeString& Prepend(cm::string_view str)
  {
    this->String_.insert(0, str);
    return *this;
  }
  CMakeString& Prepend(string_range range)
  {
    this->Prepend(cmJoin(range, {}));
    return *this;
  }

  CMakeString& ToLower()
  {
    this->String_ = cmSystemTools::LowerCase(this->String_);
    return *this;
  }
  CMakeString& ToLower(cm::string_view str)
  {
    this->String_ = cmSystemTools::LowerCase(str);
    return *this;
  }
  CMakeString& ToUpper()
  {
    this->String_ = cmSystemTools::UpperCase(this->String_);
    return *this;
  }
  CMakeString& ToUpper(cm::string_view str)
  {
    this->String_ = cmSystemTools::UpperCase(str);
    return *this;
  }

  // Throw std::out_of_range if pos or count are outside of the expected range
  CMakeString Substring(long pos = 0, long count = -1) const;

  enum class StripItems
  {
    Space,
    Genex
  };
  CMakeString& Strip(StripItems stripItems = StripItems::Space);

  // Throw std::runtime_error  if quoting string failed
  enum class QuoteItems
  {
    Regex
  };
  CMakeString& Quote(QuoteItems quoteItems = QuoteItems::Regex);

  CMakeString& Repeat(size_type count);

  // Throw std::invalid_argument if the hash algorithm is invalid
  CMakeString& Hash(cm::string_view hashAlgorithm);

  // Throw std::invalid_argument if one of the codes is invalid
  CMakeString& FromASCII(string_range codes);
  CMakeString& ToHexadecimal() { return this->ToHexadecimal(this->String_); }
  CMakeString& ToHexadecimal(cm::string_view str);

  CMakeString& MakeCIdentifier()
  {
    this->String_ = cmSystemTools::MakeCidentifier(this->String_.str());
    return *this;
  }
  CMakeString& MakeCIdentifier(std::string const& str)
  {
    this->String_ = cmSystemTools::MakeCidentifier(str);
    return *this;
  }

  static cm::string_view const RandomDefaultAlphabet;
  // Throw std::invalid_argument if the alphabet is invalid
  //       std::out_of_range if length is outside valid range
  CMakeString& Random(std::size_t length = 5,
                      cm::string_view alphabet = RandomDefaultAlphabet)
  {
    return this->Random(cmSystemTools::RandomSeed(), length, alphabet);
  }
  CMakeString& Random(unsigned int seed, std::size_t length = 5,
                      cm::string_view alphabet = RandomDefaultAlphabet);

  enum class UTC
  {
    No,
    Yes
  };
  CMakeString& Timestamp(cm::string_view format, UTC utc = UTC::No);

  enum class UUIDType
  {
    MD5,
    SHA1
  };
  enum class Case
  {
    Lower,
    Upper
  };
  // Throw std::invalid_argument if namespace or the type are invalid
  //       std::runtime_error if the UUID cannot be generated
  CMakeString& UUID(cm::string_view nameSpace, cm::string_view name,
                    UUIDType type, Case uuidCase = Case::Lower);

private:
  static bool Seeded;
  cm::String String_;
};
}
