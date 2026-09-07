/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
    file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"

#include KWSYS_HEADER(RegularExpression.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "RegularExpression.hxx.in"
#endif

#include <iostream>
#include <string>

namespace {

bool ExpectCompile(char const* pattern, bool expected)
{
  kwsys::RegularExpression re;
  bool const compiled = re.compile(pattern);
  if (compiled != expected) {
    std::cerr << "Unexpected compile result for pattern '" << pattern
              << "': " << compiled << " (expected " << expected << ")\n";
    return false;
  }
  return true;
}

bool ExpectFind(char const* pattern, char const* input, bool expected,
                std::string::size_type offset = 0, unsigned options = 0)
{
  kwsys::RegularExpression re;
  if (!re.compile(pattern)) {
    std::cerr << "Could not compile pattern '" << pattern << "'\n";
    return false;
  }

  kwsys::RegularExpressionMatch match;
  bool const found = re.find(input, match, offset, options);
  if (found != expected) {
    std::cerr << "Unexpected match result for pattern '" << pattern
              << "' against input '" << input << "': " << found
              << " (expected " << expected << ")\n";
    return false;
  }
  return true;
}

bool ExpectMatch(char const* pattern, char const* input,
                 std::string const& expectedMatch,
                 std::string::size_type expectedStart,
                 std::string::size_type expectedEnd,
                 std::string::size_type offset = 0, unsigned options = 0)
{
  kwsys::RegularExpression re;
  if (!re.compile(pattern)) {
    std::cerr << "Could not compile pattern '" << pattern << "'\n";
    return false;
  }

  kwsys::RegularExpressionMatch match;
  if (!re.find(input, match, offset, options)) {
    std::cerr << "Expected a match for pattern '" << pattern << "' against '"
              << input << "'\n";
    return false;
  }

  if (match.match(0) != expectedMatch || match.start() != expectedStart ||
      match.end() != expectedEnd) {
    std::cerr << "Unexpected match details for pattern '" << pattern
              << "' against input '" << input << "'\n"
              << "  got:    match='" << match.match(0)
              << "' start=" << match.start() << " end=" << match.end() << "\n"
              << "  expect: match='" << expectedMatch
              << "' start=" << expectedStart << " end=" << expectedEnd << "\n";
    return false;
  }

  return true;
}

bool ExpectGroups(char const* pattern, char const* input,
                  std::string const& group1, std::string const& group2)
{
  kwsys::RegularExpression re;
  if (!re.compile(pattern)) {
    std::cerr << "Could not compile pattern '" << pattern << "'\n";
    return false;
  }

  kwsys::RegularExpressionMatch match;
  if (!re.find(input, match)) {
    std::cerr << "Expected groups for pattern '" << pattern << "' against '"
              << input << "'\n";
    return false;
  }

  if (match.match(1) != group1 || match.match(2) != group2) {
    std::cerr << "Unexpected capture groups for pattern '" << pattern
              << "' against input '" << input << "'\n"
              << "  got:    g1='" << match.match(1) << "' g2='"
              << match.match(2) << "'\n"
              << "  expect: g1='" << group1 << "' g2='" << group2 << "'\n";
    return false;
  }

  return true;
}

bool ExpectGroupN(char const* pattern, char const* input, int n,
                  std::string const& expected)
{
  kwsys::RegularExpression re;
  if (!re.compile(pattern)) {
    std::cerr << "Could not compile pattern '" << pattern << "'\n";
    return false;
  }

  kwsys::RegularExpressionMatch match;
  if (!re.find(input, match)) {
    std::cerr << "Expected a match for pattern '" << pattern << "' against '"
              << input << "'\n";
    return false;
  }

  if (match.match(n) != expected) {
    std::cerr << "Unexpected capture group " << n << " for pattern '"
              << pattern << "' against input '" << input << "'\n"
              << "  got:    '" << match.match(n) << "'\n"
              << "  expect: '" << expected << "'\n";
    return false;
  }

  return true;
}

bool ExpectNumGroups(char const* pattern, int expected)
{
  kwsys::RegularExpression re;
  if (!re.compile(pattern)) {
    std::cerr << "Could not compile pattern '" << pattern << "'\n";
    return false;
  }

  if (re.num_groups() != expected) {
    std::cerr << "Unexpected num_groups for pattern '" << pattern
              << "': " << re.num_groups() << " (expected " << expected
              << ")\n";
    return false;
  }

  return true;
}

} // namespace

