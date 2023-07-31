/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cm3p/cppdap/protocol.h>

#include "cmDebuggerSourceBreakpoint.h"

class cmListFileFunction;

namespace dap {
class Session;
}

namespace cmDebugger {

struct cmDebuggerFunctionLocation
{
  int64_t StartLine;
  int64_t EndLine;
};

/** The breakpoint manager. */
class cmDebuggerBreakpointManager
{
  dap::Session* DapSession;
  std::mutex Mutex;
  std::unordered_map<std::string, std::vector<cmDebuggerSourceBreakpoint>>
    Breakpoints;
  std::unordered_map<std::string,
                     std::vector<struct cmDebuggerFunctionLocation>>
    ListFileFunctionLines;
  std::unordered_set<std::string> ListFilePendingValidations;
  int64_t NextBreakpointId = 0;

  dap::SetBreakpointsResponse HandleSetBreakpointsRequest(
    dap::SetBreakpointsRequest const& request);
  int64_t FindFunctionStartLine(std::string const& sourcePath, int64_t line);
  int64_t CalibrateBreakpointLine(std::string const& sourcePath, int64_t line);

public:
  cmDebuggerBreakpointManager(dap::Session* dapSession);
  void SourceFileLoaded(std::string const& sourcePath,
                        std::vector<cmListFileFunction> const& functions);
  std::vector<int64_t> GetBreakpoints(std::string const& sourcePath,
                                      int64_t line);
  size_t GetBreakpointCount() const;
  void ClearAll();
};

} // namespace cmDebugger
