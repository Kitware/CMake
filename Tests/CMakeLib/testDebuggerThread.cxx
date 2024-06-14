#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/types.h>

#include "cmDebuggerThread.h"
#include "cmListFileCache.h"

#include "testCommon.h"

static bool testStackFrameFunctionName()
{
  auto thread = std::make_shared<cmDebugger::cmDebuggerThread>(0, "name");
  const auto* functionName = "function_name";
  auto arguments = std::vector<cmListFileArgument>{};
  cmListFileFunction func(functionName, 10, 20, arguments);
  thread->PushStackFrame(nullptr, "CMakeLists.txt", func);

  auto stackTrace = GetStackTraceResponse(thread);

  ASSERT_TRUE(stackTrace.stackFrames[0].name == functionName);
  return true;
}

int testDebuggerThread(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testStackFrameFunctionName,
  });
}
