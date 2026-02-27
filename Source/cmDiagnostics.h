/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <array>
#include <cstddef>
#include <cstdint>

#include <cm/optional>
#include <cm/string_view>

// https://github.com/include-what-you-use/include-what-you-use/issues/1934
// IWYU pragma: no_forward_declare cmDiagnostics::DiagnosticAction
// IWYU pragma: no_forward_declare cmDiagnostics::DiagnosticCategory

// The list of diagnostic categories along with their associated data.
// Each entry is of the form `SELECT(ACTION, <default>, <parent>, <name>)`.
// Entries MUST appear in the order that a depth-first enumeration would
// produce.

#define CM_FOR_EACH_DIAGNOSTIC_TABLE(ACTION, SELECT)                          \
  SELECT(ACTION, Warn, CMD_NONE, CMD_AUTHOR)                                  \
  SELECT(ACTION, Warn, CMD_NONE, CMD_DEPRECATED)

#define CM_SELECT_CATEGORY(F, D, P, C) F(C)
#define CM_FOR_EACH_DIAGNOSTIC_CATEGORY(ACTION)                               \
  CM_FOR_EACH_DIAGNOSTIC_TABLE(ACTION, CM_SELECT_CATEGORY)

/** \class cmDiagnostic
 * \brief Handles CMake diagnostic (warning) behavior
 *
 * See the cmake-diagnostics(7) manual for an overview of this class's purpose.
 */
class cmDiagnostics
{
public:
  /// Action to take when a diagnostic is triggered
  enum DiagnosticAction : std::uint8_t
  {
    Undefined = 0,
    Ignore,
    Warn,
    SendError,
    FatalError,
  };

  /// Diagnostic category identifiers
  enum DiagnosticCategory : unsigned
  {
    CMD_NONE,
#define DIAGNOSTIC_ENUM(CATEGORY) CATEGORY,
    CM_FOR_EACH_DIAGNOSTIC_CATEGORY(DIAGNOSTIC_ENUM)
#undef DIAGNOSTIC_ENUM

    /** \brief Always the last entry.
     *
     * Used to determine the number of diagnostic categories.  Also useful to
     * avoid adding a comma the last diagnostic category when adding a new one.
     */
    CMD_COUNT
  };
  constexpr static size_t CategoryCount = static_cast<size_t>(CMD_COUNT);

  struct DiagnosticCategoryInformation
  {
    DiagnosticCategory Parent;
    DiagnosticAction DefaultAction;
  };

  constexpr static DiagnosticCategoryInformation
    CategoryInfo[CategoryCount] = {
      { CMD_NONE, Undefined }, // CMD_NONE
#define DIAGNOSTIC_CATEGORY_INFO(F, D, P, C) { P, D },
      CM_FOR_EACH_DIAGNOSTIC_TABLE(UNUSED, DIAGNOSTIC_CATEGORY_INFO)
#undef DIAGNOSTIC_CATEGORY_INFO
    };

  //! convert an action identifier into a string
  static cm::string_view GetActionString(DiagnosticAction);

  //! convert a category identifier into a string
  static cm::string_view GetCategoryString(DiagnosticCategory);

  //! Convert a string action into an identifier
  static cm::optional<DiagnosticAction> GetDiagnosticAction(
    cm::string_view name);

  //! Convert a string category into an identifier
  static cm::optional<DiagnosticCategory> GetDiagnosticCategory(
    cm::string_view name);

  /** Represent a set of diagnostic category actions.  */
  using DiagnosticMap = std::array<DiagnosticAction, CategoryCount>;
};
