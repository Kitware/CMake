/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <string>

#include <cm/optional>

namespace cmDebugger {
class cmDebuggerThread;
}

namespace dap {
struct StackTraceRequest;
struct StackTraceResponse;
}

namespace cmDebugger {

class cmDebuggerThreadManager
{
  static std::atomic<std::int64_t> NextThreadId;
  std::list<std::shared_ptr<cmDebuggerThread>> Threads;

public:
  cmDebuggerThreadManager() = default;
  std::shared_ptr<cmDebuggerThread> StartThread(std::string const& name);
  void EndThread(std::shared_ptr<cmDebuggerThread> const& thread);
  cm::optional<dap::StackTraceResponse> GetThreadStackTraceResponse(
    dap::StackTraceRequest const& request);
};

} // namespace cmDebugger
