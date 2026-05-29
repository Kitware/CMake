/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

// Built-in cache variable documentation pattern table.
//
// This file is hand-maintained.  Each entry's Pattern is a literal template
// string containing one or more angle-bracketed placeholders (``<LANG>``,
// ``<CONFIG>``, ``<PackageName>``, ``<PROJECT-NAME>``, ``<an-attribute>``,
// ``<NNNN>``, ``<n>``).  The Summary is a one-line tooltip installed as
// the ``HELPSTRING`` of cache variables created via
// ``cmake -D <var>=<value>`` whose name matches the template under the
// lexical classes defined below and for which no exact-match entry exists
// in ``cmCacheDocumentationTable``.
//
// The pattern table is consulted only on the miss path of the exact-match
// ``cmCacheDocumentationTable::Get`` lookup; exact entries always win.
//
// Maintenance discipline:
//   * Adding a new pattern: hand-write one entry below in the canonical
//     ordering ``(numPlaceholders ASC, totalLiteralLen DESC, Pattern ASC)``
//     and make sure that, with ``<`` and ``>`` stripped, the pattern
//     names an existing ``Help/variable/<NAME>.rst`` file.  The
//     ``testPatternTableOrdered`` regression test in
//     ``Tests/CMakeLib`` verifies the canonical ordering; the
//     ``CacheVarHelpCoverage`` lint verifies the structural-parity
//     invariant.
//   * Per-language or per-configuration restrictions documented in the
//     corresponding ``.rst`` file must be reproduced in the Summary
//     prose (the matcher is intentionally permissive; restrictions live
//     in the tooltip).
//   * Summaries must be single-line printable ASCII, no embedded
//     double quotes, no embedded newlines, and ``<= 200`` characters
//     (so that ``cmake-gui`` / ``ccmake`` render them legibly).  The
//     ``testPatternTableFieldsValid`` regression test enforces all of
//     these.
//
// Matcher semantics: lazy + lookahead-anchored, single pass, no
// backtracking.  For each placeholder, the matcher consumes the minimum
// class-valid prefix of the input that lets the next literal anchor
// match at the cursor.  The fixed-length classes ``<NNNN>`` and
// ``<n>`` consume exactly four or one ASCII digits respectively.
// Full-input consumption is required at the end.  First fully-matching
// entry in ``kPatterns[]`` wins.

#include "cmCachePatternTable.h"

#include <cstddef>
#include <iterator>
#include <string>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmCacheDocumentationTable.h"