int testRegularExpression(int, char*[])
{
  int failed = 0;
  auto run = [&failed](char const* name, bool ok) {
    if (!ok) {
      ++failed;
      std::cerr << "[testRegularExpression] FAILED case: " << name << "\n";
    }
  };

  // Basic matching and capture groups.
  run("basic.match",
      ExpectMatch("([A-Za-z]+)-([0-9]+)", "id abc-42 tail", "abc-42", 3, 9));
  run("basic.groups",
      ExpectGroups("([A-Za-z]+)-([0-9]+)", "id abc-42 tail", "abc", "42"));

  // Greedy behavior for '*' and '+': KWSys prefers the longest prefix that
  // allows the rest of the pattern to match.
  run("greedy.star", ExpectMatch("a.*b", "a123b456b", "a123b456b", 0, 9));
  run("greedy.plus", ExpectMatch("a.+b", "axxbxxb", "axxbxxb", 0, 7));

  // Anchor behavior with offsets: '^' only matches at absolute start unless
  // BOL_AT_OFFSET is requested.
  run("anchor.offset_without_bol_flag", ExpectFind("^abc", "xxabc", false, 2));
  run("anchor.offset_with_bol_flag",
      ExpectMatch("^abc", "xxabc", "abc", 2, 5, 2,
                  kwsys::RegularExpression::BOL_AT_OFFSET));

  // Empty-match rejection with NONEMPTY_AT_OFFSET should skip the empty match
  // and find a non-empty one later when possible.
  run("emptymatch.default", ExpectMatch("a*", "baa", "", 0, 0));
  run("emptymatch.nonempty_at_offset",
      ExpectMatch("a*", "baa", "aa", 1, 3, 0,
                  kwsys::RegularExpression::NONEMPTY_AT_OFFSET));

  // Character class edge cases.
  run("charclass.range", ExpectMatch("[a-c]+", "zzabccdz", "abcc", 2, 6));
  run("charclass.negated",
      ExpectMatch("[^a-c]+", "abcXYZ123", "XYZ123", 3, 9));
  run("charclass.invalid_reverse_range", ExpectCompile("[z-a]", false));

  // Inside a character class, '.' is a literal character, not "any".
  run("charclass.dot_is_literal", ExpectMatch("[.]", "a.b", ".", 1, 2));
  run("charclass.dot_is_literal_no_wildcard_match",
      ExpectFind("[.]", "abc", false));

  // Unlike an "unmatched" opening bracket (below), a bare close bracket is
  // treated as a literal ']', which compiles fine.
  run("charclass.unmatched_close_bracket", ExpectCompile("abc]", true));

  // Backslash is not special inside a class, so a range's bounds are always
  // the raw characters adjacent to '-'. Here the range is computed against
  // 'n' (not an escaped newline), giving the descending, invalid range
  // 'o'-'a'.
  run("charclass.no_escape_invalid_range", ExpectCompile("[\\n-a]", false));

  // Class ranges work over raw byte values, including non-printable
  // control bytes such as \001 (0x01) through \037 (0x1F).
  run("charclass.octal_literal_byte_range",
      ExpectMatch("[\001-\037]+", "a\033b", "\033", 1, 2));
  run("charclass.octal_literal_byte_range_excludes_printable",
      ExpectFind("[\001-\037]", "abc", false));

  // Alternation and backtracking.
  run("alternation.backtracking", ExpectMatch("(ab|a)b", "ab", "ab", 0, 2));
  run("alternation.first_branch_wins",
      ExpectMatch("(foo|foobar)", "foobar", "foo", 0, 3));

  // -------------------------------------------------------------------------
  // Dialect differences from other standard engines.
  // -------------------------------------------------------------------------

  // Backreferences are not supported.
  run("dialect.no_backreference.literal_1_match",
      ExpectMatch("(a)\\1", "xa1y", "a1", 1, 3));
  run("dialect.no_backreference.no_aa_match",
      ExpectFind("(a)\\1", "xaa", false));

  // '{m,n}' is not a quantifier in KWSys, braces are literal characters.
  run("dialect.literal_braces.match",
      ExpectMatch("a{2}", "xxa{2}yy", "a{2}", 2, 6));
  run("dialect.literal_braces.no_quantifier",
      ExpectFind("a{2}", "xaa", false));

  // Escapes like '\\d' are not character classes; '\\d+' means one-or-more
  // literal 'd' characters.
  run("dialect.literal_d_escape.match",
      ExpectMatch("\\d+", "xdddyy", "ddd", 1, 4));
  run("dialect.literal_d_escape.no_digit_class",
      ExpectFind("\\d+", "x123yy", false));

  // Compile-time error detection.
  run("compile.unmatched_paren", ExpectCompile("(", false));
  run("compile.nested_quantifier", ExpectCompile("a**", false));
  run("compile.unmatched_bracket", ExpectCompile("[abc", false));
  run("compile.trailing_backslash", ExpectCompile("\\", false));
  run("compile.unmatched_close_paren", ExpectCompile(")", false));
  run("compile.quantifier_with_no_operand", ExpectCompile("*abc", false));
  run("compile.empty_pattern", ExpectCompile("", true));

  // NSUBEXP (32) caps the number of capture groups a pattern may declare.
  {
    std::string manyGroups;
    for (int i = 0; i < 31; ++i) {
      manyGroups += "(a)";
    }
    run("compile.max_capture_groups_ok",
        ExpectCompile(manyGroups.c_str(), true));
    manyGroups += "(a)";
    run("compile.too_many_capture_groups",
        ExpectCompile(manyGroups.c_str(), false));
  }

  // '$' anchors at the absolute end of the string.
  run("anchor.dollar_match", ExpectMatch("abc$", "xxabc", "abc", 2, 5));
  run("anchor.dollar_no_match_mid_string", ExpectFind("abc$", "abcxx", false));

  // '?' matches the preceding atom zero or one times.
  run("quantifier.optional_present",
      ExpectMatch("colou?r", "colour", "colour", 0, 6));
  run("quantifier.optional_absent",
      ExpectMatch("colou?r", "color", "color", 0, 5));

  // '.' matches any single character, including a newline.
  run("anychar.basic", ExpectMatch("a.c", "1a9c2", "a9c", 1, 4));
  run("anychar.matches_newline", ExpectMatch("a.b", "a\nb", "a\nb", 0, 3));

  // A backslash escapes metacharacters back to their literal meaning.
  run("escape.literal_dot", ExpectMatch("a\\.b", "xa.by", "a.b", 1, 4));
  run("escape.literal_dot_rejects_any_char",
      ExpectFind("a\\.b", "xaxby", false));
  run("escape.literal_paren", ExpectMatch("a\\(b", "xa(by", "a(b", 1, 4));

  // Character class edge syntax: a leading ']' or '-' is treated as a
  // literal member of the class rather than closing it or starting a range.
  run("charclass.bracket_as_first_char",
      ExpectMatch("[]a]+", "]a]b", "]a]", 0, 3));
  run("charclass.trailing_dash_literal",
      ExpectMatch("[a-]+", "a-a-b", "a-a-", 0, 4));
  run("charclass.leading_dash_literal",
      ExpectMatch("[-a]+", "-a-ab", "-a-a", 0, 4));
  run("charclass.single_char_range", ExpectMatch("[a-a]+", "aab", "aa", 0, 2));

  // Matching is case-sensitive.
  run("case.sensitive_class", ExpectMatch("[A-Z]+", "abcXYZdef", "XYZ", 3, 6));

  // Capture groups nest and are numbered in the order their '(' appears.
  run("groups.nested.group1", ExpectGroupN("((a)(b))", "xaby", 1, "ab"));
  run("groups.nested.group2", ExpectGroupN("((a)(b))", "xaby", 2, "a"));
  run("groups.nested.group3", ExpectGroupN("((a)(b))", "xaby", 3, "b"));
  run("groups.num_groups.two", ExpectNumGroups("([A-Za-z]+)-([0-9]+)", 2));
  run("groups.num_groups.nested", ExpectNumGroups("((a)(b))", 3));
  run("groups.num_groups.none", ExpectNumGroups("abc", 0));

  // find() with an offset at (but not past) the end of the string.
  run("offset.at_string_end_no_match", ExpectFind("abc", "ab", false, 2));
  run("offset.at_string_end_empty_match", ExpectMatch("a*", "b", "", 1, 1, 1));

  // -------------------------------------------------------------------------
  // API surface beyond compile() and find().
  // -------------------------------------------------------------------------

  run("api.string_overloads", [] {
    kwsys::RegularExpression re;
    if (!re.compile(std::string("([a-z]+)-([0-9]+)"))) {
      return false;
    }
    // The input string must outlive the match.
    std::string const input("id abc-42 tail");
    if (!re.find(input)) {
      return false;
    }
    return re.match(0) == "abc-42";
  }());

  run("api.is_valid_set_invalid", [] {
    kwsys::RegularExpression re("abc");
    if (!re.is_valid()) {
      return false;
    }
    re.set_invalid();
    return !re.is_valid();
  }());

  run("api.copy_and_equality", [] {
    kwsys::RegularExpression re1("(a)(b)");
    kwsys::RegularExpression re2(re1); // Copy constructor.
    if (!(re1 == re2)) {
      return false;
    }
    kwsys::RegularExpression re3;
    re3 = re1; // Copy assignment.
    if (!(re1 == re3) || (re1 != re3)) {
      return false;
    }
    kwsys::RegularExpression different("xyz");
    return !(re1 == different);
  }());

  run("api.deep_equal", [] {
    kwsys::RegularExpression re1("(a)(b)");
    kwsys::RegularExpression re2(re1);
    if (!re1.find("xaby")) {
      return false;
    }
    if (re1.deep_equal(re2)) { // re2 has not matched yet.
      return false;
    }
    if (!re2.find("xaby")) {
      return false;
    }
    return re1.deep_equal(re2); // Same pattern, same match position.
  }());

  if (failed) {
    std::cerr << "[testRegularExpression] Total failed cases: " << failed
              << "\n";
  }

  return failed == 0 ? 0 : 1;
}
