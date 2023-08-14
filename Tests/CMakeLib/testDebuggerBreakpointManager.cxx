/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include <cm3p/cppdap/future.h>
#include <cm3p/cppdap/optional.h>
#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/session.h>
#include <cm3p/cppdap/types.h>

#include "cmDebuggerBreakpointManager.h"
#include "cmDebuggerSourceBreakpoint.h" // IWYU pragma: keep
#include "cmListFileCache.h"

#include "testCommon.h"
#include "testDebugger.h"

static bool testHandleBreakpointRequestBeforeFileIsLoaded()
{
  // Arrange
  DebuggerTestHelper helper;
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();
  dap::SetBreakpointsRequest setBreakpointRequest;
  std::string sourcePath = "C:/CMakeLists.txt";
  setBreakpointRequest.source.path = sourcePath;
  dap::array<dap::SourceBreakpoint> sourceBreakpoints(3);
  sourceBreakpoints[0].line = 1;
  sourceBreakpoints[1].line = 2;
  sourceBreakpoints[2].line = 3;
  setBreakpointRequest.breakpoints = sourceBreakpoints;

  // Act
  auto got = helper.Client->send(setBreakpointRequest).get();

  // Assert
  auto& response = got.response;
  ASSERT_TRUE(!got.error);
  ASSERT_TRUE(response.breakpoints.size() == sourceBreakpoints.size());
  ASSERT_BREAKPOINT(response.breakpoints[0], 0, sourceBreakpoints[0].line,
                    sourcePath, false);
  ASSERT_BREAKPOINT(response.breakpoints[1], 1, sourceBreakpoints[1].line,
                    sourcePath, false);
  ASSERT_BREAKPOINT(response.breakpoints[2], 2, sourceBreakpoints[2].line,
                    sourcePath, false);
  ASSERT_TRUE(breakpointManager.GetBreakpointCount() == 3);

  // setBreakpoints should override any existing breakpoints
  setBreakpointRequest.breakpoints.value().clear();
  helper.Client->send(setBreakpointRequest).get();
  ASSERT_TRUE(breakpointManager.GetBreakpointCount() == 0);

  return true;
}

static bool testHandleBreakpointRequestAfterFileIsLoaded()
{
  // Arrange
  DebuggerTestHelper helper;
  std::atomic<bool> notExpectBreakpointEvents(true);
  helper.Client->registerHandler([&](const dap::BreakpointEvent&) {
    notExpectBreakpointEvents.store(false);
  });

  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();
  std::string sourcePath = "C:/CMakeLists.txt";
  std::vector<cmListFileFunction> functions = helper.CreateListFileFunctions(
    "# Comment1\nset(var1 foo)\n# Comment2\nset(var2\nbar)\n",
    sourcePath.c_str());

  breakpointManager.SourceFileLoaded(sourcePath, functions);
  dap::SetBreakpointsRequest setBreakpointRequest;
  setBreakpointRequest.source.path = sourcePath;
  dap::array<dap::SourceBreakpoint> sourceBreakpoints(5);
  sourceBreakpoints[0].line = 1;
  sourceBreakpoints[1].line = 2;
  sourceBreakpoints[2].line = 3;
  sourceBreakpoints[3].line = 4;
  sourceBreakpoints[4].line = 5;
  setBreakpointRequest.breakpoints = sourceBreakpoints;

  // Act
  auto got = helper.Client->send(setBreakpointRequest).get();

  // Assert
  auto& response = got.response;
  ASSERT_TRUE(!got.error);
  ASSERT_TRUE(response.breakpoints.size() == sourceBreakpoints.size());
  // Line 1 is a comment. Move it to next valid function, which is line 2.
  ASSERT_BREAKPOINT(response.breakpoints[0], 0, 2, sourcePath, true);
  ASSERT_BREAKPOINT(response.breakpoints[1], 1, sourceBreakpoints[1].line,
                    sourcePath, true);
  // Line 3 is a comment. Move it to next valid function, which is line 4.
  ASSERT_BREAKPOINT(response.breakpoints[2], 2, 4, sourcePath, true);
  ASSERT_BREAKPOINT(response.breakpoints[3], 3, sourceBreakpoints[3].line,
                    sourcePath, true);
  // Line 5 is the 2nd part of line 4 function. No valid function after line 5,
  // show the breakpoint at line 4.
  ASSERT_BREAKPOINT(response.breakpoints[4], 4, sourceBreakpoints[3].line,
                    sourcePath, true);

  ASSERT_TRUE(notExpectBreakpointEvents.load());
  ASSERT_TRUE(breakpointManager.GetBreakpointCount() == 5);

  // setBreakpoints should override any existing breakpoints
  setBreakpointRequest.breakpoints.value().clear();
  helper.Client->send(setBreakpointRequest).get();
  ASSERT_TRUE(breakpointManager.GetBreakpointCount() == 0);

  return true;
}

