/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>

#include <cm/optional>

#include "cmDiagnostics.h"
#include "cmListFileCache.h"
#include "cmMessageType.h" // IWYU pragma: keep

#ifndef CMAKE_BOOTSTRAP
#  include "cmSarifLog.h"
#endif

class cmStateSnapshot;

#ifdef CMake_ENABLE_DEBUGGER
namespace cmDebugger {
class cmDebuggerAdapter;
}
#endif

class cmMessenger
{
public:
  void IssueMessage(
    MessageType type, std::string const& text,
    cmListFileBacktrace const& backtrace = cmListFileBacktrace()) const;

  void IssueDiagnostic(
    cmDiagnosticCategory category, std::string const& text,
    cmStateSnapshot const& context,
    cmListFileBacktrace const& backtrace = cmListFileBacktrace()) const;

  void DisplayMessage(MessageType type, cmDiagnosticCategory category,
                      std::string const& text,
                      cmListFileBacktrace const& backtrace) const;

  void SetTopSource(cm::optional<std::string> topSource);

#ifndef CMAKE_BOOTSTRAP
  cmSarif::ResultsLog const& GetSarifResultsLog() const { return SarifLog; }
#endif

  // Print the top of a backtrace.
  void PrintBacktraceTitle(std::ostream& out,
                           cmListFileBacktrace const& bt) const;
#ifdef CMake_ENABLE_DEBUGGER
  void SetDebuggerAdapter(
    std::shared_ptr<cmDebugger::cmDebuggerAdapter> const& debuggerAdapter)
  {
    DebuggerAdapter = debuggerAdapter;
  }
#endif

private:
  cm::optional<std::string> TopSource;

#ifndef CMAKE_BOOTSTRAP
  cmSarif::ResultsLog SarifLog;
#endif

#ifdef CMake_ENABLE_DEBUGGER
  std::shared_ptr<cmDebugger::cmDebuggerAdapter> DebuggerAdapter;
#endif
};
