/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDiagnostics.h"

#include <cassert>
#include <map>
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

cm::optional<cmDiagnostics::DiagnosticCategory>
cmDiagnostics::GetDiagnosticCategory(cm::string_view name)
{
  return stringToCategory(name);
}
