/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmDebuggerProtocol.h"

#include <string>

namespace dap {
DAP_IMPLEMENT_STRUCT_TYPEINFO(CMakeVersion, "", DAP_FIELD(major, "major"),
                              DAP_FIELD(minor, "minor"),
                              DAP_FIELD(patch, "patch"),
                              DAP_FIELD(full, "full"));

DAP_IMPLEMENT_STRUCT_TYPEINFO(
  CMakeInitializeResponse, "",
  DAP_FIELD(additionalModuleColumns, "additionalModuleColumns"),
  DAP_FIELD(completionTriggerCharacters, "completionTriggerCharacters"),
  DAP_FIELD(exceptionBreakpointFilters, "exceptionBreakpointFilters"),
  DAP_FIELD(supportSuspendDebuggee, "supportSuspendDebuggee"),
  DAP_FIELD(supportTerminateDebuggee, "supportTerminateDebuggee"),
  DAP_FIELD(supportedChecksumAlgorithms, "supportedChecksumAlgorithms"),
  DAP_FIELD(supportsBreakpointLocationsRequest,
            "supportsBreakpointLocationsRequest"),
  DAP_FIELD(supportsCancelRequest, "supportsCancelRequest"),
  DAP_FIELD(supportsClipboardContext, "supportsClipboardContext"),
  DAP_FIELD(supportsCompletionsRequest, "supportsCompletionsRequest"),
  DAP_FIELD(supportsConditionalBreakpoints, "supportsConditionalBreakpoints"),
  DAP_FIELD(supportsConfigurationDoneRequest,
            "supportsConfigurationDoneRequest"),
  DAP_FIELD(supportsDataBreakpoints, "supportsDataBreakpoints"),
  DAP_FIELD(supportsDelayedStackTraceLoading,
            "supportsDelayedStackTraceLoading"),
  DAP_FIELD(supportsDisassembleRequest, "supportsDisassembleRequest"),
  DAP_FIELD(supportsEvaluateForHovers, "supportsEvaluateForHovers"),
  DAP_FIELD(supportsExceptionFilterOptions, "supportsExceptionFilterOptions"),
  DAP_FIELD(supportsExceptionInfoRequest, "supportsExceptionInfoRequest"),
  DAP_FIELD(supportsExceptionOptions, "supportsExceptionOptions"),
  DAP_FIELD(supportsFunctionBreakpoints, "supportsFunctionBreakpoints"),
  DAP_FIELD(supportsGotoTargetsRequest, "supportsGotoTargetsRequest"),
  DAP_FIELD(supportsHitConditionalBreakpoints,
            "supportsHitConditionalBreakpoints"),
  DAP_FIELD(supportsInstructionBreakpoints, "supportsInstructionBreakpoints"),
  DAP_FIELD(supportsLoadedSourcesRequest, "supportsLoadedSourcesRequest"),
  DAP_FIELD(supportsLogPoints, "supportsLogPoints"),
  DAP_FIELD(supportsModulesRequest, "supportsModulesRequest"),
  DAP_FIELD(supportsReadMemoryRequest, "supportsReadMemoryRequest"),
  DAP_FIELD(supportsRestartFrame, "supportsRestartFrame"),
  DAP_FIELD(supportsRestartRequest, "supportsRestartRequest"),
  DAP_FIELD(supportsSetExpression, "supportsSetExpression"),
  DAP_FIELD(supportsSetVariable, "supportsSetVariable"),
  DAP_FIELD(supportsSingleThreadExecutionRequests,
            "supportsSingleThreadExecutionRequests"),
  DAP_FIELD(supportsStepBack, "supportsStepBack"),
  DAP_FIELD(supportsStepInTargetsRequest, "supportsStepInTargetsRequest"),
  DAP_FIELD(supportsSteppingGranularity, "supportsSteppingGranularity"),
  DAP_FIELD(supportsTerminateRequest, "supportsTerminateRequest"),
  DAP_FIELD(supportsTerminateThreadsRequest,
            "supportsTerminateThreadsRequest"),
  DAP_FIELD(supportsValueFormattingOptions, "supportsValueFormattingOptions"),
  DAP_FIELD(supportsWriteMemoryRequest, "supportsWriteMemoryRequest"),
  DAP_FIELD(cmakeVersion, "cmakeVersion"));

DAP_IMPLEMENT_STRUCT_TYPEINFO(
  CMakeInitializeRequest, "initialize", DAP_FIELD(adapterID, "adapterID"),
  DAP_FIELD(clientID, "clientID"), DAP_FIELD(clientName, "clientName"),
  DAP_FIELD(columnsStartAt1, "columnsStartAt1"),
  DAP_FIELD(linesStartAt1, "linesStartAt1"), DAP_FIELD(locale, "locale"),
  DAP_FIELD(pathFormat, "pathFormat"),
  DAP_FIELD(supportsArgsCanBeInterpretedByShell,
            "supportsArgsCanBeInterpretedByShell"),
  DAP_FIELD(supportsInvalidatedEvent, "supportsInvalidatedEvent"),
  DAP_FIELD(supportsMemoryEvent, "supportsMemoryEvent"),
  DAP_FIELD(supportsMemoryReferences, "supportsMemoryReferences"),
  DAP_FIELD(supportsProgressReporting, "supportsProgressReporting"),
  DAP_FIELD(supportsRunInTerminalRequest, "supportsRunInTerminalRequest"),
  DAP_FIELD(supportsStartDebuggingRequest, "supportsStartDebuggingRequest"),
  DAP_FIELD(supportsVariablePaging, "supportsVariablePaging"),
  DAP_FIELD(supportsVariableType, "supportsVariableType"));

} // namespace dap
