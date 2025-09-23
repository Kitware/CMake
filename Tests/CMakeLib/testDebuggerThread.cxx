#include <memory>
#include <string>
#include <vector>

#include <cm3p/cppdap/optional.h>
#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/types.h>

#include "cmDebuggerThread.h"
#include "cmListFileCache.h"

#include "testCommon.h"

static bool testStackFrameFunctionName(
  dap::optional<dap::StackFrameFormat> format, char const* expectedName)
{
  auto thread = std::make_shared<cmDebugger::cmDebuggerThread>(0, "name");
  auto const* functionName = "function_name";
  auto arguments = std::vector<cmListFileArgument>{ cmListFileArgument(
    "arg", cmListFileArgument::Delimiter::Unquoted, 0) };
  cmListFileFunction func(functionName, 10, 20, arguments);
  thread->PushStackFrame(nullptr, "CMakeLists.txt", func);

  auto stackTrace = GetStackTraceResponse(thread, format);

  ASSERT_TRUE(stackTrace.stackFrames[0].name == expectedName);
  return true;
}

bool testStackFrameNoFormatting()
{
  return testStackFrameFunctionName({}, "function_name");
}

bool testStackFrameFormatParameters()
{
  dap::StackFrameFormat format;
  format.parameters = true;
  return testStackFrameFunctionName(format, "function_name()");
}

bool testStackFrameFormatParameterValues()
{
  dap::StackFrameFormat format;
  format.parameters = true;
  format.parameterValues = true;
  return testStackFrameFunctionName(format, "function_name(arg)");
}

bool testStackFrameFormatLine()
{
  dap::StackFrameFormat format;
  format.line = true;
  return testStackFrameFunctionName(format, "function_name Line: 10");
}

int testDebuggerThread(int, char*[])
{
  return runTests({ testStackFrameNoFormatting, testStackFrameFormatParameters,
                    testStackFrameFormatParameterValues,
                    testStackFrameFormatLine });
}
