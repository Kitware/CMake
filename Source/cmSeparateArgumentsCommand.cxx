/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSeparateArgumentsCommand.h"

#include <algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmSeparateArgumentsCommand
bool cmSeparateArgumentsCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("must be given at least one argument.");
    return false;
  }

  std::string var;
  std::string command;
  enum Mode
  {
    ModeOld,
    ModeUnix,
    ModeWindows
  };
  Mode mode = ModeOld;
  enum Doing
  {
    DoingNone,
    DoingVariable,
    DoingMode,
    DoingCommand
  };
  Doing doing = DoingVariable;
  for (std::string const& arg : args) {
    if (doing == DoingVariable) {
      var = arg;
      doing = DoingMode;
    } else if (doing == DoingMode && arg == "NATIVE_COMMAND") {
#ifdef _WIN32
      mode = ModeWindows;
#else
      mode = ModeUnix;
#endif
      doing = DoingCommand;
    } else if (doing == DoingMode && arg == "UNIX_COMMAND") {
      mode = ModeUnix;
      doing = DoingCommand;
    } else if (doing == DoingMode && arg == "WINDOWS_COMMAND") {
      mode = ModeWindows;
      doing = DoingCommand;
    } else if (doing == DoingCommand) {
      command = arg;
      doing = DoingNone;
    } else {
      status.SetError(cmStrCat("given unknown argument ", arg));
      return false;
    }
  }

  if (mode == ModeOld) {
    // Original space-replacement version of command.
    if (const char* def = status.GetMakefile().GetDefinition(var)) {
      std::string value = def;
      std::replace(value.begin(), value.end(), ' ', ';');
      status.GetMakefile().AddDefinition(var, value);
    }
  } else {
    // Parse the command line.
    std::vector<std::string> vec;
    if (mode == ModeUnix) {
      cmSystemTools::ParseUnixCommandLine(command.c_str(), vec);
    } else // if(mode == ModeWindows)
    {
      cmSystemTools::ParseWindowsCommandLine(command.c_str(), vec);
    }

    // Construct the result list value.
    std::string value;
    const char* sep = "";
    for (std::string const& vi : vec) {
      // Separate from the previous argument.
      value += sep;
      sep = ";";

      // Preserve semicolons.
      for (char si : vi) {
        if (si == ';') {
          value += '\\';
        }
        value += si;
      }
    }
    status.GetMakefile().AddDefinition(var, value);
  }

  return true;
}
