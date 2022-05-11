/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExecProgramCommand.h"

#include <cstdio>

#include "cmsys/Process.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

using Encoding = cmProcessOutput::Encoding;

namespace {
bool RunCommand(std::string command, std::string& output, int& retVal,
                const char* directory = nullptr, bool verbose = true,
                Encoding encoding = cmProcessOutput::Auto);
}

// cmExecProgramCommand
bool cmExecProgramCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::string arguments;
  bool doingargs = false;
  int count = 0;
  std::string output_variable;
  bool haveoutput_variable = false;
  std::string return_variable;
  bool havereturn_variable = false;
  for (std::string const& arg : args) {
    if (arg == "OUTPUT_VARIABLE") {
      count++;
      doingargs = false;
      havereturn_variable = false;
      haveoutput_variable = true;
    } else if (haveoutput_variable) {
      if (!output_variable.empty()) {
        status.SetError("called with incorrect number of arguments");
        return false;
      }
      output_variable = arg;
      haveoutput_variable = false;
      count++;
    } else if (arg == "RETURN_VALUE") {
      count++;
      doingargs = false;
      haveoutput_variable = false;
      havereturn_variable = true;
    } else if (havereturn_variable) {
      if (!return_variable.empty()) {
        status.SetError("called with incorrect number of arguments");
        return false;
      }
      return_variable = arg;
      havereturn_variable = false;
      count++;
    } else if (arg == "ARGS") {
      count++;
      havereturn_variable = false;
      haveoutput_variable = false;
      doingargs = true;
    } else if (doingargs) {
      arguments += arg;
      arguments += " ";
      count++;
    }
  }

  std::string command;
  if (!arguments.empty()) {
    command = cmStrCat(cmSystemTools::ConvertToRunCommandPath(args[0]), ' ',
                       arguments);
  } else {
    command = args[0];
  }
  bool verbose = true;
  if (!output_variable.empty()) {
    verbose = false;
  }
  int retVal = 0;
  std::string output;
  bool result = true;
  if (args.size() - count == 2) {
    cmSystemTools::MakeDirectory(args[1]);
    result = RunCommand(command, output, retVal, args[1].c_str(), verbose);
  } else {
    result = RunCommand(command, output, retVal, nullptr, verbose);
  }
  if (!result) {
    retVal = -1;
  }

  if (!output_variable.empty()) {
    std::string::size_type first = output.find_first_not_of(" \n\t\r");
    std::string::size_type last = output.find_last_not_of(" \n\t\r");
    if (first == std::string::npos) {
      first = 0;
    }
    if (last == std::string::npos) {
      last = output.size() - 1;
    }

    std::string coutput = std::string(output, first, last - first + 1);
    status.GetMakefile().AddDefinition(output_variable, coutput);
  }

  if (!return_variable.empty()) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%d", retVal);
    status.GetMakefile().AddDefinition(return_variable, buffer);
  }

  return true;
}

