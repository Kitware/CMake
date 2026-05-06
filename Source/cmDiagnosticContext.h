/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <utility>

#include "cmDiagnostics.h"
#include "cmListFileCache.h"

class cmStateSnapshot;

/** \class cmDiagnosticContext
 * \brief Records context for issuing diagnostics
 */
class cmDiagnosticContext
{
public:
  explicit cmDiagnosticContext(cmListFileBacktrace backtrace)
    : Backtrace{ std::move(backtrace) }
  {
  }
  cmDiagnosticContext() = default;
  cmDiagnosticContext(cmDiagnosticContext&&) = default;
  cmDiagnosticContext(cmDiagnosticContext const&) = default;

  void RecordDiagnostic(cmDiagnosticCategory category,
                        cmStateSnapshot const& state);

  cmListFileBacktrace const& GetBacktrace() const { return this->Backtrace; }

protected:
  friend class cmMessenger;

  cmListFileBacktrace Backtrace;
  cmDiagnostics::DiagnosticMap DiagnosticState;
  bool HasState = false;
};
