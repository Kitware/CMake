/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>

// Async-signal-safe handling of a user interrupt (Ctrl+C / SIGINT on POSIX, or
// a console Ctrl event on Windows) around an instrumented command, so that the
// command's snippet can still be written before the process exits.
class cmInstrumentationInterrupt
{
public:
  // Outcome of running a callback under an installed interrupt handler.
  struct InterruptOutcome
  {
    int ExitCode;
    bool Interrupted;
    int Signal;
    // Whether the caught interrupt should be re-raised so the process exit
    // status reflects it.  True for a real OS signal; false for a test-
    // injected interrupt, which exits cleanly after flushing the snippet.
    // Always set explicitly at every construction site (no default member
    // initializer, so this stays a C++11 aggregate).
    bool ShouldRaise;
  };

  // Run `callback` with an interrupt handler installed for its duration, so a
  // user interrupt sets a flag instead of terminating the process immediately.
  // The handler is deliberately minimal (async-signal-safe): it only records
  // the signal.  When `active` is false (e.g. instrumentation is not enabled),
  // no handler is installed and the callback keeps the default signal
  // behavior.  Returns the callback's exit code, whether an interrupt was
  // caught, and the signal number (0 if none).
  static InterruptOutcome HandleInterrupt(
    bool active, std::function<int()> const& callback);

  // Restore the default disposition and re-raise the given signal so that the
  // process exits as if by the interrupt.  No-op on platforms where re-raising
  // is not the appropriate exit mechanism.
  static void RaiseInterrupt(int sig);

  // The pending interrupt signal number, or 0 if no interrupt occurred.
  static int PendingInterruptSignal();
};
