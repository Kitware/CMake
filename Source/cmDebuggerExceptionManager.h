/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

#include <cm/optional>

#include <cm3p/cppdap/protocol.h>

#include "cmMessageType.h"

namespace dap {
class Session;
struct CMakeInitializeResponse;
}

namespace cmDebugger {

struct cmDebuggerException
{
  std::string Id;
  std::string Description;
};

struct cmDebuggerExceptionFilter
{
  std::string Filter;
  std::string Label;
};

/** The exception manager. */
class cmDebuggerExceptionManager
{
  // Some older C++ standard libraries cannot hash an enum class by default.
  struct MessageTypeHash
  {
    std::size_t operator()(MessageType t) const
    {
      return std::hash<int>{}(static_cast<int>(t));
    }
  };

  dap::Session* DapSession;
  std::mutex Mutex;
  std::unordered_map<std::string, bool> RaiseExceptions;
  std::unordered_map<MessageType, cmDebuggerExceptionFilter, MessageTypeHash>
    ExceptionMap;
  cm::optional<cmDebuggerException> TheException;

  dap::SetExceptionBreakpointsResponse HandleSetExceptionBreakpointsRequest(
    dap::SetExceptionBreakpointsRequest const& request);

  dap::ExceptionInfoResponse HandleExceptionInfoRequest();

public:
  cmDebuggerExceptionManager(dap::Session* dapSession);
  void HandleInitializeRequest(dap::CMakeInitializeResponse& response);
  cm::optional<dap::StoppedEvent> RaiseExceptionIfAny(MessageType t,
                                                      std::string const& text);
  void ClearAll();
};

} // namespace cmDebugger
