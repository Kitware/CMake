/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmDebuggerAdapter.h"

#include <algorithm>
#include <climits>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <cm/memory>
#include <cm/optional>

#include <cm3p/cppdap/io.h> // IWYU pragma: keep
#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/session.h>

#include "cmDebuggerBreakpointManager.h"
#include "cmDebuggerExceptionManager.h"
#include "cmDebuggerProtocol.h"
#include "cmDebuggerSourceBreakpoint.h" // IWYU pragma: keep
#include "cmDebuggerStackFrame.h"
#include "cmDebuggerThread.h"
#include "cmDebuggerThreadManager.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmValue.h"
#include "cmVersionConfig.h"
#include <cmcppdap/include/dap/optional.h>
#include <cmcppdap/include/dap/types.h>

namespace cmDebugger {

// Event provides a basic wait and signal synchronization primitive.
class SyncEvent
{
public:
  // Wait() blocks until the event is fired.
  void Wait()
  {
    std::unique_lock<std::mutex> lock(Mutex);
    Cv.wait(lock, [&] { return Fired; });
  }

  // Fire() sets signals the event, and unblocks any calls to Wait().
  void Fire()
  {
    std::unique_lock<std::mutex> lock(Mutex);
    Fired = true;
    Cv.notify_all();
  }

private:
  std::mutex Mutex;
  std::condition_variable Cv;
  bool Fired = false;
};

class Semaphore
{
public:
  Semaphore(int count_ = 0)
    : Count(count_)
  {
  }

  inline void Notify()
  {
    std::unique_lock<std::mutex> lock(Mutex);
    Count++;
    // notify the waiting thread
    Cv.notify_one();
  }

