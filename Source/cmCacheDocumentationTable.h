/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>

#include <cm/string_view>

/** \namespace cmCacheDocumentationTable
 * \brief Compile-time lookup for built-in CMake cache variable documentation.
 *
 * The table is a hand-maintained list of short, single-line plain-text
 * summaries keyed by the canonical CMake variable name (for example
 * ``CMAKE_BUILD_TYPE``).  See the header comment in
 * ``cmCacheDocumentationTable.cxx`` for the scope of variables included
 * and the maintenance discipline.
 *
 * The sole consumer is ``cmake::ProcessCacheArg``, which reads
 * ``Summary`` to populate the ``HELPSTRING`` of cache entries created
 * via ``-D <var>=<value>`` when the user did not supply one.
 */
namespace cmCacheDocumentationTable {

/** \brief A single row in the built-in documentation table.
 *
 * The ``Name`` and ``Summary`` views reference storage with static
 * lifetime.
 */
struct Entry
{
  cm::string_view Name;
  cm::string_view Summary;
};

/** \brief Result of a single ``Get`` lookup.
 *
 * An unknown key yields an empty ``Summary``.
 */
struct LookupResult
{
  cm::string_view Summary;
};

/** \brief Look up the built-in documentation for \a varName.
 *
 * Returns a ``LookupResult`` whose ``Summary`` references storage with
 * static lifetime when \a varName corresponds to a documented CMake
 * variable.  Otherwise returns an empty ``Summary``.  The lookup runs
 * in ``O(log N)`` and performs no allocation.
 */
LookupResult Get(cm::string_view varName);

/** \brief Iterate the underlying table.
 *
 * These are exposed primarily for the ``testCacheDocumentationTable``
 * regression test in ``Tests/CMakeLib``, which verifies that the entries
 * are sorted (a precondition of ``Get``'s ``std::lower_bound`` lookup),
 * unique, and otherwise well-formed.  Callers should not rely on a
 * particular entry count or ordering beyond the sort invariant.
 */
Entry const* EntriesBegin();
Entry const* EntriesEnd();
std::size_t EntriesSize();

}
