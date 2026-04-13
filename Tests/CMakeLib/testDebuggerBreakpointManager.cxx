/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <atomic>
#include <chrono>
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

#if defined(_WIN32) || defined(__APPLE__)
#  include "cmsys/FStream.hxx"

#  include "cmSystemTools.h"

#  include "testConfig.h"
#endif

#include "testCommon.h"
#include "testDebugger.h"

class cmListFileFunction;

// Fictional absolute path used by the pre-existing breakpoint tests.
// Must be absolute on all platforms so that ToNormalizedPathOnDisk
// (used inside the breakpoint manager) does not resolve it relative
// to the working directory.
#ifdef _WIN32
static std::string const kTestSourcePath = "C:/CMakeLists.txt";
#else
static std::string const kTestSourcePath = "/CMakeLists.txt";
#endif

static bool testHandleBreakpointRequestBeforeFileIsLoaded()
{
  // Arrange
  DebuggerTestHelper helper;
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();
  dap::SetBreakpointsRequest setBreakpointRequest;
  std::string sourcePath = kTestSourcePath;
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
  helper.Client->registerHandler([&](dap::BreakpointEvent const&) {
    notExpectBreakpointEvents.store(false);
  });

  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();
  std::string sourcePath = kTestSourcePath;
  std::vector<cmListFileFunction> functions = helper.CreateListFileFunctions(
    "# Comment1\nset(var1 foo)\n# Comment2\nset(var2\nbar)\n", sourcePath);

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
  helper.Client->registerHandler([&](dap::BreakpointEvent const& event) {
    breakpointEvents.emplace_back(event);
    if (--remainingBreakpointEvents == 0) {
      allBreakpointEventsReceivedPromise.set_value();
    }
  });
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();
  dap::SetBreakpointsRequest setBreakpointRequest;
  std::string sourcePath = kTestSourcePath;
  setBreakpointRequest.source.path = sourcePath;
  dap::array<dap::SourceBreakpoint> sourceBreakpoints(5);
  sourceBreakpoints[0].line = 1;
  sourceBreakpoints[1].line = 2;
  sourceBreakpoints[2].line = 3;
  sourceBreakpoints[3].line = 4;
  sourceBreakpoints[4].line = 5;
  setBreakpointRequest.breakpoints = sourceBreakpoints;
  std::vector<cmListFileFunction> functions = helper.CreateListFileFunctions(
    "# Comment1\nset(var1 foo)\n# Comment2\nset(var2\nbar)\n", sourcePath);
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

#if defined(_WIN32) || defined(__APPLE__)

// RAII guard for temp directory cleanup on both success and assertion failure.
struct TempDirGuard
{
  std::string Path;
  ~TempDirGuard()
  {
    if (!Path.empty()) {
      cmSystemTools::RemoveADirectory(Path);
    }
  }
};

static bool testBreakpointCaseMismatch_SourceLoadedFirst()
{
  std::string tempDir = std::string(BUILD_DIR) + "/cmakedbg-bp1-XXXXXX";
  if (!cmSystemTools::MakeTempDirectory(tempDir)) {
    std::cout << "Failed to create temp directory\n";
    return false;
  }
  TempDirGuard guard;
  guard.Path = tempDir;

  std::string createdFile = tempDir + "/TestCase.cmake";
  {
    cmsys::ofstream f(createdFile.c_str());
  }

  std::string canonicalPath =
    cmSystemTools::ToNormalizedPathOnDisk(createdFile);
  std::string lowerFile = tempDir + "/testcase.cmake";
  std::string normalizedLower =
    cmSystemTools::ToNormalizedPathOnDisk(lowerFile);

  // Skip when ToNormalizedPathOnDisk does not correct on-disk case:
  // a case-sensitive filesystem, or CYGWIN (treated as POSIX, no probe).
  if (normalizedLower != canonicalPath) {
    std::cout << "Skipping: ToNormalizedPathOnDisk does not correct case\n";
    return true;
  }

  // Arrange
  DebuggerTestHelper helper;
  std::atomic<bool> noBreakpointEvents(true);
  helper.Client->registerHandler(
    [&](dap::BreakpointEvent const&) { noBreakpointEvents.store(false); });
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();

  std::vector<cmListFileFunction> functions =
    helper.CreateListFileFunctions("set(var1 foo)\n", canonicalPath);

  // Act: load source with lowercase path, set breakpoints with uppercase.
  breakpointManager.SourceFileLoaded(lowerFile, functions);

  dap::SetBreakpointsRequest req;
  req.source.path = tempDir + "/TESTCASE.CMAKE";
  dap::array<dap::SourceBreakpoint> bps(1);
  bps[0].line = 1;
  req.breakpoints = bps;
  auto got = helper.Client->send(req).get();

  // Assert: breakpoint verified because both paths normalize to canonicalPath.
  ASSERT_TRUE(!got.error);
  ASSERT_TRUE(got.response.breakpoints.size() == 1);
  ASSERT_BREAKPOINT(got.response.breakpoints[0], 0, 1, canonicalPath, true);
  ASSERT_TRUE(noBreakpointEvents.load());
  ASSERT_TRUE(breakpointManager.GetBreakpointCount() == 1);

  return true;
}

static bool testBreakpointCaseMismatch_BreakpointsSetFirst()
{
  std::string tempDir = std::string(BUILD_DIR) + "/cmakedbg-bp2-XXXXXX";
  if (!cmSystemTools::MakeTempDirectory(tempDir)) {
    std::cout << "Failed to create temp directory\n";
    return false;
  }
  TempDirGuard guard;
  guard.Path = tempDir;

  std::string createdFile = tempDir + "/TestCase.cmake";
  {
    cmsys::ofstream f(createdFile.c_str());
  }

  std::string canonicalPath =
    cmSystemTools::ToNormalizedPathOnDisk(createdFile);
  std::string lowerFile = tempDir + "/testcase.cmake";
  std::string normalizedLower =
    cmSystemTools::ToNormalizedPathOnDisk(lowerFile);

  // Skip when ToNormalizedPathOnDisk does not correct on-disk case:
  // a case-sensitive filesystem, or CYGWIN (treated as POSIX, no probe).
  if (normalizedLower != canonicalPath) {
    std::cout << "Skipping: ToNormalizedPathOnDisk does not correct case\n";
    return true;
  }

  // Arrange
  DebuggerTestHelper helper;
  std::vector<dap::BreakpointEvent> breakpointEvents;
  std::atomic<int> remainingEvents(1);
  std::promise<void> allEventsPromise;
  std::future<void> allEventsFuture = allEventsPromise.get_future();
  helper.Client->registerHandler([&](dap::BreakpointEvent const& event) {
    breakpointEvents.emplace_back(event);
    if (--remainingEvents == 0) {
      allEventsPromise.set_value();
    }
  });
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();

  // Act: set breakpoints with uppercase path before file is loaded.
  dap::SetBreakpointsRequest req;
  req.source.path = tempDir + "/TESTCASE.CMAKE";
  dap::array<dap::SourceBreakpoint> bps(1);
  bps[0].line = 1;
  req.breakpoints = bps;
  auto got = helper.Client->send(req).get();

  ASSERT_TRUE(!got.error);
  ASSERT_TRUE(got.response.breakpoints.size() == 1);
  ASSERT_BREAKPOINT(got.response.breakpoints[0], 0, 1, canonicalPath, false);

  // Act: load source with lowercase path.
  std::vector<cmListFileFunction> functions =
    helper.CreateListFileFunctions("set(var1 foo)\n", canonicalPath);
  breakpointManager.SourceFileLoaded(lowerFile, functions);

  ASSERT_TRUE(allEventsFuture.wait_for(std::chrono::seconds(10)) ==
              std::future_status::ready);

  // Assert: breakpoint event fires with verified status.
  ASSERT_TRUE(breakpointEvents.size() == 1);
  ASSERT_BREAKPOINT(breakpointEvents[0].breakpoint, 0, 1, canonicalPath, true);

  return true;
}

static bool testGetBreakpoints_CaseMismatch()
{
  std::string tempDir = std::string(BUILD_DIR) + "/cmakedbg-bp3-XXXXXX";
  if (!cmSystemTools::MakeTempDirectory(tempDir)) {
    std::cout << "Failed to create temp directory\n";
    return false;
  }
  TempDirGuard guard;
  guard.Path = tempDir;

  std::string createdFile = tempDir + "/TestCase.cmake";
  {
    cmsys::ofstream f(createdFile.c_str());
  }

  std::string canonicalPath =
    cmSystemTools::ToNormalizedPathOnDisk(createdFile);
  std::string lowerFile = tempDir + "/testcase.cmake";
  std::string normalizedLower =
    cmSystemTools::ToNormalizedPathOnDisk(lowerFile);

  // Skip when ToNormalizedPathOnDisk does not correct on-disk case:
  // a case-sensitive filesystem, or CYGWIN (treated as POSIX, no probe).
  if (normalizedLower != canonicalPath) {
    std::cout << "Skipping: ToNormalizedPathOnDisk does not correct case\n";
    return true;
  }

  // Arrange
  DebuggerTestHelper helper;
  cmDebugger::cmDebuggerBreakpointManager breakpointManager(
    helper.Debugger.get());
  helper.bind();

  std::vector<cmListFileFunction> functions =
    helper.CreateListFileFunctions("set(var1 foo)\n", canonicalPath);

  breakpointManager.SourceFileLoaded(canonicalPath, functions);

  dap::SetBreakpointsRequest req;
  req.source.path = canonicalPath;
  dap::array<dap::SourceBreakpoint> bps(1);
  bps[0].line = 1;
  req.breakpoints = bps;
  auto got = helper.Client->send(req).get();
  ASSERT_TRUE(!got.error);
  ASSERT_TRUE(got.response.breakpoints.size() == 1);
  ASSERT_BREAKPOINT(got.response.breakpoints[0], 0, 1, canonicalPath, true);

  // Act: query breakpoints with a different-case path.
  auto breakpoints =
    breakpointManager.GetBreakpoints(tempDir + "/TESTCASE.CMAKE", 1);

  // Assert: found despite case mismatch.
  ASSERT_TRUE(breakpoints.size() == 1);
  ASSERT_TRUE(breakpoints[0] == 0);

  return true;
}

#endif

int testDebuggerBreakpointManager(int, char*[])
{
  return runTests({
    testHandleBreakpointRequestBeforeFileIsLoaded,
    testHandleBreakpointRequestAfterFileIsLoaded,
    testSourceFileLoadedAfterHandleBreakpointRequest,
#if defined(_WIN32) || defined(__APPLE__)
    testBreakpointCaseMismatch_SourceLoadedFirst,
    testBreakpointCaseMismatch_BreakpointsSetFirst,
    testGetBreakpoints_CaseMismatch,
#endif
  });
}