namespace {

using cmCachePatternTable::Entry;

// The Entry layout below (two lines per row, ``{ "PATTERN",\n    "summary"
// },``) is intentional: it keeps the table easy to diff, review, and hand-edit
// against the corresponding Help/variable/*.rst manuals.  clang-format is
// suppressed across the array so contributors can update one Summary
// without the formatter reflowing every neighbor.
//
// Section headers ``// === N-placeholder, ... (K-literal-char bucket) ===``
// are organizational signposts that mirror the canonical ordering invariant
// enforced by ``testPatternTableOrdered``:
//
//     (numPlaceholders ASC, totalLiteralLen DESC, Pattern ASC)
//
// where ``totalLiteralLen`` is the byte count of the pattern after every
// ``<...>`` placeholder has been stripped.  Examples:
//
//   ``CMAKE_<LANG>_COMPILER_LAUNCHER``  -> ``CMAKE__COMPILER_LAUNCHER``  (24)
//   ``CMAKE_<LANG>_CLANG_TIDY``         -> ``CMAKE__CLANG_TIDY``         (17)
//   ``CMAKE_<LANG>_FLAGS``              -> ``CMAKE__FLAGS``              (12)
//
// Ordering literals DESC within a placeholder-count band is what makes
// ``Match()``'s first-match-wins linear scan correct: a more-anchored
// pattern (e.g. ``CMAKE_<LANG>_FLAGS_<CONFIG>_INIT``, literal=18) must
// shadow a less-anchored one (``CMAKE_<LANG>_FLAGS_<CONFIG>``, literal=13)
// so a lookup of ``CMAKE_CXX_FLAGS_DEBUG_INIT`` resolves to the former
// rather than lazily binding ``CONFIG`` to ``DEBUG_INIT`` against the
// latter.  Contributors adding a new entry: place it in the bucket whose
// literal-count matches yours, and update or add a new ``=== ... ===``
// header if your literal-count is novel.  The unit test catches mistakes.
/* clang-format off */
Entry const kPatterns[] = {

  // === 1-placeholder, (37-literal-char bucket) ===
  { "CMAKE_FRAMEWORK_MULTI_CONFIG_POSTFIX_<CONFIG>"_s,
    "Default framework filename postfix under configuration <CONFIG>."_s },

  // === 1-placeholder, (35-literal-char bucket) ===
  { "CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_<CONFIG>"_s,
    "Initializes COMPILE_PDB_OUTPUT_DIRECTORY_<CONFIG> target property."_s },
  { "CMAKE_INTERPROCEDURAL_OPTIMIZATION_<CONFIG>"_s,
    "Initializes INTERPROCEDURAL_OPTIMIZATION_<CONFIG> target property."_s },
  { "CMAKE_LINK_LIBRARY_USING_<FEATURE>_SUPPORTED"_s,
    "Set to TRUE if <FEATURE> is supported regardless of linker language."_s },

  // === 1-placeholder, (34-literal-char bucket) ===
  { "CMAKE_<LANG>_CLANG_TIDY_EXPORT_FIXES_DIR"_s,
    "Default value for the <LANG>_CLANG_TIDY_EXPORT_FIXES_DIR target property; directory where clang-tidy writes suggested fix .yaml files. Applies only when <LANG> is C, CXX, OBJC, or OBJCXX."_s },

  // === 1-placeholder, (33-literal-char bucket) ===
  { "CMAKE_LINK_GROUP_USING_<FEATURE>_SUPPORTED"_s,
    "Specifies if <FEATURE> is supported regardless of link language."_s },

  // === 1-placeholder, (31-literal-char bucket) ===
  { "CMAKE_ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>"_s,
    "Initializes ARCHIVE_OUTPUT_DIRECTORY_<CONFIG> target property."_s },
  { "CMAKE_LIBRARY_OUTPUT_DIRECTORY_<CONFIG>"_s,
    "Initializes LIBRARY_OUTPUT_DIRECTORY_<CONFIG> target property."_s },
  { "CMAKE_RUNTIME_OUTPUT_DIRECTORY_<CONFIG>"_s,
    "Initializes RUNTIME_OUTPUT_DIRECTORY_<CONFIG> target property."_s },
  { "CMAKE_USER_MAKE_RULES_OVERRIDE_<LANG>"_s,
    "Specify a CMake file that overrides platform information for <LANG>"_s },

  // === 1-placeholder, (30-literal-char bucket) ===
  { "CMAKE_LINK_LIBRARY_<FEATURE>_ATTRIBUTES"_s,
    "Defines behavior and attributes of the specified link library <FEATURE>."_s },

  // === 1-placeholder, (29-literal-char bucket) ===
  { "CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE_BEFORE"_s,
    "Path to a CMake language file included as the first step of any project() command call matching <PROJECT-NAME>. Intended for injecting custom code into project builds without modifying the source."_s },

  // === 1-placeholder, (27-literal-char bucket) ===
  { "CMAKE_<LANG>_INCLUDE_WHAT_YOU_USE"_s,
    "Default value for the <LANG>_INCLUDE_WHAT_YOU_USE target property; names an include-what-you-use command (and arguments) run alongside compilation. Applies only when <LANG> is C or CXX."_s },
  { "CMAKE_DISABLE_FIND_PACKAGE_<PackageName>"_s,
    "Set to TRUE to disable calls to find_package(<PackageName>); the call returns as if the package were not found, even when REQUIRED was specified."_s },
  { "CMAKE_PDB_OUTPUT_DIRECTORY_<CONFIG>"_s,
    "Initializes PDB_OUTPUT_DIRECTORY_<CONFIG> target property."_s },
  { "CMAKE_REQUIRE_FIND_PACKAGE_<PackageName>"_s,
    "Set to TRUE to make every call to find_package(<PackageName>) behave as if REQUIRED were specified, causing CMake to fail when the package is not found."_s },

  // === 1-placeholder, (26-literal-char bucket) ===
  { "CMAKE_MAP_IMPORTED_CONFIG_<CONFIG>"_s,
    "Initializes MAP_IMPORTED_CONFIG_<CONFIG> target property."_s },
  { "CMAKE_MODULE_LINKER_FLAGS_<CONFIG>"_s,
    "Flags used by the linker when creating a module library for the <CONFIG> configuration. Appended after the configuration-independent CMAKE_MODULE_LINKER_FLAGS."_s },
  { "CMAKE_SHARED_LINKER_FLAGS_<CONFIG>"_s,
    "Flags used by the linker when creating a shared library for the <CONFIG> configuration. Appended after the configuration-independent CMAKE_SHARED_LINKER_FLAGS."_s },
  { "CMAKE_STATIC_LINKER_FLAGS_<CONFIG>"_s,
    "Flags used by the static-library archiver when creating a static library for the <CONFIG> configuration. Appended after the configuration-independent CMAKE_STATIC_LINKER_FLAGS."_s },

  // === 1-placeholder, (25-literal-char bucket) ===
  { "CMAKE_LINK_LIBRARY_USING_<FEATURE>"_s,
    "Defines how to link a library for <FEATURE>."_s },

  // === 1-placeholder, (24-literal-char bucket) ===
  { "CMAKE_<LANG>_COMPILER_LAUNCHER"_s,
    "Default value for the <LANG>_COMPILER_LAUNCHER target property; launcher tool prepended to every <LANG> compile command. Applies only when <LANG> is C, CXX, Fortran, HIP, ISPC, OBJC, OBJCXX, or CUDA."_s },
  { "CMAKE_<LANG>_STANDARD_REQUIRED"_s,
    "Default value for the <LANG>_STANDARD_REQUIRED target property when a target is created. If TRUE, the requested language standard must be supported by the compiler."_s },
  { "CMAKE_<LANG>_VISIBILITY_PRESET"_s,
    "Default value for the <LANG>_VISIBILITY_PRESET target property when a target is created. Controls the default symbol visibility (default, hidden, protected, internal) for <LANG> code."_s },
  { "CMAKE_POLICY_DEFAULT_CMP<NNNN>"_s,
    "Default for CMake Policy CMP<NNNN> when otherwise left unset. Set to OLD or NEW to externally control the policy for projects that have not set it via cmake_minimum_required or cmake_policy."_s },
  { "CMAKE_POLICY_WARNING_CMP<NNNN>"_s,
    "Explicitly enable or disable the warning when CMake Policy CMP<NNNN> has not been set explicitly. Meaningful only for the policies that do not warn by default."_s },

  // === 1-placeholder, (23-literal-char bucket) ===
  { "CMAKE_EXE_LINKER_FLAGS_<CONFIG>"_s,
    "Flags used by the linker when creating an executable for the <CONFIG> configuration. Appended after the configuration-independent CMAKE_EXE_LINKER_FLAGS."_s },
  { "CMAKE_LINK_GROUP_USING_<FEATURE>"_s,
    "Defines how to link a group of libraries for <FEATURE>."_s },

  // === 1-placeholder, (22-literal-char bucket) ===
  { "CMAKE_<LANG>_LINKER_LAUNCHER"_s,
    "Default value for the <LANG>_LINKER_LAUNCHER target property; names a launcher tool prepended to every <LANG> link command. Applies only when <LANG> is C, CXX, OBJC, OBJCXX, or CUDA."_s },
  { "CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE"_s,
    "Path to a CMake language file included as the last step of any project() command call matching <PROJECT-NAME>. Intended for injecting custom code without modifying the project source."_s },
  { "CMAKE_XCODE_ATTRIBUTE_<an-attribute>"_s,
    "Tells the Xcode generator to set <an-attribute> to the given value in the generated Xcode project. Ignored on other generators. Low-level escape hatch for Xcode-specific settings."_s },

  // === 1-placeholder, (20-literal-char bucket) ===
  { "CMAKE_<LANG>_HOST_COMPILER"_s,
    "Host compiler executable for CUDA or HIP code compilation"_s },

  // === 1-placeholder, (17-literal-char bucket) ===
  { "CMAKE_<LANG>_CLANG_TIDY"_s,
    "Default value for the <LANG>_CLANG_TIDY target property; names a clang-tidy command (with arguments) run alongside compilation. Applies only when <LANG> is C, CXX, OBJC, or OBJCXX."_s },
  { "CMAKE_<LANG>_EXTENSIONS"_s,
    "Default value for the <LANG>_EXTENSIONS target property when a target is created. Controls whether compiler-specific extensions to the language standard are enabled."_s },
  { "CMAKE_<LANG>_FLAGS_INIT"_s,
    "Toolchain-file hook initializing CMAKE_<LANG>_FLAGS the first time the build tree is configured for <LANG>. Set in toolchain or platform files; user projects should set CMAKE_<LANG>_FLAGS instead."_s },
  { "CMAKE_<LANG>_LINK_FLAGS"_s,
    "Language-wide flags for <LANG> used when linking for all configurations. Passed to every compiler invocation that drives linking. Per-config CMAKE_<LANG>_LINK_FLAGS_<CONFIG> are appended on top."_s },
  { "CMAKE_<LANG>_PVS_STUDIO"_s,
    "Default value for the <LANG>_PVS_STUDIO target property; names a pvs-studio-analyzer command (with arguments) run alongside compilation. Applies only when <LANG> is C or CXX."_s },

  // === 1-placeholder, (15-literal-char bucket) ===
  { "CMAKE_<LANG>_COMPILER"_s,
    "Full path to the compiler used for language <LANG>. Set by CMake during compiler detection on the first configure; users may override on the command line to select a specific toolchain."_s },
  { "CMAKE_<LANG>_CPPCHECK"_s,
    "Default value for the <LANG>_CPPCHECK target property; names a cppcheck command (with arguments) run alongside compilation. Applies only when <LANG> is C or CXX."_s },
  { "CMAKE_<LANG>_STANDARD"_s,
    "Default value for the <LANG>_STANDARD target property when a target is created. Applies to <LANG> = C, CXX, CUDA, HIP, OBJC, OBJCXX. See cmake-compile-features(7)."_s },

  // === 1-placeholder, (14-literal-char bucket) ===
  { "CMAKE_<CONFIG>_POSTFIX"_s,
    "Default filename postfix for libraries under configuration <CONFIG>."_s },
  { "CMAKE_<LANG>_CPPLINT"_s,
    "Default value for the <LANG>_CPPLINT target property; names a cpplint command (with arguments) run alongside compilation. Applies only when <LANG> is C or CXX."_s },

  // === 1-placeholder, (13-literal-char bucket) ===
  { "CMAKE_<LANG>_ICSTAT"_s,
    "Default value for the <LANG>_ICSTAT target property; names an icstat static-analysis command (with arguments) run alongside compilation. Applies only when <LANG> is C or CXX."_s },

  // === 1-placeholder, (12-literal-char bucket) ===
  { "CMAKE_<LANG>_FLAGS"_s,
    "Language-wide compiler flags for <LANG>. Initialized from the language-specific environment variable (CFLAGS, CXXFLAGS, FFLAGS, etc.) the first time the language is enabled."_s },
  { "CMAKE_MATCH_<n>"_s,
    "Capture group <n> matched by the most recent regular expression (groups 0 through 9). Group 0 is the entire match; groups 1 through 9 are the parenthesized subexpressions."_s },

  // === 1-placeholder, (11-literal-char bucket) ===
  { "<PROJECT-NAME>_BINARY_DIR"_s,
    "Top-level binary directory for the named project. Created by the project() command with the given <PROJECT-NAME>; useful when add_subdirectory is used to connect several projects."_s },
  { "<PROJECT-NAME>_SOURCE_DIR"_s,
    "Top-level source directory for the named project. Created by the project() command with the given <PROJECT-NAME>; useful when add_subdirectory is used to connect several projects."_s },

  // === 1-placeholder, (8-literal-char bucket) ===
  { "<PROJECT-NAME>_VERSION"_s,
    "Value given to the VERSION option of the most recent project() command with project name <PROJECT-NAME>, if any. Component values live in <PROJECT-NAME>_VERSION_{MAJOR,MINOR,PATCH,TWEAK}."_s },

  // === 1-placeholder, (5-literal-char bucket) ===
  { "<PackageName>_ROOT"_s,
    "Initial search prefix(es) for find_package(<PackageName>); covers both the mixed-case form (CMP0074, e.g. Foo_ROOT) and the upper-case form (CMP0144, e.g. FOO_ROOT)."_s },

  // === 2-placeholder, (36-literal-char bucket) ===
  { "CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>_SUPPORTED"_s,
    "Set to TRUE if <FEATURE> is supported for linker language <LANG>."_s },

  // === 2-placeholder, (34-literal-char bucket) ===
  { "CMAKE_<LANG>_LINK_GROUP_USING_<FEATURE>_SUPPORTED"_s,
    "Specifies if <FEATURE> is supported for link language <LANG>."_s },

  // === 2-placeholder, (31-literal-char bucket) ===
  { "CMAKE_<LANG>_LINK_LIBRARY_<FEATURE>_ATTRIBUTES"_s,
    "Defines semantics of link library <FEATURE> for language <LANG>."_s },

  // === 2-placeholder, (26-literal-char bucket) ===
  { "CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>"_s,
    "Defines how to link a library for <FEATURE>."_s },

  // === 2-placeholder, (24-literal-char bucket) ===
  { "CMAKE_<LANG>_LINK_GROUP_USING_<FEATURE>"_s,
    "Defines how to link a group of libraries for <FEATURE>."_s },

  // === 2-placeholder, (20-literal-char bucket) ===
  { "CMAKE_<LANG>_USING_LINKER_<TYPE>"_s,
    "Defines how to specify the <TYPE> linker for the link step."_s },

  // === 2-placeholder, (18-literal-char bucket) ===
  { "CMAKE_<LANG>_FLAGS_<CONFIG>_INIT"_s,
    "Toolchain-file hook initializing CMAKE_<LANG>_FLAGS_<CONFIG> the first time the build tree is configured for language <LANG>. Intended for toolchain or platform files."_s },
  { "CMAKE_<LANG>_LINK_FLAGS_<CONFIG>"_s,
    "Language-wide flags for <LANG> used when linking for the <CONFIG> configuration. Passed to every compiler invocation that drives linking. Appended after CMAKE_<LANG>_LINK_FLAGS."_s },

  // === 2-placeholder, (13-literal-char bucket) ===
  { "CMAKE_<LANG>_FLAGS_<CONFIG>"_s,
    "Language-wide flags for <LANG> used when building for the <CONFIG> configuration. Appended after CMAKE_<LANG>_FLAGS on both compile and link invocations."_s },
};
/* clang-format on */

// ---------------------------------------------------------------------------
// Character classes used by the matcher.
// ---------------------------------------------------------------------------

constexpr bool IsAsciiAlpha(char c)
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr bool IsAsciiDigit(char c)
{
  return c >= '0' && c <= '9';
}

// Identifier: ``[A-Za-z][A-Za-z0-9_]*`` -- start vs. continuation.
constexpr bool IsIdentStart(char c)
{
  return IsAsciiAlpha(c);
}
constexpr bool IsIdentCont(char c)
{
  return IsAsciiAlpha(c) || IsAsciiDigit(c) || c == '_';
}

// Identifier-with-hyphen: ``[A-Za-z][A-Za-z0-9_-]*`` -- start vs.
// continuation.
constexpr bool IsIdentHyphenStart(char c)
{
  return IsAsciiAlpha(c);
}
constexpr bool IsIdentHyphenCont(char c)
{
  return IsAsciiAlpha(c) || IsAsciiDigit(c) || c == '_' || c == '-';
}

// ---------------------------------------------------------------------------
// Placeholder dispatch.
// ---------------------------------------------------------------------------

enum class PlaceholderClass
{
  Ident,       // <LANG>, <CONFIG>, <PackageName>
  IdentHyphen, // <PROJECT-NAME>, <an-attribute>
  FourDigit,   // <NNNN>
  OneDigit,    // <n>
  Unknown,     // malformed pattern entry
};

PlaceholderClass ClassOf(cm::string_view placeholder)
{
  // ``placeholder`` includes the surrounding angle brackets, e.g. "<LANG>".
  // ``<FEATURE>`` and ``<TYPE>`` are documented as identifiers by their
  // RSTs (e.g. WEAK / WHOLE_ARCHIVE for FEATURE in
  // Help/variable/CMAKE_LINK_LIBRARY_USING_FEATURE.rst and LLD / MOLD
  // for TYPE in Help/variable/CMAKE_LANG_USING_LINKER_TYPE.rst), so they
  // share the ``Ident`` lexical class with <LANG>/<CONFIG>/<PackageName>.
  // Without this clause the eleven pattern entries that use these two
  // placeholders would never match any input (regression guarded by
  // testEveryPatternReachable in Tests/CMakeLib).
  if (placeholder == "<LANG>" || placeholder == "<CONFIG>" ||
      placeholder == "<PackageName>" || placeholder == "<FEATURE>" ||
      placeholder == "<TYPE>") {
    return PlaceholderClass::Ident;
  }
  if (placeholder == "<PROJECT-NAME>" || placeholder == "<an-attribute>") {
    return PlaceholderClass::IdentHyphen;
  }
  if (placeholder == "<NNNN>") {
    return PlaceholderClass::FourDigit;
  }
  if (placeholder == "<n>") {
    return PlaceholderClass::OneDigit;
  }
  return PlaceholderClass::Unknown;
}

bool ClassAllowsStart(PlaceholderClass cls, char c)
{
  switch (cls) {
    case PlaceholderClass::Ident:
      return IsIdentStart(c);
    case PlaceholderClass::IdentHyphen:
      return IsIdentHyphenStart(c);
    case PlaceholderClass::FourDigit:
    case PlaceholderClass::OneDigit:
      return IsAsciiDigit(c);
    case PlaceholderClass::Unknown:
      break;
  }
  return false;
}

bool ClassAllowsCont(PlaceholderClass cls, char c)
{
  switch (cls) {
    case PlaceholderClass::Ident:
      return IsIdentCont(c);
    case PlaceholderClass::IdentHyphen:
      return IsIdentHyphenCont(c);
    case PlaceholderClass::FourDigit:
    case PlaceholderClass::OneDigit:
      return IsAsciiDigit(c);
    case PlaceholderClass::Unknown:
      break;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Helpers.
// ---------------------------------------------------------------------------

bool LiteralMatches(cm::string_view input, std::size_t pos,
                    cm::string_view literal)
{
  return input.size() - pos >= literal.size() &&
    input.compare(pos, literal.size(), literal) == 0;
}

// Try to match ``pattern`` against ``input`` end-to-end under the
// lazy + lookahead-anchored semantics documented at the top of this file.
// Returns true on full-consumption match; false otherwise.
bool TryMatch(cm::string_view pattern, cm::string_view input)
{
  std::size_t pi = 0;
  std::size_t ii = 0;

  while (pi < pattern.size()) {
    if (pattern[pi] != '<') {
      // Literal segment: walk until next '<' or end of pattern.
      std::size_t literal_end = pi;
      while (literal_end < pattern.size() && pattern[literal_end] != '<') {
        ++literal_end;
      }
      cm::string_view const lit = pattern.substr(pi, literal_end - pi);
      if (!LiteralMatches(input, ii, lit)) {
        return false;
      }
      pi = literal_end;
      ii += lit.size();
      continue;
    }

    // Placeholder segment: find the closing '>'.
    std::size_t const close = pattern.find('>', pi);
    if (close == cm::string_view::npos) {
      return false; // malformed pattern; matched by no input
    }
    cm::string_view const ph = pattern.substr(pi, close - pi + 1);
    PlaceholderClass const cls = ClassOf(ph);
    if (cls == PlaceholderClass::Unknown) {
      return false; // unknown placeholder class; rejected by unit test too
    }

    // Locate the next anchor literal that follows this placeholder.  An
    // empty anchor means the placeholder is the last segment in the
    // pattern (must consume to end-of-input) or that it is followed
    // immediately by another placeholder (adjacent placeholders are
    // forbidden -- testPatternTableFieldsValid rejects them at table
    // build time, so we treat the case defensively here).
    std::size_t const after_ph = close + 1;
    std::size_t anchor_end = after_ph;
    while (anchor_end < pattern.size() && pattern[anchor_end] != '<') {
      ++anchor_end;
    }
    cm::string_view const anchor =
      pattern.substr(after_ph, anchor_end - after_ph);
    bool const consume_to_eoi =
      anchor.empty() && (anchor_end == pattern.size());
    if (anchor.empty() && !consume_to_eoi) {
      return false; // adjacent placeholders are not supported
    }

    // Fixed-length classes consume exactly that many digits, then anchor.
    if (cls == PlaceholderClass::FourDigit ||
        cls == PlaceholderClass::OneDigit) {
      std::size_t const fixed_len =
        (cls == PlaceholderClass::FourDigit) ? 4u : 1u;
      if (input.size() - ii < fixed_len) {
        return false;
      }
      for (std::size_t k = 0; k < fixed_len; ++k) {
        if (!IsAsciiDigit(input[ii + k])) {
          return false;
        }
      }
      std::size_t const cur = ii + fixed_len;
      if (consume_to_eoi) {
        if (cur != input.size()) {
          return false;
        }
        ii = cur;
      } else {
        if (!LiteralMatches(input, cur, anchor)) {
          return false;
        }
        ii = cur + anchor.size();
      }
      pi = anchor_end;
      continue;
    }

    // Variable-length classes: placeholder is non-empty; consume lazily,
    // stopping at the first cursor position where the anchor matches (or
    // at end-of-input when there is no trailing literal anchor).
    if (ii >= input.size()) {
      return false;
    }
    if (!ClassAllowsStart(cls, input[ii])) {
      return false;
    }
    std::size_t cur = ii + 1; // consumed first char
    if (consume_to_eoi) {
      while (cur < input.size()) {
        if (!ClassAllowsCont(cls, input[cur])) {
          return false;
        }
        ++cur;
      }
      ii = cur;
    } else {
      while (true) {
        if (LiteralMatches(input, cur, anchor)) {
          break;
        }
        if (cur >= input.size()) {
          return false;
        }
        if (!ClassAllowsCont(cls, input[cur])) {
          return false;
        }
        ++cur;
      }
      ii = cur + anchor.size();
    }
    pi = anchor_end;
  }

  return ii == input.size();
}

} // namespace

cmCacheDocumentationTable::LookupResult cmCachePatternTable::Match(
  cm::string_view varName)
{
  for (auto const& entry : kPatterns) {
    if (TryMatch(entry.Pattern, varName)) {
      return { entry.Summary };
    }
  }
  return {};
}

cmCachePatternTable::Entry const* cmCachePatternTable::EntriesBegin()
{
  return std::begin(kPatterns);
}

cmCachePatternTable::Entry const* cmCachePatternTable::EntriesEnd()
{
  return std::end(kPatterns);
}

std::size_t cmCachePatternTable::EntriesSize()
{
  return static_cast<std::size_t>(std::end(kPatterns) - std::begin(kPatterns));
}
