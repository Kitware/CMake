/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"

namespace {

struct Result
{
  bool Option1 = false;
  bool Option2 = false;

  std::string String1;
  std::string String2;

  std::vector<std::string> List1;
  std::vector<std::string> List2;
  std::vector<std::string> List3;

  std::vector<std::vector<std::string>> Multi1;
  std::vector<std::vector<std::string>> Multi2;
  std::vector<std::vector<std::string>> Multi3;
};

std::initializer_list<cm::string_view> const args = {
  /* clang-format off */
  "OPTION_1",                // option
  "STRING_1",                // string arg missing value
  "STRING_2", "foo", "bar",  // string arg + unparsed value
  "LIST_1",                  // list arg missing values
  "LIST_2", "foo", "bar",    // list arg with 2 elems
  "LIST_3", "bar",           // list arg ...
  "LIST_3", "foo",           // ... with continuation
  "MULTI_2",                 // multi list with 0 lists
  "MULTI_3", "foo", "bar",   // multi list with first list with two elems
  "MULTI_3", "bar", "foo",   // multi list with second list with two elems
  /* clang-format on */
};

bool verifyResult(Result const& result,
                  std::vector<std::string> const& unparsedArguments,
                  std::vector<std::string> const& keywordsMissingValue)
{
  static std::vector<std::string> const foobar = { "foo", "bar" };
  static std::vector<std::string> const barfoo = { "bar", "foo" };
  static std::vector<std::string> const missing = { "STRING_1", "LIST_1" };

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return false;                                                           \
    }                                                                         \
  } while (false)

  ASSERT_TRUE(result.Option1);
  ASSERT_TRUE(!result.Option2);

  ASSERT_TRUE(result.String1.empty());
  ASSERT_TRUE(result.String2 == "foo");

  ASSERT_TRUE(result.List1.empty());
  ASSERT_TRUE(result.List2 == foobar);
  ASSERT_TRUE(result.List3 == barfoo);

  ASSERT_TRUE(result.Multi1.empty());
  ASSERT_TRUE(result.Multi2.size() == 1);
  ASSERT_TRUE(result.Multi2[0].empty());
  ASSERT_TRUE(result.Multi3.size() == 2);
  ASSERT_TRUE(result.Multi3[0] == foobar);
  ASSERT_TRUE(result.Multi3[1] == barfoo);

  ASSERT_TRUE(unparsedArguments.size() == 1);
  ASSERT_TRUE(unparsedArguments[0] == "bar");
  ASSERT_TRUE(keywordsMissingValue == missing);

  return true;
}

bool testArgumentParserDynamic()
{
  Result result;
  std::vector<std::string> unparsedArguments;
  std::vector<std::string> keywordsMissingValue;

  cmArgumentParser<void>{}
    .Bind("OPTION_1"_s, result.Option1)
    .Bind("OPTION_2"_s, result.Option2)
    .Bind("STRING_1"_s, result.String1)
    .Bind("STRING_2"_s, result.String2)
    .Bind("LIST_1"_s, result.List1)
    .Bind("LIST_2"_s, result.List2)
    .Bind("LIST_3"_s, result.List3)
    .Bind("MULTI_1"_s, result.Multi1)
    .Bind("MULTI_2"_s, result.Multi2)
    .Bind("MULTI_3"_s, result.Multi3)
    .Parse(args, &unparsedArguments, &keywordsMissingValue);

  return verifyResult(result, unparsedArguments, keywordsMissingValue);
}

bool testArgumentParserStatic()
{
  static auto const parser = //
    cmArgumentParser<Result>{}
      .Bind("OPTION_1"_s, &Result::Option1)
      .Bind("OPTION_2"_s, &Result::Option2)
      .Bind("STRING_1"_s, &Result::String1)
      .Bind("STRING_2"_s, &Result::String2)
      .Bind("LIST_1"_s, &Result::List1)
      .Bind("LIST_2"_s, &Result::List2)
      .Bind("LIST_3"_s, &Result::List3)
      .Bind("MULTI_1"_s, &Result::Multi1)
      .Bind("MULTI_2"_s, &Result::Multi2)
      .Bind("MULTI_3"_s, &Result::Multi3);

  std::vector<std::string> unparsedArguments;
  std::vector<std::string> keywordsMissingValue;
  Result const result =
    parser.Parse(args, &unparsedArguments, &keywordsMissingValue);

  return verifyResult(result, unparsedArguments, keywordsMissingValue);
}

} // namespace

int testArgumentParser(int /*unused*/, char* /*unused*/ [])
{
  if (!testArgumentParserDynamic()) {
    std::cout << "While executing testArgumentParserDynamic().\n";
    return -1;
  }

  if (!testArgumentParserStatic()) {
    std::cout << "While executing testArgumentParserStatic().\n";
    return -1;
  }

  return 0;
}
