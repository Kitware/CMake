/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm3p/cppdap/protocol.h>

#include <cmcppdap/include/dap/optional.h>
#include <cmcppdap/include/dap/typeof.h>
#include <cmcppdap/include/dap/types.h>

namespace dap {

// Represents the cmake version.
struct CMakeVersion : public InitializeResponse
{
  // The major version number.
  integer major;
  // The minor version number.
  integer minor;
  // The patch number.
  integer patch;
  // The full version string.
  string full;
};

DAP_DECLARE_STRUCT_TYPEINFO(CMakeVersion);

// Response to `initialize` request.
struct CMakeInitializeResponse : public Response
{
  // The set of additional module information exposed by the debug adapter.
  optional<array<ColumnDescriptor>> additionalModuleColumns;
  // The set of characters that should trigger completion in a REPL. If not
  // specified, the UI should assume the `.` character.
  optional<array<string>> completionTriggerCharacters;
  // Available exception filter options for the `setExceptionBreakpoints`
  // request.
  optional<array<ExceptionBreakpointsFilter>> exceptionBreakpointFilters;
  // The debug adapter supports the `suspendDebuggee` attribute on the
  // `disconnect` request.
  optional<boolean> supportSuspendDebuggee;
  // The debug adapter supports the `terminateDebuggee` attribute on the
  // `disconnect` request.
  optional<boolean> supportTerminateDebuggee;
  // Checksum algorithms supported by the debug adapter.
  optional<array<ChecksumAlgorithm>> supportedChecksumAlgorithms;
  // The debug adapter supports the `breakpointLocations` request.
  optional<boolean> supportsBreakpointLocationsRequest;
  // The debug adapter supports the `cancel` request.
  optional<boolean> supportsCancelRequest;
  // The debug adapter supports the `clipboard` context value in the `evaluate`
  // request.
  optional<boolean> supportsClipboardContext;
  // The debug adapter supports the `completions` request.
  optional<boolean> supportsCompletionsRequest;
  // The debug adapter supports conditional breakpoints.
  optional<boolean> supportsConditionalBreakpoints;
  // The debug adapter supports the `configurationDone` request.
  optional<boolean> supportsConfigurationDoneRequest;
  // The debug adapter supports data breakpoints.
  optional<boolean> supportsDataBreakpoints;
  // The debug adapter supports the delayed loading of parts of the stack,
  // which requires that both the `startFrame` and `levels` arguments and the
  // `totalFrames` result of the `stackTrace` request are supported.
  optional<boolean> supportsDelayedStackTraceLoading;
  // The debug adapter supports the `disassemble` request.
  optional<boolean> supportsDisassembleRequest;
  // The debug adapter supports a (side effect free) `evaluate` request for
  // data hovers.
  optional<boolean> supportsEvaluateForHovers;
  // The debug adapter supports `filterOptions` as an argument on the
  // `setExceptionBreakpoints` request.
  optional<boolean> supportsExceptionFilterOptions;
  // The debug adapter supports the `exceptionInfo` request.
  optional<boolean> supportsExceptionInfoRequest;
  // The debug adapter supports `exceptionOptions` on the
  // `setExceptionBreakpoints` request.
  optional<boolean> supportsExceptionOptions;
  // The debug adapter supports function breakpoints.
  optional<boolean> supportsFunctionBreakpoints;
  // The debug adapter supports the `gotoTargets` request.
  optional<boolean> supportsGotoTargetsRequest;
  // The debug adapter supports breakpoints that break execution after a
  // specified number of hits.
  optional<boolean> supportsHitConditionalBreakpoints;
  // The debug adapter supports adding breakpoints based on instruction
  // references.
  optional<boolean> supportsInstructionBreakpoints;
  // The debug adapter supports the `loadedSources` request.
  optional<boolean> supportsLoadedSourcesRequest;
  // The debug adapter supports log points by interpreting the `logMessage`
  // attribute of the `SourceBreakpoint`.
  optional<boolean> supportsLogPoints;
  // The debug adapter supports the `modules` request.
  optional<boolean> supportsModulesRequest;
  // The debug adapter supports the `readMemory` request.
  optional<boolean> supportsReadMemoryRequest;
  // The debug adapter supports restarting a frame.
  optional<boolean> supportsRestartFrame;
  // The debug adapter supports the `restart` request. In this case a client
  // should not implement `restart` by terminating and relaunching the adapter
  // but by calling the `restart` request.
  optional<boolean> supportsRestartRequest;
  // The debug adapter supports the `setExpression` request.
  optional<boolean> supportsSetExpression;
  // The debug adapter supports setting a variable to a value.
  optional<boolean> supportsSetVariable;
  // The debug adapter supports the `singleThread` property on the execution
  // requests (`continue`, `next`, `stepIn`, `stepOut`, `reverseContinue`,
  // `stepBack`).
  optional<boolean> supportsSingleThreadExecutionRequests;
  // The debug adapter supports stepping back via the `stepBack` and
  // `reverseContinue` requests.
  optional<boolean> supportsStepBack;
  // The debug adapter supports the `stepInTargets` request.
  optional<boolean> supportsStepInTargetsRequest;
  // The debug adapter supports stepping granularities (argument `granularity`)
  // for the stepping requests.
  optional<boolean> supportsSteppingGranularity;
  // The debug adapter supports the `terminate` request.
  optional<boolean> supportsTerminateRequest;
  // The debug adapter supports the `terminateThreads` request.
  optional<boolean> supportsTerminateThreadsRequest;
  // The debug adapter supports a `format` attribute on the `stackTrace`,
  // `variables`, and `evaluate` requests.
  optional<boolean> supportsValueFormattingOptions;
  // The debug adapter supports the `writeMemory` request.
  optional<boolean> supportsWriteMemoryRequest;
  // The CMake version.
  CMakeVersion cmakeVersion;
};

DAP_DECLARE_STRUCT_TYPEINFO(CMakeInitializeResponse);

// The `initialize` request is sent as the first request from the client to the
// debug adapter in order to configure it with client capabilities and to
// retrieve capabilities from the debug adapter. Until the debug adapter has
// responded with an `initialize` response, the client must not send any
// additional requests or events to the debug adapter. In addition the debug
// adapter is not allowed to send any requests or events to the client until it
// has responded with an `initialize` response. The `initialize` request may
// only be sent once.
struct CMakeInitializeRequest : public Request
{
  using Response = CMakeInitializeResponse;
  // The ID of the debug adapter.
  string adapterID;
  // The ID of the client using this adapter.
  optional<string> clientID;
  // The human-readable name of the client using this adapter.
  optional<string> clientName;
  // If true all column numbers are 1-based (default).
  optional<boolean> columnsStartAt1;
  // If true all line numbers are 1-based (default).
  optional<boolean> linesStartAt1;
  // The ISO-639 locale of the client using this adapter, e.g. en-US or de-CH.
  optional<string> locale;
  // Determines in what format paths are specified. The default is `path`,
  // which is the native format.
  //
  // May be one of the following enumeration values:
  // 'path', 'uri'
  optional<string> pathFormat;
  // Client supports the `argsCanBeInterpretedByShell` attribute on the
  // `runInTerminal` request.
  optional<boolean> supportsArgsCanBeInterpretedByShell;
  // Client supports the `invalidated` event.
  optional<boolean> supportsInvalidatedEvent;
  // Client supports the `memory` event.
  optional<boolean> supportsMemoryEvent;
  // Client supports memory references.
  optional<boolean> supportsMemoryReferences;
  // Client supports progress reporting.
  optional<boolean> supportsProgressReporting;
  // Client supports the `runInTerminal` request.
  optional<boolean> supportsRunInTerminalRequest;
  // Client supports the `startDebugging` request.
  optional<boolean> supportsStartDebuggingRequest;
  // Client supports the paging of variables.
  optional<boolean> supportsVariablePaging;
  // Client supports the `type` attribute for variables.
  optional<boolean> supportsVariableType;
};

DAP_DECLARE_STRUCT_TYPEINFO(CMakeInitializeRequest);

} // namespace dap
