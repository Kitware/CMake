/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <chrono>
#include <cstdio>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

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
#  include "cmsys/SystemTools.hxx"

#  include "cmCryptoHash.h"
#  include "cmDebuggerWindowsPipeConnection.h"
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
      .HashString(cmsys::SystemTools::GetCurrentWorkingDirectory());
#else
  std::string namedPipe = "CMakeDebuggerPipe2";
#endif

  std::unique_ptr<dap::Session> client = dap::Session::create();
  client->registerHandler([&](dap::InitializedEvent /*unused*/) {
    initializedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](dap::ExitedEvent /*unused*/) {
    exitedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](dap::TerminatedEvent const& /*unused*/) {
    terminatedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](dap::ThreadEvent const& e) {
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
    } catch (std::runtime_error const& error) {
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

bool testProtocolWithPipesAbruptDisconnect()
{
  std::promise<void> debuggerConnectionCreatedPromise;
  std::future<void> debuggerConnectionCreatedFuture =
    debuggerConnectionCreatedPromise.get_future();

  std::future<void> startedListeningFuture;

  std::promise<void> adapterFinishedPromise;
  std::future<void> adapterFinishedFuture =
    adapterFinishedPromise.get_future();

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

  auto futureTimeout = std::chrono::seconds(60);
  auto disconnectTimeout = std::chrono::seconds(10);

#ifdef _WIN32
  std::string namedPipe = R"(\\.\pipe\LOCAL\CMakeDebuggerPipe3_)" +
    cmCryptoHash(cmCryptoHash::AlgoSHA256)
      .HashString(cmsys::SystemTools::GetCurrentWorkingDirectory());
#else
  std::string namedPipe = "CMakeDebuggerPipe3";
#endif

  std::unique_ptr<dap::Session> client = dap::Session::create();
  client->registerHandler([&](dap::InitializedEvent /*unused*/) {
    initializedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](dap::ExitedEvent /*unused*/) {
    exitedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](dap::TerminatedEvent const& /*unused*/) {
    terminatedEventReceivedPromise.set_value(true);
  });
  client->registerHandler([&](dap::ThreadEvent const& e) {
    if (e.reason == "started") {
      threadStartedPromise.set_value(true);
    } else if (e.reason == "exited") {
      threadExitedPromise.set_value(true);
    }
  });

  // Raw thread (not ScopedThread): we need to be able to detach on
  // failure so the test process can exit even if the regression
  // returns. With the fix in place, ReportExitCode()'s wait on
  // DisconnectEvent unblocks when the SessionThread detects EOF on the
  // pipe; without the fix, it blocks forever and the adapter
  // destructor hangs in SessionThread.join().
  std::thread debuggerThread([&]() {
    try {
      auto connection =
        std::make_shared<cmDebugger::cmDebuggerPipeConnection>(namedPipe);
      startedListeningFuture = connection->StartedListening.get_future();
      debuggerConnectionCreatedPromise.set_value();
      std::shared_ptr<cmDebugger::cmDebuggerAdapter> debuggerAdapter =
        std::make_shared<cmDebugger::cmDebuggerAdapter>(
          connection, dap::file(stdout, false));
      // Sends thread-exited / exited / terminated events and then
      // blocks on DisconnectEvent. The test closes the client side of
      // the pipe instead of sending a DisconnectRequest, so the only
      // thing that will unblock this wait is the SessionThread's EOF
      // handling in cmDebuggerAdapter.
      debuggerAdapter->ReportExitCode(0);
      // Adapter destructed here; joins SessionThread.
    } catch (std::runtime_error const&) {
      // Swallowed: connection failures shouldn't hang the test.
    }
    adapterFinishedPromise.set_value();
  });

  ASSERT_TRUE(debuggerConnectionCreatedFuture.wait_for(futureTimeout) ==
              std::future_status::ready);
  ASSERT_TRUE(startedListeningFuture.wait_for(futureTimeout) ==
              std::future_status::ready);

  auto client2Debugger =
    std::make_shared<cmDebugger::cmDebuggerPipeClient>(namedPipe);
  client2Debugger->WaitForConnection();
  client->bind(client2Debugger, client2Debugger);

  // Drive the full handshake so that the debugger SessionThread is up
  // and ReportExitCode is blocked on DisconnectEvent.
  dap::CMakeInitializeRequest initializeRequest;
  auto initializeResponse = client->send(initializeRequest).get();
  ASSERT_TRUE(!initializeResponse.error);

  dap::LaunchRequest launchRequest;
  auto launchResponse = client->send(launchRequest).get();
  ASSERT_TRUE(!launchResponse.error);

  dap::ConfigurationDoneRequest configurationDoneRequest;
  auto configurationDoneResponse =
    client->send(configurationDoneRequest).get();
  ASSERT_TRUE(!configurationDoneResponse.error);

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

  // Abruptly close the client side without sending DisconnectRequest.
  // Regression check for the busy-loop bug: the debugger adapter must
  // detect EOF on the pipe and shut down on its own.
  //
  // Use ShutdownForTesting() rather than close(). The client dap::Session
  // has its own recvThread still blocked in read() on this same socket;
  // on Linux, ::close() on an fd does not wake a sibling thread's
  // in-flight ::read(), which would deadlock the test. ShutdownForTesting
  // calls shutdown(SHUT_RDWR) to signal EOF on the socket endpoint
  // without freeing the fd, so both reads wake up naturally.
  client2Debugger->ShutdownForTesting();

  bool finishedInTime = adapterFinishedFuture.wait_for(disconnectTimeout) ==
    std::future_status::ready;
  if (!finishedInTime) {
    // Bug reproduced: the SessionThread is spinning on EOF and
    // ReportExitCode is blocked on DisconnectEvent. Detach so the test
    // process can exit instead of hanging in std::thread's destructor.
    debuggerThread.detach();
    ASSERT_TRUE(finishedInTime);
  }
  debuggerThread.join();

  return true;
}

int testDebuggerAdapterPipe(int, char*[])
{
  return runTests(
    { testProtocolWithPipes, testProtocolWithPipesAbruptDisconnect });
}
