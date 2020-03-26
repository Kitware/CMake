/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExecuteProcessCommand.h"

#include <algorithm>
#include <cctype> /* isspace */
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

#include <cmext/algorithm>

#include "cmsys/Process.h"

#include "cm_static_string_view.hxx"

#include "cmArgumentParser.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
bool cmExecuteProcessCommandIsWhitespace(char c)
{
  return (isspace(static_cast<int>(c)) || c == '\n' || c == '\r');
}

void cmExecuteProcessCommandFixText(std::vector<char>& output,
                                    bool strip_trailing_whitespace);
void cmExecuteProcessCommandAppend(std::vector<char>& output, const char* data,
                                   int length);
}

// cmExecuteProcessCommand
bool cmExecuteProcessCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  struct Arguments
  {
    std::vector<std::vector<std::string>> Commands;
    std::string OutputVariable;
    std::string ErrorVariable;
    std::string ResultVariable;
    std::string ResultsVariable;
    std::string WorkingDirectory;
    std::string InputFile;
    std::string OutputFile;
    std::string ErrorFile;
    std::string Timeout;
    std::string CommandEcho;
    bool OutputQuiet = false;
    bool ErrorQuiet = false;
    bool OutputStripTrailingWhitespace = false;
    bool ErrorStripTrailingWhitespace = false;
    std::string Encoding;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("COMMAND"_s, &Arguments::Commands)
      .Bind("COMMAND_ECHO"_s, &Arguments::CommandEcho)
      .Bind("OUTPUT_VARIABLE"_s, &Arguments::OutputVariable)
      .Bind("ERROR_VARIABLE"_s, &Arguments::ErrorVariable)
      .Bind("RESULT_VARIABLE"_s, &Arguments::ResultVariable)
      .Bind("RESULTS_VARIABLE"_s, &Arguments::ResultsVariable)
      .Bind("WORKING_DIRECTORY"_s, &Arguments::WorkingDirectory)
      .Bind("INPUT_FILE"_s, &Arguments::InputFile)
      .Bind("OUTPUT_FILE"_s, &Arguments::OutputFile)
      .Bind("ERROR_FILE"_s, &Arguments::ErrorFile)
      .Bind("TIMEOUT"_s, &Arguments::Timeout)
      .Bind("OUTPUT_QUIET"_s, &Arguments::OutputQuiet)
      .Bind("ERROR_QUIET"_s, &Arguments::ErrorQuiet)
      .Bind("OUTPUT_STRIP_TRAILING_WHITESPACE"_s,
            &Arguments::OutputStripTrailingWhitespace)
      .Bind("ERROR_STRIP_TRAILING_WHITESPACE"_s,
            &Arguments::ErrorStripTrailingWhitespace)
      .Bind("ENCODING"_s, &Arguments::Encoding);

  std::vector<std::string> unparsedArguments;
  std::vector<std::string> keywordsMissingValue;
  Arguments const arguments =
    parser.Parse(args, &unparsedArguments, &keywordsMissingValue);

  if (!keywordsMissingValue.empty()) {
    status.SetError(" called with no value for " +
                    keywordsMissingValue.front() + ".");
    return false;
  }
  if (!unparsedArguments.empty()) {
    status.SetError(" given unknown argument \"" + unparsedArguments.front() +
                    "\".");
    return false;
  }

  if (!status.GetMakefile().CanIWriteThisFile(arguments.OutputFile)) {
    status.SetError("attempted to output into a file: " +
                    arguments.OutputFile + " into a source directory.");
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }

  // Check for commands given.
  if (arguments.Commands.empty()) {
    status.SetError(" called with no COMMAND argument.");
    return false;
  }
  for (std::vector<std::string> const& cmd : arguments.Commands) {
    if (cmd.empty()) {
      status.SetError(" given COMMAND argument with no value.");
      return false;
    }
  }

  // Parse the timeout string.
  double timeout = -1;
  if (!arguments.Timeout.empty()) {
    if (sscanf(arguments.Timeout.c_str(), "%lg", &timeout) != 1) {
      status.SetError(" called with TIMEOUT value that could not be parsed.");
      return false;
    }
  }
  // Create a process instance.
  std::unique_ptr<cmsysProcess, void (*)(cmsysProcess*)> cp_ptr(
    cmsysProcess_New(), cmsysProcess_Delete);
  cmsysProcess* cp = cp_ptr.get();

  // Set the command sequence.
  for (std::vector<std::string> const& cmd : arguments.Commands) {
    std::vector<const char*> argv(cmd.size() + 1);
    std::transform(cmd.begin(), cmd.end(), argv.begin(),
                   [](std::string const& s) { return s.c_str(); });
    argv.back() = nullptr;
    cmsysProcess_AddCommand(cp, argv.data());
  }

  // Set the process working directory.
  if (!arguments.WorkingDirectory.empty()) {
    cmsysProcess_SetWorkingDirectory(cp, arguments.WorkingDirectory.c_str());
  }

  // Always hide the process window.
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);

  // Check the output variables.
  bool merge_output = false;
  if (!arguments.InputFile.empty()) {
    cmsysProcess_SetPipeFile(cp, cmsysProcess_Pipe_STDIN,
                             arguments.InputFile.c_str());
  }
  if (!arguments.OutputFile.empty()) {
    cmsysProcess_SetPipeFile(cp, cmsysProcess_Pipe_STDOUT,
                             arguments.OutputFile.c_str());
  }
  if (!arguments.ErrorFile.empty()) {
    if (arguments.ErrorFile == arguments.OutputFile) {
      merge_output = true;
    } else {
      cmsysProcess_SetPipeFile(cp, cmsysProcess_Pipe_STDERR,
                               arguments.ErrorFile.c_str());
    }
  }
  if (!arguments.OutputVariable.empty() &&
      arguments.OutputVariable == arguments.ErrorVariable) {
    merge_output = true;
  }
  if (merge_output) {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_MergeOutput, 1);
  }

  // Set the timeout if any.
  if (timeout >= 0) {
    cmsysProcess_SetTimeout(cp, timeout);
  }

  bool echo_stdout = false;
  bool echo_stderr = false;
  bool echo_output_from_variable = true;
  std::string echo_output = status.GetMakefile().GetSafeDefinition(
    "CMAKE_EXECUTE_PROCESS_COMMAND_ECHO");
  if (!arguments.CommandEcho.empty()) {
    echo_output_from_variable = false;
    echo_output = arguments.CommandEcho;
  }

  if (!echo_output.empty()) {
    if (echo_output == "STDERR") {
      echo_stderr = true;
    } else if (echo_output == "STDOUT") {
      echo_stdout = true;
    } else if (echo_output != "NONE") {
      std::string error;
      if (echo_output_from_variable) {
        error = "CMAKE_EXECUTE_PROCESS_COMMAND_ECHO set to '";
      } else {
        error = " called with '";
      }
      error += echo_output;
      error += "' expected STDERR|STDOUT|NONE";
      if (!echo_output_from_variable) {
        error += " for COMMAND_ECHO.";
      }
      status.GetMakefile().IssueMessage(MessageType::FATAL_ERROR, error);
      return true;
    }
  }
  if (echo_stdout || echo_stderr) {
    std::string command;
    for (auto& cmd : arguments.Commands) {
      command += "'";
      command += cmJoin(cmd, "' '");
      command += "'";
      command += "\n";
    }
    if (echo_stdout) {
      std::cout << command;
    } else if (echo_stderr) {
      std::cerr << command;
    }
  }
  // Start the process.
  cmsysProcess_Execute(cp);

  // Read the process output.
  std::vector<char> tempOutput;
  std::vector<char> tempError;
  int length;
  char* data;
  int p;
  cmProcessOutput processOutput(
    cmProcessOutput::FindEncoding(arguments.Encoding));
  std::string strdata;
  while ((p = cmsysProcess_WaitForData(cp, &data, &length, nullptr))) {
    // Put the output in the right place.
    if (p == cmsysProcess_Pipe_STDOUT && !arguments.OutputQuiet) {
      if (arguments.OutputVariable.empty()) {
        processOutput.DecodeText(data, length, strdata, 1);
        cmSystemTools::Stdout(strdata);
      } else {
        cmExecuteProcessCommandAppend(tempOutput, data, length);
      }
    } else if (p == cmsysProcess_Pipe_STDERR && !arguments.ErrorQuiet) {
      if (arguments.ErrorVariable.empty()) {
        processOutput.DecodeText(data, length, strdata, 2);
        cmSystemTools::Stderr(strdata);
      } else {
        cmExecuteProcessCommandAppend(tempError, data, length);
      }
    }
  }
  if (!arguments.OutputQuiet && arguments.OutputVariable.empty()) {
    processOutput.DecodeText(std::string(), strdata, 1);
    if (!strdata.empty()) {
      cmSystemTools::Stdout(strdata);
    }
  }
  if (!arguments.ErrorQuiet && arguments.ErrorVariable.empty()) {
    processOutput.DecodeText(std::string(), strdata, 2);
    if (!strdata.empty()) {
      cmSystemTools::Stderr(strdata);
    }
  }

  // All output has been read.  Wait for the process to exit.
  cmsysProcess_WaitForExit(cp, nullptr);
  processOutput.DecodeText(tempOutput, tempOutput);
  processOutput.DecodeText(tempError, tempError);

  // Fix the text in the output strings.
  cmExecuteProcessCommandFixText(tempOutput,
                                 arguments.OutputStripTrailingWhitespace);
  cmExecuteProcessCommandFixText(tempError,
                                 arguments.ErrorStripTrailingWhitespace);

  // Store the output obtained.
  if (!arguments.OutputVariable.empty() && !tempOutput.empty()) {
    status.GetMakefile().AddDefinition(arguments.OutputVariable,
                                       tempOutput.data());
  }
  if (!merge_output && !arguments.ErrorVariable.empty() &&
      !tempError.empty()) {
    status.GetMakefile().AddDefinition(arguments.ErrorVariable,
                                       tempError.data());
  }

  // Store the result of running the process.
  if (!arguments.ResultVariable.empty()) {
    switch (cmsysProcess_GetState(cp)) {
      case cmsysProcess_State_Exited: {
        int v = cmsysProcess_GetExitValue(cp);
        char buf[16];
        sprintf(buf, "%d", v);
        status.GetMakefile().AddDefinition(arguments.ResultVariable, buf);
      } break;
      case cmsysProcess_State_Exception:
        status.GetMakefile().AddDefinition(
          arguments.ResultVariable, cmsysProcess_GetExceptionString(cp));
        break;
      case cmsysProcess_State_Error:
        status.GetMakefile().AddDefinition(arguments.ResultVariable,
                                           cmsysProcess_GetErrorString(cp));
        break;
      case cmsysProcess_State_Expired:
        status.GetMakefile().AddDefinition(
          arguments.ResultVariable, "Process terminated due to timeout");
        break;
    }
  }
  // Store the result of running the processes.
  if (!arguments.ResultsVariable.empty()) {
    switch (cmsysProcess_GetState(cp)) {
      case cmsysProcess_State_Exited: {
        std::vector<std::string> res;
        for (size_t i = 0; i < arguments.Commands.size(); ++i) {
          switch (cmsysProcess_GetStateByIndex(cp, static_cast<int>(i))) {
            case kwsysProcess_StateByIndex_Exited: {
              int exitCode =
                cmsysProcess_GetExitValueByIndex(cp, static_cast<int>(i));
              char buf[16];
              sprintf(buf, "%d", exitCode);
              res.emplace_back(buf);
            } break;
            case kwsysProcess_StateByIndex_Exception:
              res.emplace_back(cmsysProcess_GetExceptionStringByIndex(
                cp, static_cast<int>(i)));
              break;
            case kwsysProcess_StateByIndex_Error:
            default:
              res.emplace_back("Error getting the child return code");
              break;
          }
        }
        status.GetMakefile().AddDefinition(arguments.ResultsVariable,
                                           cmJoin(res, ";"));
      } break;
      case cmsysProcess_State_Exception:
        status.GetMakefile().AddDefinition(
          arguments.ResultsVariable, cmsysProcess_GetExceptionString(cp));
        break;
      case cmsysProcess_State_Error:
        status.GetMakefile().AddDefinition(arguments.ResultsVariable,
                                           cmsysProcess_GetErrorString(cp));
        break;
      case cmsysProcess_State_Expired:
        status.GetMakefile().AddDefinition(
          arguments.ResultsVariable, "Process terminated due to timeout");
        break;
    }
  }

  return true;
}

