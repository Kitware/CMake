/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeLanguageCommand.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

bool FatalError(cmExecutionStatus& status, std::string const& error)
{
  status.SetError(error);
  cmSystemTools::SetFatalErrorOccured();
  return false;
}

std::array<cm::static_string_view, 12> InvalidCommands{
  { // clang-format off
  "function"_s, "endfunction"_s,
  "macro"_s, "endmacro"_s,
  "if"_s, "elseif"_s, "else"_s, "endif"_s,
  "while"_s, "endwhile"_s,
  "foreach"_s, "endforeach"_s
  } // clang-format on
};

bool cmCMakeLanguageCommandCALL(std::vector<cmListFileArgument> const& args,
                                std::string const& callCommand,
                                size_t startArg, cmExecutionStatus& status)
{
  // ensure specified command is valid
  // start/end flow control commands are not allowed
  auto cmd = cmSystemTools::LowerCase(callCommand);
  if (std::find(InvalidCommands.cbegin(), InvalidCommands.cend(), cmd) !=
      InvalidCommands.cend()) {
    return FatalError(status,
                      cmStrCat("invalid command specified: "_s, callCommand));
  }

  cmMakefile& makefile = status.GetMakefile();
  cmListFileContext context = makefile.GetBacktrace().Top();

  cmListFileFunction func;
  func.Name = callCommand;
  func.Line = context.Line;

  // The rest of the arguments are passed to the function call above
  for (size_t i = startArg; i < args.size(); ++i) {
    cmListFileArgument lfarg;
    lfarg.Delim = args[i].Delim;
    lfarg.Line = context.Line;
    lfarg.Value = args[i].Value;
    func.Arguments.emplace_back(lfarg);
  }

  return makefile.ExecuteCommand(func, status);
}

bool cmCMakeLanguageCommandEVAL(std::vector<cmListFileArgument> const& args,
                                cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();
  cmListFileContext context = makefile.GetBacktrace().Top();
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  if (expandedArgs.size() < 2) {
    return FatalError(status, "called with incorrect number of arguments");
  }

  if (expandedArgs[1] != "CODE") {
    auto code_iter =
      std::find(expandedArgs.begin() + 2, expandedArgs.end(), "CODE");
    if (code_iter == expandedArgs.end()) {
      return FatalError(status, "called without CODE argument");
    }
    return FatalError(
      status,
      "called with unsupported arguments between EVAL and CODE arguments");
  }

  const std::string code =
    cmJoin(cmMakeRange(expandedArgs.begin() + 2, expandedArgs.end()), " ");
  return makefile.ReadListFileAsString(
    code, cmStrCat(context.FilePath, ":", context.Line, ":EVAL"));
}
}

bool cmCMakeLanguageCommand(std::vector<cmListFileArgument> const& args,
                            cmExecutionStatus& status)
{
  std::vector<std::string> expArgs;
  size_t rawArg = 0;
  size_t expArg = 0;

  // Helper to consume and expand one raw argument at a time.
  auto moreArgs = [&]() -> bool {
    while (expArg >= expArgs.size()) {
      if (rawArg >= args.size()) {
        return false;
      }
      std::vector<cmListFileArgument> tmpArg;
      tmpArg.emplace_back(args[rawArg++]);
      status.GetMakefile().ExpandArguments(tmpArg, expArgs);
    }
    return true;
  };

  if (!moreArgs()) {
    return FatalError(status, "called with incorrect number of arguments");
  }

  if (expArgs[expArg] == "CALL") {
    ++expArg; // Consume "CALL".

    // CALL requires a command name.
    if (!moreArgs()) {
      return FatalError(status, "CALL missing command name");
    }
    std::string const& callCommand = expArgs[expArg++];

    // CALL accepts no further expanded arguments.
    if (expArg != expArgs.size()) {
      return FatalError(status, "CALL command's arguments must be literal");
    }

    // Run the CALL.
    return cmCMakeLanguageCommandCALL(args, callCommand, rawArg, status);
  }

  if (expArgs[expArg] == "EVAL") {
    return cmCMakeLanguageCommandEVAL(args, status);
  }

  return FatalError(status, "called with unknown meta-operation");
}
