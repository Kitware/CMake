/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmDebuggerThread.h"

#include <cstdint>
#include <utility>

#include <cm3p/cppdap/optional.h>
#include <cm3p/cppdap/types.h>

#include "cmDebuggerStackFrame.h"
#include "cmDebuggerVariables.h"
#include "cmDebuggerVariablesHelper.h"
#include "cmDebuggerVariablesManager.h"
#include "cmListFileCache.h"

namespace cmDebugger {

cmDebuggerThread::cmDebuggerThread(int64_t id, std::string name)
  : Id(id)
  , Name(std::move(name))
  , VariablesManager(std::make_shared<cmDebuggerVariablesManager>())
{
}

void cmDebuggerThread::PushStackFrame(cmMakefile* mf,
                                      std::string const& sourcePath,
                                      cmListFileFunction const& lff)
{
  std::unique_lock<std::mutex> lock(Mutex);
  Frames.emplace_back(
    std::make_shared<cmDebuggerStackFrame>(mf, sourcePath, lff));
  FrameMap.insert({ Frames.back()->GetId(), Frames.back() });
}

void cmDebuggerThread::PopStackFrame()
{
  std::unique_lock<std::mutex> lock(Mutex);
  FrameMap.erase(Frames.back()->GetId());
  FrameScopes.erase(Frames.back()->GetId());
  FrameVariables.erase(Frames.back()->GetId());
  Frames.pop_back();
}

std::shared_ptr<cmDebuggerStackFrame> cmDebuggerThread::GetTopStackFrame()
{
  std::unique_lock<std::mutex> lock(Mutex);
  if (!Frames.empty()) {
    return Frames.back();
  }

  return {};
}

std::shared_ptr<cmDebuggerStackFrame> cmDebuggerThread::GetStackFrame(
  int64_t frameId)
{
  std::unique_lock<std::mutex> lock(Mutex);
  auto it = FrameMap.find(frameId);

  if (it == FrameMap.end()) {
    return {};
  }

  return it->second;
}

dap::ScopesResponse cmDebuggerThread::GetScopesResponse(
  int64_t frameId, bool supportsVariableType)
{
  std::unique_lock<std::mutex> lock(Mutex);
  auto it = FrameScopes.find(frameId);

  if (it != FrameScopes.end()) {
    dap::ScopesResponse response;
    response.scopes = it->second;
    return response;
  }

  auto it2 = FrameMap.find(frameId);
  if (it2 == FrameMap.end()) {
    return dap::ScopesResponse();
  }

  std::shared_ptr<cmDebuggerStackFrame> frame = it2->second;
  std::shared_ptr<cmDebuggerVariables> localVariables =
    cmDebuggerVariablesHelper::Create(VariablesManager, "Locals",
                                      supportsVariableType, frame);

  FrameVariables[frameId].emplace_back(localVariables);

  dap::Scope scope;
  scope.name = localVariables->GetName();
  scope.presentationHint = "locals";
  scope.variablesReference = localVariables->GetId();

  dap::Source source;
  source.name = frame->GetFileName();
  source.path = source.name;
  scope.source = source;

  FrameScopes[frameId].push_back(scope);

  dap::ScopesResponse response;
  response.scopes.push_back(scope);
  return response;
}

dap::VariablesResponse cmDebuggerThread::GetVariablesResponse(
  dap::VariablesRequest const& request)
{
  std::unique_lock<std::mutex> lock(Mutex);
  dap::VariablesResponse response;
  response.variables = VariablesManager->HandleVariablesRequest(request);
  return response;
}

dap::StackTraceResponse GetStackTraceResponse(
  std::shared_ptr<cmDebuggerThread> const& thread)
{
  dap::StackTraceResponse response;
  std::unique_lock<std::mutex> lock(thread->Mutex);
  for (int i = static_cast<int>(thread->Frames.size()) - 1; i >= 0; --i) {
    dap::Source source;
    source.name = thread->Frames[i]->GetFileName();
    source.path = source.name;

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif
    dap::StackFrame stackFrame;
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
    stackFrame.line = thread->Frames[i]->GetLine();
    stackFrame.column = 1;
    stackFrame.name = thread->Frames[i]->GetFunction().OriginalName();
    stackFrame.id = thread->Frames[i]->GetId();
    stackFrame.source = source;

    response.stackFrames.push_back(stackFrame);
  }

  response.totalFrames = response.stackFrames.size();
  return response;
}

} // namespace cmDebugger
