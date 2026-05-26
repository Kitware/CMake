/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cstddef>
#include <string>

#include <cm/string_view>

#include "cmCacheDocumentationTable.h"
#include "cmCachePatternTable.h"

#include "testCommon.h"

namespace {

// The table is consulted by cmake::ProcessCacheArg via a binary search
// (std::lower_bound).  That requires the entries to be sorted strictly
// ascending by Name in byte-wise (ASCII) order.  These tests are the
// regression guard for that invariant against future hand-edits.

bool testNonEmpty()
{
  ASSERT_TRUE(cmCacheDocumentationTable::EntriesSize() > 0);
  ASSERT_TRUE(cmCacheDocumentationTable::EntriesBegin() !=
              cmCacheDocumentationTable::EntriesEnd());
  return true;
}

bool testSortedAndUnique()
{
  auto const* const first = cmCacheDocumentationTable::EntriesBegin();
  auto const* const last = cmCacheDocumentationTable::EntriesEnd();
  for (auto const* it = first; it != last; ++it) {
    if (it == first) {
      continue;
    }
    auto const& prev = *(it - 1);
    auto const& curr = *it;
    if (!(prev.Name < curr.Name)) {
      std::cout << "Entries are not strictly ascending: '" << prev.Name
                << "' is followed by '" << curr.Name << "' at index "
                << static_cast<std::size_t>(it - first) << '\n';
      if (prev.Name == curr.Name) {
        std::cout << "  (duplicate Name)\n";
      } else {
        std::cout << "  (out-of-order Name)\n";
      }
      return false;
    }
  }
  return true;
}

bool testNonEmptyFields()
{
  for (auto const* it = cmCacheDocumentationTable::EntriesBegin();
       it != cmCacheDocumentationTable::EntriesEnd(); ++it) {
    if (it->Name.empty()) {
      std::cout << "Empty Name at index "
                << static_cast<std::size_t>(
                     it - cmCacheDocumentationTable::EntriesBegin())
                << '\n';
      return false;
    }
    if (it->Summary.empty()) {
      std::cout << "Empty Summary for entry '" << it->Name << "'\n";
      return false;
    }
  }
  return true;
}

bool testNameCharacters()
{
  // Names must be printable ASCII without whitespace.  Anything else
  // would either be a copy/paste mishap or would cause std::lower_bound
  // to behave in surprising ways relative to user input.
  for (auto const* it = cmCacheDocumentationTable::EntriesBegin();
       it != cmCacheDocumentationTable::EntriesEnd(); ++it) {
    for (char const c : it->Name) {
      auto const u = static_cast<unsigned char>(c);
      if (u <= 0x20 || u >= 0x7f) {
        std::cout << "Entry '" << it->Name
                  << "' contains a non-printable or whitespace character\n";
        return false;
      }
    }
  }
  return true;
}

bool testSummaryIsSingleLine()
{
  // Summaries are rendered as one-line tooltips in cmake-gui / ccmake.
  // Embedded newlines or carriage returns break that rendering.
  for (auto const* it = cmCacheDocumentationTable::EntriesBegin();
       it != cmCacheDocumentationTable::EntriesEnd(); ++it) {
    for (char const c : it->Summary) {
      if (c == '\n' || c == '\r') {
        std::cout << "Summary for entry '" << it->Name
                  << "' contains an embedded newline or carriage return\n";
        return false;
      }
    }
  }
  return true;
}

bool testGetRoundTrip()
{
  // For every entry in the table, Get(Name) must return the same Summary.
  // This catches a regression where the binary-search predicate or the
  // sort invariant silently drifts apart.
  for (auto const* it = cmCacheDocumentationTable::EntriesBegin();
       it != cmCacheDocumentationTable::EntriesEnd(); ++it) {
    auto const got = cmCacheDocumentationTable::Get(it->Name);
    if (got.Summary != it->Summary) {
      std::cout << "Get(\"" << it->Name
                << "\").Summary did not round-trip to the table's Summary\n";
      return false;
    }
  }
  // And an obviously-unknown key must return the sentinel.
  auto const missing = cmCacheDocumentationTable::Get(
    cm::string_view("__definitely_not_a_cmake_variable__"));
  if (!missing.Summary.empty()) {
    std::cout << "Get() returned a non-empty Summary for an unknown key\n";
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// Pattern table tests.
// ---------------------------------------------------------------------------
//
// The pattern table is consulted by cmCacheDocumentationTable::Get on the
// exact-match miss path.  The tests below pin down its structural
// invariants and the most important matcher behaviors (lazy
// lookahead-anchored matching, fixed-length digit classes, exact wins over
// pattern, permissive matching with restriction-in-prose).

// Count the angle-bracketed placeholders in a pattern string.  A
// placeholder is the inclusive run from a ``<`` to the next ``>``.
std::size_t CountPlaceholders(cm::string_view pattern)
{
  std::size_t n = 0;
  for (std::size_t i = 0; i < pattern.size(); ++i) {
    if (pattern[i] == '<') {
      ++n;
      auto const close = pattern.find('>', i);
      if (close == cm::string_view::npos) {
        return n; // malformed; let testPatternTableFieldsValid flag it
      }
      i = close;
    }
  }
  return n;
}

// Total length of the literal (non-placeholder) characters in a pattern.
std::size_t LiteralLength(cm::string_view pattern)
{
  std::size_t total = 0;
  for (std::size_t i = 0; i < pattern.size(); ++i) {
    if (pattern[i] == '<') {
      auto const close = pattern.find('>', i);
      if (close == cm::string_view::npos) {
        break;
      }
      i = close;
      continue;
    }
    ++total;
  }
  return total;
}

bool testPatternTableNonEmpty()
{
  ASSERT_TRUE(cmCachePatternTable::EntriesSize() > 0);
  ASSERT_TRUE(cmCachePatternTable::EntriesBegin() !=
              cmCachePatternTable::EntriesEnd());
  return true;
}

bool testPatternTableFieldsValid()
{
  // Per-entry invariants: non-empty Pattern and Summary, at least one
  // placeholder, single-line printable ASCII Summary of bounded length,
  // and (a stricter rule derived from the matcher design) no two adjacent
  // placeholders with no separating literal -- the matcher's
  // lookahead-anchored semantics require a literal anchor between
  // placeholders to be unambiguous.
  for (auto const* it = cmCachePatternTable::EntriesBegin();
       it != cmCachePatternTable::EntriesEnd(); ++it) {
    if (it->Pattern.empty()) {
      std::cout << "Empty Pattern at index "
                << static_cast<std::size_t>(
                     it - cmCachePatternTable::EntriesBegin())
                << '\n';
      return false;
    }
    if (it->Summary.empty()) {
      std::cout << "Empty Summary for pattern '" << it->Pattern << "'\n";
      return false;
    }
    if (CountPlaceholders(it->Pattern) == 0) {
      std::cout << "Pattern '" << it->Pattern
                << "' has no <...> placeholder; "
                   "use cmCacheDocumentationTable for exact-match entries\n";
      return false;
    }
    if (it->Summary.size() > 200) {
      std::cout << "Summary for pattern '" << it->Pattern
                << "' exceeds the 200-character tooltip limit\n";
      return false;
    }
    for (char const c : it->Summary) {
      auto const u = static_cast<unsigned char>(c);
      if (u < 0x20 || u >= 0x7f) {
        std::cout << "Summary for pattern '" << it->Pattern
                  << "' contains a non-printable or non-ASCII character\n";
        return false;
      }
    }
    // Adjacent placeholders are forbidden (matcher cannot disambiguate
    // them without a separating literal anchor).  Look for ``><``.
    for (std::size_t i = 1; i < it->Pattern.size(); ++i) {
      if (it->Pattern[i] == '<' && it->Pattern[i - 1] == '>') {
        std::cout << "Pattern '" << it->Pattern
                  << "' contains adjacent placeholders; insert a literal "
                     "separator between them\n";
        return false;
      }
    }
  }
  return true;
}

bool testPatternTableOrdered()
{
  // kPatterns[] must obey ``(numPlaceholders ASC, totalLiteralLen DESC,
  // Pattern ASC)``.  This determinism guarantees that a contributor
  // re-ordering the array cannot silently flip which of several equally
  // valid pattern matches wins for a given input.
  auto const* const first = cmCachePatternTable::EntriesBegin();
  auto const* const last = cmCachePatternTable::EntriesEnd();
  for (auto const* it = first; it != last; ++it) {
    if (it == first) {
      continue;
    }
    auto const& prev = *(it - 1);
    auto const& curr = *it;
    auto const np_prev = CountPlaceholders(prev.Pattern);
    auto const np_curr = CountPlaceholders(curr.Pattern);
    auto const ll_prev = LiteralLength(prev.Pattern);
    auto const ll_curr = LiteralLength(curr.Pattern);
    bool ordered;
    if (np_prev != np_curr) {
      ordered = np_prev < np_curr;
    } else if (ll_prev != ll_curr) {
      ordered = ll_prev > ll_curr;
    } else {
      ordered = prev.Pattern < curr.Pattern;
    }
    if (!ordered) {
      std::cout << "Pattern table out of canonical order between '"
                << prev.Pattern << "' (placeholders=" << np_prev
                << ", literal_len=" << ll_prev << ") and '" << curr.Pattern
                << "' (placeholders=" << np_curr << ", literal_len=" << ll_curr
                << "). Expected (numPlaceholders ASC, totalLiteralLen DESC, "
                   "Pattern ASC).\n";
      return false;
    }
  }
  return true;
}

bool testPatternMatchLangPositive()
{
  // CMAKE_<LANG>_CLANG_TIDY must match for several different <LANG>
  // values, and all must return the same static Summary pointer (the
  // permissive matcher returns the pattern entry's storage verbatim).
  auto const cxx = cmCacheDocumentationTable::Get("CMAKE_CXX_CLANG_TIDY");
  auto const hip = cmCacheDocumentationTable::Get("CMAKE_HIP_CLANG_TIDY");
  auto const fortran =
    cmCacheDocumentationTable::Get("CMAKE_Fortran_CLANG_TIDY");
  ASSERT_TRUE(!cxx.Summary.empty());
  ASSERT_TRUE(!hip.Summary.empty());
  ASSERT_TRUE(!fortran.Summary.empty());
  // Same static-storage Summary pointer.
  ASSERT_TRUE(cxx.Summary.data() == hip.Summary.data());
  ASSERT_TRUE(cxx.Summary.data() == fortran.Summary.data());
  return true;
}

bool testPatternMatchConfigPositive()
{
  // CMAKE_<LANG>_FLAGS_<CONFIG> matches both standard configurations and
  // user-defined ones (the matcher does not enforce a canonical CONFIG
  // set; it accepts any identifier).
  auto const debug = cmCacheDocumentationTable::Get("CMAKE_CXX_FLAGS_DEBUG");
  auto const release =
    cmCacheDocumentationTable::Get("CMAKE_CXX_FLAGS_RELEASE");
  auto const coverage =
    cmCacheDocumentationTable::Get("CMAKE_CXX_FLAGS_COVERAGE");
  ASSERT_TRUE(!debug.Summary.empty());
  ASSERT_TRUE(!release.Summary.empty());
  ASSERT_TRUE(!coverage.Summary.empty());
  ASSERT_TRUE(debug.Summary.data() == release.Summary.data());
  ASSERT_TRUE(debug.Summary.data() == coverage.Summary.data());
  return true;
}

bool testPatternMatchNNNN()
{
  // <NNNN> is exactly 4 ASCII digits.  3-digit, 5-digit, and non-digit
  // inputs must all miss.
  ASSERT_TRUE(!cmCacheDocumentationTable::Get("CMAKE_POLICY_DEFAULT_CMP0048")
                 .Summary.empty());
  ASSERT_TRUE(cmCacheDocumentationTable::Get("CMAKE_POLICY_DEFAULT_CMP48")
                .Summary.empty());
  ASSERT_TRUE(cmCacheDocumentationTable::Get("CMAKE_POLICY_DEFAULT_CMP00048")
                .Summary.empty());
  ASSERT_TRUE(cmCacheDocumentationTable::Get("CMAKE_POLICY_DEFAULT_CMPxyz0")
                .Summary.empty());
  return true;
}

bool testPatternMatchN()
{
  // <n> is exactly 1 ASCII digit.  Multi-digit and non-digit must miss.
  ASSERT_TRUE(
    !cmCacheDocumentationTable::Get("CMAKE_MATCH_5").Summary.empty());
  ASSERT_TRUE(
    cmCacheDocumentationTable::Get("CMAKE_MATCH_12").Summary.empty());
  ASSERT_TRUE(cmCacheDocumentationTable::Get("CMAKE_MATCH_a").Summary.empty());
  return true;
}

bool testPatternMatchMultiPlaceholder()
{
  // CMAKE_CXX_FLAGS_DEBUG must match the two-placeholder pattern
  // ``CMAKE_<LANG>_FLAGS_<CONFIG>``, not the one-placeholder pattern
  // ``CMAKE_<LANG>_FLAGS`` (which fails full-input consumption on a
  // longer input by design).
  auto const got = cmCacheDocumentationTable::Get("CMAKE_CXX_FLAGS_DEBUG");
  ASSERT_TRUE(!got.Summary.empty());
  // Locate the two-placeholder entry and assert pointer equality.
  cm::string_view two_ph_summary;
  for (auto const* it = cmCachePatternTable::EntriesBegin();
       it != cmCachePatternTable::EntriesEnd(); ++it) {
    if (it->Pattern == "CMAKE_<LANG>_FLAGS_<CONFIG>") {
      two_ph_summary = it->Summary;
      break;
    }
  }
  ASSERT_TRUE(!two_ph_summary.empty());
  ASSERT_TRUE(got.Summary.data() == two_ph_summary.data());
  return true;
}

bool testPatternMatchPackage()
{
  // The single <PackageName>_ROOT pattern covers both mixed-case
  // (CMP0074) and upper-case (CMP0144) spellings: the identifier
  // lexical class ``[A-Za-z][A-Za-z0-9_]*`` accepts both.
  ASSERT_TRUE(!cmCacheDocumentationTable::Get("Boost_ROOT").Summary.empty());
  ASSERT_TRUE(!cmCacheDocumentationTable::Get("Qt6_ROOT").Summary.empty());
  ASSERT_TRUE(!cmCacheDocumentationTable::Get("BOOST_ROOT").Summary.empty());
  return true;
}

bool testExactWinsOverPattern()
{
  // CMAKE_CUDA_STANDARD is in the exact-match table and would also match
  // the pattern CMAKE_<LANG>_STANDARD.  The exact entry must win.
  auto const got = cmCacheDocumentationTable::Get("CMAKE_CUDA_STANDARD");
  ASSERT_TRUE(!got.Summary.empty());
  // Find the exact entry's Summary in cmCacheDocumentationTable.
  cm::string_view exact_summary;
  for (auto const* it = cmCacheDocumentationTable::EntriesBegin();
       it != cmCacheDocumentationTable::EntriesEnd(); ++it) {
    if (it->Name == "CMAKE_CUDA_STANDARD") {
      exact_summary = it->Summary;
      break;
    }
  }
  ASSERT_TRUE(!exact_summary.empty());
  // Pointer equality: the exact-match path returns the table's static
  // storage; the pattern-match path would return cmCachePatternTable's
  // (different) storage.
  ASSERT_TRUE(got.Summary.data() == exact_summary.data());
  return true;
}

bool testPatternMatchNegativeUnknown()
{
  // A name that matches no pattern at all returns the sentinel.
  auto const got =
    cmCacheDocumentationTable::Get("CMAKE_TOTAL_NONSENSE_VARIABLE_XYZZY");
  ASSERT_TRUE(got.Summary.empty());
  return true;
}

bool testPatternUnderscoreInLang()
{
  // The lazy matcher allows ``<LANG>`` to contain underscores (the
  // identifier class is ``[A-Za-z][A-Za-z0-9_]*``); the lookahead-anchor
  // search finds the right boundary.  This pins the design decision:
  // permissive matching with restriction-in-prose, not regex-strict
  // language enforcement.  A made-up ``MY_WEIRD_LANG`` is still picked
  // up by the CMAKE_<LANG>_CLANG_TIDY pattern.
  auto const got =
    cmCacheDocumentationTable::Get("CMAKE_MY_WEIRD_LANG_CLANG_TIDY");
  ASSERT_TRUE(!got.Summary.empty());
  return true;
}

bool testEveryPatternReachable()
{
  // For each pattern we synthesize a canonical instantiation by
  // replacing each placeholder with a class-appropriate token:
  // ``Aa`` for identifier-class placeholders (LANG / CONFIG /
  // PackageName / PROJECT-NAME / an-attribute / FEATURE / TYPE),
  // ``0000`` for the fixed-length ``<NNNN>``, and ``0`` for the
  // fixed-length ``<n>``.  The matcher's lazy + lookahead-anchored
  // semantics then either commit (Summary non-empty -> reachable) or
  // reject (Summary empty -> dead pattern, test fails).
  for (auto const* it = cmCachePatternTable::EntriesBegin();
       it != cmCachePatternTable::EntriesEnd(); ++it) {
    std::string instance;
    cm::string_view const pat = it->Pattern;
    for (std::size_t i = 0; i < pat.size();) {
      if (pat[i] != '<') {
        instance.push_back(pat[i]);
        ++i;
        continue;
      }
      auto const close = pat.find('>', i);
      if (close == cm::string_view::npos) {
        std::cout << "Malformed pattern '" << pat << "' (unterminated '<')\n";
        return false;
      }
      cm::string_view const ph = pat.substr(i, close - i + 1);
      if (ph == "<NNNN>") {
        instance += "0000";
      } else if (ph == "<n>") {
        instance += "0";
      } else {
        // Identifier-class placeholder (Ident or IdentHyphen).  ``Aa``
        // satisfies both lexical classes and is short enough not to
        // accidentally collide with any anchor literal in the table.
        instance += "Aa";
      }
      i = close + 1;
    }
    auto const got = cmCacheDocumentationTable::Get(instance);
    if (got.Summary.empty()) {
      std::cout << "Dead pattern '" << pat << "': canonical instantiation '"
                << instance
                << "' is not matched by any entry in the pattern table\n";
      return false;
    }
  }
  return true;
}

}

int testCacheDocumentationTable(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    testNonEmpty,
    testSortedAndUnique,
    testNonEmptyFields,
    testNameCharacters,
    testSummaryIsSingleLine,
    testGetRoundTrip,
    // Pattern-table cases.
    testPatternTableNonEmpty,
    testPatternTableFieldsValid,
    testPatternTableOrdered,
    testPatternMatchLangPositive,
    testPatternMatchConfigPositive,
    testPatternMatchNNNN,
    testPatternMatchN,
    testPatternMatchMultiPlaceholder,
    testPatternMatchPackage,
    testExactWinsOverPattern,
    testPatternMatchNegativeUnknown,
    testPatternUnderscoreInLang,
    testEveryPatternReachable,
  });
}
