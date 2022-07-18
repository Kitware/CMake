/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"

namespace {

struct Result : public ArgumentParser::ParseResult
{
  bool Option1 = false;
  bool Option2 = false;

  std::string String1;
  cm::optional<std::string> String2;
  cm::optional<std::string> String3;
  ArgumentParser::Maybe<std::string> String4;

  ArgumentParser::NonEmpty<std::vector<std::string>> List1;
  ArgumentParser::NonEmpty<std::vector<std::string>> List2;
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> List3;
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> List4;
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> List5;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> List6;

  std::vector<std::vector<std::string>> Multi1;
  std::vector<std::vector<std::string>> Multi2;
  cm::optional<std::vector<std::vector<std::string>>> Multi3;
  cm::optional<std::vector<std::vector<std::string>>> Multi4;
};

std::initializer_list<cm::string_view> const args = {
  /* clang-format off */
  "OPTION_1",                // option
  // "OPTION_2",             // option that is not present
  "STRING_1",                // string arg missing value
  "STRING_2", "foo", "bar",  // string arg + unparsed value, presence captured
  // "STRING_3",             // string arg that is not present
  "STRING_4",                // string arg allowed to be missing value
  "LIST_1",                  // list arg missing values
  "LIST_2", "foo", "bar",    // list arg with 2 elems
  "LIST_3", "bar",           // list arg ...
  "LIST_3", "foo",           // ... with continuation
  "LIST_4",                  // list arg missing values, presence captured
  // "LIST_5",               // list arg that is not present
  "LIST_6",                  // list arg allowed to be empty
  "MULTI_2",                 // multi list with 0 lists
  "MULTI_3", "foo", "bar",   // multi list with first list with two elems
  "MULTI_3", "bar", "foo",   // multi list with second list with two elems
  // "MULTI_4",              // multi list arg that is not present
  /* clang-format on */
};

bool verifyResult(Result const& result,
                  std::vector<std::string> const& unparsedArguments,
                  std::vector<cm::string_view> const& keywordsMissingValue)
{
  static std::vector<std::string> const foobar = { "foo", "bar" };
  static std::vector<std::string> const barfoo = { "bar", "foo" };
  static std::vector<cm::string_view> const missing = { "STRING_1"_s,
                                                        "LIST_1"_s,
                                                        "LIST_4"_s };
  static std::map<cm::string_view, std::string> const keywordErrors = {
    { "STRING_1"_s, "  missing required value\n" },
    { "LIST_1"_s, "  missing required value\n" },
    { "LIST_4"_s, "  missing required value\n" }
  };

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return false;                                                           \
    }                                                                         \
  } while (false)

  ASSERT_TRUE(!result);

  ASSERT_TRUE(result.Option1);
  ASSERT_TRUE(!result.Option2);

  ASSERT_TRUE(result.String1.empty());
  ASSERT_TRUE(result.String2);
  ASSERT_TRUE(*result.String2 == "foo");
  ASSERT_TRUE(!result.String3);
  ASSERT_TRUE(result.String4.empty());

  ASSERT_TRUE(result.List1.empty());
  ASSERT_TRUE(result.List2 == foobar);
  ASSERT_TRUE(result.List3);
  ASSERT_TRUE(*result.List3 == barfoo);
  ASSERT_TRUE(result.List4);
  ASSERT_TRUE(result.List4->empty());
  ASSERT_TRUE(!result.List5);
  ASSERT_TRUE(result.List6);
  ASSERT_TRUE(result.List6->empty());

  ASSERT_TRUE(result.Multi1.empty());
  ASSERT_TRUE(result.Multi2.size() == 1);
  ASSERT_TRUE(result.Multi2[0].empty());
  ASSERT_TRUE(result.Multi3);
  ASSERT_TRUE((*result.Multi3).size() == 2);
  ASSERT_TRUE((*result.Multi3)[0] == foobar);
  ASSERT_TRUE((*result.Multi3)[1] == barfoo);
  ASSERT_TRUE(!result.Multi4);

  ASSERT_TRUE(unparsedArguments.size() == 1);
  ASSERT_TRUE(unparsedArguments[0] == "bar");
  ASSERT_TRUE(keywordsMissingValue == missing);

  ASSERT_TRUE(result.GetKeywordErrors().size() == keywordErrors.size());
  for (auto const& ke : result.GetKeywordErrors()) {
    auto const ki = keywordErrors.find(ke.first);
    ASSERT_TRUE(ki != keywordErrors.end());
    ASSERT_TRUE(ke.second == ki->second);
  }

  return true;
}

