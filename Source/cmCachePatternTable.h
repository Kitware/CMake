/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>

#include <cm/string_view>

#include "cmCacheDocumentationTable.h"

/** \namespace cmCachePatternTable
 * \brief Compile-time pattern-matching fallback for built-in CMake cache
 *        variable documentation.
 *
 * The pattern table is a hand-maintained list of placeholder-shaped
 * tooltips, e.g. ``CMAKE_<LANG>_FLAGS`` or
 * ``CMAKE_POLICY_DEFAULT_CMP<NNNN>``, that complement the exact-match
 * entries in ``cmCacheDocumentationTable``.  It is consulted only on
 * the miss path: ``cmCacheDocumentationTable::Get`` first attempts an
 * exact-match lookup, then falls back to ``cmCachePatternTable::Match``
 * for names that follow a known template shape.
 *
 * Placeholder lexical classes recognized by the matcher (see
 * ``cmCachePatternTable.cxx`` for details):
 *
 *  * ``<LANG>``, ``<CONFIG>``, ``<PackageName>``: identifier
 *    (``[A-Za-z][A-Za-z0-9_]*``).
 *  * ``<PROJECT-NAME>``, ``<an-attribute>``: identifier with
 *    hyphens (``[A-Za-z][A-Za-z0-9_-]*``).
 *  * ``<NNNN>``: exactly 4 ASCII digits (``[0-9]{4}``).
 *  * ``<n>``: exactly 1 ASCII digit (``[0-9]``).
 *
 * Matching is leftmost-anchor, lazy on each placeholder, with full
 * input consumption required at the end.  The matcher is intentionally
 * permissive: per-language or per-configuration restrictions documented
 * in the corresponding ``Help/variable/<NAME>.rst`` file are reproduced
 * in the entry's Summary prose, not enforced by the matcher.  First
 * fully-matching entry in ``kPatterns[]`` wins; entries are kept in
 * ``(numPlaceholders ASC, totalLiteralLen DESC, Pattern ASC)`` order so
 * that more-specific patterns are tried before less specific ones.
 */
namespace cmCachePatternTable {

/** \brief A single row in the pattern documentation table.
 *
 * The ``Pattern`` and ``Summary`` views reference storage with static
 * lifetime.  ``Pattern`` is a literal template string containing one or
 * more angle-bracketed placeholders such as ``<LANG>`` or ``<NNNN>``.
 */
struct Entry
{
  cm::string_view Pattern;
  cm::string_view Summary;
};

/** \brief Look up the built-in documentation for \a varName by pattern.
 *
 * Walks ``kPatterns[]`` in declaration order and returns the first
 * entry whose ``Pattern`` template fully matches \a varName under the
 * lexical classes above.  On miss returns an empty ``Summary``.
 * Performs no allocation; runs in ``O(N * |varName|)`` for the small
 * (~30) table.
 *
 * The result type is reused from ``cmCacheDocumentationTable`` so that
 * the single call site in ``cmCacheDocumentationTable::Get`` can return
 * either an exact-match or a pattern-match result without translation.
 */
cmCacheDocumentationTable::LookupResult Match(cm::string_view varName);

/** \brief Iterate the underlying pattern table.
 *
 * Exposed for the ``testCacheDocumentationTable`` unit test in
 * ``Tests/CMakeLib``, which verifies that the entries are well-formed
 * and obey the canonical ordering rule.  Callers should not rely on a
 * particular entry count or ordering beyond what the unit test
 * asserts.
 */
Entry const* EntriesBegin();
Entry const* EntriesEnd();
std::size_t EntriesSize();

}
