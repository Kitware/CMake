/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <cm/optional>

#include <cm3p/cppdap/io.h> // IWYU pragma: keep

#include "cmMessageType.h" // IWYU pragma: keep

class cmListFileFunction;
class cmMakefile;

namespace cmDebugger {
class Semaphore;
class SyncEvent;
class cmDebuggerBreakpointManager;
class cmDebuggerExceptionManager;
class cmDebuggerThread;
class cmDebuggerThreadManager;
}

namespace dap {
class Session;
}

namespace cmDebugger {

class cmDebuggerConnection
{
public:
  virtual ~cmDebuggerConnection() = default;
  virtual bool StartListening(std::string& errorMessage) = 0;
  virtual void WaitForConnection() = 0;
  virtual std::shared_ptr<dap::Reader> GetReader() = 0;
  virtual std::shared_ptr<dap::Writer> GetWriter() = 0;
};

class cmDebuggerAdapter
{
public:
  cmDebuggerAdapter(std::shared_ptr<cmDebuggerConnection> connection,
                    std::string const& dapLogPath);
  cmDebuggerAdapter(std::shared_ptr<cmDebuggerConnection> connection,
                    cm::optional<std::shared_ptr<dap::Writer>> logger);
  ~cmDebuggerAdapter();

  void ReportExitCode(int exitCode);

  void OnFileParsedSuccessfully(
    std::string const& sourcePath,
    std::vector<cmListFileFunction> const& functions);
  void OnBeginFunctionCall(cmMakefile* mf, std::string const& sourcePath,
                           cmListFileFunction const& lff);
  void OnEndFunctionCall();
  void OnBeginFileParse(cmMakefile* mf, std::string const& sourcePath);
  void OnEndFileParse();

  void OnMessageOutput(MessageType t, std::string const& text);

private:
  void ClearStepRequests();
  std::shared_ptr<cmDebuggerConnection> Connection;
  std::unique_ptr<dap::Session> Session;
  std::shared_ptr<dap::Writer> SessionLog;
  std::thread SessionThread;
  std::atomic<bool> SessionActive;
  std::mutex Mutex;
  std::unique_ptr<SyncEvent> DisconnectEvent;
  std::unique_ptr<SyncEvent> ConfigurationDoneEvent;
  std::unique_ptr<Semaphore> ContinueSem;
  std::atomic<int64_t> NextStepFrom;
  std::atomic<bool> StepInRequest;
  std::atomic<int64_t> StepOutDepth;
  std::atomic<bool> PauseRequest;
  std::unique_ptr<cmDebuggerThreadManager> ThreadManager;
  std::shared_ptr<cmDebuggerThread> DefaultThread;
  std::unique_ptr<cmDebuggerBreakpointManager> BreakpointManager;
  std::unique_ptr<cmDebuggerExceptionManager> ExceptionManager;
  bool SupportsVariableType;
};

} // namespace cmDebugger