bool testArgumentParserDynamic()
{
  Result result;
  std::vector<std::string> unparsedArguments;
  std::vector<cm::string_view> keywordsMissingValue;

  static_cast<ArgumentParser::ParseResult&>(result) =
    cmArgumentParser<void>{}
      .Bind("OPTION_1"_s, result.Option1)
      .Bind("OPTION_2"_s, result.Option2)
      .Bind("STRING_1"_s, result.String1)
      .Bind("STRING_2"_s, result.String2)
      .Bind("STRING_3"_s, result.String3)
      .Bind("STRING_4"_s, result.String4)
      .Bind("LIST_1"_s, result.List1)
      .Bind("LIST_2"_s, result.List2)
      .Bind("LIST_3"_s, result.List3)
      .Bind("LIST_4"_s, result.List4)
      .Bind("LIST_5"_s, result.List5)
      .Bind("LIST_6"_s, result.List6)
      .Bind("MULTI_1"_s, result.Multi1)
      .Bind("MULTI_2"_s, result.Multi2)
      .Bind("MULTI_3"_s, result.Multi3)
      .Bind("MULTI_4"_s, result.Multi4)
      .Parse(args, &unparsedArguments, &keywordsMissingValue);

  return verifyResult(result, unparsedArguments, keywordsMissingValue);
}

static auto const parserStatic = //
  cmArgumentParser<Result>{}
    .Bind("OPTION_1"_s, &Result::Option1)
    .Bind("OPTION_2"_s, &Result::Option2)
    .Bind("STRING_1"_s, &Result::String1)
    .Bind("STRING_2"_s, &Result::String2)
    .Bind("STRING_3"_s, &Result::String3)
    .Bind("STRING_4"_s, &Result::String4)
    .Bind("LIST_1"_s, &Result::List1)
    .Bind("LIST_2"_s, &Result::List2)
    .Bind("LIST_3"_s, &Result::List3)
    .Bind("LIST_4"_s, &Result::List4)
    .Bind("LIST_5"_s, &Result::List5)
    .Bind("LIST_6"_s, &Result::List6)
    .Bind("MULTI_1"_s, &Result::Multi1)
    .Bind("MULTI_2"_s, &Result::Multi2)
    .Bind("MULTI_3"_s, &Result::Multi3)
    .Bind("MULTI_4"_s, &Result::Multi4)
  /* keep semicolon on own line */;

bool testArgumentParserStatic()
{
  std::vector<std::string> unparsedArguments;
  std::vector<cm::string_view> keywordsMissingValue;
  Result const result =
    parserStatic.Parse(args, &unparsedArguments, &keywordsMissingValue);
  return verifyResult(result, unparsedArguments, keywordsMissingValue);
}

bool testArgumentParserStaticBool()
{
  std::vector<std::string> unparsedArguments;
  std::vector<cm::string_view> keywordsMissingValue;
  Result result;
  ASSERT_TRUE(parserStatic.Parse(result, args, &unparsedArguments,
                                 &keywordsMissingValue) == false);
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

  if (!testArgumentParserStaticBool()) {
    std::cout << "While executing testArgumentParserStaticBool().\n";
    return -1;
  }

  return 0;
}