namespace {
bool RunCommand(std::string command, std::string& output, int& retVal,
                const char* dir, bool verbose, Encoding encoding)
{
  if (cmSystemTools::GetRunCommandOutput()) {
    verbose = false;
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  // if the command does not start with a quote, then
  // try to find the program, and if the program can not be
  // found use system to run the command as it must be a built in
  // shell command like echo or dir
  if (!command.empty() && command[0] == '\"') {
    // count the number of quotes
    int count = 0;
    for (char c : command) {
      if (c == '\"') {
        count++;
        if (count > 2) {
          break;
        }
      }
    }
    // if there are more than two double quotes use
    // GetShortPathName, the cmd.exe program in windows which
    // is used by system fails to execute if there are more than
    // one set of quotes in the arguments
    if (count > 2) {
      cmsys::RegularExpression quoted("^\"([^\"]*)\"[ \t](.*)");
      if (quoted.find(command)) {
        std::string shortCmd;
        std::string cmd = quoted.match(1);
        std::string args = quoted.match(2);
        if (!cmSystemTools::FileExists(cmd)) {
          shortCmd = cmd;
        } else if (!cmSystemTools::GetShortPath(cmd, shortCmd)) {
          cmSystemTools::Error("GetShortPath failed for " + cmd);
          return false;
        }
        shortCmd += " ";
        shortCmd += args;

        command = shortCmd;
      } else {
        cmSystemTools::Error("Could not parse command line with quotes " +
                             command);
      }
    }
  }
#endif

  // Allocate a process instance.
  cmsysProcess* cp = cmsysProcess_New();
  if (!cp) {
    cmSystemTools::Error("Error allocating process instance.");
    return false;
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  if (dir) {
    cmsysProcess_SetWorkingDirectory(cp, dir);
  }
  if (cmSystemTools::GetRunCommandHideConsole()) {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  }
  cmsysProcess_SetOption(cp, cmsysProcess_Option_Verbatim, 1);
  const char* cmd[] = { command.c_str(), nullptr };
  cmsysProcess_SetCommand(cp, cmd);
#else
  std::string commandInDir;
  if (dir) {
    commandInDir = cmStrCat("cd \"", dir, "\" && ", command);
  } else {
    commandInDir = command;
  }
#  ifndef __VMS
  commandInDir += " 2>&1";
#  endif
  command = commandInDir;
  if (verbose) {
    cmSystemTools::Stdout("running ");
    cmSystemTools::Stdout(command);
    cmSystemTools::Stdout("\n");
  }
  fflush(stdout);
  fflush(stderr);
  const char* cmd[] = { "/bin/sh", "-c", command.c_str(), nullptr };
  cmsysProcess_SetCommand(cp, cmd);
#endif

  cmsysProcess_Execute(cp);

  // Read the process output.
  int length;
  char* data;
  int p;
  cmProcessOutput processOutput(encoding);
  std::string strdata;
  while ((p = cmsysProcess_WaitForData(cp, &data, &length, nullptr))) {
    if (p == cmsysProcess_Pipe_STDOUT || p == cmsysProcess_Pipe_STDERR) {
      if (verbose) {
        processOutput.DecodeText(data, length, strdata);
        cmSystemTools::Stdout(strdata);
      }
      output.append(data, length);
    }
  }

  if (verbose) {
    processOutput.DecodeText(std::string(), strdata);
    if (!strdata.empty()) {
      cmSystemTools::Stdout(strdata);
    }
  }

  // All output has been read.  Wait for the process to exit.
  cmsysProcess_WaitForExit(cp, nullptr);
  processOutput.DecodeText(output, output);

  // Check the result of running the process.
  std::string msg;
  switch (cmsysProcess_GetState(cp)) {
    case cmsysProcess_State_Exited:
      retVal = cmsysProcess_GetExitValue(cp);
      break;
    case cmsysProcess_State_Exception:
      retVal = -1;
      msg += "\nProcess terminated due to: ";
      msg += cmsysProcess_GetExceptionString(cp);
      break;
    case cmsysProcess_State_Error:
      retVal = -1;
      msg += "\nProcess failed because: ";
      msg += cmsysProcess_GetErrorString(cp);
      break;
    case cmsysProcess_State_Expired:
      retVal = -1;
      msg += "\nProcess terminated due to timeout.";
      break;
  }
  if (!msg.empty()) {
#if defined(_WIN32) && !defined(__CYGWIN__)
    // Old Windows process execution printed this info.
    msg += "\n\nfor command: ";
    msg += command;
    if (dir) {
      msg += "\nin dir: ";
      msg += dir;
    }
    msg += "\n";
    if (verbose) {
      cmSystemTools::Stdout(msg);
    }
    output += msg;
#else
    // Old UNIX process execution only put message in output.
    output += msg;
#endif
  }

  // Delete the process instance.
  cmsysProcess_Delete(cp);

  return true;
}
}