namespace {
void cmExecuteProcessCommandFixText(std::vector<char>& output,
                                    bool strip_trailing_whitespace)
{
  // Remove \0 characters and the \r part of \r\n pairs.
  unsigned int in_index = 0;
  unsigned int out_index = 0;
  while (in_index < output.size()) {
    char c = output[in_index++];
    if ((c != '\r' ||
         !(in_index < output.size() && output[in_index] == '\n')) &&
        c != '\0') {
      output[out_index++] = c;
    }
  }

  // Remove trailing whitespace if requested.
  if (strip_trailing_whitespace) {
    while (out_index > 0 &&
           cmExecuteProcessCommandIsWhitespace(output[out_index - 1])) {
      --out_index;
    }
  }

  // Shrink the vector to the size needed.
  output.resize(out_index);

  // Put a terminator on the text string.
  output.push_back('\0');
}

void cmExecuteProcessCommandAppend(std::vector<char>& output, const char* data,
                                   int length)
{
#if defined(__APPLE__)
  // HACK on Apple to work around bug with inserting at the
  // end of an empty vector.  This resulted in random failures
  // that were hard to reproduce.
  if (output.empty() && length > 0) {
    output.push_back(data[0]);
    ++data;
    --length;
  }
#endif
  cm::append(output, data, data + length);
}
}