static bool testSourceFileLoadedAfterHandleBreakpointRequest()
{
  // Arrange
  DebuggerTestHelper helper;
  std::vector<dap::BreakpointEvent> breakpointEvents;
  std::atomic<int> remainingBreakpointEvents(5);
  std::promise<void> allBreakpointEventsReceivedPromise;
  std::future<void> allBreakpointEventsReceivedFuture =
    allBreakpointEventsReceivedPromise.get_future();
  helper.Client->registerHandler([&](const dap::BreakpointEvent& event) {
    breakpointEvents.emplace_back(event);
    if (--remainingBreakpointEvents == 0) {
      allBreakpointEventsReceivedPromise.set_value();
    }
  });
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();
  dap::SetBreakpointsRequest setBreakpointRequest;
  std::string sourcePath = "C:/CMakeLists.txt";
  setBreakpointRequest.source.path = sourcePath;
  dap::array<dap::SourceBreakpoint> sourceBreakpoints(5);
  sourceBreakpoints[0].line = 1;
  sourceBreakpoints[1].line = 2;
  sourceBreakpoints[2].line = 3;
  sourceBreakpoints[3].line = 4;
  sourceBreakpoints[4].line = 5;
  setBreakpointRequest.breakpoints = sourceBreakpoints;
  std::vector<cmListFileFunction> functions = helper.CreateListFileFunctions(
    "# Comment1\nset(var1 foo)\n# Comment2\nset(var2\nbar)\n",
    sourcePath.c_str());
  auto got = helper.Client->send(setBreakpointRequest).get();

  // Act
  breakpointManager.SourceFileLoaded(sourcePath, functions);
  ASSERT_TRUE(allBreakpointEventsReceivedFuture.wait_for(
                std::chrono::seconds(10)) == std::future_status::ready);

  // Assert
  ASSERT_TRUE(breakpointEvents.size() > 0);
  // Line 1 is a comment. Move it to next valid function, which is line 2.
  ASSERT_BREAKPOINT(breakpointEvents[0].breakpoint, 0, 2, sourcePath, true);
  ASSERT_BREAKPOINT(breakpointEvents[1].breakpoint, 1,
                    sourceBreakpoints[1].line, sourcePath, true);
  // Line 3 is a comment. Move it to next valid function, which is line 4.
  ASSERT_BREAKPOINT(breakpointEvents[2].breakpoint, 2, 4, sourcePath, true);
  ASSERT_BREAKPOINT(breakpointEvents[3].breakpoint, 3,
                    sourceBreakpoints[3].line, sourcePath, true);
  // Line 5 is the 2nd part of line 4 function. No valid function after line 5,
  // show the breakpoint at line 4.
  ASSERT_BREAKPOINT(breakpointEvents[4].breakpoint, 4,
                    sourceBreakpoints[3].line, sourcePath, true);
  return true;
}

int testDebuggerBreakpointManager(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testHandleBreakpointRequestBeforeFileIsLoaded,
    testHandleBreakpointRequestAfterFileIsLoaded,
    testSourceFileLoadedAfterHandleBreakpointRequest,
  });
}
