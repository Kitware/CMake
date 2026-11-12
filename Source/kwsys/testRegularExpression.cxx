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

// Check the start/end offsets of capture group 'n'. A group that did not
// participate in the match reports std::string::npos for both.
bool ExpectGroupSpan(char const* pattern, char const* input, int n,
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

  if (match.start(n) != expectedStart || match.end(n) != expectedEnd) {
    std::cerr << "Unexpected span for group " << n << " of pattern '"
              << pattern << "' against input '" << input << "'\n"
              << "  got:    start=" << match.start(n)
              << " end=" << match.end(n) << "\n"
              << "  expect: start=" << expectedStart << " end=" << expectedEnd
              << "\n";
    return false;
  }

  return true;
}

// Every byte value except NUL, in increasing order.
std::string AllNonNulBytes()
{
  std::string s;
  for (int c = 1; c <= 255; ++c) {
    s += static_cast<char>(c);
  }
  return s;
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

  // Search a non-terminated buffer without looking past its supplied size.
  run("api.bounded_find", [] {
    char const input[] = { 'x', 'a', 'b', 'c', 'z' };
    kwsys::RegularExpressionMatch match;
    for (char const* pattern : { "abc$", ".*abc$", "x(a[b-c]c)$" }) {
      kwsys::RegularExpression re(pattern);
      if (!re.find(input, 4, match) || match.end() != 4 ||
          !re.find(input, 4, 0, 0) || re.end() != 4 ||
          re.find(input, 3, match) || match.isValid()) {
        return false;
      }
    }
    kwsys::RegularExpression re("^abc$");
    if (re.find(input, 4, match, 1) ||
        !re.find(input, 4, match, 1,
                 kwsys::RegularExpression::BOL_AT_OFFSET) ||
        match.start() != 1 || match.match() != "abc") {
      return false;
    }
    kwsys::RegularExpression empty("^$");
    return empty.find(nullptr, 0, match) && match.start() == 0 &&
      match.end() == 0;
  }());

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

  // By contrast, a literal newline byte '\n' adjacent to '-' forms the
  // ascending, valid range '\n'(0x0A)-'a'(0x61).
  run("charclass.literal_newline_in_range", ExpectCompile("[\n-a]", true));
  run("charclass.literal_newline_range_match",
      ExpectMatch("[\n-a]+", "q\n19abz", "\n19a", 1, 5));

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
  // Escapes: '\<c>' always matches the literal byte <c>. None of the escape
  // sequences other engines give special meaning to are special here.
  // -------------------------------------------------------------------------

  run("escape.w_is_literal", ExpectMatch("\\w", "a_w", "w", 2, 3));
  run("escape.s_is_literal", ExpectMatch("\\s", "a s", "s", 2, 3));
  run("escape.s_no_whitespace_class", ExpectFind("\\s", " \t\n", false));
  run("escape.b_is_literal", ExpectMatch("a\\b", "a ab", "ab", 2, 4));
  run("escape.B_is_literal", ExpectMatch("\\B", "bB", "B", 1, 2));
  run("escape.n_is_literal", ExpectMatch("\\n", "\nn", "n", 1, 2));
  run("escape.n_not_newline", ExpectFind("\\n", "\n", false));
  run("escape.t_is_literal", ExpectMatch("\\t", "\tt", "t", 1, 2));
  run("escape.digit_1_is_literal", ExpectMatch("\\1", "a1", "1", 1, 2));
  run("escape.digit_0_is_literal", ExpectMatch("\\0", "a0", "0", 1, 2));
  run("escape.x_hex_is_literal", ExpectMatch("\\x41", "Ax41", "x41", 1, 4));
  run("escape.x_hex_not_byte", ExpectFind("\\x41", "A", false));
  run("escape.u_hex_is_literal",
      ExpectMatch("\\u0041", "Au0041", "u0041", 1, 6));
  run("escape.open_brace", ExpectMatch("\\{", "a{", "{", 1, 2));
  run("escape.close_bracket", ExpectMatch("\\]", "a]", "]", 1, 2));
  run("escape.open_bracket", ExpectMatch("\\[", "a[", "[", 1, 2));
  run("escape.backslash", ExpectMatch("\\\\", "a\\b", "\\", 1, 2));
  run("escape.star", ExpectMatch("a\\*", "aa*", "a*", 1, 3));
  run("escape.plus", ExpectMatch("a\\+", "aa+", "a+", 1, 3));
  run("escape.question", ExpectMatch("a\\?", "aa?", "a?", 1, 3));
  run("escape.pipe", ExpectMatch("a\\|b", "a|b", "a|b", 0, 3));
  run("escape.caret", ExpectMatch("a\\^", "a^", "a^", 0, 2));
  run("escape.dollar", ExpectMatch("\\$a", "x$a", "$a", 1, 3));
  run("escape.close_paren", ExpectMatch("\\)", "a)", ")", 1, 2));
  // An escaped character is a single atom, so a quantifier binds to it.
  run("escape.quantified_atom", ExpectMatch("x\\d+", "xddd", "xddd", 0, 4));

  // -------------------------------------------------------------------------
  // Bracket expressions.
  // -------------------------------------------------------------------------

  run("charclass.negated_close_bracket_first",
      ExpectMatch("[^]a]+", "]ab-", "b-", 2, 4));
  run("charclass.negated_dash_first", ExpectMatch("[^-]+", "--x-", "x", 2, 3));

  // Backslash is an ordinary member inside brackets, so '[/\]' is a complete
  // set of '/' and '\'.
  run("charclass.backslash_is_member",
      ExpectMatch("[/\\]+", "a/\\b", "/\\", 1, 3));
  // '[\]]' is the set {'\'} followed by a literal ']'.
  run("charclass.backslash_then_close",
      ExpectMatch("[\\]]", "]\\]", "\\]", 1, 3));
  run("charclass.backslash_does_not_escape_close",
      ExpectFind("[\\]]", "]", false));

  // There are no POSIX classes.
  run("charclass.no_posix_class.match",
      ExpectMatch("[[:alpha:]]", "x]a]", "a]", 2, 4));
  run("charclass.no_posix_class.no_letter_class",
      ExpectFind("[[:alpha:]]", "b]", false));
  run("charclass.no_posix_class.colon_member",
      ExpectMatch("[[:alpha:]]", ":]", ":]", 0, 2));

  // A range following another range starts just past the previous range's
  // end, so '[a-c-e]' is '[a-e]' and does not contain '-'.
  run("charclass.chained_range",
      ExpectMatch("[a-c-e]+", "-abcde-", "abcde", 1, 6));
  run("charclass.chained_range_no_dash", ExpectFind("[a-c-e]", "-", false));

  // A dash right before the closing bracket is literal, even after a member.
  run("charclass.dash_before_close", ExpectMatch("[a-]]", "a-]", "-]", 1, 3));

  // Ranges compare unsigned byte values, so they work above 0x7F.
  run("charclass.high_byte_range",
      ExpectMatch("[\x80-\xff]+",
                  "a\xc3\xa9"
                  "b",
                  "\xc3\xa9", 1, 3));
  run("charclass.high_byte_range_excludes_ascii",
      ExpectFind("[\x80-\xff]", "abc", false));
  run("charclass.high_byte_range_upper_bound",
      ExpectFind("[\x80-\x90]", "\xa0", false));
  run("charclass.high_byte_reverse_range",
      ExpectCompile("[\xff-\x80]", false));
  {
    std::string const all = AllNonNulBytes();
    run("charclass.full_byte_range",
        ExpectMatch("^[\x01-\xff]+$", all.c_str(), all, 0, 255));

    // Every non-NUL byte listed explicitly: ']' first, '-' last.
    std::string members;
    for (char c : all) {
      if (c != ']' && c != '-') {
        members += c;
      }
    }
    std::string const every = "[]" + members + "-]";
    run("charclass.every_byte_listed",
        ExpectMatch(("^" + every + "+$").c_str(), all.c_str(), all, 0, 255));
    std::string const none = "[^]" + members + "-]";
    run("charclass.every_byte_negated",
        ExpectFind(none.c_str(), all.c_str(), false));
  }

  // -------------------------------------------------------------------------
  // '.' matches any byte, including line terminators, but not the end.
  // -------------------------------------------------------------------------

  run("anychar.matches_cr", ExpectMatch("a.b", "a\rb", "a\rb", 0, 3));
  run("anychar.matches_high_byte",
      ExpectMatch("a.b",
                  "a\xff"
                  "b",
                  "a\xff"
                  "b",
                  0, 3));
  run("anychar.not_at_end", ExpectFind("a.", "a", false));

  // -------------------------------------------------------------------------
  // Anchors: '^' and '$' refer to the whole input, never to lines, and may
  // appear anywhere in a pattern.
  // -------------------------------------------------------------------------

  run("anchor.in_alternation.middle",
      ExpectMatch("(^|;)x(;|$)", "a;x", ";x", 1, 3));
  run("anchor.in_alternation.middle_groups",
      ExpectGroups("(^|;)x(;|$)", "a;x", ";", ""));
  run("anchor.in_alternation.start",
      ExpectMatch("(^|;)x(;|$)", "x;b", "x;", 0, 2));
  run("anchor.in_alternation.end_group_span",
      ExpectGroupSpan("(^|;)x(;|$)", "a;x", 2, 3, 3));
  run("anchor.caret_mid_pattern_never_matches",
      ExpectFind("a^b", "a^b", false));
  run("anchor.dollar_mid_pattern_never_matches",
      ExpectFind("a$b", "a$b", false));
  run("anchor.dollar_caret_empty_input", ExpectMatch("$^", "", "", 0, 0));
  run("anchor.caret_not_after_newline", ExpectFind("^b", "a\nb", false));
  run("anchor.dollar_not_before_newline", ExpectFind("a$", "a\n", false));

  // -------------------------------------------------------------------------
  // Quantifiers.
  // -------------------------------------------------------------------------

  // '*' and '+' may not apply to an operand that can match empty, but '?'
  // may.
  run("quantifier.optional_empty_capable_group", ExpectCompile("(a*)?", true));
  run("quantifier.optional_anchor_group", ExpectCompile("(^)?", true));
  run("quantifier.optional_empty_branch_group", ExpectCompile("(a|)?", true));
  run("quantifier.optional_empty_capable_group_span",
      ExpectGroupSpan("(a*)?", "", 1, 0, 0));

  // Priority: earlier alternatives win, quantifiers are greedy, and the
  // matcher backtracks into a quantifier when the rest fails.
  run("priority.first_alternative", ExpectMatch("ab|abc", "abc", "ab", 0, 2));
  run("priority.backtrack_through_groups.match",
      ExpectMatch("(a|ab)(c|bcd)(d*)", "abcd", "abcd", 0, 4));
  run("priority.backtrack_through_groups.group1",
      ExpectGroupN("(a|ab)(c|bcd)(d*)", "abcd", 1, "a"));
  run("priority.backtrack_through_groups.group2",
      ExpectGroupN("(a|ab)(c|bcd)(d*)", "abcd", 2, "bcd"));
  run("priority.backtrack_through_groups.group3",
      ExpectGroupSpan("(a|ab)(c|bcd)(d*)", "abcd", 3, 4, 4));
  run("priority.greedy_then_backtrack",
      ExpectMatch("a*ab", "aaab", "aaab", 0, 4));

  // -------------------------------------------------------------------------
  // Literal braces in patterns.
  // -------------------------------------------------------------------------

  run("dialect.literal_braces.variable_reference",
      ExpectMatch("\\${[A-Za-z_0-9]*}", "x${FOO}y", "${FOO}", 1, 7));
  run("dialect.literal_braces.cmake_named_curly",
      ExpectMatch("^[A-Za-z0-9/_.+-]+{", "a/b{x", "a/b{", 0, 4));

  // -------------------------------------------------------------------------
  // Capture groups.
  // -------------------------------------------------------------------------

  // A group reports its last participating iteration, and groups nested in
  // a repeated group are not reset by later iterations that skip them.
  run("groups.repeated.match", ExpectMatch("((a)|b)*", "ab", "ab", 0, 2));
  run("groups.repeated.no_reset", ExpectGroups("((a)|b)*", "ab", "b", "a"));
  run("groups.repeated.last_iteration",
      ExpectGroupSpan("((a)|b)*", "ab", 1, 1, 2));

  // A group that did not participate has no position.
  run("groups.nonparticipating.span",
      ExpectGroupSpan("(a)|b", "b", 1, std::string::npos, std::string::npos));
  run("groups.nonparticipating.string", ExpectGroupN("(a)|b", "b", 1, ""));

  // An empty group participates with an empty span.
  run("groups.empty.span", ExpectGroupSpan("()", "x", 1, 0, 0));
  run("groups.empty.after_char", ExpectGroupSpan("x()", "ax", 1, 2, 2));

  {
    std::string pattern;
    std::string input;
    for (int i = 0; i < 31; ++i) {
      pattern += "(a)";
      input += "a";
    }
    run("groups.max.num_groups", ExpectNumGroups(pattern.c_str(), 31));
    run("groups.max.last_group",
        ExpectGroupSpan(pattern.c_str(), input.c_str(), 31, 30, 31));
  }

  // -------------------------------------------------------------------------
  // Offsets and options.
  // -------------------------------------------------------------------------

  unsigned const bol = kwsys::RegularExpression::BOL_AT_OFFSET;
  unsigned const nonEmpty = kwsys::RegularExpression::NONEMPTY_AT_OFFSET;

  // NONEMPTY_AT_OFFSET rejects an empty match at the offset but still tries
  // the remaining alternatives at that same position.
  run("options.empty_alternative_default", ExpectMatch("(|a)", "a", "", 0, 0));
  run("options.nonempty_tries_next_alternative",
      ExpectMatch("(|a)", "a", "a", 0, 1, 0, nonEmpty));
  run("options.nonempty_tries_next_alternative_group",
      ExpectGroupSpan("(|a)", "a", 1, 0, 1, 0, nonEmpty));
  // Only an empty match at the offset itself is rejected.
  run("options.nonempty_allows_later_empty",
      ExpectMatch("a*", "bb", "", 1, 1, 0, nonEmpty));
  run("options.nonempty_at_end_no_match",
      ExpectFind("a*", "b", false, 1, nonEmpty));

  // All four option combinations for '^a*' at an offset.
  run("options.caret.none", ExpectFind("^a*", "xab", false, 1));
  run("options.caret.bol", ExpectMatch("^a*", "xb", "", 1, 1, 1, bol));
  run("options.caret.nonempty", ExpectFind("^a*", "xab", false, 1, nonEmpty));
  run("options.caret.bol_nonempty",
      ExpectMatch("^a*", "xab", "a", 1, 2, 1, bol | nonEmpty));
  run("options.caret.bol_nonempty_no_match",
      ExpectFind("^a*", "xb", false, 1, bol | nonEmpty));

  // Offset equal to the input length.
  run("offset.at_end_dollar", ExpectMatch("$", "abc", "", 3, 3, 3));
  run("offset.at_end_caret_bol", ExpectMatch("^", "abc", "", 3, 3, 3, bol));
  run("offset.at_end_empty_pattern", ExpectMatch("", "abc", "", 3, 3, 3));

  // -------------------------------------------------------------------------
  // The input is treated as a NUL-terminated string.
  // -------------------------------------------------------------------------

  run("nul.stops_matching", [] {
    kwsys::RegularExpression re("c");
    std::string const input("ab\0cd", 5);
    return !re.find(input);
  }());
  run("nul.dollar_matches_at_nul", [] {
    kwsys::RegularExpression re("b$");
    std::string const input("ab\0cd", 5);
    return re.find(input) && re.start() == 1 && re.end() == 2;
  }());

  // -------------------------------------------------------------------------
  // Compile errors.
  // -------------------------------------------------------------------------

  run("error.unmatched_open_paren", ExpectCompile("(", false));
  run("error.unmatched_close_paren", ExpectCompile("a)", false));
  run("error.unmatched_bracket", ExpectCompile("[abc", false));
  run("error.invalid_range", ExpectCompile("[b-a]", false));
  run("error.trailing_backslash", ExpectCompile("a\\", false));
  run("error.too_many_parens", [] {
    std::string pattern;
    for (int i = 0; i < 32; ++i) {
      pattern += "(a)";
    }
    return ExpectCompile(pattern.c_str(), false);
  }());

  // Quantifier errors.
  run("error.leading_star", ExpectCompile("*a", false));
  run("error.leading_plus", ExpectCompile("+a", false));
  run("error.leading_question", ExpectCompile("?a", false));
  run("error.star_after_pipe", ExpectCompile("a|*", false));
  run("error.star_after_open_paren", ExpectCompile("(*)", false));
  run("error.star_star", ExpectCompile("a**", false));
  run("error.lazy_star", ExpectCompile("a*?", false));
  run("error.lazy_plus", ExpectCompile("a+?", false));
  run("error.lazy_question", ExpectCompile("a??", false));
  run("error.possessive_star", ExpectCompile("a*+", false));
  run("error.star_on_caret", ExpectCompile("^*", false));
  run("error.star_on_star_group", ExpectCompile("(a*)*", false));
  run("error.plus_on_optional_group", ExpectCompile("(a?)+", false));
  run("error.star_on_empty_group", ExpectCompile("()*", false));
  run("error.plus_on_empty_branch_group", ExpectCompile("(|a)+", false));
  run("error.star_on_anchor_group", ExpectCompile("(^)*", false));

  // The compiled program must be smaller than 64 KiB.
  run("error.expression_too_big", [] {
    std::string pattern;
    for (int i = 0; i < 300; ++i) {
      pattern += "[\x01-\xff]";
    }
    return ExpectCompile(pattern.c_str(), false);
  }());
  run("compile.large_expression_ok", [] {
    std::string pattern;
    for (int i = 0; i < 200; ++i) {
      pattern += "[\x01-\xff]";
    }
    return ExpectCompile(pattern.c_str(), true);
  }());

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

  run("api.find_on_invalid", [] {
    kwsys::RegularExpression empty;
    if (empty.is_valid() || empty.find("abc")) {
      return false;
    }
    kwsys::RegularExpression re("a");
    re.set_invalid();
    kwsys::RegularExpressionMatch match;
    return !re.find("abc", match) && !match.isValid();
  }());

  run("api.copy_invalid", [] {
    kwsys::RegularExpression invalid;
    kwsys::RegularExpression copy(invalid);
    if (copy.is_valid()) {
      return false;
    }
    kwsys::RegularExpression assigned;
    assigned = invalid;
    return !assigned.is_valid();
  }());

  run("api.assign_invalid_over_valid", [] {
    kwsys::RegularExpression invalid;
    kwsys::RegularExpression re("abc");
    if (!re.find("xabc")) {
      return false;
    }
    re = invalid;
    return !re.is_valid() && !re.find("abc");
  }());

  run("api.copy_keeps_last_match", [] {
    kwsys::RegularExpression re("(b+)");
    if (!re.find("abbc")) {
      return false;
    }
    kwsys::RegularExpression copy(re);
    return copy.match(1) == "bb" && copy.start(1) == 1 && copy.end(1) == 3;
  }());

  run("api.equal_when_compiled_separately", [] {
    kwsys::RegularExpression re1("(a|b)*c");
    kwsys::RegularExpression re2("(a|b)*c");
    return re1 == re2 && !(re1 != re2);
  }());

  run("api.regmatch", [] {
    kwsys::RegularExpression re("(b)(c)?");
    if (!re.find("abd")) {
      return false;
    }
    kwsys::RegularExpressionMatch const& m = re.regMatch();
    return m.isValid() && m.start() == re.start() && m.end() == re.end() &&
      m.match(1) == "b" && m.start(2) == std::string::npos;
  }());

  run("api.match_object_clear", [] {
    kwsys::RegularExpression re("b");
    kwsys::RegularExpressionMatch match;
    if (!re.find("abc", match) || !match.isValid()) {
      return false;
    }
    match.clear();
    return !match.isValid() && match.start() == std::string::npos;
  }());

  run("api.failed_find_invalidates_match", [] {
    kwsys::RegularExpression re("b");
    kwsys::RegularExpressionMatch match;
    if (!re.find("abc", match)) {
      return false;
    }
    return !re.find("xyz", match) && !match.isValid();
  }());

  if (failed) {
    std::cerr << "[testRegularExpression] Total failed cases: " << failed
              << "\n";
  }

  return failed == 0 ? 0 : 1;
}
