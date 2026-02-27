/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDiagnostics.h"

#include <cassert>
#include <map>
#include <string>
#include <utility>

#include <cmext/string_view>

#include "cmStringAlgorithms.h"

namespace {
cm::optional<cmDiagnostics::DiagnosticCategory> stringToCategory(
  cm::string_view input)
{
  using Map = std::map<cm::string_view, cmDiagnostics::DiagnosticCategory>;
  static Map const mapping = {
#define CATEGORY_MAP(C) { #C ""_s, cmDiagnostics::C },
    CM_FOR_EACH_DIAGNOSTIC_CATEGORY(CATEGORY_MAP)
#undef CATEGORY_MAP
  };

  assert(!input.empty());
  if (input.size() >= 4 && cmHasLiteralPrefix(input, "CMD_")) {
    auto const i = mapping.find(input);
    if (i != mapping.end()) {
      return i->second;
    }
  }

  return cm::nullopt;
}
}

#if __cplusplus < 201703L
// Prior to C++17, the compiler is unhappy if this member doesn't have explicit
// storage... and clang-tidy is unhappy if it does.
// NOLINTNEXTLINE(*-redundant-declaration)
constexpr cmDiagnostics::DiagnosticCategoryInformation
  cmDiagnostics::CategoryInfo[cmDiagnostics::CategoryCount];
#endif

cm::string_view cmDiagnostics::GetActionString(DiagnosticAction action)
{
  switch (action) {
    case Ignore:
      return "IGNORE"_s;
    case Warn:
      return "WARN"_s;
    case SendError:
      return "SEND_ERROR"_s;
    case FatalError:
      return "FATAL_ERROR"_s;
    default:
      return {};
  }
}

cm::string_view cmDiagnostics::GetCategoryString(DiagnosticCategory category)
{
  static cm::string_view const names[CategoryCount] = {
    {}, // CMD_NONE
#define CATEGORY_NAME(C) #C ""_s,
    CM_FOR_EACH_DIAGNOSTIC_CATEGORY(CATEGORY_NAME)
#undef CATEGORY_MAP
  };

  if (category < CategoryCount) {
    return names[category];
  }
  return {};
}

cm::optional<cmDiagnostics::DiagnosticAction>
cmDiagnostics::GetDiagnosticAction(cm::string_view name)
{
  if (name == "IGNORE"_s) {
    return DiagnosticAction::Ignore;
  }
  if (name == "WARN"_s) {
    return DiagnosticAction::Warn;
  }
  if (name == "SEND_ERROR"_s) {
    return DiagnosticAction::SendError;
  }
  if (name == "FATAL_ERROR"_s) {
    return DiagnosticAction::FatalError;
  }

  return cm::nullopt;
}

cm::optional<cmDiagnostics::DiagnosticCategory>
cmDiagnostics::GetDiagnosticCategory(cm::string_view name)
{
  return stringToCategory(name);
}
