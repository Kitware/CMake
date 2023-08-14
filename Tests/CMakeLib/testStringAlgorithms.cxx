/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmStringAlgorithms.h"

int testStringAlgorithms(int /*unused*/, char* /*unused*/[])
{
  int failed = 0;

  auto assert_ok = [&failed](bool test, cm::string_view title) {
    if (test) {
      std::cout << "Passed: " << title << "\n";
    } else {
      std::cout << "Failed: " << title << "\n";
      ++failed;
    }
  };

  auto assert_string = [&failed](cm::string_view generated,
                                 cm::string_view expected,
                                 cm::string_view title) {
    if (generated == expected) {
      std::cout << "Passed: " << title << "\n";
    } else {
      std::cout << "Failed: " << title << "\n";
      std::cout << "Expected: " << expected << "\n";
      std::cout << "Got: " << generated << "\n";
      ++failed;
    }
  };

  // ----------------------------------------------------------------------
  // Test cmTrimWhitespace
  {
    std::string base = "base";
    std::string spaces = "  \f\f\n\n\r\r\t\t\v\v";
    assert_string(cmTrimWhitespace(spaces + base), base,
                  "cmTrimWhitespace front");
    assert_string(cmTrimWhitespace(base + spaces), base,
                  "cmTrimWhitespace back");
    assert_string(cmTrimWhitespace(spaces + base + spaces), base,
                  "cmTrimWhitespace front and back");
  }

  // ----------------------------------------------------------------------
  // Test cmRemoveQuotes
  {
    auto test = [&assert_string](cm::string_view source,
                                 cm::string_view expected,
                                 cm::string_view title) {
      assert_string(cmRemoveQuotes(source), expected, title);
    };

    test("", "", "cmRemoveQuotes empty");
    test("\"", "\"", "cmRemoveQuotes single quote");
    test("\"\"", "", "cmRemoveQuotes double quote");
    test("\"a", "\"a", "cmRemoveQuotes quote char");
    test("\"ab", "\"ab", "cmRemoveQuotes quote char char");
    test("a\"", "a\"", "cmRemoveQuotes char quote");
    test("ab\"", "ab\"", "cmRemoveQuotes char char quote");
    test("a", "a", "cmRemoveQuotes single char");
    test("ab", "ab", "cmRemoveQuotes two chars");
    test("abc", "abc", "cmRemoveQuotes three chars");
    test("\"abc\"", "abc", "cmRemoveQuotes quoted chars");
    test("\"\"abc\"\"", "\"abc\"", "cmRemoveQuotes quoted quoted chars");
  }

  // ----------------------------------------------------------------------
  // Test cmEscapeQuotes
  {
    assert_string(cmEscapeQuotes("plain"), "plain", "cmEscapeQuotes plain");
    std::string base = "\"base\"\"";
    std::string result = "\\\"base\\\"\\\"";
    assert_string(cmEscapeQuotes(base), result, "cmEscapeQuotes escaped");
  }

  // ----------------------------------------------------------------------
  // Test cmJoin
  {
    typedef std::string ST;
    typedef std::vector<std::string> VT;
    assert_string(cmJoin(ST("abc"), ";"), "a;b;c", "cmJoin std::string");
    assert_string(cmJoin(VT{}, ";"), "", "cmJoin std::vector empty");
    assert_string(cmJoin(VT{ "a" }, ";"), "a", "cmJoin std::vector single");
    assert_string(cmJoin(VT{ "a", "b", "c" }, ";"), "a;b;c",
                  "cmJoin std::vector multiple");
    assert_string(cmJoin(VT{ "a", "b", "c" }, "<=>"), "a<=>b<=>c",
                  "cmJoin std::vector long sep");
  }

  // ----------------------------------------------------------------------
  // Test cmTokenize
  {
    typedef std::vector<std::string> VT;
    assert_ok(cmTokenize("", ";") == VT{ "" }, "cmTokenize empty");
    assert_ok(cmTokenize(";", ";") == VT{ "" }, "cmTokenize sep");
    assert_ok(cmTokenize("abc", ";") == VT{ "abc" }, "cmTokenize item");
    assert_ok(cmTokenize("abc;", ";") == VT{ "abc" }, "cmTokenize item sep");
    assert_ok(cmTokenize(";abc", ";") == VT{ "abc" }, "cmTokenize sep item");
    assert_ok(cmTokenize("abc;;efg", ";") == VT{ "abc", "efg" },
              "cmTokenize item sep sep item");
    assert_ok(cmTokenize("a1;a2;a3;a4", ";") == VT{ "a1", "a2", "a3", "a4" },
              "cmTokenize multiple items");
  }

  // ----------------------------------------------------------------------
  // Test cmStrCat
  {
    int ni = -1100;
    unsigned int nui = 1100u;
    long int nli = -12000l;
    unsigned long int nuli = 12000ul;
    long long int nlli = -130000ll;
    unsigned long long int nulli = 130000ull;
    std::string val =
      cmStrCat("<test>", ni, ',', nui, ',', nli, ",", nuli, ", ", nlli,
               std::string(", "), nulli, cm::string_view("</test>"));
    std::string expect =
      "<test>-1100,1100,-12000,12000, -130000, 130000</test>";
    assert_string(val, expect, "cmStrCat strings and integers");
  }
  {
    float const val = 1.5f;
    float const div = 0.00001f;
    float f = 0.0f;
    std::istringstream(cmStrCat("", val)) >> f;
    f -= val;
    assert_ok((f < div) && (f > -div), "cmStrCat float");
  }
  {
    double const val = 1.5;
    double const div = 0.00001;
    double d = 0.0;
    std::istringstream(cmStrCat("", val)) >> d;
    d -= val;
    assert_ok((d < div) && (d > -div), "cmStrCat double");
  }
  {
    std::string val;
    std::string expect;
    val.reserve(50 * cmStrLen("cmStrCat move ") + 1);
    auto data = val.data();
    auto capacity = val.capacity();
    bool moved = true;
    for (int i = 0; i < 100; i++) {
      if (i % 2 == 0) {
        val = cmStrCat(std::move(val), "move ");
        expect += "move ";
      } else {
        val = cmStrCat("cmStrCat ", std::move(val));
        expect = "cmStrCat " + std::move(expect);
      }
      if (val.data() != data || val.capacity() != capacity) {
        moved = false;
      }
    }
    assert_ok(moved, "cmStrCat move");
    assert_string(val, expect, "cmStrCat move");
  }

  // ----------------------------------------------------------------------
  // Test cmWrap
  {
    typedef std::vector<std::string> VT;
    assert_string(cmWrap("<", VT{}, ">", "; "), //
                  "",                           //
                  "cmWrap empty, string prefix and suffix");
    assert_string(cmWrap("<", VT{ "abc" }, ">", "; "), //
                  "<abc>",                             //
                  "cmWrap single, string prefix and suffix");
    assert_string(cmWrap("<", VT{ "a1", "a2", "a3" }, ">", "; "), //
                  "<a1>; <a2>; <a3>",                             //
                  "cmWrap multiple, string prefix and suffix");

    assert_string(cmWrap('<', VT{}, '>', "; "), //
                  "",                           //
                  "cmWrap empty, char prefix and suffix");
    assert_string(cmWrap('<', VT{ "abc" }, '>', "; "), //
                  "<abc>",                             //
                  "cmWrap single, char prefix and suffix");
    assert_string(cmWrap('<', VT{ "a1", "a2", "a3" }, '>', "; "), //
                  "<a1>; <a2>; <a3>",                             //
                  "cmWrap multiple, char prefix and suffix");
  }

  // ----------------------------------------------------------------------
  // Test cmHas(Literal)Prefix and cmHas(Literal)Suffix
  {
    std::string str("abc");
    assert_ok(cmHasPrefix(str, 'a'), "cmHasPrefix char");
    assert_ok(!cmHasPrefix(str, 'c'), "cmHasPrefix char not");
    assert_ok(cmHasPrefix(str, "ab"), "cmHasPrefix string");
    assert_ok(!cmHasPrefix(str, "bc"), "cmHasPrefix string not");
    assert_ok(cmHasPrefix(str, str), "cmHasPrefix complete string");
    assert_ok(cmHasLiteralPrefix(str, "ab"), "cmHasLiteralPrefix string");
    assert_ok(!cmHasLiteralPrefix(str, "bc"), "cmHasLiteralPrefix string not");

    assert_ok(cmHasSuffix(str, 'c'), "cmHasSuffix char");
    assert_ok(!cmHasSuffix(str, 'a'), "cmHasSuffix char not");
    assert_ok(cmHasSuffix(str, "bc"), "cmHasSuffix string");
    assert_ok(!cmHasSuffix(str, "ab"), "cmHasSuffix string not");
    assert_ok(cmHasSuffix(str, str), "cmHasSuffix complete string");
    assert_ok(cmHasLiteralSuffix(str, "bc"), "cmHasLiteralSuffix string");
    assert_ok(!cmHasLiteralSuffix(str, "ab"), "cmHasLiteralPrefix string not");
  }

  // ----------------------------------------------------------------------
  // Test cmStrToLong
  {
    long value;
    assert_ok(cmStrToLong("1", &value) && value == 1,
              "cmStrToLong parses a positive decimal integer.");
    assert_ok(cmStrToLong(" 1", &value) && value == 1,
              "cmStrToLong parses a decimal integer after whitespace.");

    assert_ok(cmStrToLong("-1", &value) && value == -1,
              "cmStrToLong parses a negative decimal integer.");
    assert_ok(
      cmStrToLong(" -1", &value) && value == -1,
      "cmStrToLong parses a negative decimal integer after whitespace.");

    assert_ok(!cmStrToLong("1x", &value),
              "cmStrToLong rejects trailing content.");
  }

  // ----------------------------------------------------------------------
  // Test cmStrToULong
  {
    unsigned long value;
    assert_ok(cmStrToULong("1", &value) && value == 1,
              "cmStrToULong parses a decimal integer.");
    assert_ok(cmStrToULong(" 1", &value) && value == 1,
              "cmStrToULong parses a decimal integer after whitespace.");
    assert_ok(!cmStrToULong("-1", &value),
              "cmStrToULong rejects a negative number.");
    assert_ok(!cmStrToULong(" -1", &value),
              "cmStrToULong rejects a negative number after whitespace.");
    assert_ok(!cmStrToULong("1x", &value),
              "cmStrToULong rejects trailing content.");
  }

  // ----------------------------------------------------------------------
  // Test cmStrToLongLong
  {
    long long value;
    assert_ok(cmStrToLongLong("1", &value) && value == 1,
              "cmStrToLongLong parses a positive decimal integer.");
    assert_ok(cmStrToLongLong(" 1", &value) && value == 1,
              "cmStrToLongLong parses a decimal integer after whitespace.");

    assert_ok(cmStrToLongLong("-1", &value) && value == -1,
              "cmStrToLongLong parses a negative decimal integer.");
    assert_ok(
      cmStrToLongLong(" -1", &value) && value == -1,
      "cmStrToLongLong parses a negative decimal integer after whitespace.");

    assert_ok(!cmStrToLongLong("1x", &value),
              "cmStrToLongLong rejects trailing content.");
  }

  // ----------------------------------------------------------------------
  // Test cmStrToULongLong
  {
    unsigned long long value;
    assert_ok(cmStrToULongLong("1", &value) && value == 1,
              "cmStrToULongLong parses a decimal integer.");
    assert_ok(cmStrToULongLong(" 1", &value) && value == 1,
              "cmStrToULongLong parses a decimal integer after whitespace.");
    assert_ok(!cmStrToULongLong("-1", &value),
              "cmStrToULongLong rejects a negative number.");
    assert_ok(!cmStrToULongLong(" -1", &value),
              "cmStrToULongLong rejects a negative number after whitespace.");
    assert_ok(!cmStrToULongLong("1x", &value),
              "cmStrToULongLong rejects trailing content.");
  }

  // ----------------------------------------------------------------------
  // Test cmStrLen
  {
    constexpr auto len = cmStrLen("Hello world!");
    assert_ok(len == 12,
              "cmStrLen returns length of non-empty literal string");
    assert_ok(cmStrLen("") == 0,
              "cmStrLen returns length of empty literal string");
  }

  return failed;
}
