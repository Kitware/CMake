/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <chrono>
#include <cstdio>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
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

#ifdef _WIN32
#  include "cmCryptoHash.h"
#  include "cmDebuggerWindowsPipeConnection.h"
#  include "cmSystemTools.h"
#else
#  include "cmDebuggerPosixPipeConnection.h"
#endif

#include "testCommon.h"
#include "testDebugger.h"

bool testProtocolWithPipes()
{
  std::promise<void> debuggerConnectionCreatedPromise;
  std::future<void> debuggerConnectionCreatedFuture =
    debuggerConnectionCreatedPromise.get_future();

  std::future<void> startedListeningFuture;

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

#ifdef _WIN32
  std::string namedPipe = R"(\\.\pipe\LOCAL\CMakeDebuggerPipe2_)" +
    cmCryptoHash(cmCryptoHash::AlgoSHA256)
      .HashString(cmSystemTools::GetCurrentWorkingDirectory());
#else
  std::string namedPipe = "CMakeDebuggerPipe2";
#endif

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

  ScopedThread debuggerThread([&]() -> int {
    try {
      auto connection =
        std::make_shared<cmDebugger::cmDebuggerPipeConnection>(namedPipe);
      startedListeningFuture = connection->StartedListening.get_future();
      debuggerConnectionCreatedPromise.set_value();
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
    } catch (const std::runtime_error& error) {
      std::cerr << "Error: Failed to create debugger adapter.\n";
      std::cerr << error.what() << "\n";
      return -1;
    }
  });

  ASSERT_TRUE(debuggerConnectionCreatedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(startedListeningFuture.wait_for(futureTimeout) ==
              std::future_status::ready);

  auto client2Debugger =
    std::make_shared<cmDebugger::cmDebuggerPipeClient>(namedPipe);

  client2Debugger->WaitForConnection();
  client->bind(client2Debugger, client2Debugger);

  dap::CMakeInitializeRequest initializeRequest;
  auto response = client->send(initializeRequest);
  auto initializeResponse = response.get();
  ASSERT_TRUE(!initializeResponse.error);
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
  ASSERT_TRUE(terminatedEventReceivedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(threadStartedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(threadExitedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(exitedEventReceivedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);

  dap::DisconnectRequest disconnectRequest;
  auto disconnectResponse = client->send(disconnectRequest).get();
  disconnectResponseReceivedPromise.set_value(true);
  ASSERT_TRUE(!disconnectResponse.error);

  return true;
}

int testDebuggerAdapterPipe(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testProtocolWithPipes,
  });
}
