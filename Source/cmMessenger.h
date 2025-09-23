/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>

#include <cm/optional>

#include "cmListFileCache.h"
#include "cmMessageType.h" // IWYU pragma: keep

#ifndef CMAKE_BOOTSTRAP
#  include "cmSarifLog.h"
#endif

#ifdef CMake_ENABLE_DEBUGGER
namespace cmDebugger {
class cmDebuggerAdapter;
}
#endif

class cmMessenger
{
public:
  void IssueMessage(
    MessageType t, std::string const& text,
    cmListFileBacktrace const& backtrace = cmListFileBacktrace()) const;

  void DisplayMessage(MessageType t, std::string const& text,
                      cmListFileBacktrace const& backtrace) const;

  void SetTopSource(cm::optional<std::string> topSource);

  void SetSuppressDevWarnings(bool suppress)
  {
    this->SuppressDevWarnings = suppress;
  }
  void SetSuppressDeprecatedWarnings(bool suppress)
  {
    this->SuppressDeprecatedWarnings = suppress;
  }
  void SetDevWarningsAsErrors(bool error)
  {
    this->DevWarningsAsErrors = error;
  }
  void SetDeprecatedWarningsAsErrors(bool error)
  {
    this->DeprecatedWarningsAsErrors = error;
  }

  bool GetSuppressDevWarnings() const { return this->SuppressDevWarnings; }
  bool GetSuppressDeprecatedWarnings() const
  {
    return this->SuppressDeprecatedWarnings;
  }
  bool GetDevWarningsAsErrors() const { return this->DevWarningsAsErrors; }
  bool GetDeprecatedWarningsAsErrors() const
  {
    return this->DeprecatedWarningsAsErrors;
  }

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
  bool IsMessageTypeVisible(MessageType t) const;
  MessageType ConvertMessageType(MessageType t) const;

  cm::optional<std::string> TopSource;

#ifndef CMAKE_BOOTSTRAP
  cmSarif::ResultsLog SarifLog;
#endif

  bool SuppressDevWarnings = false;
  bool SuppressDeprecatedWarnings = false;
  bool DevWarningsAsErrors = false;
  bool DeprecatedWarningsAsErrors = false;
#ifdef CMake_ENABLE_DEBUGGER
  std::shared_ptr<cmDebugger::cmDebuggerAdapter> DebuggerAdapter;
#endif
};
