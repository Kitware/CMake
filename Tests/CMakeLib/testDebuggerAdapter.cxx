/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <chrono>
#include <cstdio>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include <cm3p/cppdap/future.h>
#include <cm3p/cppdap/io.h>
#include <cm3p/cppdap/optional.h>
#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/session.h>
#include <cm3p/cppdap/types.h>

#include "cmDebuggerAdapter.h"
#include "cmDebuggerProtocol.h"
#include "cmVersionConfig.h"

#include "testCommon.h"
#include "testDebugger.h"

class DebuggerLocalConnection : public cmDebugger::cmDebuggerConnection
{
public:
  DebuggerLocalConnection()
    : ClientToDebugger(dap::pipe())
    , DebuggerToClient(dap::pipe())
  {
  }

  bool StartListening(std::string& errorMessage) override
  {
    errorMessage = "";
    return true;
  }
  void WaitForConnection() override {}

  std::shared_ptr<dap::Reader> GetReader() override
  {
    return ClientToDebugger;
  };

  std::shared_ptr<dap::Writer> GetWriter() override
  {
    return DebuggerToClient;
  }

  std::shared_ptr<dap::ReaderWriter> ClientToDebugger;
  std::shared_ptr<dap::ReaderWriter> DebuggerToClient;
};

bool runTest(std::function<bool(dap::Session&)> onThreadExitedEvent)
{
  std::promise<bool> debuggerAdapterInitializedPromise;
  std::future<bool> debuggerAdapterInitializedFuture =
    debuggerAdapterInitializedPromise.get_future();

  std::promise<bool> initializedEventReceivedPromise;
  std::future<bool> initializedEventReceivedFuture =
    initializedEventReceivedPromise.get_future();

  std::promise<bool> exitedEventReceivedPromise;
  std::future<bool> exitedEventReceivedFuture =
    exitedEventReceivedPromise.get_future();

  std::promise<bool> terminatedEventReceivedPromise;
  std::future<bool> terminatedEventReceivedFuture =
    terminatedEventReceivedPromise.get_future();

  std::promise<bool> threadStartedPromise;
  std::future<bool> threadStartedFuture = threadStartedPromise.get_future();

  std::promise<bool> threadExitedPromise;
  std::future<bool> threadExitedFuture = threadExitedPromise.get_future();

  std::promise<bool> disconnectResponseReceivedPromise;
  std::future<bool> disconnectResponseReceivedFuture =
    disconnectResponseReceivedPromise.get_future();

  auto futureTimeout = std::chrono::seconds(60);

  auto connection = std::make_shared<DebuggerLocalConnection>();
  std::unique_ptr<dap::Session> client = dap::Session::create();
  client->registerHandler([&](const dap::InitializedEvent& e) {
    (void)e;
    initializedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](const dap::ExitedEvent& e) {
    (void)e;
    exitedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](const dap::TerminatedEvent& e) {
    (void)e;
    terminatedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](const dap::ThreadEvent& e) {
    if (e.reason == "started") {
      threadStartedPromise.set_value(true);
    } else if (e.reason == "exited") {
      threadExitedPromise.set_value(true);
    }
  });

  client->bind(connection->DebuggerToClient, connection->ClientToDebugger);

  ScopedThread debuggerThread([&]() -> int {
    std::shared_ptr<cmDebugger::cmDebuggerAdapter> debuggerAdapter =
      std::make_shared<cmDebugger::cmDebuggerAdapter>(
        connection, dap::file(stdout, false));

    debuggerAdapterInitializedPromise.set_value(true);
    debuggerAdapter->ReportExitCode(0);

    // Ensure the disconnectResponse has been received before
    // destructing debuggerAdapter.
    ASSERT_TRUE(disconnectResponseReceivedFuture.wait_for(futureTimeout) ==
                std::future_status::ready);
    return 0;
  });

  dap::CMakeInitializeRequest initializeRequest;
  auto initializeResponse = client->send(initializeRequest).get();
  ASSERT_TRUE(initializeResponse.response.cmakeVersion.full == CMake_VERSION);
  ASSERT_TRUE(initializeResponse.response.cmakeVersion.major ==
              CMake_VERSION_MAJOR);
  ASSERT_TRUE(initializeResponse.response.cmakeVersion.minor ==
              CMake_VERSION_MINOR);
  ASSERT_TRUE(initializeResponse.response.cmakeVersion.patch ==
              CMake_VERSION_PATCH);
  ASSERT_TRUE(initializeResponse.response.supportsExceptionInfoRequest);
  ASSERT_TRUE(
    initializeResponse.response.exceptionBreakpointFilters.has_value());

  dap::LaunchRequest launchRequest;
  auto launchResponse = client->send(launchRequest).get();
  ASSERT_TRUE(!launchResponse.error);

  dap::ConfigurationDoneRequest configurationDoneRequest;
  auto configurationDoneResponse =
    client->send(configurationDoneRequest).get();
  ASSERT_TRUE(!configurationDoneResponse.error);

  ASSERT_TRUE(debuggerAdapterInitializedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(initializedEventReceivedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(threadStartedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(threadExitedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);

  if (onThreadExitedEvent) {
    ASSERT_TRUE(onThreadExitedEvent(*client));
  }

  ASSERT_TRUE(exitedEventReceivedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(terminatedEventReceivedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);

  dap::DisconnectRequest disconnectRequest;
  auto disconnectResponse = client->send(disconnectRequest).get();
  disconnectResponseReceivedPromise.set_value(true);
  ASSERT_TRUE(!disconnectResponse.error);

  return true;
}

bool testBasicProtocol()
{
  return runTest(nullptr);
}

bool testThreadsRequestAfterThreadExitedEvent()
{
  return runTest([](dap::Session& session) -> bool {
    // Try requesting threads again after receiving the thread exited event.
    // Some clients do this to ensure that their thread list is up-to-date.
    dap::ThreadsRequest threadsRequest;
    auto threadsResponse = session.send(threadsRequest).get();
    ASSERT_TRUE(!threadsResponse.error);

    // CMake only has one DAP thread. Once that thread exits, there should be
    // no threads left.
    ASSERT_TRUE(threadsResponse.response.threads.empty());

    return true;
  });
}

int testDebuggerAdapter(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testBasicProtocol,
    testThreadsRequestAfterThreadExitedEvent,
  });
}