  inline void Wait()
  {
    std::unique_lock<std::mutex> lock(Mutex);
    while (Count == 0) {
      // wait on the mutex until notify is called
      Cv.wait(lock);
    }
    Count--;
  }

private:
  std::mutex Mutex;
  std::condition_variable Cv;
  int Count;
};

cmDebuggerAdapter::cmDebuggerAdapter(
  std::shared_ptr<cmDebuggerConnection> connection,
  std::string const& dapLogPath)
  : cmDebuggerAdapter(std::move(connection),
                      dapLogPath.empty()
                        ? cm::nullopt
                        : cm::optional<std::shared_ptr<dap::Writer>>(
                            dap::file(dapLogPath.c_str())))
{
}

cmDebuggerAdapter::cmDebuggerAdapter(
  std::shared_ptr<cmDebuggerConnection> connection,
  cm::optional<std::shared_ptr<dap::Writer>> logger)
  : Connection(std::move(connection))
  , SessionActive(true)
  , DisconnectEvent(cm::make_unique<SyncEvent>())
  , ConfigurationDoneEvent(cm::make_unique<SyncEvent>())
  , ContinueSem(cm::make_unique<Semaphore>())
  , ThreadManager(cm::make_unique<cmDebuggerThreadManager>())
{
  if (logger.has_value()) {
    SessionLog = std::move(logger.value());
  }
  ClearStepRequests();

  Session = dap::Session::create();
  BreakpointManager =
    cm::make_unique<cmDebuggerBreakpointManager>(Session.get());
  ExceptionManager =
    cm::make_unique<cmDebuggerExceptionManager>(Session.get());

  // Handle errors reported by the Session. These errors include protocol
  // parsing errors and receiving messages with no handler.
  Session->onError([this](const char* msg) {
    if (SessionLog) {
      dap::writef(SessionLog, "dap::Session error: %s\n", msg);
    }

    std::cout << "[CMake Debugger] DAP session error: " << msg << std::endl;

    BreakpointManager->ClearAll();
    ExceptionManager->ClearAll();
    ClearStepRequests();
    ContinueSem->Notify();
    DisconnectEvent->Fire();
    SessionActive.store(false);
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
  Session->registerHandler([this](const dap::CMakeInitializeRequest& req) {
    SupportsVariableType = req.supportsVariableType.value(false);
    dap::CMakeInitializeResponse response;
    response.supportsConfigurationDoneRequest = true;
    response.cmakeVersion.major = CMake_VERSION_MAJOR;
    response.cmakeVersion.minor = CMake_VERSION_MINOR;
    response.cmakeVersion.patch = CMake_VERSION_PATCH;
    response.cmakeVersion.full = CMake_VERSION;
    ExceptionManager->HandleInitializeRequest(response);
    return response;
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Events_Initialized
  Session->registerSentHandler(
    [&](const dap::ResponseOrError<dap::CMakeInitializeResponse>&) {
      Session->send(dap::InitializedEvent());
    });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
  Session->registerHandler([this](const dap::ThreadsRequest& req) {
    (void)req;
    std::unique_lock<std::mutex> lock(Mutex);
    dap::ThreadsResponse response;

    // If a client requests threads during shutdown (like after receiving the
    // thread exited event), DefaultThread won't be set.
    if (DefaultThread) {
      dap::Thread thread;
      thread.id = DefaultThread->GetId();
      thread.name = DefaultThread->GetName();
      response.threads.push_back(thread);
    }

    return response;
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
  Session->registerHandler([this](const dap::StackTraceRequest& request)
                             -> dap::ResponseOrError<dap::StackTraceResponse> {
    std::unique_lock<std::mutex> lock(Mutex);

    cm::optional<dap::StackTraceResponse> response =
      ThreadManager->GetThreadStackTraceResponse(request.threadId);
    if (response.has_value()) {
      return response.value();
    }

    return dap::Error("Unknown threadId '%d'", int(request.threadId));
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
  Session->registerHandler([this](const dap::ScopesRequest& request)
                             -> dap::ResponseOrError<dap::ScopesResponse> {
    std::unique_lock<std::mutex> lock(Mutex);
    return DefaultThread->GetScopesResponse(request.frameId,
                                            SupportsVariableType);
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
  Session->registerHandler([this](const dap::VariablesRequest& request)
                             -> dap::ResponseOrError<dap::VariablesResponse> {
    return DefaultThread->GetVariablesResponse(request);
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
  Session->registerHandler([this](const dap::PauseRequest& req) {
    (void)req;
    PauseRequest.store(true);
    return dap::PauseResponse();
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
  Session->registerHandler([this](const dap::ContinueRequest& req) {
    (void)req;
    ContinueSem->Notify();
    return dap::ContinueResponse();
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
  Session->registerHandler([this](const dap::NextRequest& req) {
    (void)req;
    NextStepFrom.store(DefaultThread->GetStackFrameSize());
    ContinueSem->Notify();
    return dap::NextResponse();
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
  Session->registerHandler([this](const dap::StepInRequest& req) {
    (void)req;
    // This would stop after stepped in, single line stepped or stepped out.
    StepInRequest.store(true);
    ContinueSem->Notify();
    return dap::StepInResponse();
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
  Session->registerHandler([this](const dap::StepOutRequest& req) {
    (void)req;
    StepOutDepth.store(DefaultThread->GetStackFrameSize() - 1);
    ContinueSem->Notify();
    return dap::StepOutResponse();
  });

  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
  Session->registerHandler([](const dap::LaunchRequest& req) {
    (void)req;
    return dap::LaunchResponse();
  });

  // Handler for disconnect requests
  Session->registerHandler([this](const dap::DisconnectRequest& request) {
    (void)request;
    BreakpointManager->ClearAll();
    ExceptionManager->ClearAll();
    ClearStepRequests();
    ContinueSem->Notify();
    DisconnectEvent->Fire();
    SessionActive.store(false);
    return dap::DisconnectResponse();
  });

  Session->registerHandler([this](const dap::EvaluateRequest& request) {
    dap::EvaluateResponse response;
    if (request.frameId.has_value()) {
      std::shared_ptr<cmDebuggerStackFrame> frame =
        DefaultThread->GetStackFrame(request.frameId.value());

      auto var = frame->GetMakefile()->GetDefinition(request.expression);
      if (var) {
        response.type = "string";
        response.result = var;
        return response;
      }
    }

    return response;
  });

  // The ConfigurationDone request is made by the client once all configuration
  // requests have been made.
  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
  Session->registerHandler([this](const dap::ConfigurationDoneRequest& req) {
    (void)req;
    ConfigurationDoneEvent->Fire();
    return dap::ConfigurationDoneResponse();
  });

  std::string errorMessage;
  if (!Connection->StartListening(errorMessage)) {
    throw std::runtime_error(errorMessage);
  }

  // Connect to the client. Write a well-known message to stdout so that
  // clients know it is safe to attempt to connect.
  std::cout << "Waiting for debugger client to connect..." << std::endl;
  Connection->WaitForConnection();
  std::cout << "Debugger client connected." << std::endl;

  if (SessionLog) {
    Session->connect(spy(Connection->GetReader(), SessionLog),
                     spy(Connection->GetWriter(), SessionLog));
  } else {
    Session->connect(Connection->GetReader(), Connection->GetWriter());
  }

  // Start the processing thread.
  SessionThread = std::thread([this] {
    while (SessionActive.load()) {
      if (auto payload = Session->getPayload()) {
        payload();
      }
    }
  });

  ConfigurationDoneEvent->Wait();

  DefaultThread = ThreadManager->StartThread("CMake script");
  dap::ThreadEvent threadEvent;
  threadEvent.reason = "started";
  threadEvent.threadId = DefaultThread->GetId();
  Session->send(threadEvent);
}

cmDebuggerAdapter::~cmDebuggerAdapter()
{
  if (SessionThread.joinable()) {
    SessionThread.join();
  }

  Session.reset(nullptr);

  if (SessionLog) {
    SessionLog->close();
  }
}

void cmDebuggerAdapter::ReportExitCode(int exitCode)
{
  ThreadManager->EndThread(DefaultThread);
  dap::ThreadEvent threadEvent;
  threadEvent.reason = "exited";
  threadEvent.threadId = DefaultThread->GetId();
  DefaultThread.reset();

  dap::ExitedEvent exitEvent;
  exitEvent.exitCode = exitCode;

  dap::TerminatedEvent terminatedEvent;

  if (SessionActive.load()) {
    Session->send(threadEvent);
    Session->send(exitEvent);
    Session->send(terminatedEvent);
  }

  // Wait until disconnected or error.
  DisconnectEvent->Wait();
}

void cmDebuggerAdapter::OnFileParsedSuccessfully(
  std::string const& sourcePath,
  std::vector<cmListFileFunction> const& functions)
{
  BreakpointManager->SourceFileLoaded(sourcePath, functions);
}

void cmDebuggerAdapter::OnBeginFunctionCall(cmMakefile* mf,
                                            std::string const& sourcePath,
                                            cmListFileFunction const& lff)
{
  std::unique_lock<std::mutex> lock(Mutex);
  DefaultThread->PushStackFrame(mf, sourcePath, lff);

  if (lff.Line() == 0) {
    // File just loaded, continue to first valid function call.
    return;
  }

  auto hits = BreakpointManager->GetBreakpoints(sourcePath, lff.Line());
  lock.unlock();

  bool waitSem = false;
  dap::StoppedEvent stoppedEvent;
  stoppedEvent.allThreadsStopped = true;
  stoppedEvent.threadId = DefaultThread->GetId();
  if (!hits.empty()) {
    ClearStepRequests();
    waitSem = true;

    dap::array<dap::integer> hitBreakpoints;
    hitBreakpoints.resize(hits.size());
    std::transform(hits.begin(), hits.end(), hitBreakpoints.begin(),
                   [&](const int64_t& id) { return dap::integer(id); });
    stoppedEvent.reason = "breakpoint";
    stoppedEvent.hitBreakpointIds = hitBreakpoints;
  }

  if (long(DefaultThread->GetStackFrameSize()) <= NextStepFrom.load() ||
      StepInRequest.load() ||
      long(DefaultThread->GetStackFrameSize()) <= StepOutDepth.load()) {
    ClearStepRequests();
    waitSem = true;

    stoppedEvent.reason = "step";
  }

  if (PauseRequest.load()) {
    ClearStepRequests();
    waitSem = true;

    stoppedEvent.reason = "pause";
  }

  if (waitSem) {
    Session->send(stoppedEvent);
    ContinueSem->Wait();
  }
}

void cmDebuggerAdapter::OnEndFunctionCall()
{
  DefaultThread->PopStackFrame();
}

static std::shared_ptr<cmListFileFunction> listFileFunction;

void cmDebuggerAdapter::OnBeginFileParse(cmMakefile* mf,
                                         std::string const& sourcePath)
{
  std::unique_lock<std::mutex> lock(Mutex);

  listFileFunction = std::make_shared<cmListFileFunction>(
    sourcePath, 0, 0, std::vector<cmListFileArgument>());
  DefaultThread->PushStackFrame(mf, sourcePath, *listFileFunction);
}

void cmDebuggerAdapter::OnEndFileParse()
{
  DefaultThread->PopStackFrame();
  listFileFunction = nullptr;
}

void cmDebuggerAdapter::OnMessageOutput(MessageType t, std::string const& text)
{
  cm::optional<dap::StoppedEvent> stoppedEvent =
    ExceptionManager->RaiseExceptionIfAny(t, text);
  if (stoppedEvent.has_value()) {
    stoppedEvent->threadId = DefaultThread->GetId();
    Session->send(*stoppedEvent);
    ContinueSem->Wait();
  }
}

void cmDebuggerAdapter::ClearStepRequests()
{
  NextStepFrom.store(INT_MIN);
  StepInRequest.store(false);
  StepOutDepth.store(INT_MIN);
  PauseRequest.store(false);
}

} // namespace cmDebugger
