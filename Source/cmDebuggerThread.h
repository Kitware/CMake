/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <cm3p/cppdap/protocol.h>

class cmListFileFunction;
class cmMakefile;

namespace cmDebugger {
class cmDebuggerStackFrame;
class cmDebuggerVariables;
class cmDebuggerVariablesManager;
}

namespace cmDebugger {

class cmDebuggerThread
{
  int64_t Id;
  std::string Name;
  std::vector<std::shared_ptr<cmDebuggerStackFrame>> Frames;
  std::unordered_map<int64_t, std::shared_ptr<cmDebuggerStackFrame>> FrameMap;
  std::mutex Mutex;
  std::unordered_map<int64_t, std::vector<dap::Scope>> FrameScopes;
  std::unordered_map<int64_t,
                     std::vector<std::shared_ptr<cmDebuggerVariables>>>
    FrameVariables;
  std::shared_ptr<cmDebuggerVariablesManager> VariablesManager;

public:
  cmDebuggerThread(int64_t id, std::string name);
  int64_t GetId() const { return this->Id; }
  const std::string& GetName() const { return this->Name; }
  void PushStackFrame(cmMakefile* mf, std::string const& sourcePath,
                      cmListFileFunction const& lff);
  void PopStackFrame();
  std::shared_ptr<cmDebuggerStackFrame> GetTopStackFrame();
  std::shared_ptr<cmDebuggerStackFrame> GetStackFrame(int64_t frameId);
  size_t GetStackFrameSize() const { return this->Frames.size(); }
  dap::ScopesResponse GetScopesResponse(int64_t frameId,
                                        bool supportsVariableType);
  dap::VariablesResponse GetVariablesResponse(
    dap::VariablesRequest const& request);
  friend dap::StackTraceResponse GetStackTraceResponse(
    std::shared_ptr<cmDebuggerThread> const& thread);
};

} // namespace cmDebugger
